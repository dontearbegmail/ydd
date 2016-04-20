#ifndef YDPHRASE_H
#define YDPHRASE_H

#include <string>
#include <vector>
#include "phrases_keywords.h"
#include "ydremote.h"

namespace ydd
{
    struct YdPhrase
    {
	unsigned long id;
	std::string value;
	std::vector<phrases_keywords> keywords;
    };

    enum GeneralState 
    {
	init,
	inProgress,
	ok,
	errDatabase,
	errNetwork,
	yderrBadAuth,
	yderrNoPoints,
	yderrFreeReportsFailed,
	yderrOther
    };

    struct YdReport
    {
	YdReport() :
	    isFinished(false),
	    state(ydd::GeneralState::init)
	{
	}
	YdReport(std::vector<YdPhrase> _phrases, bool _isFinished, 
		ydd::GeneralState _state = ydd::GeneralState::init) : 
	    phrases(_phrases),
	    isFinished(_isFinished),
	    state(_state)
	{
	}
	std::vector<YdPhrase> phrases;
	bool isFinished;
	ydd::GeneralState state;
    };
}

#endif /* YDPHRASE_H */
