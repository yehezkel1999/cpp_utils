
#include <ostream>
#include "Socket.h"

const int utils::Socket::minBuff = 32;

utils::Socket::Socket(int type, ADDRESS_FAMILY family, int protocol)
    : _sock(0), _isValid(createSocket(type, protocol, family)), _sockStruct(sockaddr_in()) {
    _sockStruct.sin_family = family;
}
utils::Socket::Socket(SOCKET socket, sockaddr_in &sockStruct)
    : _sock(socket), _isValid(socket != INVALID_SOCKET), _sockStruct(sockStruct) {}
utils::Socket::Socket(const Socket &other)
    : _sock(other._sock), _isValid(other._isValid), _sockStruct(other._sockStruct) {}
utils::Socket::Socket(Socket &&other)
    : _sock(other._sock), _isValid(other._isValid), _sockStruct(other._sockStruct) {
        other._sock = 0;
        other._isValid = false;
        other._sockStruct = sockaddr_in();
    }

bool utils::Socket::createSocket(int type, int protocol, ADDRESS_FAMILY family) {
    _sock = ::socket(family, type, protocol);
    return _sock != INVALID_SOCKET;
}

void utils::Socket::bind(const char *address, int port) {
    validateSocket();
    
    _sockStruct.sin_addr.s_addr = inet_addr(address);
    _sockStruct.sin_port = htons(port);

    if (::bind(_sock, (struct sockaddr *)&_sockStruct, sizeof(_sockStruct)) == SOCKET_ERROR)
        throw socket_error("Could not bind the socket on the given host and port, please make sure "
                                    "the input is correct and the port is open and free");
}
void utils::Socket::listen(int backlog) {
    validateSocket();

    if (::listen(_sock, backlog) == SOCKET_ERROR)
        throw socket_error("Something went wrong when listening, please make sure the port is still open and free");
}
utils::Socket utils::Socket::accept() {
    validateSocket();

    struct sockaddr_in client = sockaddr_in();
    socklen_t c = sizeof(struct sockaddr_in);
    SOCKET new_socket = ::accept(_sock, (struct sockaddr *)&client, &c);

    if (new_socket == INVALID_SOCKET)
        throw socket_error("Could not accept the given client");

    return Socket(new_socket, client);
}

void utils::Socket::connect(const char *address, int port) {
    validateSocket();
    
    _sockStruct.sin_addr.s_addr = inet_addr(address);
    _sockStruct.sin_port = htons(port);

    if (::connect(_sock, (const struct sockaddr *)&_sockStruct, sizeof(_sockStruct)) < 0)
        throw socket_error("Could not connect to the given host on the given port, please make sure the input is correct");
}
void utils::Socket::send(const char *message, size_type size) const {
    validateSocket();

    if (::send(_sock, message, size, 0) < 0)
        throw socket_error("Sending info failed, please make sure the endpoint is still alive");
}
const char *utils::Socket::recv(int bufferSize) const {
    DEBUG(assert(bufferSize > Socket::minBuff && "buffer cannot be less than Socket::minBuff");)

    char *data = new char[bufferSize];
    try { recv(data, bufferSize); }
    catch(...) { delete[] data; throw; }
    delete[] data;

    return data;
}
int utils::Socket::recv(char *buffer, size_type size) const {
    validateSocket();

    int recvSize = ::recv(_sock, buffer, size - 1, 0);

    if (recvSize == SOCKET_ERROR) 
        throw socket_error("Receiving info failed, please make sure the endpoint is still alive");
    
    buffer[recvSize] = '\0';
    return recvSize;
}
void utils::Socket::close() {
    if (_isValid) {
#ifdef _WINDOWS
        ::closesocket(_sock);
#else
        ::close(_sock);
#endif // _WINDOWS
        _isValid = false;
    }
}

utils::Socket &utils::Socket::operator=(const Socket &rhs) {
    if (this != &rhs) {
        _sock = rhs._sock;
        _isValid = rhs._isValid;
        _sockStruct = rhs._sockStruct;
    }
    return *this;
}
utils::Socket &utils::Socket::operator=(Socket &&rhs) {
    if (this != &rhs) {
        _sock = rhs._sock;
        _isValid = rhs._isValid;
        _sockStruct = rhs._sockStruct;

        rhs._sock = 0;
        rhs._isValid = false;
        rhs._sockStruct = sockaddr_in();
    }
    return *this;
}

bool utils::Socket::operator==(const Socket &rhs) const {
    return _sockStruct.sin_family == rhs._sockStruct.sin_family
        && _sockStruct.sin_port == rhs._sockStruct.sin_port
        && strcmp(getAddress(), rhs.getAddress()) == 0;
}

std::ostream &operator<<(std::ostream &output, const utils::Socket &source) {
    output << "(";
    output << source.getAddress();
    output << ":";
    output << source.getPort();
    output << ")";

    return output;
}
