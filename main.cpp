#include "server.hpp"

int main(int argc, char** argv)
{
    if (argc < 2){
        std::perror("Not enougth args");
        exit(1);
    }

    Server serv(argv[1]);
    serv.connect();
    serv.start_listen();
       
    return 0;
}
