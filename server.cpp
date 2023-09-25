#include "server.hpp"

Server::Server() : PORT{8080} {
    out = open("8080.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (!out){
        std::perror("File is not open");
        exit(5);
    }
}

Server::Server(int port) : PORT{(uint16_t)port} {
    char str_port[6];
    sprintf(str_port, "%d", port);
    
    const char* filename = strcat(str_port, ".txt");
    out = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (!out){
        std::perror("File is not open");
        exit(5);
    }           
}
Server::Server(char* port) : PORT{(uint16_t)atoi(port)} {
    const char* filename = strcat(port, ".txt");
    out = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (!out){
        std::perror("File is not open");
        exit(5);
    }
}

Server::~Server() {
    if (out)
        close(out);
}

void Server::connect() {
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0){
        std::perror("socket");
        exit(2);
    }
    const int val = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        exit(3);
    }
}

void Server::start_listen(){
    listen(listener, BACKLOG);
    
    struct io_uring ring;
    struct io_uring_params params;
    memset(&params, 0, sizeof(params));
    
    if (io_uring_queue_init_params(MAX_CONNECTIONS, &ring, &params) < 0){
        perror("io_uring_init_failed...\n");
        exit(6);
    }
    
    if (!(params.features & IORING_FEAT_FAST_POLL)) {
        printf("IORING_FEAT_FAST_POLL not available in the kernel, quiting...\n");
        exit(0);
    }
    
    add_accept(&ring, listener, (struct sockaddr *) &client_addr, &client_addr_len);

    while(1)
    {
        struct io_uring_cqe *cqe;
        io_uring_submit(&ring);
        
        io_uring_wait_cqe(&ring, &cqe);
        
        struct io_uring_cqe *cqes[BACKLOG];
        int cqe_count = io_uring_peek_batch_cqe(&ring, cqes, sizeof(cqes) / sizeof(cqes[0]));
        for (int i = 0; i < cqe_count; ++i) {
            cqe = cqes[i];

            conn_info *user_data = (conn_info *) io_uring_cqe_get_data(cqe);

            unsigned type = user_data->type;
            if (type == ACCEPT) {
                int sock_conn_fd = cqe->res;
                add_socket_read(&ring, sock_conn_fd, MAX_MESSAGE_LEN);
                add_accept(&ring, listener, (struct sockaddr *) &client_addr, &client_addr_len);
                //std::cout << "ACCEPT" << std::endl;
            } else if (type == READ) {
                int bytes_read = cqe->res;
                if (bytes_read <= 0) {
                    shutdown(user_data->fd, SHUT_RDWR);
                    exit(0);
                } else {
                   add_socket_write(&ring, user_data->fd, bytes_read);
                }
                //std::cout << "READ" << std::endl;
               
            } else if (type == WRITE){
                add_socket_read(&ring, user_data->fd, MAX_MESSAGE_LEN);
                //std::cout << "WRITE" << std::endl;
            }

            io_uring_cqe_seen(&ring, cqe);
        }
    }
}
    
void Server::add_accept(struct io_uring *ring, int fd, struct sockaddr *client_addr, socklen_t *client_len) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe, fd, client_addr, client_len, 0);

    conn_info *conn_i = &conns[fd];
    conn_i->fd = fd;
    conn_i->type = ACCEPT;

    io_uring_sqe_set_data(sqe, conn_i);
}

void Server::add_socket_read(struct io_uring *ring, int fd, size_t size) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

    io_uring_prep_recv(sqe, fd, &bufs[fd], size, 0);

    conn_info *conn_i = &conns[fd];
    conn_i->fd = fd;
    conn_i->type = READ;

    io_uring_sqe_set_data(sqe, conn_i);
}

void Server::add_socket_write(struct io_uring *ring, int fd, size_t size) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  
    io_uring_prep_write(sqe, out, bufs[fd], size, 0);

    conn_info *conn_i = &conns[fd];
    conn_i->fd = fd;
    conn_i->type = WRITE;

    io_uring_sqe_set_data(sqe, conn_i);
}
