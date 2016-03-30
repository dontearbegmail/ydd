#include "general.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "deleteoldreports.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

void timerHandler(const boost::system::error_code& /*e*/)
{
    std::cout << "Timer" << std::endl;
}

int main()
{
    using namespace ydd;
    using namespace std;
    using namespace sql;

    try {
	sql::Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
	sql::ResultSet *res;

	driver = get_driver_instance();
	con = driver->connect("unix:///var/lib/mysqld/mysqld.sock", "ydd", "123qwe123");
	con->setSchema("ydd_173025");

	stmt = con->createStatement();
	res = stmt->executeQuery("SELECT * FROM `tasks_phrases`");
	while (res->next()) {
	    cout << "\t... MySQL says it again: ";
	    cout << res->getString("phrase") << endl;
	}
	delete res;
	delete stmt;
	delete con;

    } catch (sql::SQLException &e) {
	cout << "# ERR: SQLException in " << __FILE__;
	cout << "(" << __FUNCTION__ << ") on line "
	    << __LINE__ << endl;
	cout << "# ERR: " << e.what();
	cout << " (MySQL error code: " << e.getErrorCode();
	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }

    string token = "c9f13bf86c694e629440c6d56dd29b1e";

    // what is 'reactor pattern?
    try
    {
	boost::asio::io_service io_service;
	DeleteOldReports dro(token, io_service);
	io_service.post(boost::bind(&DeleteOldReports::run, &dro));
	//io_service.run();
    }
    catch (std::exception& e)
    {
	std::cout << "Exception: " << e.what() << endl;
    }

    return 0;
}
