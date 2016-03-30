#ifndef YDPROCESSLOG_H
#define YDPROCESSLOG_H

#include "ydtask.h"
#include <string>
#include <boost/asio.hpp>

namespace ydd
{
    class YdProcessLog
    {
	public:
	    YdProcessLog(YdTask::UserIdType userId, YdTask::TaskIdType taskId);
	    void log(int priority, std::string& message);
	    //void log(int priority, boost::asio::streambuf& httpResponse);
	private:
	    std::string prefix_;
    };
}

#endif /* YDPROCESSLOG_H */
