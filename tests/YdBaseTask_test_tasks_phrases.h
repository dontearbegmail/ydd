#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

sql_create_3(YdBaseTask_test_tasks_phrases, 
	3, 0,
	mysqlpp::sql_int_unsigned, taskid,
	mysqlpp::sql_tinyint, finished,
	mysqlpp::sql_varchar, phrase)

