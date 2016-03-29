#ifndef DELETEOLDREPORTS_H
#define DELETEOLDREPORTS_H

#include <boost/asio.hpp>
#include <iostream>
#include "ydrequest.h"
#include "ydrgetwsreportlist.h"
#include "ydrdeletewsreport.h"

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
}
#endif /* DELETEOLDREPORTS_H */
