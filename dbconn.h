#ifndef DBCONN_H
#define DBCONN_H

#include <mysql++/mysql++.h>
#include <string>

/* Any database exception must thrown down the app top level */

namespace ydd
{
    class DbConn
    {
	public:
	    typedef unsigned long UserIdType;
	    typedef unsigned long TaskIdType;

	    DbConn();
	    void switchUserDb(UserIdType userId);
	    void check();
	    mysqlpp::Connection& get();

	protected:
	    mysqlpp::Connection connection_;
	    UserIdType currentUserId_;
	    std::string currentDb_;
    };
}

#endif /* DBCONN_H */
