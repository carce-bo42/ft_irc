#ifndef LOG_H
#define LOG_H

/* Inspired by :
 * https://stackoverflow.com/questions/5028302/small-logger-class
 */

#include <iostream>
#include <vector>
#include <string>

enum typelog {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR
};

const std::string loglevel[4] = {
    "DBUG",
    "INFO",
    "WARN",
    "ERRO"
};

class LOG {
public:
    LOG();
    LOG(typelog type);
    ~LOG();
    // Uses << operator from msg 
    template<class T>
    LOG &operator<<(const T &msg) {
        std::cout << msg;
        opened = true;
        return *this;
    }
private:
    bool opened;
};

#endif  /* LOG_H */
