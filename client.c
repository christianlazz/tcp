
#include <stdio.h>       // printf(), fprintf(), perror()
#include <stdlib.h>      // EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>      // strlen()
#include <unistd.h>      // close()
#include <sys/socket.h>  // socket(), connect(), send(), recv()
#include <netinet/in.h>  // sockaddr_in, htons(), AF_INET
#include <arpa/inet.h>   // inet_pton()

// The TCP port our server is listening on.
// Both client and server must agree on the destination port.
#define PORT 9090

// Maximum size of our receiving buffer, in bytes.
#define BUFFER_SIZE 1024

// 127.0.0.1 is the loopback IP address.
// It refers to this same computer.
//
// Later, you could replace this with another
// computer's IP address.
#define IPADDRESS "127.0.0.1"

int main(void) {

    // ==================================================
    // STEP 1: CREATE A SOCKET
    // ==================================================

    // socket() asks Linux to create a socket object.
    //
    // PARAMETERS:
    //
    // 1. AF_INET
    //    Use the IPv4 address family.
    //
    // 2. SOCK_STREAM
    //    Create a stream-oriented socket.
    //    With AF_INET and protocol 0, this uses TCP.
    //
    //    A TCP stream is an ordered sequence of bytes.
    //    Linux manages retransmissions, acknowledgments,
    //    and other TCP mechanisms for us.
    //
    // 3. 0
    //    Select the default protocol for the chosen
    //    address family and socket type (TCP here).
    //
    // RETURN VALUE:
    //
    // Success: A nonnegative file descriptor (e.g. 3)
    // Failure: -1
    //
    // The descriptor identifies the socket in OUR process.
    // It is NOT an IP address or a TCP port number.
    //
    // No remote connection exists yet.

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        // perror() prints a description of the error
        // recorded by the failed system call.
        perror("socket");
        return EXIT_FAILURE;
    }


    // ==================================================
    // STEP 2: PREPARE THE SERVER'S ADDRESS
    // ==================================================

    // sockaddr_in is a C structure used to represent
    // an IPv4 socket address.
    //
    // It contains fields such as:
    //   sin_family -> IPv4 address family
    //   sin_port   -> TCP/UDP port number
    //   sin_addr   -> IPv4 address
    //
    // {0} initializes all fields to zero.
    //
    // IMPORTANT:
    // This is only a structure in our program's memory.
    // Creating it does not send anything over the network.

    struct sockaddr_in server = {0};


    // Tell the address structure we're using IPv4.
    //
    // This must match the address family used when
    // creating our socket.

    server.sin_family = AF_INET;


    // Specify the destination TCP port.
    //
    // PORT is defined as 9090.
    //
    // htons() means "Host TO Network Short".
    //
    // It converts a 16-bit integer into network byte
    // order (big-endian).
    //
    // The network socket API expects sin_port to be
    // stored in network byte order.
    //
    // Note: This does not bind our client to port 9090.
    // It specifies the REMOTE server's port.

    server.sin_port = htons(PORT);


    // Convert a readable IPv4 address into the binary
    // representation required by sockaddr_in.
    //
    // inet_pton() = Internet Presentation TO Network.
    //
    // PARAMETERS:
    //
    // 1. AF_INET
    //    The address is IPv4.
    //
    // 2. IPADDRESS
    //    The string "127.0.0.1".
    //
    // 3. &server.sin_addr
    //    Pointer to the memory where the converted
    //    binary IP address should be stored.
    //
    // RETURN VALUE:
    //
    //  1 = Address successfully converted
    //  0 = The supplied text isn't a valid address
    // -1 = An error occurred (e.g. invalid address family)
    //
    // This only converts data in memory.
    // It does not connect to the server.

    int result = inet_pton(
        AF_INET,
        IPADDRESS,
        &server.sin_addr
    );

    if (result != 1) {
        fprintf(stderr, "Invalid IP address\n");
        close(sock);
        return EXIT_FAILURE;
    }


    // ==================================================
    // STEP 3: ESTABLISH THE TCP CONNECTION
    // ==================================================

    // connect() asks Linux to establish a connection
    // using our socket and the destination address.
    //
    // PARAMETERS:
    //
    // 1. sock
    //    File descriptor for our TCP socket.
    //    Linux uses it to find the socket object.
    //
    // 2. (struct sockaddr *)&server
    //    Pointer to our destination address.
    //
    //    &server means "address of the server struct".
    //
    //    The cast converts sockaddr_in* into the
    //    generic sockaddr* type expected by connect().
    //
    // 3. sizeof(server)
    //    Number of bytes occupied by the address struct.
    //    Linux needs this to interpret the structure.
    //
    // WHAT HAPPENS INSIDE LINUX:
    //
    // - Linux chooses a suitable local IP address.
    // - Linux usually assigns a temporary client port.
    // - TCP initiates the three-way handshake.
    // - The server's listening socket can receive
    //   this connection request.
    //
    // For a normal blocking socket, connect() returns
    // success once the connection is established.
    //
    // RETURN VALUE:
    //
    //  0 = Connection established
    // -1 = Error

    if (connect(
        sock,
        (struct sockaddr *)&server,
        sizeof(server)
    ) == -1) {

        perror("connect");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Connected to server!\n");


    // ==================================================
    // STEP 4: PREPARE A MESSAGE
    // ==================================================

    // This pointer refers to a string in our program.
    //
    // No network activity happens here.
    //
    // The message is:
    // "Hello Server!"
    //
    // strlen() will report its length as 13 bytes
    // because these are single-byte ASCII characters.
    //
    // strlen() does NOT include the terminating '\0'.

    const char *msg = "Hello Server!";


    // ==================================================
    // STEP 5: SEND THE MESSAGE
    // ==================================================

    // send() gives bytes from our program to Linux
    // for transmission through the connected socket.
    //
    // PARAMETERS:
    //
    // 1. sock
    //    Socket descriptor identifying the connection.
    //
    // 2. msg
    //    Pointer to the first byte to send.
    //
    // 3. strlen(msg)
    //    The number of bytes we're asking to send.
    //    In this case, 13 bytes.
    //
    // 4. 0
    //    No special send flags.
    //
    // WHAT HAPPENS:
    //
    // Linux accepts bytes into the socket's outgoing
    // data path.
    //
    // TCP can group these bytes into TCP segments.
    //
    // TCP headers include information such as:
    // - Source/destination ports
    // - Sequence numbers
    // - Acknowledgment information
    //
    // IP headers contain source/destination IP addresses.
    //
    // Because 127.0.0.1 is a loopback address, this
    // traffic stays in the local networking stack.
    // It does not physically go through Wi-Fi.
    //
    // IMPORTANT:
    // A successful send() does not prove the receiving
    // application has already read the message.
    //
    // RETURN VALUE:
    //
    // Positive number = Bytes accepted for sending
    // -1              = Error
    //
    // send() can accept fewer bytes than requested.
    // We'll handle that by using a loop.

    size_t total_sent = 0;
    size_t message_length = strlen(msg);

    while (total_sent < message_length) {

        ssize_t sent = send(
            sock,
            msg + total_sent,
            message_length - total_sent,
            0
        );

        // msg + total_sent points to the first byte
        // we haven't successfully sent yet.
        //
        // message_length - total_sent is how many
        // bytes still need to be sent.

        if (sent < 0) {
            perror("send");
            close(sock);
            return EXIT_FAILURE;
        }

        if (sent == 0) {
            fprintf(stderr, "No bytes sent\n");
            close(sock);
            return EXIT_FAILURE;
        }

        // Advance by the number of bytes accepted.
        total_sent += (size_t)sent;
    }

    printf("Sent: %s\n", msg);


    // ==================================================
    // STEP 6: CREATE A BUFFER FOR RECEIVING DATA
    // ==================================================

    // This reserves 1024 bytes in our program's memory.
    //
    // recv() will copy incoming bytes into this array.
    //
    // TCP itself does not provide null-terminated
    // C strings. It only provides bytes.

    char buffer[BUFFER_SIZE];


    // ==================================================
    // STEP 7: RECEIVE THE SERVER'S RESPONSE
    // ==================================================

    // recv() reads bytes from the socket's receive buffer.
    //
    // PARAMETERS:
    //
    // 1. sock
    //    Connected socket descriptor.
    //
    // 2. buffer
    //    Pointer to memory where bytes will be copied.
    //
    // 3. BUFFER_SIZE - 1
    //    Receive at most 1023 bytes.
    //
    //    We reserve one byte for the '\0' terminator
    //    so we can print the response as a C string.
    //
    // 4. 0
    //    No special receive flags.
    //
    // WHAT HAPPENS:
    //
    // Linux uses the socket descriptor to identify
    // the TCP connection.
    //
    // Received TCP data is made available in order
    // through the socket's receive buffer.
    //
    // recv() copies available bytes into our array.
    //
    // On a blocking socket, recv() normally waits
    // if no data is currently available.
    //
    // RETURN VALUE:
    //
    // > 0 = Number of bytes received
    //   0 = Peer closed its sending side orderly
    //  -1 = Error
    //
    // TCP IS A BYTE STREAM:
    //
    // One recv() might not receive an entire message.
    // For this demo, we expect a short response and
    // demonstrate one receive operation.

    ssize_t bytes = recv(
        sock,
        buffer,
        BUFFER_SIZE - 1,
        0
    );


    // ==================================================
    // STEP 8: INTERPRET THE RECEIVED BYTES
    // ==================================================

    if (bytes > 0) {

        // recv() did not add a '\0' terminator.
        //
        // We add one so printf("%s") knows where
        // the string ends.
        //
        // This is safe because we received at most
        // BUFFER_SIZE - 1 bytes.

        buffer[bytes] = '\0';

        printf("Server: %s\n", buffer);

    } else if (bytes == 0) {

        printf("Server closed the connection.\n");

    } else {

        perror("recv");
    }


    // ==================================================
    // STEP 9: CLOSE THE SOCKET
    // ==================================================

    // close() releases our file descriptor.
    //
    // PARAMETERS:
    //
    // 1. sock
    //    The file descriptor to close.
    //
    // When the last reference to the socket is closed,
    // Linux handles the appropriate TCP cleanup.
    //
    // This includes normal TCP connection teardown
    // under ordinary socket settings.
    //
    // We cannot keep using sock after closing it.

    close(sock);

    return EXIT_SUCCESS;
}
