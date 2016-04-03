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
	    //logQuery(query, info, "Simple message");
	    //BOOST_REQUIRE_EQUAL(query.str(), "");
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

BOOST_FIXTURE_TEST_CASE(logQuery_simple, FxYdTask)
{
    tydt.test_logQuery_simple(query);
}
