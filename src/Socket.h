#ifndef SOCKET_H_
#define SOCKET_H_

//help from http://www.binarytides.com/

#include <exception>
#include <stdexcept>
#include <cassert>

#ifndef DEBUG
#ifndef NDEBUG
#define DEBUG(x) x
#else // !NDEBUG
#define DEBUG(x)
#endif // !NDEBUG
#endif // !DEBUG


#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) // if windows

#define _WINDOWS

#include <WinSock2.h>

typedef int socklen_t;

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif // !_WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#if(_WIN32_WINNT < 0x0600)
typedef short ADDRESS_FAMILY;
#endif //(_WIN32_WINNT < 0x0600)

#pragma comment(lib, "ws2_32.lib")

#else // not windows

#define _UNIX

typedef int SOCKET;
typedef unsigned short ADDRESS_FAMILY;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#endif // if windows

namespace utils {
    
    class socket_error : public std::runtime_error {
    public:
        socket_error(const char *message = "socket error") : std::runtime_error(message) {}
        socket_error(const socket_error &socket_error) : std::runtime_error(socket_error) {}
        ~socket_error() {}
    };


    class Socket {
    public:
        using size_type = unsigned int;

    private:
        Socket(SOCKET socket, sockaddr_in &sockStruct);
        void validateSocket() const { DEBUG(assert(_isValid && "Socket is invalid, please make sure all the information is correct");) }
        bool createSocket(int type, int protocol, ADDRESS_FAMILY family);

    public:
        static const int minBuff;

        Socket(int type = SOCK_STREAM, ADDRESS_FAMILY family = AF_INET, int protocol = 0);
        Socket(const Socket &other);
        Socket(Socket &&other);
        virtual ~Socket() {}

        bool isValid() { return _isValid; }
        SOCKET getSocket() { return _sock; }
        sockaddr_in &getSocketInfo() { return _sockStruct; }
        const char *getAddress() const { return inet_ntoa(_sockStruct.sin_addr);};
        int getPort() const { return ntohs(_sockStruct.sin_port); }; 

        void bind(const char* address, int port);
        void listen(int backlog);
        Socket accept();

        void connect(const char *address, int port);

        void send(const char *message) const { send(message, strlen(message)); }
        void send(const char *message, size_type size) const;
        const char *recv(int bufferSize = 256) const;
        int recv(char *buffer, size_type size) const;
        void close();

        virtual Socket &operator=(const Socket &rhs);
        virtual Socket &operator=(Socket &&rhs);

        virtual bool operator==(const Socket &rhs) const;
        virtual bool operator!=(const Socket &rhs) const { return !(*this == rhs); }

        friend std::ostream &operator<<(std::ostream &output, const Socket &source);

    private:	
        SOCKET _sock;
        bool _isValid;
        sockaddr_in _sockStruct;
    };

    using socket = Socket;
}
#endif // !SOCKET_H_