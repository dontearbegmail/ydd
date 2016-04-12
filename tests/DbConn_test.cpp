#include "../dbconn.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace ydd;

class TestDbConn : public DbConn
{
    public:
	TestDbConn()
	{
	}

	void test_switchUserDb_nonExistingUserId()
	{
	    BOOST_REQUIRE_THROW(switchUserDb(std::numeric_limits<UserIdType>::max()), mysqlpp::Exception);
	    BOOST_REQUIRE_EQUAL(currentUserId_, 0);
	    BOOST_REQUIRE_EQUAL(currentDb_, "");
	}
	
	void test_switchUserDb_existingUserId()
	{
	    BOOST_REQUIRE_NO_THROW(switchUserDb(50));
	    BOOST_REQUIRE_EQUAL(currentUserId_, 50);
	    BOOST_REQUIRE_EQUAL(currentDb_, "ydd_50");
	}

	void test_switchUserDb_sameUserId()
	{
	    BOOST_REQUIRE_NO_THROW(switchUserDb(50));
	    BOOST_REQUIRE_EQUAL(currentUserId_, 50);
	    BOOST_REQUIRE_EQUAL(currentDb_, "ydd_50");

	    BOOST_REQUIRE_NO_THROW(switchUserDb(50));
	    BOOST_REQUIRE_EQUAL(currentUserId_, 50);
	    BOOST_REQUIRE_EQUAL(currentDb_, "ydd_50");
	}

	void test_switchDbTasks_simple()
	{
	    BOOST_REQUIRE_NO_THROW(switchDbTasks());
	    BOOST_REQUIRE_EQUAL(currentUserId_, 0);
	    BOOST_REQUIRE_EQUAL(currentDb_, "ydd_tasks");
	}

	void test_switchDbTasks_halfCombo()
	{
	    BOOST_REQUIRE_NO_THROW(switchUserDb(50));
	    BOOST_REQUIRE_NO_THROW(switchDbTasks());
	    BOOST_REQUIRE_EQUAL(currentUserId_, 0);
	    BOOST_REQUIRE_EQUAL(currentDb_, "ydd_tasks");
	}

	void test_switchDbTasks_threeQuartersCombo()
	{
	    BOOST_REQUIRE_THROW(switchUserDb(std::numeric_limits<UserIdType>::max()), mysqlpp::Exception);
	    BOOST_REQUIRE_NO_THROW(switchUserDb(50));
	    BOOST_REQUIRE_NO_THROW(switchDbTasks());
	    BOOST_REQUIRE_EQUAL(currentUserId_, 0);
	    BOOST_REQUIRE_EQUAL(currentDb_, "ydd_tasks");
	}

	void test_switchDbTasks_combo()
	{
	    BOOST_REQUIRE_NO_THROW(switchUserDb(50));
	    BOOST_REQUIRE_THROW(switchUserDb(std::numeric_limits<UserIdType>::max()), mysqlpp::Exception);
	    BOOST_REQUIRE_NO_THROW(switchDbTasks());
	    BOOST_REQUIRE_EQUAL(currentUserId_, 0);
	    BOOST_REQUIRE_EQUAL(currentDb_, "ydd_tasks");
	    BOOST_REQUIRE_NO_THROW(switchUserDb(50));
	    BOOST_REQUIRE_EQUAL(currentUserId_, 50);
	    BOOST_REQUIRE_EQUAL(currentDb_, "ydd_50");
	}
};

struct FxDbConn
{
    FxDbConn() :
	dbc()
    {
    }

    TestDbConn dbc;
};

BOOST_FIXTURE_TEST_CASE(switchUserDb_nonExistingUserId, FxDbConn)
{
    dbc.test_switchUserDb_nonExistingUserId();
}

BOOST_FIXTURE_TEST_CASE(switchUserDb_existingUserId, FxDbConn)
{
    dbc.test_switchUserDb_existingUserId();
}

BOOST_FIXTURE_TEST_CASE(test_switchDbTasks_simple, FxDbConn)
{
    dbc.test_switchDbTasks_simple();
}

BOOST_FIXTURE_TEST_CASE(test_switchDbTasks_halfCombo, FxDbConn)
{
    dbc.test_switchDbTasks_halfCombo();
}

BOOST_FIXTURE_TEST_CASE(test_switchDbTasks_threeQuartersCombo, FxDbConn)
{
    dbc.test_switchDbTasks_threeQuartersCombo();
}

BOOST_FIXTURE_TEST_CASE(test_switchDbTasks_combo, FxDbConn)
{
    dbc.test_switchDbTasks_combo();
}
