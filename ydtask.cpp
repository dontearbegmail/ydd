#include "ydtask.h"
#include "general.h"
#include "logresult.h"

namespace ydd
{
    YdTask::YdTask(DbConn& dbc, DbConn::UserIdType userId, DbConn::TaskIdType taskId) :
	dbc_(dbc),
	userId_(userId),
	taskId_(taskId)
    {
    }

    /* Doest not reset query! */
    void YdTask::logQuery(mysqlpp::Query& query, LogLevel level, const char* message, 
	    std::string* os)
    {
	/* set @p = 0;
	 * call sp_add_log_record(taskid, level, "test", html_to_log, @p);
	 * select @p */
	query << "SET @p = 0;" << 
	    "CALL sp_add_log_record(%0, %1, %2q, NULL, @p);" << 
	    "SELECT @p;";
	query.parse();
	std::string s = query.str((mysqlpp::sql_int_unsigned) taskId_,
		(mysqlpp::sql_tinyint_unsigned) level,
		(mysqlpp::sql_varchar) message);
	/* All this slow shit is needed for unit tests */
	query.reset();
	query << s;
	if(os != NULL)
	    *os = s;
    }

    void YdTask::log(LogLevel level, const char* message)
    {
	using namespace mysqlpp;
	Connection& con = dbc_.get();
	try
	{
	    Query query = con.query();
	    logQuery(query, level, message);
	    std::vector<LogResult> res;
	    query.storein(res);
	    bool fail = false;
	    if(res.size() == 0)
	    {
		msyslog(LOG_ERR, "Didn't get return value for sp_add_log_record.");
		fail = true;
	    }
	    else
	    {
		int result = res[0].result;
		if(result != 1)
		{
		    msyslog(LOG_ERR, "sp_add_log_record should have returned 1, "
			    "but it returned %d", result);
		    fail = true;
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
}
