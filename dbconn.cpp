#include "dbconn.h"
#include "general.h"
#include "yddconf.h"
#include <mysql_error.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

namespace ydd
{
    DbConn::DbConn() : 
	currentUserId_(0)
    {
	try
	{
	    /* Any exception should be catched at app-global level  */
	    driver_ = sql::mysql::get_mysql_driver_instance();
	    connection_.reset(driver_->connect(YddConf::DbString, YddConf::DbUser, 
			YddConf::DbPassword));
	}
	catch(sql::SQLException &e)
	{
	    msyslog(LOG_ERR, "Got sql::SQLException: %s", e.what());
	    throw(e);
	}
    }

    void DbConn::checkConnection()
    {
	try 
	{
	    /* Any exception should be catched at app-global level */
	    int reconnAttempts = 0;
	    bool ok = connection_->isValid();
	    while(!ok && (reconnAttempts <= YddConf::DbMaxReconnectionAttempts))
	    {
		msyslog(LOG_WARNING, "Reconnection attempt #%d of %d", reconnAttempts, 
			YddConf::DbMaxReconnectionAttempts);
		reconnAttempts++;
		ok = connection_->reconnect();
	    }
	    if(!ok)
	    {
		msyslog(LOG_ERR, "Maximun reconnection attempts reached with no success");
		throw("Max DB reconnection attempts reached");
	    }
	}
	catch(sql::SQLException &e)
	{
	    msyslog(LOG_ERR, "Got sql::SQLException: %s", e.what());
	    throw(e);
	}
    }

    void DbConn::switchUser(YdTask::UserIdType userId)
    {
	if(userId == currentUserId_)
	    return;
	currentUserId_ = userId;
	currentSchema_ = YddConf::DbPrefix;
	currentSchema_.append(std::to_string(currentUserId_));
	try
	{
	    connection_->setSchema(currentSchema_);
	}
	catch(sql::SQLException &e)
	{
	    msyslog(LOG_ERR, "Got sql::SQLException: %s", e.what());
	    throw(e);
	}
    }

    YdTask::UserIdType DbConn::getCurrentUserId() const
    {
	return currentUserId_;
    }

    std::string& DbConn::getCurrentSchema() 
    {
	return currentSchema_;
    }
}
