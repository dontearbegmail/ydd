#ifndef YDPHRASE_H
#define YDPHRASE_H

#include <string>
#include <vector>
#include "sqlkeyword.h"

namespace ydd
{
    struct YdPhrase
    {
	unsigned long id;
	std::string value;
	std::vector<SqlKeyword> keywords;
    };

    struct YdReport
    {
	std::vector<YdPhrase> phrases;
	bool isFinished;
    };
}

#endif /* YDPHRASE_H */
