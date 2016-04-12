#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

sql_create_2(YdBaseTask_test_tasks, 
	2, 0,
	mysqlpp::sql_int_unsigned, id,
	mysqlpp::sql_timestamp_null, finished)

