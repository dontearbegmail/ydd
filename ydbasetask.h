#ifndef YDBASETASK_H
#define YDBASETASK_H

#include "dbconn.h"
#include <string>
#include <vector>
#include <queue>
#include <boost/asio.hpp>
#include "ydphrase.h"
#include <functional>

namespace ydd
{
    class YdBaseTask
    {
	public:
	    enum LogLevel {info, error, debug, warning};

	    YdBaseTask(boost::asio::io_service& ios, DbConn& dbc, 
		    DbConn::UserIdType userId, DbConn::TaskIdType taskId);
	    virtual ~YdBaseTask();
	    void log(LogLevel level, const char* message, std::string* html = NULL);
	protected:
	    boost::asio::io_service& ios_;
	    DbConn& dbc_;
	    DbConn::UserIdType userId_;
	    DbConn::TaskIdType taskId_;
	    std::vector<YdReport> reports_;
	    std::function<void()> callback_;

	    void dispatch();
	    /* Don't forget that dbc_.switchUserDb(userId_) should be called 
	     * before these three !!! */
	    void storeReports(mysqlpp::Connection& conn);
	    void storePhrase(YdPhrase& phrase, mysqlpp::Connection& conn);
	    size_t getPhrasesFromDb(size_t numPhrases, mysqlpp::Connection& conn);

	    void setCompleted(mysqlpp::Connection& conn);
	    size_t countFreePhrasesSlots();

	    void logQuery(mysqlpp::Query& query, LogLevel level, const char* message, 
		    std::string* html = NULL, std::string* os = NULL);

	    virtual void startReportProcessing(YdReport& report);
    };
}

#endif /* YDBASETASK_H */
