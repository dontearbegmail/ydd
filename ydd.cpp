#include "general.h"
#include <iostream>
#include "ydclient.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>

void timerHandler(const boost::system::error_code& /*e*/)
{
}

int main()
{
    using namespace ydd;
    using namespace std;
    try
    {
	std::string request = "{\"method\":\"GetAvailableVersions\","
	    "\"locale\":\"ru\",\"token\":\"c9f13bf86c694e629440c6d56dd29b1e\"}";

	boost::asio::io_service io_service;
	//YdClient c(request, io_service, true);
	//boost::asio::deadline_timer t(io_service, boost::posix_time::seconds(50));
	//t.async_wait(&timerHandler);
	io_service.run();
	vector<string> state = {"In progress", "Failed", "OK"};
	//cout << "State: " << state[c.getState()] << endl << c.getJsonResponse() << endl;
    }
    catch (std::exception& e)
    {
	std::cout << "Exception: " << e.what() << endl;
    }

    return 0;
}
