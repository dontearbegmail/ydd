#ifndef YDRGETWSREPORTLIST_H
#define YDRGETWSREPORTLIST_H

#include "ydrequest.h"

namespace ydd
{
    class YdrGetWsReportList : public YdRequest
    {
	public:
	    YdrGetWsReportList(std::string& token, boost::asio::io_service& ios, bool useSandbox);
	    virtual void run();
	    virtual void processResult();
	protected:
	    virtual void generateRequest();
    };
}

#endif /* YDRGETWSREPORTLIST_H */
