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
	    logQuery(query, info, "Simple message", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'Simple message', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_oneDoubleQuote(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "One double \" quote", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'One double \\\" quote', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_oneSingleQuote(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "One ' single quote", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'One \\\' single quote', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_combinedQuotes1(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "'Comb'ined \"quotes\" 1", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, '\\\'Comb\\\'ined \\\"quotes\\\" 1', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_combinedQuotes2(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "\"Combined\" 'quotes 2\"", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, '\\\"Combined\\\" \\\'quotes 2\\\"', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_sqlInjection1(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "select * from tasks_phrases", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'select * from tasks_phrases', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_sqlInjection2(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "select * from `tasks_phrases where `id` = 0", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'select * from `tasks_phrases where `id` = 0', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_specialchars(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "±§!@#$%^&*()_+№;:`{?-+=/][}{<>~`", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, '±§!@#$%^&*()_+№;:`{?-+=/][}{<>~`', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_log_sqlInjection1(mysqlpp::Query query)
	{
	    BOOST_REQUIRE_NO_THROW(log(info, "select * from tasks_phrases"));
	}

	void test_log_sqlInjection2(mysqlpp::Query query)
	{
	    BOOST_REQUIRE_NO_THROW(log(info, "select * from `tasks_phrases where `id` = 0"));
	}

	void test_log_specialchars(mysqlpp::Query query)
	{
	    BOOST_REQUIRE_NO_THROW(log(info, "±§!@#$%^&*()_+№;:`{?-+=/][}{<>~`"));
	}

	void test_logQuery_XSS1(mysqlpp::Query query)
	{
	    std::string q;
	    logQuery(query, info, "<script type=\"text/javascript\">alert('XSS');</script>", 
		    NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, "
		    "'<script type=\\\"text/javascript\\\">alert(\\\'XSS\\\');</script>', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_log_XSS1(mysqlpp::Query query)
	{
	    BOOST_REQUIRE_NO_THROW(log(info, "<script type=\"text/javascript\">alert('XSS');</script>"));
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

BOOST_FIXTURE_TEST_CASE(logQuery_oneDoubleQuote, FxYdTask)
{
    tydt.test_logQuery_oneDoubleQuote(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_oneSingleQuote, FxYdTask)
{
    tydt.test_logQuery_oneSingleQuote(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_combinedQuotes1, FxYdTask)
{
    tydt.test_logQuery_combinedQuotes1(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_combinedQuotes2, FxYdTask)
{
    tydt.test_logQuery_combinedQuotes2(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_sqlInjection1, FxYdTask)
{
    tydt.test_logQuery_sqlInjection1(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_sqlInjection2, FxYdTask)
{
    tydt.test_logQuery_sqlInjection2(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_specialchars, FxYdTask)
{
    tydt.test_logQuery_specialchars(query);
}

BOOST_FIXTURE_TEST_CASE(log_sqlInjection1, FxYdTask)
{
    tydt.test_log_sqlInjection1(query);
}

BOOST_FIXTURE_TEST_CASE(log_sqlInjection2, FxYdTask)
{
    tydt.test_log_sqlInjection2(query);
}

BOOST_FIXTURE_TEST_CASE(log_specialchars, FxYdTask)
{
    tydt.test_log_specialchars(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_XSS1, FxYdTask)
{
    tydt.test_logQuery_XSS1(query);
}

BOOST_FIXTURE_TEST_CASE(log_XSS1, FxYdTask)
{
    tydt.test_log_XSS1(query);
}


