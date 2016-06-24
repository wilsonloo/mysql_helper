#ifndef EVL_ENGINE_DB_MYSQL_FIELD_PARSER_H_
#define EVL_ENGINE_DB_MYSQL_FIELD_PARSER_H_

#ifdef _PF_WINDOWNS_
#include <WinSock2.h>
#endif // _PF_WINDOWNS_

#include <cstdio>
#include <mysql.h>
#include <boost/shared_ptr.hpp>
#include "field_info.hpp"
#include "logger.h"

namespace evl
{
	namespace mysql
	{
		typedef boost::shared_ptr<FieldInfo> FieldInfoSharedPtr;
		
		
		static FieldInfoSharedPtr ParseField(MYSQL_FIELD* row_field)
		{
			BOOST_ASSERT(row_field != NULL);

			switch (row_field->type)
			{
			case MYSQL_TYPE_LONG:
				{
					return FieldInfoSharedPtr(new LongFieldInfo(row_field));
				}
				break;
			case MYSQL_TYPE_LONGLONG:
				{
					return FieldInfoSharedPtr(new LongLongFieldInfo(row_field));
				}
				break;
			case MYSQL_TYPE_SHORT:
				{
					return FieldInfoSharedPtr(new ShortFieldInfo(row_field));
				}
				break;
			case MYSQL_TYPE_TINY:
				{
					return FieldInfoSharedPtr(new TinyFieldInfo(row_field));
				}
				break;
			case MYSQL_TYPE_STRING:
				{
					return FieldInfoSharedPtr(new StringFieldInfo(row_field));
				}
				break;
			case MYSQL_TYPE_VAR_STRING:
				{
					return FieldInfoSharedPtr(new VarstringFieldInfo(row_field));
				}
				break;
			default:
				{
					EVL_ENGINE_MYSQL_LOG("unknown field type: %d", static_cast<evl::def::uint32>(row_field->type));
					BOOST_ASSERT(false);
				}
				break;
			}

			return FieldInfoSharedPtr(static_cast<FieldInfo*>(NULL));
		}
	}
}

#endif // EVL_ENGINE_DB_MYSQL_FIELD_PARSER_H_
