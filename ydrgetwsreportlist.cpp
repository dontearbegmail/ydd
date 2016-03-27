#include "ydrgetwsreportlist.h"
#include "general.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp> 

namespace ydd
{
    YdrGetWsReportList::YdrGetWsReportList(std::string& token, boost::asio::io_service& ios, bool useSandbox) :
	YdRequest(token, ios, useSandbox)
    {
    }

    void YdrGetWsReportList::generateRequest()
    {
	ptRequest_.put("method", "GetWordstatReportList");
	ptRequest_.put("locale", locale_);
	ptRequest_.put("token", token_);
    }

    void YdrGetWsReportList::run()
    {
	YdRequest::run();
    }

    void YdrGetWsReportList::processResult()
    {
	using namespace boost::property_tree;
	YdRequest::processResult();
	if(state_ != ydNoError)
	    return;
	reportList_.clear();
	bool idOk = false, statusOk = false;
	YdRemote::ReportIdType id;
	std::string strStatus;
	ReportStatus status;
	ReportStatusInfo sti;
	try
	{
	    BOOST_FOREACH(ptree::value_type& v, ptResponse_.get_child("data"))
	    {
		// v.second contains subtree {"StatusReport":"Done","ReportID":"nnnn"}
		idOk = YdRequest::getNodeVal<YdRemote::ReportIdType>(v.second, "ReportID", id);
		statusOk = YdRequest::getNodeVal<std::string>(v.second, "StatusReport", strStatus);
		if(statusOk)
		{
		    if(strStatus == "Done")
			status = Done;
		    else if(strStatus == "Pending")
			status = Pending;
		    else if(strStatus == "Failed")
			status = Failed;
		    else 
			statusOk = false;
		}
		if(idOk && statusOk)
		{
		    sti.id = id;
		    sti.status = status;
		    reportList_.push_back(sti);
		}
		else
		{
		    break;
		}
	    }
	}
	catch(std::exception& e)
	{
	    idOk = false;
	    msyslog(LOG_ERR, "JSON parse expcetion: %s", e.what());
	}
	if(!idOk || !statusOk)
	    state_ = ydDataParseError;
	else
	    state_ = ydOk;
    }

    YdrGetWsReportList::ReportList& YdrGetWsReportList::getReportList()
    {
	return reportList_;
    }
}
