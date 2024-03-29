#include "general.h"

#define EXPAND_PHRASES_KEYWORDS_SSQLS_STATICS
#include "phrases_keywords.h"

#include "ydbasetask.h"
#include "ydremote.h"

namespace ydd
{
    YdBaseTask::YdBaseTask(boost::asio::io_service& ios, DbConn& dbc, 
	    DbConn::UserIdType userId, DbConn::TaskIdType taskId) :
	ios_(ios),
	dbc_(dbc),
	userId_(userId),
	taskId_(taskId),
	state_(ydd::GeneralState::init)
    {
	// ios.post(<delete old reports with callback to this->dispatch()>);
    }

    YdBaseTask::~YdBaseTask()
    {
    }

    /* Doest not reset query! */
    void YdBaseTask::logQuery(mysqlpp::Query& query, LogLevel level, const char* message, 
	    std::string* html, std::string* os)
    {
	/* call sp_add_log_record(taskid, level, "test", html_to_log, @ret);
	 * select @ret */
	query <<  
	    "CALL sp_add_log_record(" <<
	    taskId_ << ", " <<
	    level << ", " <<
	    mysqlpp::quote << message << ", ";
	bool nullHtml = true;
	if(html != NULL)
	{
	    if(!(*html).empty()) 
	    {
		nullHtml = false;
		query << mysqlpp::quote << *html;
	    }
	}
	if(nullHtml)
	{
	    query << "NULL";
	}
	query << ", @ret);SELECT @ret;";
	if(os != NULL)
	{
	    *os = query.str();
	}
    }

    void YdBaseTask::log(LogLevel level, const char* message, std::string* html)
    {
	using namespace mysqlpp;
	Connection& con = dbc_.get();
	try
	{
	    dbc_.switchUserDb(userId_);
	    Query query = con.query();
	    logQuery(query, level, message, html);
	    StoreQueryResult res = query.store();
	    while(query.more_results())
		res = query.store_next();

	    bool fail = false;
	    if(res.num_rows() == 0 || res.num_fields() == 0)
	    {
		msyslog(LOG_ERR, "Didn't get return value for sp_add_log_record.");
		fail = true;
	    }
	    else
	    {
		std::string fn = res.field_name(0);
		if(res.field_name(0) != "@ret")
		{
		    msyslog(LOG_ERR, "The value returned by sp_add_log_record should be "
			    "selected in @ret, but it's not present in the MySQL result");
		    fail = true;
		}
		else 
		{
		    int result = res[0][0];
		    if(result != 1)
		    {
			msyslog(LOG_ERR, "sp_add_log_record should have returned 1, "
				"but it returned %d", result);
			fail = true;
		    }
		}
	    }
	    if(fail)
	    {
		msyslog(LOG_WARNING, "Looks like sp_add_log_record failed, so the log message "
			"is posted here: level = %d, message = %s", level, message);
	    }
	}
	catch(const mysqlpp::Exception& e)
	{
	    state_ = errDatabase;
	    msyslog(LOG_ERR, "Got a MySQL exception: %s", e.what());
	    throw(e);
	}
    }

    void YdBaseTask::dispatch()
    {
	using namespace mysqlpp;
	bool needShutdown = false;
	dbc_.switchUserDb(userId_);
	Connection& conn = dbc_.get();
	storeReports(conn);
	if(state_ == inProgress)
	{
	    size_t phrasesToGet = countFreePhrasesSlots();
	    if(phrasesToGet == 0)
		return;
	    size_t dispatchedReports = reports_.size();
	    size_t newPhrasesCount = getPhrasesFromDb(phrasesToGet, conn);
	    /* All phrases processed successfully */
	    if(newPhrasesCount == 0 && dispatchedReports == 0)
	    {
		state_ = ok;
		needShutdown = true;
	    }
	    else
	    {
		/* Start processing new reports */
		for(size_t i = dispatchedReports; i < reports_.size(); i++)
		{
		    startReportProcessing(reports_[i]);
		}
	    }
	}
	/* One of the reports returned an error (detected in "this->storeReports, 
	 * and state_ is set there also) - shutdown gracefully */
	else
	{
	    if(state_ == errDatabase || state_ == errNetwork || state_ == yderrBadAuth)
	    {
		needShutdown = true;
	    }
	    else if(reports_.size() == 0)
	    {
		needShutdown = true;
	    }
	}
	if(needShutdown)
	{
	    /* Mark the task as completed in the DB, write state_ to DB */
	    setCompleted(conn);
	    /* ... and invoke the upper level callback for the task completion */
	    if(callback_)
		ios_.post(callback_);
	}
    }

    void YdBaseTask::startReportProcessing(YdReport& report)
    {
    }

    void YdBaseTask::setCompleted(mysqlpp::Connection& conn)
    {
	using namespace mysqlpp;
	dbc_.switchDbTasks();
	Query query = conn.query();
	try
	{
	    query << "UPDATE `tasks` SET `finished` = CURRENT_TIMESTAMP, "
		"`finishedState` = " << state_ <<
		" WHERE `id` = " << taskId_;
	    query.execute();
	}
	catch(mysqlpp::Exception& e)
	{
	    state_ = errDatabase;
	    msyslog(LOG_ERR, "Got mysqlpp::Exception: %s", e.what());
	    throw(e);
	}
	dbc_.switchUserDb(userId_);
    }

    /* Don't forget that dbc_.switchUserDb(userId_) should be called before !!! */
    size_t YdBaseTask::getPhrasesFromDb(size_t numPhrases, mysqlpp::Connection& conn)
    {
	using namespace mysqlpp;
	size_t totalPhrases = 0;
	if(numPhrases == 0)
	    return 0;
	try
	{
	    std::vector<YdPhrase>* phrases = NULL;
	    Row row;
	    unsigned long id;
	    std::string value;

	    Query query = conn.query();
	    query << 
		"SELECT `id`, `phrase` FROM `tasks_phrases` " 
		"WHERE `taskid` = " << taskId_ << " AND `finished` = 0 LIMIT " << numPhrases;
	    UseQueryResult res = query.use();
	    while(row = res.fetch_row())
	    {
		id = row[0];
		value.assign((const char*)row[1]);;
		if(phrases == NULL)
		{
		    reports_.push_back({{}, false});
		    phrases = &(reports_.back().phrases);
		}
		phrases->push_back({id, value});
		totalPhrases++;
		if(phrases->size() == YdRemote::PhrasesPerReport)
		{
		    phrases = NULL;
		}
	    }
	}
	catch(mysqlpp::Exception& e)
	{
	    state_ = errDatabase;
	    msyslog(LOG_ERR, "Got mysqlpp::Exception: %s", e.what());
	    throw(e);
	}
	return totalPhrases;
    }

    /* Don't forget that dbc_.switchUserDb(userId_) should be called before !!! */
    void YdBaseTask::storeReports(mysqlpp::Connection& conn)
    {
	for(std::vector<YdReport>::iterator it_rep = reports_.begin();
		it_rep != reports_.end(); )
	{
	    if(it_rep->isFinished)
	    {
		if(it_rep->state == GeneralState::ok)
		{
		    // TODO: neet tests when state_ != inProgress && it_rep->state == ok
		    if(state_ == GeneralState::inProgress)
		    {
			// The report is all done: put it into the db and remove from reports_
			for(std::vector<YdPhrase>::iterator it = it_rep->phrases.begin();
				it != it_rep->phrases.end(); ++it)
			{
			    storePhrase(*it, conn);
			}
		    }
		}
		else
		{
		    state_ = it_rep->state;
		}
		it_rep = reports_.erase(it_rep);
	    }
	    else
	    {
		++it_rep;
	    }
	}
    }

    /* Don't forget that dbc_.switchUserDb(userId_) should be called before !!! */
    void YdBaseTask::storePhrase(YdPhrase& phrase, mysqlpp::Connection& conn)
    {
	using namespace mysqlpp;
	try
	{
	    Query query = conn.query();
	    {
		Transaction trans(conn);
		if(!phrase.keywords.empty())
		{
		    query.insert(phrase.keywords.begin(), phrase.keywords.end());
		    query.execute();
		}
		/* UPDATE `tasks_phrases` SET `finished`= 1 WHERE `id` = phrase.id */
		query << 
		    "UPDATE `tasks_phrases` SET `finished` = 1 WHERE `id` = " <<
		    phrase.id;
		query.execute();
		trans.commit();
	    }
	}
	catch(mysqlpp::Exception& e)
	{
	    state_ = errDatabase;
	    msyslog(LOG_ERR, "Got mysqlpp::Exception: %s", e.what());
	    throw(e);
	}
    }

    size_t YdBaseTask::countFreePhrasesSlots()
    {
	size_t slots = 0;
	ssize_t freeReports = YdRemote::MaxReports - reports_.size();
	if(freeReports > 0)
	    slots = freeReports * YdRemote::PhrasesPerReport;
	return slots;
    }

}
