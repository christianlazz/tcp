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
#define IPADD "192.168.4.26"
#define MAX_CLIENTS 3

int main(void){
    //General order: socket, bind, listen, accept
    printf("started connection process...\n");
    int serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serversocket==-1) {perror("socket"); return 1;}
    printf("Created socket\n");

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(serversocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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
    int clients = 1; //nfds
    for (;;){
        if (clients == 2){printf("Client Maximum Reached!\n"); break;}
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
    close()


/*struct pollfd fd[MAX_CLIENTS + 1]; Will use again
    int fds = 1;
    fd[0].fd = serversocket;
    fd[0].events = POLLIN;

    const char* welcome = "Welcome to tcp chat room\n";
    for (;;){
        if (poll(fd, fds, -1) == -1) {perror("poll"); break;}
        if (fd[0].revents & POLLIN){
            int client = accept(serversocket, NULL, NULL); 
            if (client==-1){perror("accept"); return 1;}
            if (fds >= MAX_CLIENTS + 1){
                char* full =  "Chat room full.";
                send(client, full, strlen(full), 0);
                close(client);
            }else{
                fd[fds].fd = client;
                fd[fds].events = POLLIN;
                fd[fds].revents = 0;
                send(client, welcome, strlen(welcome), 0);
                printf("Client connected (fd %d), total: %d\n", client, fds - 1);
            }
        }
    }
*/
    int serveraccept = accept(serversocket, NULL, NULL);
    if (serveraccept==-1){perror("accept"); return 1;}
    printf("CONNECTION ESTABLISHED!\n");

    char buff[BUFFER_SIZE];
    char* msg = "Welcome to my test tcp chat room!\n";
    send(serveraccept, msg, strlen(msg), 0);

    ssize_t bytes;
    while (bytes = recv(serveraccept, buff, BUFFER_SIZE - 1, 0)){
        buff[bytes] = '\0';
        printf("Client says: %s\n", buff);
    } 
    printf("\nClient Disconnected!\n");

    close(serveraccept);
    close(serversocket);
    return 0;
}