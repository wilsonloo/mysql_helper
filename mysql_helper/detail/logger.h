#ifndef EVL_ENGINE_DB_MYSQL_LOGGER_H_
#define EVL_ENGINE_DB_MYSQL_LOGGER_H_

#include <iostream>

#define EVL_ENGINE_MYSQL_LOG(...) \
	printf("[function: %s, line: %d] ", __FUNCTION__, __LINE__); \
	fprintf(stdout, __VA_ARGS__); \
	printf("\n");

#endif // EVL_ENGINE_DB_MYSQL_LOGGER_H_