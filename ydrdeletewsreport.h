#ifndef YDRDELETEWSREPORT_H
#define YDRDELETEWSREPORT_H

#include "ydrequest.h"
#include <vector>

namespace ydd
{
    class YdrDeleteWsReport : public YdRequest
    {
	public:
	    YdrDeleteWsReport(std::string& token, YdRemote::ReportIdType reportId,
		    boost::asio::io_service& ios, bool useSandbox,
		    YdProcess::Callback ydProcessCallback);
	    virtual void run();
	    virtual void processResult();
	    bool isDeleted();
	    YdRemote::ReportIdType getReportId();
	protected:
	    YdRemote::ReportIdType reportId_;
	    long ydResult_;
	    virtual void generateRequest();
    };
}

#endif /* YDRDELETEWSREPORT_H */
