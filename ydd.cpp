#include "general.h"
#include <iostream>
#include "ydclient.h"


#include <time.h>

int main()
{
    using namespace ydd;
    using namespace std;
    try
    {
	time_t mytime = time(NULL);
	char smt[20];
	strftime(smt, 20, "%Y-%m-%d %H:%M:%S", localtime(&mytime));
	cout << "Request time " << smt << endl << "Request: " << endl;
	std::string request = "{\"method\":\"GetAvailableVersions\","
	    "\"locale\":\"ru\",\"token\":\"c9f13bf86c694e629440c6d56dd29b1e\"}";

	boost::asio::io_service io_service;
	YdClient c(request, io_service, true);
	io_service.run();
    }
    catch (std::exception& e)
    {
	std::cout << "Exception: " << e.what() << endl;
    }

    return 0;
}
