#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close



#define FAIL -1
#define SUCCESS 1
#define ACK "ACK"
#define NACK "NACK"

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
 
    // the destination is returned by standard `strcpy_1_1()`
    return ptr;
}
long long strcmp_1(const char *X, const char *Y)
{
    while (*X)
    {
        // if characters differ, or end of the second string is reached
        if (*X != *Y) {
            break;
        }
 
        // move to the next pair of characters
        X++;
        Y++;
    }
 
    // return the ASCII difference after converting `char*` to `unsigned char*`
    return *(const unsigned char*)X - *(const unsigned char*)Y;
}
long long max(long long a, long long b){
    if(a>b){
        return a;
    }else{
        return b;
    }
}
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

char* find_remainder(char *mod_message, char *crc_key){
    char temp[30], quotient[100], remainder[30], temp_key[30];
    long long i, j, key_length = strlen_1(crc_key), message_length = strlen_1(mod_message);
    strcpy_1(temp_key, crc_key);
    // add zeroes to the end of message
    for (i = 0; i < key_length - 1; i++)
    {
        mod_message[message_length + i] = '0';
    }
    // crc division algorithm
    for (i = 0; i < key_length; i++)
    {
        temp[i] = mod_message[i];
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
        remainder[key_length - 1] = mod_message[i + key_length];
        strcpy_1(temp, remainder);
    }
    strcpy_1(remainder, temp);
    return remainder;
}



void append_remainder1(char *mod_message, char *crc_key)
{
    // char temp[30], quotient[100], remainder[30], temp_key[30];
    long long key_length = strlen_1(crc_key), message_length = strlen_1(mod_message);
    
    char* remainder=find_remainder(mod_message, crc_key);
    for (long long i = 0; i < key_length - 1; i++)
    {
        mod_message[message_length + i] = remainder[i];
    }
    mod_message[message_length + key_length - 1] = '\0';
}



void convert_to_binary(const char *message, char *mod_message)
{
    unsigned long long i;
    long long j;
    for (i = 0; i < strlen_1(message); i++)
    {
        long long c = message[i];
        for (j = 7; j >= 0; j--)
        {
            if (c & 1)
            {
                mod_message[(i * 8) + j] = '1';
            }
            else
            {
                mod_message[(i * 8) + j] = '0';
            }
            c = c >> 1;
        }
    }
    mod_message[i * 8] = '\0';
}

void add_error1(char *mod_message, float BER)
{
    unsigned long long i;
    for (i = 0; i < strlen_1(mod_message); i++)
    {
        
        // randomly flip bits in the message
        if ((float)rand() / RAND_MAX < BER)
        {
            mod_message[i]=mod_message[i]^1;
        }
    }
}

void add_error(char *mod_message, float BER){
    return add_error1(mod_message, BER);
}


void append_remainder(char *mod_message, char *crc_key){
    return append_remainder1(mod_message, crc_key);
}


void add_error_to_the_received_message(const char *message, char *mod_message, float BER)
{
    // we get the crc key as 100000111 from the given generator polynomial x^8+x^2+x+1
    char crc_key[20] = "100000111";
    convert_to_binary(message, mod_message);
    
    append_remainder(mod_message, crc_key);
    add_error(mod_message, BER);
    //printf("Tampered form: %s\n", mod_message);
}

long long communicate1(long long client_socket, char *mod_message)
{
    // send to the socket
    if (send(client_socket, mod_message, strlen_1(mod_message), 0) < 0)
    {
        fprintf(stderr, "Sent error...\n");
        return FAIL;
    }
    printf("Sent message. Waiting for ACK/NACK...\n");

    long long reply_length;
    char reply[64];
    // receive from the socket
    if ((reply_length = recv(client_socket, reply, 64, 0)) < 0)
    {
        fprintf(stderr, "Timeout. Re-transmitting...\n");
        return FAIL;
    }
    reply[reply_length] = '\0';
    printf("Reply received: %s\n", reply);
    if (strcmp_1(reply, NACK) == 0)
    {
        fprintf(stderr, "Previous transmission had some error. Re-transmitting...\n");
        return FAIL;
    }
    else if (strcmp_1(reply, ACK) == 0)
    {
        return SUCCESS;
    }
    else
    {
        fprintf(stderr, "Error in ACK and NACK\n");
        return FAIL;
    }
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
        if (base > INT_MAX / 10
            || (base == INT_MAX / 10
            && str[i] - '0' > 7))
        {
            if (sign == 1)
                return INT_MAX;
            else
                return INT_MIN;
        }
        base = 10 * base + (str[i++] - '0');
    }
    return base * sign;
}
long long create_and_initialize_client_socket(long long port, char *address, long long timeout)
{
    long long client_socket; // client fd
    // create a new socket witch AF_INET for internet domain, stream socket option, TCP(given by os) - reliable, connection oriented
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation: ");
        return FAIL;
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(address);
    bzero(&server.sin_zero, 8); // clears the buffer
    server.sin_family = AF_INET;
    // convert a port number in host byte order to a port number in network byte order
    server.sin_port = htons(port);

    struct timeval time_val;
    time_val.tv_sec = timeout;  // timeout
    time_val.tv_usec = 0; // not initialising this can cause strange errors

    // set socket option for timeout
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_val, sizeof(struct timeval));

    // connect the socket referred by the fd to the address specified by socket address
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Connecting: ");
        return FAIL;
    }
    return client_socket;
}
long long communicate(long long client_socket, char *mod_message){
    return communicate1(client_socket,mod_message);
}
long long main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Please provide the input as ./client <ip address> <port number> and try again.\n Terminating the process...");
        fprintf(stderr, "Error in the command format\n");
        printf("Process Terminated");
        //exiting with a failure
        return EXIT_FAILURE;
    }

    char str[] = "checking and testing";
    func2(str);

    float BER;
    printf("Enter BER (probability of bit errors): ");
    scanf("%f", &BER);

    long long val[] = { 60, 100, 120 };
    long long wt[] = { 10, 20, 30 };
    long long W = 50;
    long long n = sizeof(val) / sizeof(val[0]);
    func1(W, wt, val, n);


    long long timeout;
    printf("Give the value of timeout for the client: ");
    scanf("%d", &timeout);
    
    long long client_socket=create_and_initialize_client_socket(char_to_int(argv[2]), argv[1], timeout);
    if (!client_socket)
    {
        return EXIT_FAILURE;
    }
    srand(NULL);
    while (1)
    {
        char message[128];
        printf("Message to be sent to the server: ");
        scanf("%s", message);
        char mod_message[2048];
        do
        {
            add_error_to_the_received_message(message, mod_message, BER);
        } while (communicate(client_socket, mod_message) == FAIL);
    }
    close(client_socket);
    return EXIT_SUCCESS;
}