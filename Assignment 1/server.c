// including necessary header files for socket creation and other tasks using the socket api
#define INT_MIN (-__INT_MAX__ -1)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <zconf.h>
#include <math.h>


#define FAIL 0
#define SUCCESS 1
#define ACK "ACK"
#define NACK "NACK"

// global so we can close them from the signal handler
long long server_socket, client_socket;

unsigned long long strlen_1(char *s)
{
    unsigned long long count = 0;
    while(*s!='\0')
    {
        count++;
        s++;
    }
    return count;
}


char* strcpy_1(char* destination, const char* source)
{
    // return if no memory is allocated to the destination
    if (destination == NULL) {
        return NULL;
    }
 
    // take a pointer pointing to the beginning of the destination string
    char *ptr = destination;
 
    // copy the C-string pointed by source into the array
    // pointed by destination
    while (*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }
 
    // include the terminating null character
    *destination = '\0';
 
    // the destination is returned by standard `strcpy_1()`
    return ptr;
}


long long check_remainder1(char* remainder, long long key_length){
    for (long long i = 0; i < key_length - 1; i++)
    {
        if (remainder[i] != '0')
        {
            return FAIL;
        }
    }
    return SUCCESS;
}

long long check_remainder(char* remainder, long long key_length){
    return check_remainder1(remainder, key_length);
}

long long check_crc1(char *input, char *crc_key)
{
    char temp[300], quotient[1000], remainder[300], temp_key[300];
    long long i, j, key_length = strlen_1(crc_key), message_length = strlen_1(input);
    strcpy_1(temp_key, crc_key);
    for (i = 0; i< key_length - 1; i++)
    {
        input[message_length + i] = '0';
    }
    for (i = 0; i < key_length; i++)
    {
        temp[i] = input[i];
    }
    for (i = 0; i < message_length; i++)
    {
        quotient[i] = temp[0];
        if (quotient[i] == '0')
        {
            for (j = 0; j < key_length; j++)
            {
                crc_key[j] = '0';
            }
        }
        else
        {
            for (j = 0; j < key_length; j++)
            {
                crc_key[j] = temp_key[j];
            }
        }
        for (j = key_length - 1; j > 0; j--)
        {
            if (temp[j] == crc_key[j])
            {
                remainder[j - 1] = '0';
            }
            else
            {
                remainder[j - 1] = '1';
            }
        }
        remainder[key_length - 1] = input[i + key_length];
        strcpy_1(temp, remainder);
    }
    strcpy_1(remainder, temp);
    return check_remainder(remainder, key_length);
}
long long func2(char* str)
{
    long long n = strlen(str); // calculating size of string
    if (n < 2)
        return n; // if string is empty then size will be 0.
                  // if n==1 then, answer will be 1(single
                  // character will always palindrome)
 
    long long maxLength = 1, start = 0;
    long long low, high;
    for (long long i = 0; i < n; i++) {
        low = i - 1;
        high = i + 1;
        while (high < n
               && str[high] == str[i]) // increment 'high'
            high++;
 
        while (low >= 0
               && str[low] == str[i]) // decrement 'low'
            low--;
 
        while (low >= 0 && high < n
               && str[low] == str[high]) {
            low--; // decrement low
            high++; // increment high
        }
 
        long long length = high - low - 1;
        if (maxLength < length) {
            maxLength = length;
            start = low + 1;
        }
    }
    
    return maxLength;
}
long long check_crc(char *input, char *crc_key){
    return check_crc1(input, crc_key);
}

void decode(char* message, long long data_len){
    for (long long i = 0; i < data_len - 8; i++)
    {
        long long j;
        long long v = 0;
        for (j = i; j < i + 8; j++)
        {
            if (message[j] == '1')
            {
                v += pow(2, i + 7 - j);
            }
        }
        printf("%c", (char)v);
        i = i + 7;
    }
    printf("\n");

}

long long char_to_int(const char* str)
{
    long long sign = 1, base = 0, i = 0;
     
    // if whitespaces then ignore.
    while (str[i] == ' ')
    {
        i++;
    }
     
    // sign of number
    if (str[i] == '-' || str[i] == '+')
    {
        sign = 1 - 2 * (str[i++] == '-');
    }
   
    // checking for valid input
    while (str[i] >= '0' && str[i] <= '9')
    {
        // handling overflow test case
        if (base > __INT_MAX__ / 10
            || (base == __INT_MAX__ / 10
            && str[i] - '0' > 7))
        {
            if (sign == 1)
                return __INT_MAX__;
            else
                return INT_MIN;
        }
        base = 10 * base + (str[i++] - '0');
    }
    return base * sign;
}
long long check_packet_drop1(long long drop_prob){
    if (((float)rand() / RAND_MAX) < drop_prob)
            {
                printf("Oops....packet dropped :( \n");
                return 1;
            }
    return 0;
}
long long check_packet_drop(long long drop_prob){
    return check_packet_drop1(drop_prob);
}
void show_message(char *message, long long data_len)
{
    printf("Message received: ");
    decode(message, data_len);
}


void process_client_request(long long client_socket, float drop_probability)
{
    long long data_len;
    do
    {
        char message[2048];
        // receive from the socket, returns the size of the message sent by the client
        data_len = read(client_socket, message, 2048);
        // \0 is adderd t o mark the end of the character string message
        message[data_len] = '\0';
        // we get the crc key as 100000111 from the given generator polynomial x^8+x^2+x+1
        char crc_key[20] = "100000111";

       // string message_received=message;
        // if crc is check is true, we have the correct data and we send ack else we send nack
        if (data_len && check_crc(message, crc_key) == SUCCESS)
        {
            show_message(message, data_len);
            printf("Sending ACK....");
            if(check_packet_drop(drop_probability)){
                continue;
            }
            // send to the socket
            long long sent = send(client_socket, ACK, strlen_1(ACK), 0);
            printf("Sent successfully!\n");
        }
        else if (data_len)
        {
            show_message(message, data_len);
            printf("Erroneous message received :( sending NACK....");
            
            if(check_packet_drop(drop_probability)){
                continue;
            }
            // send to the socket
            long long sent = send(client_socket, NACK, strlen_1(NACK), 0);
            printf("Sent to socket successfully!\n");
        }
        else
        {
            close(client_socket);
        }
    } while (data_len); // continue till we get proper data from the socket
}
void define_server_struct(long long port_number){
    struct sockaddr_in server;

    //inintializing the sever structure
    server.sin_family = AF_INET;
    server.sin_port = htons(port_number); // convert a port number in host byte order to a port number in network byte order
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 8); // clears the buffer

}

long long max(long long a, long long b)
{
    return (a > b) ? a : b;
}
 
// Returns the maximum value that
// can be put in a func1 of capacity W
long long func1(long long W, long long wt[], long long val[], long long n)
{
    long long i, w;
    long long K[n + 1][W + 1];
 
    // Build table K[][] in bottom up manner
    for (i = 0; i <= n; i++)
    {
        for (w = 0; w <= W; w++)
        {
            if (i == 0 || w == 0)
                K[i][w] = 0;
            else if (wt[i - 1] <= w)
                K[i][w] = max(val[i - 1]
                          + K[i - 1][w - wt[i - 1]],
                          K[i - 1][w]);
            else
                K[i][w] = K[i - 1][w];
        }
    }
 
    return K[n][W];
}

long long create_and_initialize_sever_socket1(long long port_number, long long buff_size)
{
    // create a new socket witch AF_INET for internet domain, stream socket option, TCP(given by os) - reliable, connection oriented
    //server_socket variable takes the value of socket file directory
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        //printinhg the error messaage if there is error in creating the socket
        perror("socket: ");
        return FAIL;
    }

    printf("Created socket...\n");

    //defining the server using the sockaddr_in structure
    struct sockaddr_in server;

    //inintializing the sever structure
    server.sin_family = AF_INET;
    server.sin_port = htons(port_number); // convert a port number in host byte order to a port number in network byte order
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 8); // clears the buffer

    // bind function binds the socket to the address and port number specified in addr
    if ((bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0))
    {
        perror("binding: ");
        return FAIL;
    }
    printf("Binding successful...\n");

    // put the server socket in a passive mode, where it waits for the client to approach the server to make a connection
    // buff_size defines the maximum length to which the queue of pending connections for sockfd may grow
    // if a connection request arrives when the queue is full, the client may receive an error
    if ((listen(server_socket, buff_size)) < 0)
    {
        perror("listening: ");
        return FAIL;
    }
    printf("Listening...\n");
    return SUCCESS;
}
long long create_and_initialize_sever_socket(long long port_number, long long buff_size){
    return create_and_initialize_sever_socket1(port_number, buff_size);
}
//signum=2
void signal_callback(long long signum)
{
    printf("Signal caught: %d.\n Releasing resources...\n", signum);
    close(server_socket);
    printf("server sockect closed");
    close(client_socket);
    printf("client sockect closed");
    printf("............process terminated............");
    exit(signum);
}


long long main(int argc, char **argv)
{
    long long port_number=char_to_int(argv[1]);
    printf("making a server that accepts requests from port: %d ...\n", port_number);
    

    if (argc != 2)
    {
        printf("Please provide the input as ./server <port number> and try again.\n Terminating the process...");
        fprintf(stderr, "Error in the command format\n");
        printf("Process Terminated");
        //exiting with a failure
        return EXIT_FAILURE;
    }

    

    //asiking for the drop probability of the packets
    float drop_probability;
    printf("Drop probability: ");
    scanf("%f", &drop_probability);

    //asking for the buffer size
    long long buff_size;
    printf("The maximum number of client requests that can be served at a time:");
    scanf("%d", &buff_size);

    //debugging
    char str[] = "checking and testing";
    func2(str);

    //creating and initializing the sever socket
    if (!create_and_initialize_sever_socket(port_number,buff_size))
    {
        printf("wasn't able to create the server socket with port numnber %d", port_number);
        //exiting with a failure
        return EXIT_FAILURE;
    }
    

    //makes use of the computer's internal clock to control the choice of the seed
    srand(NULL);


    long long val[] = { 60, 100, 120 };
    long long wt[] = { 10, 20, 30 };
    long long W = 50;
    long long n = sizeof(val) / sizeof(val[0]);
    func1(W, wt, val, n);

    // signalling the interrupt in case ctrl+c is hit to deallocate all the resources used and releases the open socket
    signal(SIGINT, signal_callback);


    for(;;)
    {

        // creating the client 
        struct sockaddr_in client;
        unsigned long long len;
        
        // extract the first connection request on the queue of pending connections for the listening socket
        // creates a new connected socket, and returns a new file descriptor referring to that socket
        // connection is established between client and server, and they are ready to transfer data

        //accept function extracts the first connection on the queue of pending connections on socket s. It then creates and returns a handle to the new socket.The accept() function shall extract the first connection on the queue of pending connections, create a new socket with the same socket type protocol and address family as the specified socket, and allocate a new file descriptor for that socket.
        long long client_socket = accept(server_socket, (struct sockaddr *)&client, &len);
        if (client_socket < 0)
        {
            perror("Accepting: ");
            return EXIT_FAILURE;
        }
        printf("Accepted...\n");

        long long pid;
        if ((pid = fork()) < 0)
        {
            perror("forking: ");
            return EXIT_FAILURE;
        }
        if (pid == 0)
        {
            // child process closes the old socket and works with the new one
            close(server_socket);
            process_client_request(client_socket, drop_probability);
            printf("Done...You can terminate the server now!\n");

            // after working with the new socket, it simply exits
            return EXIT_SUCCESS;
        }
        else
        {
            // parent process does not need new socket, so it closes it
            // and keeps listening on old socket
            close(client_socket);
        }
    }
}
