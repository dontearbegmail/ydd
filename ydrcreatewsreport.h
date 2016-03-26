#ifndef YDRCREATEWSREPORT_H
#define YDRCREATEWSREPORT_H

#include "ydrequest.h"
#include <vector>
#include <string>

namespace ydd
{
    class YdrCreateWsReport : public YdRequest
    {
	public:
	    typedef std::vector<std::string> Phrases;
	    typedef std::vector<long> GeoId;
	    YdrCreateWsReport(std::string& token, Phrases& phrases, GeoId& geoId, 
		    boost::asio::io_service& ios, bool useSandbox);
	    virtual void run();
	    virtual void processResult();
	protected:
	    Phrases& phrases_;
	    GeoId& geoId_;
	    virtual void generateRequest();
    };
}

#endif /* YDRCREATEWSREPORT_H */
