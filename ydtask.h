#ifndef YDTASK_H
#define YDTASK_H

#include "dbconn.h"

namespace ydd
{
    class YdTask
    {
	public:
	    enum LogLevel {info, error, debug, warning};
	    YdTask(DbConn& dbc, DbConn::UserIdType userId, DbConn::TaskIdType taskId);
	    void log(LogLevel level, const char* message);
	protected:
	    DbConn& dbc_;
	    DbConn::UserIdType userId_;
	    DbConn::TaskIdType taskId_;


	    void logQuery(mysqlpp::Query& query, LogLevel level, const char* message,
		    std::string* os = NULL);
    };
}

#endif /* YDTASK_H */
