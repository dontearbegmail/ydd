#ifndef DBCONN_H
#define DBCONN_H

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <memory>
#include <string>

namespace ydd
{
    class DbConn
    {
	public:
	    typedef std::shared_ptr<sql::Connection> PConnection;

	    DbConn();
	private:
	    sql::mysql::MySQL_Driver* driver_;
	    PConnection connection_;

	    void checkConnection();
    };
}

#endif /* DBCONN_H */
