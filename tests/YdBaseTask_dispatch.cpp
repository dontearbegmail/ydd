#include "../ydbasetask.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../general.h"
#include <algorithm>
#include <functional>
#include <random>
#include <time.h>

using namespace ydd;

std::default_random_engine generator;
std::uniform_int_distribution<unsigned long> distribution(0, 50);

class DispatchYdBaseTask : public YdBaseTask
{
    public:
	DispatchYdBaseTask(boost::asio::io_service& ios, DbConn& dbc, 
		DbConn::UserIdType userId, DbConn::TaskIdType taskId) : 
	    YdBaseTask(ios, dbc, userId, taskId)
	{
	    generator.seed(time(NULL));
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

	YdPhrase* findPhrase(unsigned long id)
	{
	    YdPhrase* p = nullptr;
	    std::vector<YdPhrase>::iterator pit;
	    for(std::vector<PhrasesSet>::iterator it = phrasesSets_.begin();
		    (it != phrasesSets_.end()) && !p; ++it)
	    {
		pit = std::find_if(it->phrases.begin(), it->phrases.end(),
			[id](YdPhrase& ph) {return ph.id == id;});
		if(pit != it->phrases.end())
		{
		    p = &(*pit);
		}
	    }
	    return p;
	}

	virtual void startReportProcessing(YdReport& report)
	{
	    YdPhrase* found;
	    for(std::vector<YdPhrase>::iterator it = report.phrases.begin();
		    (it != report.phrases.end()) && found; ++it)
	    {
		found = findPhrase(it->id);
		if(found)
		{
		    it->keywords = found->keywords;
		}
	    }
	    if(!found)
	    {
		// put here error processing for some kind of YD error
	    }
	    report.isFinished = true;
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	}

	struct PhrasesSet
	{
	    PhrasesSet(unsigned long _start, unsigned long _end, int _finished,
		    unsigned long _taskId, unsigned long _lastId) :
		start(_start), end(_end), finished(_finished), taskId(_taskId), 
		lastId(_lastId)
	    {
		std::string v, kw;
		unsigned long size;
		for(unsigned long i = start; i <= end; i++)
		{
		    v = "phrase";
		    v.append(std::to_string(i));
		    phrases.push_back({lastId++, v});
		    YdPhrase& last = phrases.back();

		    size = distribution(generator);
		    for(unsigned long j = 0; j < size; j++)
		    {
			kw = last.value;
			kw.push_back('_');
			kw.append(std::to_string(j));
			last.keywords.push_back({last.id, kw, distribution(generator)});
		    }
		}
	    }
	    unsigned long start, end;
	    int finished;
	    unsigned long taskId;
	    unsigned long lastId;
	    std::vector<YdPhrase> phrases;
	};

	std::vector<PhrasesSet> phrasesSets_;

	void pushCallback(mysqlpp::Query& query, PhrasesSet& ps)
	{
	    query << "CALL `sp_fill_test_tasks_phrases_set`(" << ps.start << ", " <<
		ps.end << ", " << ps.finished << ", " << ps.taskId << ");";
	}

	void dispatch()
	{
	    YdBaseTask::dispatch();
	}

	void test_findPhrase()
	{
	    phrasesSets_.push_back({1, 10, false, 1, 1});
	    YdPhrase* f;
	    BOOST_REQUIRE(f = findPhrase(1));
	    BOOST_REQUIRE(f == &(phrasesSets_[0].phrases[0]));
	}

	bool phrasesKeywordsOk(mysqlpp::Query& query)
	{
	    using namespace std;
	    bool ok = false;
	    vector<phrases_keywords> dbres;
	    query << "SELECT `phraseid`, `keyword`, `shows` FROM `phrases_keywords`;";
	    query.storein(dbres);

	    size_t numdb = dbres.size();
	    size_t headdb = 0;
	    bool mism = false; // mismatch
	    bool oor = false; // out of range
	    for(auto ps_it = phrasesSets_.begin();
		    (ps_it != phrasesSets_.end()) && !mism && !oor; ++ps_it)
	    {
		for(auto ph_it = ps_it->phrases.begin();
			ph_it != ps_it->phrases.end(); ++ph_it)
		{
		    oor = (headdb + ph_it->keywords.size()) > numdb;
		    if(oor)
			break;
		    auto m = mismatch(ph_it->keywords.begin(), ph_it->keywords.end(),
			    dbres.begin() + headdb);
		    mism = m.first != ph_it->keywords.end();
		    if(mism)
			break;
		    headdb += ph_it->keywords.size();
		}
	    }
	    ok = !mism && !oor;

	    return ok;
	}

	void test_simple(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetPhrasesKeywords(query);
	    phrasesSets_.push_back({1, 7, false, 1, 1});
	    pushCallback(query, phrasesSets_.back());
	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	}
};

struct FxDYdBaseTask
{
    FxDYdBaseTask() :
	ios(),
	dbc(),
	userId(51),
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

BOOST_FIXTURE_TEST_CASE(test_simple, FxDYdBaseTask)
{
    tydt.test_simple(query);
}

BOOST_FIXTURE_TEST_CASE(test_findPhrase, FxDYdBaseTask)
{
    tydt.test_findPhrase();
}
