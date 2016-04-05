#ifndef YDBASETASK_H
#define YDBASETASK_H

#include "dbconn.h"
#include <string>
#include <boost/function.hpp>
#include <boost/asio.hpp>

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

	    void logQuery(mysqlpp::Query& query, LogLevel level, const char* message, 
		    std::string* html = NULL, std::string* os = NULL);
    };
}

#endif /* YDBASETASK_H */
