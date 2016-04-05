#ifndef SQLPHRASE_H
#define SQLPHRASE_H

#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

sql_create_4(SqlPhrase, 
	4, 0,
	mysqlpp::sql_int_unsigned, taskid,
	mysqlpp::sql_varchar, phrase,
	mysqlpp::sql_int_unsigned, id,
	mysqlpp::sql_tinyint_unsigned, finished)

#endif /* SQLPHRASE_H */
