#if !defined(EXPAND_SQLKEYWORD_SSQLS_STATICS)
#   define MYSQLPP_SSQLS_NO_STATICS
#endif

#ifndef SQLKEYWORD_H
#define SQLKEYWORD_H

#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

sql_create_2(SqlKeyword, 
	2, 0,
	mysqlpp::sql_int_unsigned, phraseid,
	mysqlpp::sql_varchar, keyword)

#endif /* SQLKEYWORD_H */
