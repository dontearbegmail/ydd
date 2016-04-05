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
	    void storeReport(YdReport& report);

	    /* Don't forget that dbc_.switchUserDb(userId_) should be called before !!! */
	    void storePhrase(YdPhrase& phrase, mysqlpp::Connection& con);

	    void logQuery(mysqlpp::Query& query, LogLevel level, const char* message, 
		    std::string* html = NULL, std::string* os = NULL);
    };
}

#endif /* YDBASETASK_H */
