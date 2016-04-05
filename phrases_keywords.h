#if !defined(EXPAND_PHRASES_KEYWORDS_SSQLS_STATICS)
#   define MYSQLPP_SSQLS_NO_STATICS
#endif

#ifndef PHRASES_KEYWORDS_H
#define PHRASES_KEYWORDS_H

#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

sql_create_2(phrases_keywords, 
	2, 0,
	mysqlpp::sql_int_unsigned, phraseid,
	mysqlpp::sql_varchar, keyword)

#endif /* PHRASES_KEYWORDS_H */
