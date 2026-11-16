#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>

#define PORT 9090
#define BUFFER_SIZE 1024
#define IPADD "127.0.0.1"
#define MAX_CLIENTS 2

int main(void){
    //General order: socket, bind, listen, accept
    printf("started connection process...\n");
    int serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serversocket==-1) {perror("socket"); return 1;}
    printf("Created socket\n");

    struct sockaddr_in addr = {0};  //s
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(serversocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // fixes bind: already in use.

    int bindstatus = bind(serversocket, (struct sockaddr*)&addr, sizeof(addr));
    if (bindstatus==-1){perror("bind"); return 1;}
    printf("Binded to socket\n");

    int listenstatus = listen(serversocket, 5);
    if (listenstatus==-1){perror("listen"); return 1;}
    printf("listening to ip address: %s on port: 9090\n", IPADD);

    struct pollfd fds[3];
    fds[0].fd = serversocket; //fd to watch.
    fds[0].events = POLLIN; //data ready to read on listening socket, event is what I want to know abt.
    //revents is what happens, kernel sets this.
    //prototype: int poll(struct pollfd *fds, nfds_t nfds, int timeout); *#include <poll>*
    char* wmsg = "victor say what\n";
    int clientarray [MAX_CLIENTS] = {0, 0}; //array to hold client fds
    int clients = 1; //nfds
    for (;;){
        if (clients == MAX_CLIENTS + 1){printf("Client Maximum Reached!\n"); break;}
        if (poll(fds,clients,-1)==-1){perror("poll"); break;}
        if (fds[0].revents & POLLIN){
            int client = accept(serversocket, NULL, NULL); 
            if (client==-1){perror("accept"); break;} 
            fds[clients].fd = client;
            fds[clients].events = POLLIN;
            send(client, wmsg, strlen(wmsg), 0);
            printf("client: %d connected\n", clients);
            clients+=1;
        }
    }
ssize_t bytes;
    char buff[BUFFER_SIZE];
    printf("\nClient Disconnected!\n");
    close(fds[1].fd);
    close(serversocket);
    return 0;
}