#ifndef IRC42_EXCEPTIONS_H
#define IRC42_EXCEPTIONS_H

#include <exception>
#include <iostream>
#include "Log.hpp"

/* Throw is cost ineficcient when used as a return value.
 * These exceptions are not to be used to guide program
 * branching. These exceptions are meant to provide information
 * on a problem that the program cannot continue running with.
 */

namespace irc {
namespace exc {

class MallocError : public std::exception {
    public:
    virtual const char* what() const throw() {
        return ("Fatal error (malloc)");
    }
};

class ServerSetUpError : public std::exception {
    public:
    virtual const char* what() const throw() {
        return ("Server : Set up error");
    }
};

class FatalError : public std::exception {
    
    public:
    FatalError(const std::string msg) : m_msg(msg) {}
    virtual ~FatalError() throw() {}

    virtual const char* what() const throw ()  {
            LOG(ERROR)<< "Fatal internal error : " << m_msg;
            return m_msg.c_str();
    }

   const std::string m_msg;
};

}
}

#endif /* IRC42_EXCEPTIONS_H */
