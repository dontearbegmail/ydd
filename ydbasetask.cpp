#include "general.h"

#define EXPAND_PHRASES_KEYWORDS_SSQLS_STATICS
#include "phrases_keywords.h"

#include "ydbasetask.h"
#include "ydremote.h"

namespace ydd
{
    YdBaseTask::YdBaseTask(DbConn& dbc, DbConn::UserIdType userId, DbConn::TaskIdType taskId) :
	dbc_(dbc),
	userId_(userId),
	taskId_(taskId)
    {
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
	    msyslog(LOG_ERR, "Got a MySQL exception: %s", e.what());
	    throw(e);
	}
    }

    void YdBaseTask::dispatch()
    {
	using namespace mysqlpp;
	dbc_.switchUserDb(userId_);
	Connection& conn = dbc_.get();
	storeReports(conn);
	size_t phrasesToGet = countFreePhrasesSlots();
	if(phrasesToGet > 0)
	{
	}
    }

    /* Don't forget that dbc_.switchUserDb(userId_) should be called before !!! */
    void YdBaseTask::getPhrasesFromDb(size_t numPhrases, mysqlpp::Connection& conn)
    {
	using namespace mysqlpp;
	if(numPhrases == 0)
	    return;
	try
	{
	    Query query = conn.query();
	    query << 
		"SELECT `id`, `phrase` FROM `tasks_phrases` " 
		"WHERE `taskid` = " << taskId_ << " AND `finished` = 0 LIMIT " << numPhrases;
	}
	catch(mysqlpp::Exception& e)
	{
	    msyslog(LOG_ERR, "Got mysqlpp::Exception: %s", e.what());
	    throw(e);
	}
    }

    /* Don't forget that dbc_.switchUserDb(userId_) should be called before !!! */
    void YdBaseTask::storeReports(mysqlpp::Connection& conn)
    {
	for(std::vector<YdReport>::iterator it_rep = reports_.begin();
		it_rep != reports_.end(); ++it_rep)
	{
	    if(it_rep->isFinished)
	    {
		// The report is all done: put it into the db and set available
		for(std::vector<YdPhrase>::iterator it = it_rep->phrases.begin();
			it != it_rep->phrases.end(); ++it)
		{
		    storePhrase(*it, conn);
		}
	    }
	    reports_.erase(it_rep);
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
