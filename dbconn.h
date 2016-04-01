#ifndef DBCONN_H
#define DBCONN_H

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <memory>
#include <string>
#include "ydtask.h"

namespace ydd
{
    class DbConn
    {
	public:
	    typedef std::shared_ptr<sql::Connection> PConnection;

	    DbConn();
	    void switchUser(YdTask::UserIdType userId);

	    YdTask::UserIdType getCurrentUserId() const;
	    std::string& getCurrentSchema();
	private:
	    sql::mysql::MySQL_Driver* driver_;
	    PConnection connection_;

	    /* this is not mysql user, but our system's user */
	    YdTask::UserIdType currentUserId_;
	    std::string currentSchema_;

	    void checkConnection();
    };
}

#endif /* DBCONN_H */
