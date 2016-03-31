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
    DbConn::DbConn()
    {
	/* Any exception should be catched at app-global level  */
	driver_ = sql::mysql::get_mysql_driver_instance();
	connection_.reset(driver_->connect(YddConf::DbString, YddConf::DbUser, 
		YddConf::DbPassword));
    }

    void DbConn::checkConnection()
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
	    msyslog(LOG_ERROR, "Maximun reconnection attempts reached with no success");
	    throw("Max DB reconnection attempts reached");
	}
    }
}
