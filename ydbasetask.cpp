#include "general.h"

#define EXPAND_SQLKEYWORD_SSQLS_STATICS
#include "sqlkeyword.h"

#include "ydbasetask.h"

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
	for(std::vector<YdReport>::iterator it = reports_.begin();
		it != reports_.end(); ++it)
	{
	    if(it->isFinished)
	    {
		// The report is all done: put it into the db and remove from reports_ vector
	    }
	}
    }

    void YdBaseTask::storeReport(YdReport& report)
    {
    }

    /* Don't forget that dbc_.switchUserDb(userId_) should be called before !!! */
    void YdBaseTask::storePhrase(YdPhrase& phrase, mysqlpp::Connection& con)
    {
	using namespace mysqlpp;
	try
	{
	    Query query = con.query();
	    {
		Transaction trans(con);
		query.insert(phrase.keywords.begin(), phrase.keywords.end());
		query.execute();
		/* UPDATE `tasks_phrases` SET `finished`= 1 WHERE `id` = phrase.id */
		query << 
		    "UPDATE `tasks_phrases` SET `finished`= 1 WHERE `id` = " <<
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
}
