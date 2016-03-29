#include "ydprocess.h"
#include "ydrequest.h"

namespace ydd
{
    YdProcess::YdProcess(std::string& token, boost::asio::io_service& ios) :
	ios_(ios),
	token_(token),
	currentRequest_(NULL)
    {
    }

    YdProcess::~YdProcess()
    {
	resetCurrentRequest();
    }

    void YdProcess::resetCurrentRequest()
    {
	if(currentRequest_ != NULL)
	{
	    delete currentRequest_;
	    currentRequest_ = NULL;
	}
    }
}
