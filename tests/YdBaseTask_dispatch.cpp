#include "../ydbasetask.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../general.h"
#include <algorithm>
#include <functional>
#include <random>
#include <time.h>
#include "YdBaseTask_test_tasks_phrases.h"
#include "../ydphrase.h"

using namespace ydd;

std::default_random_engine generator;
std::uniform_int_distribution<unsigned long> distribution(0, 50);

class DispatchYdBaseTask : public YdBaseTask
{
    public:
	bool testingErrors_;
	size_t curReportIdx_;
	struct ReportToState {size_t reportIdx; ydd::GeneralState state;};
	std::vector<ReportToState> reportsToState_;
	DispatchYdBaseTask(boost::asio::io_service& ios, DbConn& dbc, 
		DbConn::UserIdType userId, DbConn::TaskIdType taskId) : 
	    YdBaseTask(ios, dbc, userId, taskId)
	{
	    testingErrors_ = false;
	    curReportIdx_ = 0;
	    PhrasesSet::setMainTaskId(taskId_);
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
	    state_ = inProgress;
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
	    query << "UPDATE `tasks` SET `finished` = NULL, `finishedState` = NULL;";
	    query.execute();
	    flushQuery(query);
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

	void resetDb(mysqlpp::Query& query)
	{
	    resetPhrasesKeywords(query);
	    resetTasks(query);
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
		BOOST_ERROR("Ouch...");
	    }
	    report.isFinished = true;
	    report.state = GeneralState::ok;
	    if(testingErrors_)
	    {
		GeneralState s;
		if(std::any_of(reportsToState_.begin(), reportsToState_.end(),
			    [&s, this](ReportToState r) 
			    {
				if(r.reportIdx == this->curReportIdx_)
				{
				    s = r.state;
				    return true;
				}
				return false;
			    }))
		{
		    report.state = s;
		}
	    }
	    ++curReportIdx_;
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	}

	struct PhrasesSet
	{
	    static unsigned long mainTaskId;
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
		    if((finished == 0) && (taskId == mainTaskId))
		    {
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
	    }

	    static void setMainTaskId(unsigned long id)
	    {
		mainTaskId = id;
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

	bool tasksPhrasesOk(mysqlpp::Query& query)
	{
	    using namespace std;
	    bool ok = false;
	    vector<YdBaseTask_test_tasks_phrases> dbres, expected;
	    query << "SELECT `taskid`, `finished`, `phrase` FROM `tasks_phrases`;";
	    query.storein(dbres);

	    for(auto ps_it = phrasesSets_.begin(); 
		    ps_it != phrasesSets_.end(); ++ps_it)
	    {
		signed char finished = 1;
		if(ps_it->taskId != taskId_)
		    finished = ps_it->finished;

		for(auto ph_it = ps_it->phrases.begin();
			ph_it != ps_it->phrases.end(); ++ph_it)
		{
		    expected.push_back({ps_it->taskId, finished, ph_it->value});
		}
	    }
	    ok = expected == dbres;

	    return ok;
	}

	void test_simple(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);
	    phrasesSets_.push_back({1, 7, 0, 1, 1});
	    pushCallback(query, phrasesSets_.back());
	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_simple_2reports(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);
	    phrasesSets_.push_back({1, 11, 0, 1, 1});
	    pushCallback(query, phrasesSets_.back());
	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_simple_with_finished(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);
	    phrasesSets_.push_back({1, 4, 1, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());
	    phrasesSets_.push_back({5, 9, 0, taskId_, 5});
	    pushCallback(query, phrasesSets_.back());
	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_simple_with_finished_2reports(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);
	    phrasesSets_.push_back({1, 14, 1, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());
	    phrasesSets_.push_back({15, 19, 0, taskId_, 15});
	    pushCallback(query, phrasesSets_.back());
	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_simple_with_finished_3reports(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);

	    phrasesSets_.push_back({1, 14, 1, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());

	    unsigned long newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 10, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_simple_59phrases(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);

	    phrasesSets_.push_back({1, 59, 0, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());
	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_simple_167phrases(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);

	    phrasesSets_.push_back({1, 167, 0, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());
	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_simple_with_finished_250phrases(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);

	    phrasesSets_.push_back({1, 113, 1, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());

	    unsigned long newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 136, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_finished_in_the_middle_313phrases(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);

	    phrasesSets_.push_back({1, 49, 1, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());

	    unsigned long newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 100, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 161, 1, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_finished_in_the_middle_and_end_400phrases(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);

	    phrasesSets_.push_back({1, 9, 0, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());

	    unsigned long newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 39, 1, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 100, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 247, 1, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart, 1, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_2tasks_simple(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);

	    phrasesSets_.push_back({1, 5, 0, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());

	    unsigned long newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 5, 0, taskId_ + 1, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_2tasks_big(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);

	    phrasesSets_.push_back({1, 151, 0, taskId_, 1});
	    pushCallback(query, phrasesSets_.back());

	    unsigned long newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 197, 0, taskId_ + 1, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_3tasks_main_in_the_middle(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);
	    taskId_ = 5;

	    DispatchYdBaseTask::PhrasesSet::mainTaskId = taskId_;

	    phrasesSets_.push_back({1, 49, 0, 2, 1});
	    pushCallback(query, phrasesSets_.back());

	    unsigned long newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 100, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 49, 0, 7, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}

	void test_5tasks_main_split_with_finished(mysqlpp::Query& query)
	{
	    dbc_.switchUserDb(userId_);
	    resetDb(query);
	    taskId_ = 5;

	    DispatchYdBaseTask::PhrasesSet::mainTaskId = taskId_;

	    // 1..19 => task = 3 non-finished 
	    phrasesSets_.push_back({1, 19, 0, 3, 1});
	    pushCallback(query, phrasesSets_.back());

	    // 20..49 => task = 5 non-finished 
	    unsigned long newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 29, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 50..51 => task = 5 finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 1, 1, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 52..166 => task = 7 non-finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 114, 0, 7, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 167..199 => task = 3 finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 32, 1, 3, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 200..250 => task = 5 non-finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 50, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 251..351 => task = 1 finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 100, 1, 1, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 352..358 => task = 7 finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 6, 1, 7, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 359..359 => task = 5 non-finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 360..401 => task = 1 non-finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 41, 0, 1, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 402..410 => task = 6 non-finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 8, 0, 6, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 411..550 => task = 5 finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart + 139, 1, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 551..551 => task = 6 finished 
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart, 1, 6, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    // 552..552 => task = 5 non-finished
	    newStart = phrasesSets_.back().end + 1;
	    phrasesSets_.push_back({newStart, newStart, 0, taskId_, 
		    phrasesSets_.back().lastId});
	    pushCallback(query, phrasesSets_.back());

	    query.exec();
	    flushQuery(query);
	    ios_.post(std::bind(&DispatchYdBaseTask::dispatch, this));
	    ios_.run();
	    BOOST_REQUIRE(phrasesKeywordsOk(query));
	    BOOST_REQUIRE(tasksPhrasesOk(query));
	}
};

unsigned long DispatchYdBaseTask::PhrasesSet::mainTaskId = 0;

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
	DispatchYdBaseTask::PhrasesSet::mainTaskId = taskId;
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

BOOST_FIXTURE_TEST_CASE(test_simple_2reports, FxDYdBaseTask)
{
    tydt.test_simple_2reports(query);
}

BOOST_FIXTURE_TEST_CASE(test_simple_with_finished, FxDYdBaseTask)
{
    tydt.test_simple_with_finished(query);
}

BOOST_FIXTURE_TEST_CASE(test_simple_with_finished_2reports, FxDYdBaseTask)
{
    tydt.test_simple_with_finished_2reports(query);
}

BOOST_FIXTURE_TEST_CASE(test_simple_with_finished_3reports, FxDYdBaseTask)
{
    tydt.test_simple_with_finished_3reports(query);
}

BOOST_FIXTURE_TEST_CASE(test_simple_59phrases, FxDYdBaseTask)
{
    tydt.test_simple_59phrases(query);
}

BOOST_FIXTURE_TEST_CASE(test_simple_167phrases, FxDYdBaseTask)
{
    tydt.test_simple_167phrases(query);
}

BOOST_FIXTURE_TEST_CASE(test_simple_with_finished_250phrases, FxDYdBaseTask)
{
    tydt.test_simple_with_finished_250phrases(query);
}

BOOST_FIXTURE_TEST_CASE(test_finished_in_the_middle_313phrases, FxDYdBaseTask)
{
    tydt.test_finished_in_the_middle_313phrases(query);
}

BOOST_FIXTURE_TEST_CASE(test_finished_in_the_middle_and_end_400phrases, FxDYdBaseTask)
{
    tydt.test_finished_in_the_middle_and_end_400phrases(query);
}

BOOST_FIXTURE_TEST_CASE(test_2tasks_simple, FxDYdBaseTask)
{
    tydt.test_2tasks_simple(query);
}

BOOST_FIXTURE_TEST_CASE(test_2tasks_big, FxDYdBaseTask)
{
    tydt.test_2tasks_big(query);
}

BOOST_FIXTURE_TEST_CASE(test_3tasks_main_in_the_middle, FxDYdBaseTask)
{
    tydt.test_3tasks_main_in_the_middle(query);
}

//test_5tasks_main_split_with_finished
BOOST_FIXTURE_TEST_CASE(test_5tasks_main_split_with_finished, FxDYdBaseTask)
{
    tydt.test_5tasks_main_split_with_finished(query);
}
