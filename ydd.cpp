#include "general.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include "ydrequest.h"
#include "ydrcreatewsreport.h"
#include "ydrgetwsreportlist.h"
#include "ydrdeletewsreport.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

void timerHandler(const boost::system::error_code& /*e*/)
{
    std::cout << "Timer" << std::endl;
}

namespace ydd
{

    class DeleteOldReports : public YdProcess
    {
	public:
	    DeleteOldReports(std::string& token, boost::asio::io_service& ios);
	    ~DeleteOldReports();
	    void resetCurrentRequest();
	    void run();
	    void step1Handler();
	    void step2Handler();
	protected:
	    boost::asio::io_service& ios_;
	    std::string token_;
	    YdRequest* currentRequest_;
	    YdrGetWsReportList::ReportList reportList_;
    };

    DeleteOldReports::DeleteOldReports(std::string& token, boost::asio::io_service& ios) :
	ios_(ios),
	token_(token),
	currentRequest_(NULL)
    {
    }

    DeleteOldReports::~DeleteOldReports()
    {
	resetCurrentRequest();
    }

    void DeleteOldReports::resetCurrentRequest()
    {
	if(currentRequest_ != NULL)
	{
	    delete currentRequest_;
	    currentRequest_ = NULL;
	}
    }

    void DeleteOldReports::run()
    {
	currentRequest_ = new YdrGetWsReportList(token_, ios_, true,
		boost::bind(&DeleteOldReports::step1Handler, this));
	ios_.post(boost::bind(&YdRequest::run, currentRequest_));
    }

    void DeleteOldReports::step1Handler()
    {
	using namespace std;
	YdrGetWsReportList* r = dynamic_cast<YdrGetWsReportList*>(currentRequest_);
	if(r->getState() == YdRequest::ydOk)
	{
	    reportList_ = r->getReportList();
	    if(reportList_.size() == 0)
	    {
		cout << "There are no reports in your account" << endl;
	    }
	    else
	    {
		cout << "There are the following " << reportList_.size() << " report(s) in your "
		    "account: " << endl;
		for(YdrGetWsReportList::ReportList::iterator it = reportList_.begin();
			it != reportList_.end(); ++it)
		{
		    cout << "Id: " << it->id << ", Status: " << it->status << endl;
		}
	    }
	}
	else
	{
	    cout << "ERROR: " << r->getJsonResponse();
	}
	resetCurrentRequest();
	ios_.post(boost::bind(&DeleteOldReports::step2Handler, this));
    }

    void DeleteOldReports::step2Handler()
    {
	using namespace std;
	if(currentRequest_ != NULL)
	{
	    YdrDeleteWsReport* r = dynamic_cast<YdrDeleteWsReport*>(currentRequest_);
	    cout << "Delete result for report id " << reportList_.back().id <<
		": ";
	    if(r->isDeleted())
		cout << "success" << endl;
	    else
		cout << "failure" << endl;
	    reportList_.pop_back();
	    resetCurrentRequest();
	}
	if(reportList_.size() > 0)
	{
	    currentRequest_ = new YdrDeleteWsReport(token_, reportList_.back().id,
		    ios_, true, boost::bind(&DeleteOldReports::step2Handler, this));
	    ios_.post(boost::bind(&YdRequest::run, currentRequest_));
	}
    }

}

int main()
{
    using namespace ydd;
    using namespace std;

    string token = "c9f13bf86c694e629440c6d56dd29b1e";

    try
    {
	boost::asio::io_service io_service;
	DeleteOldReports dro(token, io_service);
	io_service.post(boost::bind(&DeleteOldReports::run, &dro));
	io_service.run();
    }
    catch (std::exception& e)
    {
	std::cout << "Exception: " << e.what() << endl;
    }

    return 0;
}
