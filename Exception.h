#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
#include <stdexcept>
#define INITIALIZATION_FAILED 1
#define CONNECTION_FAILED 2
#define THREAD_FAILED 3
#define BINDING_FAILED 4
#define ACCEPTING_FAILED 5

using ErrorType = int;

class MySocketException : public std::exception{
public:
    MySocketException(ErrorType e):_error_type(e){};

    const char* what() const throw(){
        switch (_error_type)
        {
        case INITIALIZATION_FAILED:
            return "ERROR: Initializing failed.";
            break;
        case CONNECTION_FAILED:
            return "ERROR: Failed to connecting to the server.";
            break;
        case THREAD_FAILED:
            return "ERROR: Failed to create a thread.";
            break;
        case BINDING_FAILED:
            return "ERROR: Failed to bind with this port.";
            break;
        case ACCEPTING_FAILED:
            return "ERROR: Failed to accept the client.";
            break;
        default:
            break;
        }
        return "test";
    }
private:
    ErrorType _error_type;
};


#endif