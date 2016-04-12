#include "dbconn.h"
#include "general.h"
#include "yddconf.h"

#define USER_ID_TYPE_PRINTF_FORMAT "%lu"

namespace ydd
{
    DbConn::DbConn() :
	connection_(true),
	currentUserId_(0)
    {
	try
	{
	    connection_.set_option(new mysqlpp::ReconnectOption(false));
	    connection_.set_option(new mysqlpp::MultiStatementsOption(true));
	    connection_.set_option(new mysqlpp::MultiResultsOption(true));
	    connection_.connect(NULL, YddConf::DbString.c_str(), YddConf::DbUser.c_str(), 
		YddConf::DbPassword.c_str());
	}
	catch(const mysqlpp::Exception& ex)
	{
	    msyslog(LOG_ERR, "MySQL connection failed: %s", ex.what());
	    throw(ex);
	}
    }

    void DbConn::switchDbTasks()
    {
	std::string name = YddConf::DbPrefix + "tasks";
	if(currentDb_ == name)
	    return;
	switchDb(name);
	currentUserId_ = 0;
	currentDb_ = name;
    }

    void DbConn::switchDb(std::string& dbName)
    {
	try
	{
	    connection_.select_db(dbName);
	}
	catch(const mysqlpp::Exception& e)
	{
	    msyslog(LOG_ERR, "Got exception while trying to switch db to "
		    "`%s`: %s", dbName.c_str(), e.what());
	    throw(e);
	}
    }

    void DbConn::switchUserDb(UserIdType userId)
    {
	using namespace std;
	if(userId == currentUserId_)
	    return;
	string newDb = YddConf::DbPrefix;
	newDb.append(to_string(userId));
	switchDb(newDb);
	currentUserId_ = userId;
	currentDb_ = newDb;
    }

    void DbConn::check()
    {
	mysqlpp::NoExceptions ne(connection_);
	int reconnAttempts = 0;
	bool ok = connection_.ping();
	if(!ok)
	    msyslog(LOG_WARNING, "Database connection lost, will try to reconnect");
	while(!ok && (reconnAttempts < YddConf::DbMaxReconnectionAttempts))
	{
	    msyslog(LOG_WARNING, "Reconnection attempt #%d of %d", reconnAttempts, 
		    YddConf::DbMaxReconnectionAttempts);
	    reconnAttempts++;
	    connection_.connect(currentDb_.empty() ? NULL : currentDb_.c_str(),
		    YddConf::DbString.c_str(), YddConf::DbUser.c_str(), 
		    YddConf::DbPassword.c_str());
	    ok = connection_.ping();
	}
	if(!ok)
	{
	    msyslog(LOG_ERR, "Max reconnection attempts reached with no success");
	    throw(new mysqlpp::ConnectionFailed(connection_.error()));
	}
    }

    mysqlpp::Connection& DbConn::get()
    {
	check();
	return connection_;
    }

}
