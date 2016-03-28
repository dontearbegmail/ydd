#ifndef YDRGETVERSION_H
#define YDRGETVERSION_H

#include "ydrequest.h"

namespace ydd
{
    class YdrGetVersion : public YdRequest
    {
	public:
	    YdrGetVersion(std::string& token, boost::asio::io_service& ios, bool useSandbox,
		    YdProcess::Callback ydProcessCallback);
	    virtual void run();
	    virtual void processResult();
	    bool getVersion(long& version);
	protected:
	    virtual void generateRequest();
	    long version_;
    };
}

#endif /* YDRGETVERSION_H */
