#ifndef YDPROCESS_H
#define YDPROCESS_H

#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <string>

namespace ydd 
{
    class YdRequest;
    class YdProcess
    {
	public:
	    typedef boost::function<void()> Callback;

	    YdProcess(std::string& token, boost::asio::io_service& ios);
	    virtual ~YdProcess();
	    virtual void run() = 0;
	protected:
	    boost::asio::io_service& ios_;
	    std::string& token_;
	    YdRequest* currentRequest_;

	    void resetCurrentRequest();
    };
}
#endif /* YDPROCESS_H */
