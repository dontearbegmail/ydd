#include "ydrgetversion.h"

namespace ydd
{
    YdrGetVersion::YdrGetVersion(std::string& token, boost::asio::io_service& ios, 
	    bool useSandbox, YdProcess::Callback ydProcessCallback) :
	YdRequest(token, ios, useSandbox, ydProcessCallback)
    {
    }

    void YdrGetVersion::generateRequest()
    {
	ptRequest_.put("method", "GetVersion");
	ptRequest_.put("locale", locale_);
	ptRequest_.put("token", token_);
    }

    void YdrGetVersion::run()
    {
	YdRequest::run();
    }

    bool YdrGetVersion::getVersion(long& version)
    {
	if(state_ == YdRequest::ydOk)
	{
	    version = version_;
	    return true;
	}
	return false;
    }

    void YdrGetVersion::processResult()
    {
	YdRequest::processResult();
	if(state_ != ydNoError)
	    return;
	long v;
	if(YdRequest::getNodeVal<long>(ptResponse_, "data", v))
	{
	    version_ = v;
	    state_ = ydOk;
	}
	else
	{
	    state_ = ydDataParseError;
	}
	runYdProcessCallback();
    }
}
