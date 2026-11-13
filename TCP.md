TCP commands:

Socket: socket(int domain, int type, int protocol);
    -ex: server_fd = socket(AF_INET, SOCK_STREAM, 0);
    - AFINET: Using ipv4 family, using AFINET6 uses ipv6, AF_UNIX is local communication between processes
    -SOCK_STREAM: Requests stream packet
    -AF_INET with protocol 0, gives TCP socket

bind(server_fd, (struct sockaddr *)&address, sizeof(address)); - Associates local address with socket
 -Declaration: int bind(int sockfd,
         const struct sockaddr *addr,
         socklen_t addrlen);x

ssize_t recv(int sockfd,
             void *buf,
             size_t len,
             int flags);

int connect(int sockfd,
            const struct sockaddr *addr,
            socklen_t addrlen);
These are the general steps for establishing a connection through a passive open:
    Obtain a socket descriptor, used for listening.
    Bind a local port to the socket descriptor.
    Start to listen on that socket descriptor.
    Wait for connections, and accept them.