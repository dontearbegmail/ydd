#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

sql_create_2(YdBaseTask_test_tasks_phrases, 
	2, 0,
	mysqlpp::sql_tinyint, finished,
	mysqlpp::sql_varchar, phrase)

