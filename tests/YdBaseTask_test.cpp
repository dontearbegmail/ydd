#include "../ydbasetask.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../general.h"
#include "YdBaseTask_test_tasks_phrases.h"
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <functional>

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

	std::vector<bool> __getReportsStates()
	{
	    std::vector<bool> ret;
	    for(std::vector<YdReport>::iterator it_rep = reports_.begin();
		    it_rep != reports_.end(); ++it_rep)
	    {
		ret.push_back(it_rep->isFinished);
	    }
	    return ret;
	}

	void test_storeReports_simple(mysqlpp::Connection& conn)
	{
	    dbc_.switchUserDb(userId_);
	    reports_.push_back({{}, true});
	    BOOST_REQUIRE_NO_THROW(storeReports(conn));
	    BOOST_REQUIRE(reports_.size() == 0);
	}

	void test_storeReports_noreports(mysqlpp::Connection& conn)
	{
	    dbc_.switchUserDb(userId_);
	    BOOST_REQUIRE_NO_THROW(storeReports(conn));
	    BOOST_REQUIRE(reports_.size() == 0);
	}

	bool phrases_predicate(const YdPhrase& p1, const YdPhrase& p2)
	{
	    bool keywordsEqual = p1.keywords == p2.keywords;
	    return ((p1.id == p2.id) && (p1.value == p2.value) && keywordsEqual);
	}

	bool areEqualPV(std::vector<YdPhrase> pv, YdReport r)
	{
	    using namespace std;
	    if(r.phrases.size() != pv.size())
		return false;
	    return std::equal(r.phrases.begin(), r.phrases.end(), pv.begin(), 
		    bind(&TestYdBaseTask::phrases_predicate, this, placeholders::_1, placeholders::_2));
	}

	void test_storeReports_none(mysqlpp::Connection& conn)
	{
	    std::vector<YdPhrase> p0 = {{132, "phrase0_1"}, {173, "phrase0_2"}};
	    std::vector<YdPhrase> p1 = {{40, "phrase1_1"}, {7, "phrase1_2"}, {33, "phrase1_3"}};
	    std::vector<YdPhrase> p2 = {{200, "phrase2_1"}};
	    std::vector<YdPhrase> p3 = {{400, "phrase3_1"}, {301, "phrase3_2"}};
	    std::vector<YdPhrase> p4 = {{201, "phrase4_1"}, {3, "phrase4_2"}, {44, "phrase4_3"}, {170, "phrase4_4"}};

	    reports_.push_back({p0, false});
	    reports_.push_back({p1, false});
	    reports_.push_back({p2, false});
	    reports_.push_back({p3, false});
	    reports_.push_back({p4, false});

	    dbc_.switchUserDb(userId_);
	    BOOST_REQUIRE_NO_THROW(storeReports(conn));
	    BOOST_REQUIRE(reports_.size() == 5);
	    BOOST_REQUIRE(areEqualPV(p0, reports_[0]));
	    BOOST_REQUIRE(areEqualPV(p1, reports_[1]));
	    BOOST_REQUIRE(areEqualPV(p2, reports_[2]));
	    BOOST_REQUIRE(areEqualPV(p3, reports_[3]));
	    BOOST_REQUIRE(areEqualPV(p4, reports_[4]));
	}

	void test_storeReports_1and4(mysqlpp::Connection& conn)
	{
	    std::vector<YdPhrase> p0 = {{132, "phrase0_1"}, {173, "phrase0_2"}};
	    std::vector<YdPhrase> p1 = {{40, "phrase1_1"}, {7, "phrase1_2"}, {33, "phrase1_3"}};
	    std::vector<YdPhrase> p2 = {{200, "phrase2_1"}};
	    std::vector<YdPhrase> p3 = {{400, "phrase3_1"}, {301, "phrase3_2"}};
	    std::vector<YdPhrase> p4 = {{201, "phrase4_1"}, {3, "phrase4_2"}, {44, "phrase4_3"}, {170, "phrase4_4"}};

	    reports_.push_back({p0, false});
	    reports_.push_back({p1, true});
	    reports_.push_back({p2, false});
	    reports_.push_back({p3, false});
	    reports_.push_back({p4, true});

	    dbc_.switchUserDb(userId_);
	    BOOST_REQUIRE_NO_THROW(storeReports(conn));
	    BOOST_REQUIRE(reports_.size() == 3);
	    BOOST_REQUIRE(areEqualPV(p0, reports_[0]));
	    BOOST_REQUIRE(areEqualPV(p2, reports_[1]));
	    BOOST_REQUIRE(areEqualPV(p3, reports_[2]));
	}

	void test_storeReports_all(mysqlpp::Connection& conn)
	{
	    std::vector<YdPhrase> p0 = {{132, "phrase0_1"}, {173, "phrase0_2"}};
	    std::vector<YdPhrase> p1 = {{40, "phrase1_1"}, {7, "phrase1_2"}, {33, "phrase1_3"}};
	    std::vector<YdPhrase> p2 = {{200, "phrase2_1"}};
	    std::vector<YdPhrase> p3 = {{400, "phrase3_1"}, {301, "phrase3_2"}};
	    std::vector<YdPhrase> p4 = {{201, "phrase4_1"}, {3, "phrase4_2"}, {44, "phrase4_3"}, {170, "phrase4_4"}};

	    reports_.push_back({p0, true});
	    reports_.push_back({p1, true});
	    reports_.push_back({p2, true});
	    reports_.push_back({p3, true});
	    reports_.push_back({p4, true});

	    dbc_.switchUserDb(userId_);
	    BOOST_REQUIRE_NO_THROW(storeReports(conn));
	    BOOST_REQUIRE(reports_.size() == 0);
	}

	void test_storeReports_last(mysqlpp::Connection& conn)
	{
	    std::vector<YdPhrase> p0 = {{132, "phrase0_1"}, {173, "phrase0_2"}};
	    std::vector<YdPhrase> p1 = {{40, "phrase1_1"}, {7, "phrase1_2"}, {33, "phrase1_3"}};
	    std::vector<YdPhrase> p2 = {{200, "phrase2_1"}};
	    std::vector<YdPhrase> p3 = {{400, "phrase3_1"}, {301, "phrase3_2"}};
	    std::vector<YdPhrase> p4 = {{201, "phrase4_1"}, {3, "phrase4_2"}, {44, "phrase4_3"}, {170, "phrase4_4"}};

	    reports_.push_back({p0, false});
	    reports_.push_back({p1, false});
	    reports_.push_back({p2, false});
	    reports_.push_back({p3, false});
	    reports_.push_back({p4, true});

	    dbc_.switchUserDb(userId_);
	    BOOST_REQUIRE_NO_THROW(storeReports(conn));
	    BOOST_REQUIRE(reports_.size() == 4);
	    BOOST_REQUIRE(areEqualPV(p0, reports_[0]));
	    BOOST_REQUIRE(areEqualPV(p1, reports_[1]));
	    BOOST_REQUIRE(areEqualPV(p2, reports_[2]));
	    BOOST_REQUIRE(areEqualPV(p3, reports_[3]));
	}

	bool reportCheck(YdReport& report, unsigned long start, unsigned long finish, bool isFinished)
	{
	    if(report.isFinished != isFinished)
		return false;
	    if(report.phrases.size() != (finish - start + 1))
		return false;
	    unsigned long id = start;
	    std::string v = "phrase", t;
	    for(std::vector<YdPhrase>::iterator it = report.phrases.begin();
		    it != report.phrases.end(); ++it, ++id)
	    {
		t = v;
		t.append(std::to_string(id));
		if(it->value != t)
		    return false;
	    }
	    return true;
	}

	void test_getPhrasesFromDb_simple(mysqlpp::Connection& conn)
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

	    size_t n = countFreePhrasesSlots();
	    BOOST_REQUIRE_EQUAL(n, 50);
	    getPhrasesFromDb(n, conn);
	    BOOST_REQUIRE_EQUAL(reports_.size(), 1);
	    BOOST_REQUIRE(reportCheck(reports_[0], 1, 7, false));
	}

	void test_getPhrasesFromDb_17phrases_1freereport(mysqlpp::Connection& conn)
	{
	    mysqlpp::Query query = conn.query();
	    dbc_.switchUserDb(userId_);
	    flush_storePhrase(query);
	    query << 
		"CALL `sp_fill_test_tasks_phrases_set`(1, 7, 0, 1);" <<
		"CALL `sp_fill_test_tasks_phrases_set`(21, 37, 0, 2);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(71, 72, 0, 7);";
	    query.exec();
	    flushQuery(query);

	    taskId_ = 2;
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    size_t n = countFreePhrasesSlots();
	    BOOST_REQUIRE_EQUAL(n, 10);
	    getPhrasesFromDb(n, conn);
	    BOOST_REQUIRE_EQUAL(reports_.size(), 5);
	    BOOST_REQUIRE(reportCheck(reports_[4], 21, 30, false));
	}

	void test_getPhrasesFromDb_13phrases_2freereports(mysqlpp::Connection& conn)
	{
	    mysqlpp::Query query = conn.query();
	    dbc_.switchUserDb(userId_);
	    flush_storePhrase(query);
	    query << 
		"CALL `sp_fill_test_tasks_phrases_set`(1, 7, 0, 1);" <<
		"CALL `sp_fill_test_tasks_phrases_set`(21, 37, 0, 2);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(71, 83, 0, 7);";
	    query.exec();
	    flushQuery(query);

	    taskId_ = 7;
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    size_t n = countFreePhrasesSlots();
	    BOOST_REQUIRE_EQUAL(n, 20);
	    getPhrasesFromDb(n, conn);
	    BOOST_REQUIRE_EQUAL(reports_.size(), 5);
	    BOOST_REQUIRE(reportCheck(reports_[3], 71, 80, false));
	    BOOST_REQUIRE(reportCheck(reports_[4], 81, 83, false));
	}
	
	void test_getPhrasesFromDb_10phrases_1freereport_readyPhrasesPresent(mysqlpp::Connection& conn)
	{
	    mysqlpp::Query query = conn.query();
	    dbc_.switchUserDb(userId_);
	    flush_storePhrase(query);
	    query << 
		"CALL `sp_fill_test_tasks_phrases_set`(1, 7, 0, 1);" <<
		"CALL `sp_fill_test_tasks_phrases_set`(8, 12, 1, 1);" <<
		"CALL `sp_fill_test_tasks_phrases_set`(21, 37, 1, 2);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(38, 47, 0, 2);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(71, 83, 1, 7);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(84, 110, 1, 7);";
	    query.exec();
	    flushQuery(query);

	    taskId_ = 2;
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    reports_.push_back({{}, false});
	    size_t n = countFreePhrasesSlots();
	    BOOST_REQUIRE_EQUAL(n, 10);
	    getPhrasesFromDb(n, conn);
	    BOOST_REQUIRE_EQUAL(reports_.size(), 5);
	    BOOST_REQUIRE(reportCheck(reports_[4], 38, 47, false));
	}

	void test_getPhrasesFromDb_73phrases_5freereports_readyPhrasesPresent(mysqlpp::Connection& conn)
	{
	    mysqlpp::Query query = conn.query();
	    dbc_.switchUserDb(userId_);
	    flush_storePhrase(query);
	    query << 
		"CALL `sp_fill_test_tasks_phrases_set`(1, 7, 0, 1);" <<
		"CALL `sp_fill_test_tasks_phrases_set`(8, 12, 1, 1);" <<
		"CALL `sp_fill_test_tasks_phrases_set`(31, 43, 1, 2);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(44, 116, 0, 2);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(171, 183, 1, 7);" << 
		"CALL `sp_fill_test_tasks_phrases_set`(184, 210, 0, 7);";
	    query.exec();
	    flushQuery(query);

	    taskId_ = 2;
	    size_t n = countFreePhrasesSlots();
	    BOOST_REQUIRE_EQUAL(n, 50);
	    getPhrasesFromDb(n, conn);
	    BOOST_REQUIRE_EQUAL(reports_.size(), 5);
	    BOOST_REQUIRE(reportCheck(reports_[0], 44, 53, false));
	    BOOST_REQUIRE(reportCheck(reports_[1], 54, 63, false));
	    BOOST_REQUIRE(reportCheck(reports_[2], 64, 73, false));
	    BOOST_REQUIRE(reportCheck(reports_[3], 74, 83, false));
	    BOOST_REQUIRE(reportCheck(reports_[4], 84, 93, false));
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

BOOST_FIXTURE_TEST_CASE(storeReports_simple, FxYdBaseTask)
{
    tydt.test_storeReports_simple(conn);
}

BOOST_FIXTURE_TEST_CASE(storeReports_noreports, FxYdBaseTask)
{
    tydt.test_storeReports_noreports(conn);
}

BOOST_FIXTURE_TEST_CASE(storeReports_1and4, FxYdBaseTask)
{
    tydt.test_storeReports_1and4(conn);
}

BOOST_FIXTURE_TEST_CASE(storeReports_none, FxYdBaseTask)
{
    tydt.test_storeReports_none(conn);
}

BOOST_FIXTURE_TEST_CASE(storeReports_all, FxYdBaseTask)
{
    tydt.test_storeReports_all(conn);
}

BOOST_FIXTURE_TEST_CASE(storeReports_last, FxYdBaseTask)
{
    tydt.test_storeReports_last(conn);
}

BOOST_FIXTURE_TEST_CASE(getPhrasesFromDb_simple, FxYdBaseTask)
{
    tydt.test_getPhrasesFromDb_simple(conn);
}

BOOST_FIXTURE_TEST_CASE(test_getPhrasesFromDb_17phrases_1freereport, FxYdBaseTask)
{
    tydt.test_getPhrasesFromDb_17phrases_1freereport(conn);
}

BOOST_FIXTURE_TEST_CASE(test_getPhrasesFromDb_13phrases_2freereports, FxYdBaseTask)
{
    tydt.test_getPhrasesFromDb_13phrases_2freereports(conn);
}

BOOST_FIXTURE_TEST_CASE(test_getPhrasesFromDb_73phrases_5freereports_readyPhrasesPresent, FxYdBaseTask)
{
    tydt.test_getPhrasesFromDb_73phrases_5freereports_readyPhrasesPresent(conn);
}

