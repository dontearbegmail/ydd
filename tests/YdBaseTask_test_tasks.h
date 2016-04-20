#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

sql_create_3(YdBaseTask_test_tasks, 
	3, 0,
	mysqlpp::sql_int_unsigned, id,
	mysqlpp::sql_timestamp_null, finished,
	mysqlpp::sql_tinyint_unsigned_null, finishedState)

