#include "ydrgetwsreportlist.h"

namespace ydd
{
    YdrGetWsReportList::YdrGetWsReportList(std::string& token, boost::asio::io_service& ios, bool useSandbox) :
	YdRequest(token, ios, useSandbox)
    {
    }

    void YdrGetWsReportList::generateRequest()
    {
	ptRequest_.put("method", "GetWsReportList");
	ptRequest_.put("locale", locale_);
	ptRequest_.put("token", token_);
    }

    void YdrGetWsReportList::run()
    {
	YdRequest::run();
    }

    void YdrGetWsReportList::processResult()
    {
	YdRequest::processResult();
    }
}
