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

    struct YdReport
    {
	std::vector<YdPhrase> phrases;
	bool isFinished;
	YdReport() : isFinished(false) {}
	void reset()
	{
	    isFinished = false;
	    phrases.clear();
	}
    };
}

#endif /* YDPHRASE_H */
