#include "ydprocesslog.h"
#include "general.h"

namespace ydd
{
    YdProcessLog::YdProcessLog(YdTask::UserIdType userId, YdTask::TaskIdType taskId)
    {
	prefix_ = "@";
	prefix_.append(std::to_string(userId));
	prefix_.append("_");
	prefix_.append(std::to_string(taskId));
	prefix_.append("@");
    }

    void YdProcessLog::log(int priority, std::string& message)
    {
	msyslog(priority, "%s %s", prefix_.c_str(), message.c_str());
    }
}
