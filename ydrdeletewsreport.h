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
		    boost::asio::io_service& ios, bool useSandbox);
	    virtual void run();
	    virtual void processResult();
	    long getYdResult();
	protected:
	    YdRemote::ReportIdType reportId_;
	    long ydResult_;
	    virtual void generateRequest();
    };
}

#endif /* YDRDELETEWSREPORT_H */
