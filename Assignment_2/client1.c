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

char* find_remainder(char *mod_message, char *crc_key){
    char temp[30], quotient[100], remainder[30], temp_key[30];
    int i, j, key_length = strlen(crc_key), message_length = strlen(mod_message);
    strcpy(temp_key, crc_key);
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
        strcpy(temp, remainder);
    }
    strcpy(remainder, temp);
    return remainder;
}



void append_remainder(char *mod_message, char *crc_key)
{
    // char temp[30], quotient[100], remainder[30], temp_key[30];
    int key_length = strlen(crc_key), message_length = strlen(mod_message);
    
    char* remainder=find_remainder(mod_message, crc_key);
    for (int i = 0; i < key_length - 1; i++)
    {
        mod_message[message_length + i] = remainder[i];
    }
    mod_message[message_length + key_length - 1] = '\0';
}



void convert_to_binary(const char *message, char *mod_message)
{
    unsigned int i;
    int j;
    for (i = 0; i < strlen(message); i++)
    {
        int c = message[i];
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

void add_error(char *mod_message, float BER)
{
    unsigned int i;
    for (i = 0; i < strlen(mod_message); i++)
    {
        
        // randomly flip bits in the message
        if ((float)rand() / RAND_MAX < BER)
        {
            mod_message[i]=mod_message[i]^1;
        }
    }
}

void message_transform(const char *message, char *mod_message, float BER)
{
    // we get the crc key as 100000111 from the given generator polynomial x^8+x^2+x+1
    char crc_key[20] = "100000111";
    convert_to_binary(message, mod_message);
    
    append_remainder(mod_message, crc_key);
    add_error(mod_message, BER);
    //printf("Tampered form: %s\n", mod_message);
}

int communicate(int client_socket, char *mod_message)
{
    // send to the socket
    if (send(client_socket, mod_message, strlen(mod_message), 0) < 0)
    {
        fprintf(stderr, "Sent error...\n");
        return FAIL;
    }
    printf("Sent message. Waiting for ACK/NACK...\n");

    int reply_length;
    char reply[64];
    // receive from the socket
    if ((reply_length = recv(client_socket, reply, 64, 0)) < 0)
    {
        fprintf(stderr, "Timeout. Re-transmitting...\n");
        return FAIL;
    }
    reply[reply_length] = '\0';
    printf("Reply received: %s\n", reply);
    if (strcmp(reply, NACK) == 0)
    {
        fprintf(stderr, "Previous transmission had some error. Re-transmitting...\n");
        return FAIL;
    }
    else if (strcmp(reply, ACK) == 0)
    {
        return SUCCESS;
    }
    else
    {
        fprintf(stderr, "Error in ACK and NACK\n");
        return FAIL;
    }
}

int create_and_initialize_client_socket(int port, char *address, int timeout)
{
    int client_socket; // client fd
    // create a new socket witch AF_INET for internet domain, stream socket option, TCP(given by os) - reliable, connection oriented
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation: ");
        return FAIL;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
     // convert a port number in host byte order to a port number in network byte order
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address);
    bzero(&server.sin_zero, 8); // clears the buffer

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

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Please provide the input as ./client <ip address> <port number> and try again.\n Terminating the process...");
        fprintf(stderr, "Error in the command format\n");
        printf("Process Terminated");
        //exiting with a failure
        return EXIT_FAILURE;
    }

    float BER;
    printf("Enter BER (probability of bit errors): ");
    scanf("%f", &BER);
    int timeout;
    printf("Give the value of timeout for the client: ");
    scanf("%d", &timeout);
    
    int client_socket=create_and_initialize_client_socket(atoi(argv[2]), argv[1], timeout);
    if (!client_socket)
    {
        return EXIT_FAILURE;
    }
    srand(NULL);
    while (1)
    {
        char message[64];
        printf("Enter your message: ");
        scanf("%s", message);
        char mod_message[1024];
        do
        {
            message_transform(message, mod_message, BER);
        } while (communicate(client_socket, mod_message) == FAIL);
    }
    close(client_socket);
    return EXIT_SUCCESS;
}