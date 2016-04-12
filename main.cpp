#include "general.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "dbconn.h"

#include "deleteoldreports.h"
#include "ydrgetversion.h"
#include "ydbasetask.h"


void timerHandler(const boost::system::error_code& /*e*/)
{
    std::cout << "Timer" << std::endl;
}

int main()
{
    using namespace ydd;
    using namespace std;
    string token = "c9f13bf86c694e629440c6d56dd29b1e";

    // what is 'reactor pattern?
    try
    {
	boost::asio::io_service ios;
	DbConn dbc;
	YdBaseTask ydt(ios, dbc, 50, 1);
	std::string html = "POST /v4/json/ HTTP/1.1\r\nHost:api-sandbox.direct.yandex.ru\r\n";
	ydt.log(YdBaseTask::info, "m'\"test", &html);

	/* DeleteOldReports dro(token, io_service);
	io_service.post(boost::bind(&DeleteOldReports::run, &dro));
	YdrGetVersion r(token, io_service, true, NULL);
	r.run();
	io_service.run();
	std::cout << r.getJsonResponse() << std::endl;*/
    }
    catch (const mysqlpp::ConnectionFailed& err) {
	cerr << "Failed to connect to database server: " <<
	    err.what() << endl;
	return 1;
    }
catch (std::exception& e)
    {
	std::cout << "Exception: " << e.what() << endl;
    }
    
    return 0;
}
