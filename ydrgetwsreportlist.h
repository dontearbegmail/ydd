#ifndef YDRGETWSREPORTLIST_H
#define YDRGETWSREPORTLIST_H

#include "ydrequest.h"
#include <vector>

namespace ydd
{
    class YdrGetWsReportList : public YdRequest
    {
	public:
	    enum ReportStatus {Done, Pending, Failed};
	    struct ReportStatusInfo
	    {
		unsigned long id;
		ReportStatus status;
	    };
	    typedef std::vector<ReportStatusInfo> ReportList;

	    YdrGetWsReportList(std::string& token, boost::asio::io_service& ios, bool useSandbox);
	    virtual void run();
	    virtual void processResult();
	    ReportList& getReportList();
	protected:
	    virtual void generateRequest();
	    ReportList reportList_;
    };
}

#endif /* YDRGETWSREPORTLIST_H */
