#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <liburing.h>

#include <iostream>
#include <cstdio>

#include <cstdlib>
#include <cstring>

#define MAX_CONNECTIONS 1024
#define MAX_MESSAGE_LEN 128
#define BACKLOG 32

class Server{    
    typedef struct conn_info {
        uint32_t fd;
        uint32_t type;
    } conn_info;
    
    enum {
        ACCEPT,
        READ,
        WRITE,
    };
    
    int listener;
    const uint16_t PORT;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int out;    

    conn_info conns[MAX_CONNECTIONS];
    char bufs[MAX_CONNECTIONS][MAX_MESSAGE_LEN] = {0};
    
    public:
        Server();
        Server(int port);
        Server(char* port);
        
        ~Server();
 
        void connect();
        
        void start_listen();
            
        void add_accept(struct io_uring *ring, int fd, struct sockaddr *client_addr, socklen_t *client_len);

        void add_socket_read(struct io_uring *ring, int fd, size_t size);

        void add_socket_write(struct io_uring *ring, int fd, size_t size);
};

#endif
