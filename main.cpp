#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc < 2){
        std::perror("Not enougth args");
        exit(1);
    }

    // should do some OOP magic
    const uint16_t PORT = atoi(argv[1]);
    
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0){
        std::perror("socket");
        exit(1);
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }
    
    int sock, bytes_read;
    char buf[128];
    listen(listener, 1);
    while(1)
    {
        sock = accept(listener, NULL, NULL);
        if(sock < 0)
        {
            perror("accept");
            exit(3);
        }
        
        while (1){
            bytes_read = recv(sock, buf, 128, 0);
            if (bytes_read <= 0)
                break;
            std::cout << bytes_read << std::endl;
            send(sock, "neACCEPTED\n", sizeof("ACCEPTED\n"), 0);
        }
        close(sock);
    }
        
    return 0;
}

/*#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>

int main(int argc, char** argv){
    const uint16_t PORT = atoi(argv[1]);
    
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    
    if (listener < 0){
        std::perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    
    return 0;
}*/

/*

    
*/
