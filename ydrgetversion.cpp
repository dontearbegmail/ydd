#include "ydrgetversion.h"
#include <iostream>

namespace ydd
{
    YdrGetVersion::YdrGetVersion(std::string& token, boost::asio::io_service& ios, bool useSandbox) :
	YdRequest(token, ios, useSandbox)
    {
	generateRequest();
    }

    void YdrGetVersion::generateRequest()
    {
	ptRequest_.put("method", "GetVersion");
	ptRequest_.put("locale", locale_);
	ptRequest_.put("token", token_);
    }

    void YdrGetVersion::processResult()
    {
	std::cout << ydClient_.getJsonResponse() << std::endl;
    }
}
