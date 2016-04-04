#include "../ydtask.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace ydd;

class TestYdTask : public YdTask
{
    public:
	TestYdTask(DbConn& dbc, DbConn::UserIdType userId, DbConn::TaskIdType taskId) : 
	    YdTask(dbc, userId, taskId)
	{
	}

	void test_logQuery_simple(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "Simple message", &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "SET @p = 0;"
		    "CALL sp_add_log_record(1, 0, 'Simple message', NULL, @p);"
		    "SELECT @p;");
	}

	void test_logQuery_oneDoubleQuote(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "One double \" quote", &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "SET @p = 0;"
		    "CALL sp_add_log_record(1, 0, 'One double \\\" quote', NULL, @p);"
		    "SELECT @p;");
	}

	void test_logQuery_oneSingleQuote(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "One ' single quote", &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "SET @p = 0;"
		    "CALL sp_add_log_record(1, 0, 'One \\\' single quote', NULL, @p);"
		    "SELECT @p;");
	}
};

struct FxYdTask
{
    FxYdTask() :
	dbc(),
	userId(173025),
	taskId(1),
	tydt(dbc, userId, taskId),
	conn(dbc.get()),
	query(conn.query())
    {
    }

    DbConn dbc;
    DbConn::UserIdType userId;
    DbConn::TaskIdType taskId;
    TestYdTask tydt;
    mysqlpp::Connection& conn;
    mysqlpp::Query query;
};
/*
BOOST_FIXTURE_TEST_CASE(logQuery_simple, FxYdTask)
{
    tydt.test_logQuery_simple(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_oneDoubleQuote, FxYdTask)
{
    tydt.test_logQuery_oneDoubleQuote(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_oneSingleQuote, FxYdTask)
{
    tydt.test_logQuery_oneSingleQuote(query);
}*/
