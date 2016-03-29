#ifndef DELETEOLDREPORTS_H
#define DELETEOLDREPORTS_H

#include "ydprocess.h"
#include "ydrgetwsreportlist.h"
#include "ydrdeletewsreport.h"

namespace ydd
{

    class DeleteOldReports : public YdProcess
    {
	public:
	    DeleteOldReports(std::string& token, boost::asio::io_service& ios);
	    virtual void run();
	    void step1Handler();
	    void step2Handler();
	protected:
	    YdrGetWsReportList::ReportList reportList_;
    };
}
#endif /* DELETEOLDREPORTS_H */
