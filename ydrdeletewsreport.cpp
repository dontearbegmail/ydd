#include "ydrdeletewsreport.h"
#include "general.h"
#include <boost/property_tree/ptree.hpp>

namespace ydd
{
    YdrDeleteWsReport::YdrDeleteWsReport(std::string& token, 
	    YdRemote::ReportIdType reportId, boost::asio::io_service& ios, 
	    bool useSandbox) :
	YdRequest(token, ios, useSandbox),
	reportId_(reportId),
	ydResult_(0)
    {
    }

    void YdrDeleteWsReport::generateRequest()
    {
	ptRequest_.put("method", "DeleteWordstatReport");
	ptRequest_.put("locale", locale_);
	ptRequest_.put("token", token_);
	ptRequest_.put("param", reportId_);
    }

    void YdrDeleteWsReport::run()
    {
	YdRequest::run();
    }

    void YdrDeleteWsReport::processResult()
    {
	using namespace boost::property_tree;
	YdRequest::processResult();
	if(state_ != ydNoError)
	    return;
	long ydResult;
	bool resultOk = YdRequest::getNodeVal<long>(ptResponse_, "data", ydResult);
	if(resultOk)
	{
	    state_ = ydOk;
	    ydResult_ = ydResult;
	}
	else
	{
	    state_ = ydDataParseError;
	}
    }

    long YdrDeleteWsReport::getYdResult()
    {
	return ydResult_;
    }
}
