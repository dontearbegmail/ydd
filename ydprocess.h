#ifndef YDPROCESS_H
#define YDPROCESS_H

#include "ydrequest.h"
#include <boost/function.hpp>

class YdProcess
{
    public:
	typedef boost::function<void()> Callback;
};

#endif /* YDPROCESS_H */
