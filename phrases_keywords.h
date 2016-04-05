#if !defined(EXPAND_PHRASES_KEYWORDS_SSQLS_STATICS)
#   define MYSQLPP_SSQLS_NO_STATICS
#endif

#ifndef PHRASES_KEYWORDS_H
#define PHRASES_KEYWORDS_H

#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

sql_create_3(phrases_keywords, 
	3, 0,
	mysqlpp::sql_int_unsigned, phraseid,
	mysqlpp::sql_varchar, keyword,
	mysqlpp::sql_int_unsigned, shows)

#endif /* PHRASES_KEYWORDS_H */
