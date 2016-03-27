#include "general.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include "ydrcreatewsreport.h"
#include "ydrgetwsreportlist.h"
#include "ydrdeletewsreport.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

void timerHandler(const boost::system::error_code& /*e*/)
{
    std::cout << "Timer" << std::endl;
}

int main()
{
    using namespace ydd;
    using namespace std;

    /*using namespace boost::property_tree;
    ptree pt;
    ptree child, subarr;

    child.put("", "превед");
    subarr.push_back(std::make_pair("", child));
    child.put("", "кагдила");
    subarr.push_back(std::make_pair("", child));
    pt.add_child("Phrases", subarr);

    ostringstream buf; 
    write_json(buf, pt, false);
    string json = buf.str();

    cout << json << endl;

    cout << json.length() << endl;*/

    string token = "c9f13bf86c694e629440c6d56dd29b1e";

    try
    {
	std::string request = "{\"method\":\"GetAvailableVersions\","
	    "\"locale\":\"ru\",\"token\":\"c9f13bf86c694e629440c6d56dd29b1e\"}";

	boost::asio::io_service io_service;
	YdrCreateWsReport::Phrases phrases = {"гипсокартон", "плитка", "саморезы"};
	YdrCreateWsReport::GeoId geoId;
	//YdrCreateWsReport r(token, phrases, geoId, io_service, true);
	YdrGetWsReportList r(token, io_service, true);
	r.run();
	boost::asio::deadline_timer t(io_service, boost::posix_time::seconds(1));
	t.async_wait(&timerHandler);

	YdrDeleteWsReport d(token, 16443, io_service, true);
	d.run();
	
	io_service.run();

	if(r.getState() == YdRequest::ydError)
	{
	    string es;
	    r.getYdErrorString(es);
	    cout << es << endl;
	}
	else 
	{
	    cout << r.getJsonResponse() << endl;
	    
	    /* YdrGetWsReportList output */
	    YdrGetWsReportList::ReportList& rl = r.getReportList();
	    for(YdrGetWsReportList::ReportList::iterator it = rl.begin();
		    it != rl.end(); ++it)
	    {
		cout << it->id << " : " << it->status << endl;
	    }
	}

	if(r.getState() == YdRequest::ydError)
	{
	    string es;
	    d.getYdErrorString(es);
	    cout << es << endl;
	}
	else 
	{
	    cout << d.getJsonResponse() << endl;
	    cout << "YdResult: " << d.getYdResult() << endl;
	}
    }
    catch (std::exception& e)
    {
	std::cout << "Exception: " << e.what() << endl;
    }

    return 0;
}
