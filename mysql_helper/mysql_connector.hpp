#ifndef EVL_ENGINE_MYSQL_CONNECTOR_H_
#define EVL_ENGINE_MYSQL_CONNECTOR_H_

#ifdef _PF_WINDOWS_
#include <WinSock2.h>
#endif // _PF_WINDOWS_

#include <iostream>
#include <string>
#include <vector>
#include <mysql.h>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include "typedef.h"
#include "detail/field_parser.hpp"

namespace evl
{
	namespace mysql
	{
		enum MysqlResultEnum
		{
			MYSQL_RESULT_SUCCEED,
			MYSQL_RESULT_NOT_CONNECTED, // 数据库未连接
			MYSQL_RESULT_FAILED_TO_PARSE_FIELD,
			MYSQL_RESULT_FAILED_TO_STORE_DATASET,
			MYSQL_RESULT_ERROR,
			MYSQL_RESULT_UNKNOWN_ERROR,
		};

		enum MysqlActionType
		{
			MYSQL_ACTION_TYPE_QUERY = 0,
			MYSQL_ACTION_TYPE_EXECUTE,
		};

		//////////////////////////////////////////////////////////////////////////
		struct MysqlData
		{
			evl::def::int32 error_code;
			const char* error_desc;

			union
			{
				// update， insert, delete 受影响行
				evl::def::uint32 affected_rows;
				// 结果集行数
				evl::def::uint32 dataset_rows;
			};

			// 行
			std::vector<void*> rows;

			MysqlData()
			{
				error_code = 0;
				affected_rows = 0;
				error_desc = NULL;
			}

			void* AllocRow(evl::def::uint32 size)
			{
				void* row = malloc(size);
				if (row == NULL)
					return row;

				rows.push_back(row);
				return row;
			}

			~MysqlData()
			{
				BOOST_FOREACH(void* row, rows)
				{
					if (row != NULL)
						free(row);
				}

				rows.clear();
			}
		};

		//////////////////////////////////////////////////////////////////////////
		// 数据库查询器
		class MysqlConnectorImpl
		{
		public:
			virtual bool is_connected() const = 0;
			virtual MYSQL* get_real_mysql() = 0;
		};

		class MysqlCursor
		{
			typedef boost::function<void(MysqlResultEnum, const MysqlData&)> ExecuteResultHandlerType;
		public:
			explicit MysqlCursor(MysqlConnectorImpl& connection)
				:connection_(connection)
			{}

			virtual ~MysqlCursor()
			{}

			void Execute(MysqlActionType action, const char* sql, const ExecuteResultHandlerType result_handler)
			{
				BOOST_ASSERT(sql != NULL);

				MysqlData result_data;
				if (!connection_.is_connected())
				{
					if (result_handler != 0)
						result_handler(MYSQL_RESULT_NOT_CONNECTED, result_data);

					return;
				}

				MYSQL* real_mysql = connection_.get_real_mysql();
				BOOST_ASSERT(real_mysql != NULL);
				evl::def::int32 ret = mysql_query(real_mysql, sql);
				if (ret != 0)
				{
					result_data.error_code = mysql_errno(real_mysql);
					result_data.error_desc = mysql_error(real_mysql);
					
					if (result_handler != 0)
					{
						result_handler(MYSQL_RESULT_ERROR, result_data);
					}
					else
					{
						if (result_data.error_code != 0)
						{
							std::cerr << "mysql error code: " << result_data.error_code << std::endl;
							std::cerr << "mysql error desc: " << result_data.error_desc << std::endl;
						}
					}

					return;
				}

				if (action == MYSQL_ACTION_TYPE_EXECUTE)
				{
					result_data.affected_rows = (evl::def::uint32)mysql_affected_rows(real_mysql);

					if (result_handler != 0)
						result_handler(MYSQL_RESULT_SUCCEED, result_data);

					return;
				}
				else if (action == MYSQL_ACTION_TYPE_QUERY)
				{
					// 处理结果集
					MYSQL_RES* res = mysql_store_result(real_mysql);
					BOOST_ASSERT(res != NULL);

					// 查询出来的行
					result_data.dataset_rows = (evl::def::uint32)mysql_num_rows(res);

					// 获取列信息
					MYSQL_FIELD* field = NULL;
					std::vector<FieldInfoSharedPtr> field_infos;
					while (field = mysql_fetch_field(res))
					{
						FieldInfoSharedPtr field_info = ParseField(field);
						if (field_info.get() == NULL)
						{
							mysql_free_result(res);

							if (result_handler != 0)
								result_handler(MYSQL_RESULT_FAILED_TO_PARSE_FIELD, result_data);

							return;
						}

						field_infos.push_back(field_info);
					}

					MysqlResultEnum result_enum = StoreDataSet(res, field_infos, result_data) ?
					MYSQL_RESULT_SUCCEED : MYSQL_RESULT_FAILED_TO_STORE_DATASET;

					mysql_free_result(res);

					if (result_handler != 0)
						result_handler(result_enum, result_data);

					return;
				}

				if (result_handler != 0)
					result_handler(MYSQL_RESULT_UNKNOWN_ERROR, result_data);
			}

			inline MysqlConnectorImpl& get_connector(){ return connection_; }

		private:
			bool StoreDataSet(MYSQL_RES* res, const std::vector<FieldInfoSharedPtr>& field_infos, MysqlData& result_data)
			{
				// 获取自动最大总长度
				evl::def::uint32 total_max_feild_len = 0;
				BOOST_FOREACH(const FieldInfoSharedPtr& field_info, field_infos)
				{
					total_max_feild_len += field_info->get_real_max_length();
				}

				// 逐行存储数据
				MYSQL_ROW row;
				while (row = mysql_fetch_row(res))
				{
					StoreRowData(row, field_infos, total_max_feild_len, result_data);
				}

				return result_data.rows.size() == mysql_num_rows(res);
			}

			bool StoreRowData(MYSQL_ROW row, const std::vector<FieldInfoSharedPtr>& field_infos, evl::def::uint32 all_field_max_len, MysqlData& result_data)
			{
				// 开辟一行的空间用以存储数据
				void* row_block = result_data.AllocRow(all_field_max_len);
				if (row_block == NULL)
				{
					BOOST_ASSERT(false);
					return false;
				}

				memset(row_block, 0, all_field_max_len);

				// 逐个处理列
				evl::def::uint32 offset = 0;
				evl::def::uint32 field_count = field_infos.size();
				for (evl::def::uint32 n = 0; n < field_count; ++n)
				{
					if (field_infos.at(n)->Cast(row[n], static_cast<char*>(row_block)+offset))
					{
						offset += field_infos.at(n)->get_real_max_length();
					}
					else
					{
						// todo 转换失败
					}

					BOOST_ASSERT(offset <= all_field_max_len);
				}

				return true;
			}


		private:
			MysqlConnectorImpl& connection_;
		};
		typedef boost::shared_ptr<MysqlCursor> Cursor;

		//////////////////////////////////////////////////////////////////////////
		// 数据库连接管理器
		class MysqlConnector : public MysqlConnectorImpl
		{
		public:
			MysqlConnector()
				: mysql_connection_(NULL)
				, connected_(false)
			{

			}

			virtual ~MysqlConnector()
			{

			}

			// 连接数据库
			bool Connect(const char* host,
				const evl::def::uint16 port,
				const char* user,
				const char* passwd,
				const char* dbname,
				const char* charset)
			{
				BOOST_ASSERT(!connected_);

				mysql_connection_ = mysql_init(NULL);
				if (!mysql_real_connect(mysql_connection_, host, user, passwd, dbname, port, NULL, CLIENT_MULTI_STATEMENTS))
				{
					std::cerr << "failed to connect mysql: " << mysql_error(mysql_connection_) << std::endl;
					BOOST_ASSERT(false);
					return false;
				}

				connected_ = true;
				return true;
			}

			// 关闭连接
			void Close()
			{
				if (connected_)
				{
					connected_ = false;
					if (mysql_connection_ != NULL)
					{
						mysql_close(mysql_connection_);
						mysql_connection_ = NULL;
					}
				}
			}

			// 获取查询器
			Cursor GetCursor()
			{
				Cursor cursor;
				cursor.reset(static_cast<MysqlCursor*>(NULL));

				if (is_connected())
				{
					cursor.reset(new MysqlCursor(*this));
				}

				return cursor;
			}

			// 获取实际数据库连接
			virtual MYSQL* get_real_mysql(){ return mysql_connection_; }
			virtual bool is_connected()const{ return connected_; }


		private:
			MYSQL* mysql_connection_;
			bool connected_;
		};

	}
}

#endif // VM_MYSQL_CONNECTOR_H_
