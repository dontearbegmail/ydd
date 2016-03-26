#ifndef YDRGETVERSION_H
#define YDRGETVERSION_H

#include "ydrequest.h"

namespace ydd
{
    class YdrGetVersion : public YdRequest
    {
	public:
	    YdrGetVersion(std::string& token, boost::asio::io_service& ios, bool useSandbox);
	    virtual void run();
	protected:
	    void generateRequest();
    };
}

#endif /* YDRGETVERSION_H */
