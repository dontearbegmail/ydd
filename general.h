#ifndef GENERAL_H
#define GENERAL_H

#include <errno.h>
#include <syslog.h>
#include <cstddef>


namespace ydd {
    /* at least 128 bytes for SSL!!! */
    static const std::size_t ERR_BUF_LEN = 256;
}

#define msyslog(prior, fmt, args...) syslog(prior, "[%s %s] " fmt, __FILE__, __func__, ##args) 

#endif /* GENERAL_H */
