#include "../ydbasetask.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../general.h"
#include <algorithm>
#include <functional>
#include <random>

using namespace ydd;

class DispatchYdBaseTask : public YdBaseTask
{
    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned long> distribution;
    public:
	DispatchYdBaseTask(boost::asio::io_service& ios, DbConn& dbc, 
		DbConn::UserIdType userId, DbConn::TaskIdType taskId) : 
	    YdBaseTask(ios, dbc, userId, taskId),
	    distribution(1, 50)
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

	virtual ~DispatchYdBaseTask()
	{
	}

	void flushQuery(mysqlpp::Query& query)
	{ 
	    while(query.more_results())
		query.store_next();
	}

	void resetTasks(mysqlpp::Query& query)
	{
	    dbc_.switchDbTasks();
	    query << "UPDATE `tasks` SET `finished` = NULL;";
	    query.execute();
	    dbc_.switchUserDb(userId_);
	}

	void resetPhrasesKeywords(mysqlpp::Query& query)
	{
	    query << 
		"DELETE FROM `phrases_keywords`;"
		"ALTER TABLE `phrases_keywords` AUTO_INCREMENT = 1;"
		"DELETE FROM `tasks_phrases`;"
		"ALTER TABLE `tasks_phrases` AUTO_INCREMENT = 1";
	    query.exec();
	    flushQuery(query);
	}

	virtual void startReportProcessing(YdReport& report)
	{
	}

	void generatePhraseKeywords(YdPhrase& phrase)
	{
	    unsigned long size = distribution(generator);
	    std::string kw;
	    for(unsigned long i = 0; i < size; i++)
	    {
		kw = phrase.value;
		kw.push_back('_');
		kw.append(std::to_string(i));
		phrase.keywords.push_back({phrase.id, kw, distribution(generator)});
	    }
	}

	struct PhrasesSet
	{
	    unsigned long start, end;
	    int finished;
	    unsigned long taskId;
	    unsigned long lastId;
	    std::vector<YdPhrase> phrases;
	};

	void generatePhrases(PhrasesSet& ps)
	{
	    std::string v;
	    for(unsigned long i = ps.start; i <= ps.end; i++)
	    {
		v = "phrase";
		v.append(std::to_string(i));
		ps.phrases.push_back({ps.lastId++, v});
		generatePhraseKeywords(ps.phrases.back());
	    }
	}

	std::vector<PhrasesSet> phrasesSets_;
	void pushCallback(mysqlpp::Query& query, PhrasesSet& ps)
	{
	    query << "CALL `sp_fill_test_tasks_phrases_set`(" << ps.start << ", " <<
		ps.end << ", " << ps.finished << ", " << ps.taskId << ");";
	}

	void test_simple(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetPhrasesKeywords(query);
	    phrasesSets_.push_back({1, 7, false, 1, 0});
	    pushCallback(query, phrasesSets_.back());
	    query.exec();
	    flushQuery(query);
	}
};

struct FxDYdBaseTask
{
    FxDYdBaseTask() :
	ios(),
	dbc(),
	userId(50),
	taskId(1),
	tydt(ios, dbc, userId, taskId),
	conn(dbc.get()),
	query(conn.query())
    {
    }

    boost::asio::io_service ios;
    DbConn dbc;
    DbConn::UserIdType userId;
    DbConn::TaskIdType taskId;
    DispatchYdBaseTask tydt;
    mysqlpp::Connection& conn;
    mysqlpp::Query query;
};

