#ifndef EVL_ENGINE_DB_MYSQL_FIELD_INFO_HPP_
#define EVL_ENGINE_DB_MYSQL_FIELD_INFO_HPP_

#include <WinSock2.h>
#include <sstream>
#include <mysql.h>
#include <boost/assert.hpp>
#include <boost/any.hpp>
#include "../typedef.h"

namespace evl
{
	namespace mysql
	{
		//////////////////////////////////////////////////////////////////////////
		// �ֶλ���
		class FieldInfo
		{
		public:
			explicit FieldInfo(MYSQL_FIELD* mysql_field)
				: mysql_field_(mysql_field)
			{}

			// ��ת���ַ�����ʵ�ʳ���
			virtual evl::def::uint32 get_real_max_length()const = 0;

			// ���ֶ�ת���ʵ���ֶζ�Ӧ������
			virtual bool Cast(const char* from, /*OUT*/void* to) = 0;

		protected:
			MYSQL_FIELD* mysql_field_;
		};

		//////////////////////////////////////////////////////////////////////////
		class LongFieldInfo : public FieldInfo
		{
			typedef FieldInfo Super;
		public:
			explicit LongFieldInfo(MYSQL_FIELD* mysql_field)
				: Super(mysql_field)
			{}

			// ��ת���ַ�����ʵ�ʳ���
			virtual evl::def::uint32 get_real_max_length() const
			{
				return sizeof(evl::def::uint32);
			}

			// ���ֶ�ת���ʵ���ֶζ�Ӧ������
			virtual bool Cast(const char* from, /*OUT*/void* to)
			{
				BOOST_ASSERT(from != NULL);
				BOOST_ASSERT(to != NULL);

				evl::def::uint32 tmp = atoi(from);
				memcpy(to, &tmp, get_real_max_length());

				return true;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class LongLongFieldInfo : public FieldInfo
		{
			typedef FieldInfo Super;
		public:
			explicit LongLongFieldInfo(MYSQL_FIELD* mysql_field)
				: Super(mysql_field)
			{}

			// ��ת���ַ�����ʵ�ʳ���
			virtual evl::def::uint32 get_real_max_length() const
			{
				return sizeof(evl::def::uint64);
			}

			// ���ֶ�ת���ʵ���ֶζ�Ӧ������
			virtual bool Cast(const char* from, /*OUT*/void* to)
			{
				BOOST_ASSERT(from != NULL);
				BOOST_ASSERT(to != NULL);

				evl::def::uint64 tmp = 0;
				std::stringstream ss; ss << from; ss >> tmp;
				ss.clear();
				memcpy(to, &tmp, get_real_max_length());

				return true;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class ShortFieldInfo : public FieldInfo
		{
			typedef FieldInfo Super;
		public:
			explicit ShortFieldInfo(MYSQL_FIELD* mysql_field)
				: Super(mysql_field)
			{}

			// ��ת���ַ�����ʵ�ʳ���
			virtual evl::def::uint32 get_real_max_length() const
			{
				return sizeof(evl::def::uint16);
			}

			// ���ֶ�ת���ʵ���ֶζ�Ӧ������
			virtual bool Cast(const char* from, /*OUT*/void* to)
			{
				BOOST_ASSERT(from != NULL);
				BOOST_ASSERT(to != NULL);

				evl::def::uint32 tmp = atoi(from);
				memcpy(to, &tmp, get_real_max_length());

				return true;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class TinyFieldInfo : public FieldInfo
		{
			typedef FieldInfo Super;
		public:
			explicit TinyFieldInfo(MYSQL_FIELD* mysql_field)
				: Super(mysql_field)
			{}

			// ��ת���ַ�����ʵ�ʳ���
			virtual evl::def::uint32 get_real_max_length() const
			{
				return sizeof(evl::def::uint8);
			}

			// ���ֶ�ת���ʵ���ֶζ�Ӧ������
			virtual bool Cast(const char* from, /*OUT*/void* to)
			{
				BOOST_ASSERT(from != NULL);
				BOOST_ASSERT(to != NULL);

				evl::def::uint32 tmp = atoi(from);
				memcpy(to, &tmp, get_real_max_length());

				return true;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class VarstringFieldInfo : public FieldInfo
		{
			typedef FieldInfo Super;
		public:
			explicit VarstringFieldInfo(MYSQL_FIELD* mysql_field)
				: Super(mysql_field)
			{}

			// ��ת���ַ�����ʵ�ʳ���
			virtual evl::def::uint32 get_real_max_length() const
			{
				return mysql_field_->length;
			}

			// ���ֶ�ת���ʵ���ֶζ�Ӧ������
			virtual bool Cast(const char* from, /*OUT*/void* to)
			{
				BOOST_ASSERT(strlen(from) <= get_real_max_length());
				memcpy(static_cast<char*>(to), from, get_real_max_length());

				return true;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class StringFieldInfo : public FieldInfo
		{
			typedef FieldInfo Super;
		public:
			explicit StringFieldInfo(MYSQL_FIELD* mysql_field)
				: Super(mysql_field)
			{}

			// ��ת���ַ�����ʵ�ʳ���
			virtual evl::def::uint32 get_real_max_length() const
			{
				return mysql_field_->length;
			}

			// ���ֶ�ת���ʵ���ֶζ�Ӧ������
			virtual bool Cast(const char* from, /*OUT*/void* to)
			{
				BOOST_ASSERT(strlen(from) <= get_real_max_length());
				memcpy(static_cast<char*>(to), from, get_real_max_length());

				return true;
			}
		};
	}
}

#endif // EVL_ENGINE_DB_MYSQL_FIELD_INFO_HPP_