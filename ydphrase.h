#ifndef YDPHRASE_H
#define YDPHRASE_H

#include <string>
#include <vector>

namespace ydd
{
    struct YdPhrase
    {
	unsigned long id;
	std::string value;
	std::vector<std::string> keywords;
    };

    struct YdReport
    {
	std::vector<YdPhrase> phrases;
    };
}

#endif /* YDPHRASE_H */
