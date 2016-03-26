#include "general.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include "ydrgetversion.h"

void timerHandler(const boost::system::error_code& /*e*/)
{
}


int main()
{
    using namespace ydd;
    using namespace std;

    string token = "c9f13bf86c694e629440c6d56dd29b1e";

    try
    {

	std::string request = "{\"method\":\"GetAvailableVersions\","
	    "\"locale\":\"ru\",\"token\":\"c9f13bf86c694e629440c6d56dd29b1e\"}";

	boost::asio::io_service io_service;
	YdrGetVersion r(token, io_service, true);
	r.run();
	//boost::asio::deadline_timer t(io_service, boost::posix_time::seconds(50));
	//t.async_wait(&timerHandler);
	
	io_service.run();
    }
    catch (std::exception& e)
    {
	std::cout << "Exception: " << e.what() << endl;
    }

    return 0;
}
