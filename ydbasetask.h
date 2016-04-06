#ifndef YDBASETASK_H
#define YDBASETASK_H

#include "dbconn.h"
#include <string>
#include <vector>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include "ydphrase.h"

namespace ydd
{
    class YdBaseTask
    {
	public:
	    enum LogLevel {info, error, debug, warning};

	    YdBaseTask(DbConn& dbc, DbConn::UserIdType userId, DbConn::TaskIdType taskId);
	    virtual ~YdBaseTask();
	    void log(LogLevel level, const char* message, std::string* html = NULL);
	protected:
	    DbConn& dbc_;
	    DbConn::UserIdType userId_;
	    DbConn::TaskIdType taskId_;

	    std::vector<YdReport> reports_;

	    void dispatch();
	    /* Don't forget that dbc_.switchUserDb(userId_) should be called 
	     * before these both !!! */
	    void storeReports(mysqlpp::Connection& conn);
	    void storePhrase(YdPhrase& phrase, mysqlpp::Connection& conn);
	    size_t countFreePhrasesSlots();

	    void logQuery(mysqlpp::Query& query, LogLevel level, const char* message, 
		    std::string* html = NULL, std::string* os = NULL);
    };
}

#endif /* YDBASETASK_H */
