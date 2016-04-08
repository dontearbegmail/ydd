#include "../ydbasetask.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../general.h"
#include "YdBaseTask_test_tasks_phrases.h"
#include <stdlib.h>
#include <time.h>

using namespace ydd;

class TestYdBaseTask : public YdBaseTask
{
    public:
	TestYdBaseTask(DbConn& dbc, DbConn::UserIdType userId, DbConn::TaskIdType taskId) : 
	    YdBaseTask(dbc, userId, taskId)
	{
	    using namespace mysqlpp;
	    dbc_.switchUserDb(userId_);
	    Query query = dbc_.get().query();
	    query << "DROP PROCEDURE IF EXISTS `sp_fill_test_tasks_phrases_set`";
	    query.execute();
	    query << 
		"CREATE PROCEDURE `sp_fill_test_tasks_phrases_set`"
		    "(IN `p_first` INT UNSIGNED, IN `p_last` INT UNSIGNED, "
		    " IN `p_finished` BOOLEAN, IN `p_taskid` INT UNSIGNED) "
		"BEGIN\r\n"
		"\tDECLARE i INT;\r\n"
		"\tDECLARE phr VARCHAR(32);\r\n"
		"\tSET i = p_first;\r\n"
		"\tWHILE i <= p_last DO\r\n"
		"\t\tSET phr = CONCAT('phrase', i);\r\n"
		"\t\tINSERT INTO tasks_phrases(`taskid`, `phrase`, `finished`) \r\n"
		"\t\t\tVALUES (p_taskid, phr, p_finished);\r\n"
		"\t\tSET i = i + 1;\r\n"
		"\tEND WHILE;\r\n"
		"END";
	    query.execute();
	}

	virtual ~TestYdBaseTask()
	{
	}

	void test_logQuery_simple(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "Simple message", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'Simple message', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_oneDoubleQuote(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "One double \" quote", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'One double \\\" quote', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_oneSingleQuote(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "One ' single quote", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'One \\\' single quote', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_combinedQuotes1(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "'Comb'ined \"quotes\" 1", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, '\\\'Comb\\\'ined \\\"quotes\\\" 1', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_combinedQuotes2(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "\"Combined\" 'quotes 2\"", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, '\\\"Combined\\\" \\\'quotes 2\\\"', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_sqlInjection1(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "select * from tasks_phrases", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'select * from tasks_phrases', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_sqlInjection2(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "select * from `tasks_phrases where `id` = 0", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, 'select * from `tasks_phrases where `id` = 0', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_logQuery_specialchars(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "±§!@#$%^&*()_+№;:`{?-+=/][}{<>~`", NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, '±§!@#$%^&*()_+№;:`{?-+=/][}{<>~`', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_log_sqlInjection1(mysqlpp::Query& query)
	{
	    BOOST_REQUIRE_NO_THROW(log(info, "select * from tasks_phrases"));
	}

	void test_log_sqlInjection2(mysqlpp::Query& query)
	{
	    BOOST_REQUIRE_NO_THROW(log(info, "select * from `tasks_phrases where `id` = 0"));
	}

	void test_log_specialchars(mysqlpp::Query& query)
	{
	    BOOST_REQUIRE_NO_THROW(log(info, "±§!@#$%^&*()_+№;:`{?-+=/][}{<>~`"));
	}

	void test_logQuery_XSS1(mysqlpp::Query& query)
	{
	    std::string q;
	    logQuery(query, info, "<script type=\"text/javascript\">alert('XSS');</script>", 
		    NULL, &q);
	    BOOST_REQUIRE_EQUAL(query.str(), 
		    "CALL sp_add_log_record(1, 0, "
		    "'<script type=\\\"text/javascript\\\">alert(\\\'XSS\\\');</script>', NULL, @ret);"
		    "SELECT @ret;");
	}

	void test_log_XSS1(mysqlpp::Query& query)
	{
	    BOOST_REQUIRE_NO_THROW(log(info, "<script type=\"text/javascript\">alert('XSS');</script>"));
	}

	/*
	DELETE FROM `phrases_keywords`;
	ALTER TABLE `phrases_keywords` AUTO_INCREMENT = 1;
	DELETE FROM `tasks_phrases`;
	ALTER TABLE `tasks_phrases` AUTO_INCREMENT = 1
	*/
	void flush_storePhrase(mysqlpp::Query& query)
	{
	    query << 
		"DELETE FROM `phrases_keywords`;"
		"ALTER TABLE `phrases_keywords` AUTO_INCREMENT = 1;"
		"DELETE FROM `tasks_phrases`;"
		"ALTER TABLE `tasks_phrases` AUTO_INCREMENT = 1";
	    query.exec();
	    flushQuery(query);
	}

	void flushQuery(mysqlpp::Query& query)
	{ 
	    while(query.more_results())
		query.store_next();
	}

	void test_storePhrase_simple(mysqlpp::Connection& conn)
	{
	    mysqlpp::Query query = conn.query();
	    dbc_.switchUserDb(userId_);
	    flush_storePhrase(query);
	    query << 
		"CALL `sp_fill_test_tasks_phrases_set`(1, 7, 0, 1);" <<
		"CALL `sp_fill_test_tasks_phrases_set`(21, 24, 0, 2);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(71, 72, 0, 7);";
	    query.exec();
	    flushQuery(query);

	    YdPhrase p1 = {1, ""};
	    p1.keywords.push_back({1, "keyword1_1", 11});
	    p1.keywords.push_back({1, "keyword1_2", 12});
	    p1.keywords.push_back({1, "keyword1_3", 13});
	    storePhrase(p1, conn);

	    YdPhrase p4 = {4, ""};
	    p4.keywords.push_back({4, "keyword4_1", 41});
	    p4.keywords.push_back({4, "keyword4_2", 42});
	    p4.keywords.push_back({4, "keyword4_3", 43});
	    p4.keywords.push_back({4, "keyword4_4", 44});
	    p4.keywords.push_back({4, "keyword4_5", 45});
	    storePhrase(p4, conn);

	    YdPhrase p7 = {7, ""};
	    p7.keywords.push_back({7, "keyword7_1", 71});
	    storePhrase(p7, conn);

	    /* phrase has no keywords */
	    YdPhrase p10 = {10, ""};
	    storePhrase(p10, conn);

	    std::vector<phrases_keywords> result, input;
	    query << "SELECT `phraseid`, `keyword`, `shows` FROM `phrases_keywords`;";
	    query.storein(result);

	    input = p1.keywords;
	    input.insert(input.end(), p4.keywords.begin(), p4.keywords.end());
	    input.insert(input.end(), p7.keywords.begin(), p7.keywords.end());

	    BOOST_REQUIRE(result == input);

	    std::vector<YdBaseTask_test_tasks_phrases> tp_result, expected;
	    expected.push_back({1, 1, "phrase1"});
	    expected.push_back({1, 0, "phrase2"});
	    expected.push_back({1, 0, "phrase3"});
	    expected.push_back({1, 1, "phrase4"});
	    expected.push_back({1, 0, "phrase5"});
	    expected.push_back({1, 0, "phrase6"});
	    expected.push_back({1, 1, "phrase7"});
	    expected.push_back({2, 0, "phrase21"});
	    expected.push_back({2, 0, "phrase22"});
	    expected.push_back({2, 1, "phrase23"});
	    expected.push_back({2, 0, "phrase24"});
	    expected.push_back({7, 0, "phrase71"});
	    expected.push_back({7, 0, "phrase72"});

	    query << "SELECT `taskid`, `finished`, `phrase` FROM `tasks_phrases`;";
	    query.storein(tp_result);
	    BOOST_REQUIRE(tp_result == expected);
	}

	void test_countFreePhrasesSlots_used3expected20()
	{
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    BOOST_REQUIRE_EQUAL(countFreePhrasesSlots(), 20);
	}

	void test_countFreePhrasesSlots_used0expected50()
	{
	    BOOST_REQUIRE_EQUAL(countFreePhrasesSlots(), 50);
	}

	void test_countFreePhrasesSlots_used5expected0()
	{
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    BOOST_REQUIRE_EQUAL(countFreePhrasesSlots(), 0);
	}

	void test_countFreePhrasesSlots_used6expected0()
	{
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    BOOST_REQUIRE_EQUAL(countFreePhrasesSlots(), 0);
	}

	void test_storeReports(mysqlpp::Connection& conn)
	{
	    mysqlpp::Query query = conn.query();
	    dbc_.switchUserDb(userId_);
	    reports_.push_back({{}, true});
	    BOOST_REQUIRE_NO_THROW(storeReports(conn));
	}
};

struct FxYdBaseTask
{
    FxYdBaseTask() :
	dbc(),
	userId(50),
	taskId(1),
	tydt(dbc, userId, taskId),
	conn(dbc.get()),
	query(conn.query())
    {
    }

    DbConn dbc;
    DbConn::UserIdType userId;
    DbConn::TaskIdType taskId;
    TestYdBaseTask tydt;
    mysqlpp::Connection& conn;
    mysqlpp::Query query;
};

BOOST_FIXTURE_TEST_CASE(logQuery_simple, FxYdBaseTask)
{
    tydt.test_logQuery_simple(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_oneDoubleQuote, FxYdBaseTask)
{
    tydt.test_logQuery_oneDoubleQuote(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_oneSingleQuote, FxYdBaseTask)
{
    tydt.test_logQuery_oneSingleQuote(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_combinedQuotes1, FxYdBaseTask)
{
    tydt.test_logQuery_combinedQuotes1(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_combinedQuotes2, FxYdBaseTask)
{
    tydt.test_logQuery_combinedQuotes2(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_sqlInjection1, FxYdBaseTask)
{
    tydt.test_logQuery_sqlInjection1(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_sqlInjection2, FxYdBaseTask)
{
    tydt.test_logQuery_sqlInjection2(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_specialchars, FxYdBaseTask)
{
    tydt.test_logQuery_specialchars(query);
}

BOOST_FIXTURE_TEST_CASE(log_sqlInjection1, FxYdBaseTask)
{
    tydt.test_log_sqlInjection1(query);
}

BOOST_FIXTURE_TEST_CASE(log_sqlInjection2, FxYdBaseTask)
{
    tydt.test_log_sqlInjection2(query);
}

BOOST_FIXTURE_TEST_CASE(log_specialchars, FxYdBaseTask)
{
    tydt.test_log_specialchars(query);
}

BOOST_FIXTURE_TEST_CASE(logQuery_XSS1, FxYdBaseTask)
{
    tydt.test_logQuery_XSS1(query);
}

BOOST_FIXTURE_TEST_CASE(log_XSS1, FxYdBaseTask)
{
    tydt.test_log_XSS1(query);
}

BOOST_FIXTURE_TEST_CASE(storePhrase_simple, FxYdBaseTask)
{
    BOOST_REQUIRE_NO_THROW(tydt.test_storePhrase_simple(conn));
}

BOOST_FIXTURE_TEST_CASE(countFreePhrasesSlots_used3expected20, FxYdBaseTask)
{
    BOOST_REQUIRE_NO_THROW(tydt.test_countFreePhrasesSlots_used3expected20());
}

BOOST_FIXTURE_TEST_CASE(countFreePhrasesSlots_used0expected50, FxYdBaseTask)
{
    BOOST_REQUIRE_NO_THROW(tydt.test_countFreePhrasesSlots_used0expected50());
}

BOOST_FIXTURE_TEST_CASE(countFreePhrasesSlots_used5expected0, FxYdBaseTask)
{
    BOOST_REQUIRE_NO_THROW(tydt.test_countFreePhrasesSlots_used5expected0());
}

BOOST_FIXTURE_TEST_CASE(countFreePhrasesSlots_used6expected0, FxYdBaseTask)
{
    BOOST_REQUIRE_NO_THROW(tydt.test_countFreePhrasesSlots_used6expected0());
}

BOOST_FIXTURE_TEST_CASE(storeReports, FxYdBaseTask)
{
    tydt.test_storeReports(conn);
}
