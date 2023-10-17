// including necessary header files for socket creation and other tasks using the socket api
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
#include <string>
#include <iostream>
#include <math.h>

using namespace std;
#define FAIL 0
#define SUCCESS 1
#define ACK "ACK"
#define NACK "NACK"

// global so we can close them from the signal handler
int server_socket, client_socket;


// bool check_no_remainder(int key_length, string remainder){
//     for (int i = 0; i < key_length - 1; i++)
//     {
//         if (remainder[i] != '0')
//         {
//             return FAIL;
//         }
//     }
//     return SUCCESS;
// }
// int check_crc(string input, string crc_key, int len)
// {
//     string temp="", quotient="", remainder="", temp_key="";
//     int i, j, key_length = crc_key.length(), message_length = len;
//     temp_key=crc_key;
//     for (i = 0; i< key_length - 1; i++)
//     {
//         input[message_length + i] = '0';
//     }
//     for (i = 0; i < key_length; i++)
//     {
//         temp[i] = input[i];
//     }
//     for (i = 0; i < message_length; i++)
//     {
//         quotient[i] = temp[0];
//         if (quotient[i] == '0')
//         {
//             for (j = 0; j < key_length; j++)
//             {
//                 crc_key[j] = '0';
//             }
//         }
//         else
//         {
//             for (j = 0; j < key_length; j++)
//             {
//                 crc_key[j] = temp_key[j];
//             }
//         }
//         for (j = key_length - 1; j > 0; j--)
//         {
//             if (temp[j] == crc_key[j])
//             {
//                 remainder[j - 1] = '0';
//             }
//             else
//             {
//                 remainder[j - 1] = '1';
//             }
//         }
//         remainder[key_length - 1] = input[i + key_length];
//         temp=remainder;
        
//     }
//     // string offset="";
//     // for (i = 0; i< key_length - 1; i++)
//     // {
//     //     offset+="0";
//     // }
//     // // for (i = 0; i< key_length - 1; i++)
//     // // {
//     // //     input[message_length + i] = '0';
//     // // }
//     // input+=offset;
//     // temp=input.substr(0,key_length);
//     // // for (i = 0; i < key_length; i++)
//     // // {
//     // //     temp[i] = input[i];
//     // // }
//     // for (i = 0; i < message_length; i++)
//     // {
//     //     quotient[i] = temp[0];
//     //     if (quotient[i] == '0')
//     //     {
//     //         crc_key=offset+"0";
//     //         // for (j = 0; j < key_length; j++)
//     //         // {
//     //         //     crc_key[j] = '0';
//     //         // }
//     //     }
//     //     else
//     //     {
//     //         crc_key=temp_key;
//     //         // for (j = 0; j < key_length; j++)
//     //         // {
//     //         //     crc_key[j] = temp_key[j];
//     //         // }
//     //     }
//     //     for (j = key_length - 1; j > 0; j--)
//     //     {
//     //         if (temp[j] == crc_key[j])
//     //         {
//     //             remainder[j - 1] = '0';
//     //         }
//     //         else
//     //         {
//     //             remainder[j - 1] = '1';
//     //         }
//     //     }
//     //     remainder[key_length - 1] = input[i + key_length];
//     //     temp=remainder;
//     // }
//     remainder=temp;
//     for (i = 0; i < key_length - 1; i++)
//     {
//         if (remainder[i] != '0')
//         {
//             return FAIL;
//         }
//     }
//     return SUCCESS;
// }
int check_crc(char *input, char *crc_key)
{
    char temp[30], quotient[100], remainder[30], temp_key[30];
    int i, j, key_length = strlen(crc_key), message_length = strlen(input);
    strcpy(temp_key, crc_key);
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
        strcpy(temp, remainder);
    }
    strcpy(remainder, temp);

    // if the remainder is 0, then the input is correct
    for (i = 0; i < key_length - 1; i++)
    {
        if (remainder[i] != '0')
        {
            return FAIL;
        }
    }
    return SUCCESS;
}


string decode(char* message, int data_len){
    unsigned int i;
    string final_message;
    // convert the data from bits to char
    for (i = 0; i < data_len - 8; i++)
    {
        int j;
        int v = 0;
        for (j = i; j < i + 8; j++)
        {
            if (message[j] == '1')
            {
                v += pow(2, i + 7 - j);
            }
        }
        final_message=final_message+(char)v;
        i = i + 7;
    }
    
    return final_message;

}
bool check_packet_drop(int drop_prob){
    if (((float)rand() / RAND_MAX) < drop_prob)
            {
                printf("Packet dropped!\n");
                return true;
            }
    return false;
}
int max(int a, int b);
 
/* Returns length of func1 for X[0..m-1], Y[0..n-1] */
int func1( char *X, char *Y, int m, int n )
{
if (m == 0 || n == 0)
    return 0;
if (X[m-1] == Y[n-1])
    return 1 + func1(X, Y, m-1, n-1);
else
    return max(func1(X, Y, m, n-1), func1(X, Y, m-1, n));
}
 
/* Utility function to get max of 2 integers */
int max(int a, int b)
{
    return (a > b)? a : b;
}
 
void show_message(char *message, int data_len)
{
    printf("Message received: ");
    string final_message=decode(message, data_len);
    int length = final_message.length();
    cout<<final_message<<endl;
    printf("\n");
}
int max(int a, int b) { return (a > b) ? a : b; }
 
// Returns the maximum value that can be
// put in a func2 of capacity W
int func2(int W, int wt[], int val[], int n)
{
    // Base Case
    if (n == 0 || W == 0)
        return 0;
 
    // If weight of the nth item is more than
    // func2 capacity W, then this item cannot
    // be included in the optimal solution
    if (wt[n - 1] > W)
        return func2(W, wt, val, n - 1);
 
    // Return the maximum of two cases:
    // (1) nth item included
    // (2) not included
    else
        return max(
            val[n - 1]
                + func2(W - wt[n - 1],
                           wt, val, n - 1),
            func2(W, wt, val, n - 1));
}
void process_client_request(int client_socket, float drop_probability)
{
    int data_len;
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
            int sent = send(client_socket, ACK, strlen(ACK), 0);
            printf("Sent successfully!\n");
        }
        else if (data_len)
        {
            show_message(message, data_len);
            printf("Message retrieved had some errors, sending NACK....");
            // if (((float)rand() / RAND_MAX) < drop_probability)
            // {
            //     printf("Packet dropped!\n");
            //     continue;
            // }
            if(check_packet_drop(drop_probability)){
                continue;
            }
            // send to the socket
            int sent = send(client_socket, NACK, strlen(NACK), 0);
            printf("Sent successfully!\n");
        }
        else
        {
            close(client_socket);
        }
    } while (data_len); // continue till we get proper data from the socket
}

int create_and_initialize_sever_socket(int port_number, int buff_size)
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

//signum=2
void signal_callback(int signum)
{
    printf("Signal caught: %d.\n Releasing resources...\n", signum);
    close(server_socket);
    printf("server sockect closed");
    close(client_socket);
    printf("client sockect closed");
    printf("............process terminated............");
    exit(signum);
}

void signal_callback1(int signum){
    signal_callback(signum);
}
int main(int argc, char **argv)
{
    int port_number=atoi(argv[1]);
    printf("making a server that accepts requests from port: %d ...\n", port_number);
    

    if (argc != 2)
    {
        printf("Please provide the input as ./server <port number> and try again.\n Terminating the process...");
        fprintf(stderr, "Error in the command format\n");
        printf("Process Terminated");
        //exiting with a failure
        return EXIT_FAILURE;
    }

    char X[] = "AGGTAB";
    char Y[] = "GXTXAYB";
 
    int m = strlen(X);
    int n = strlen(Y);
    func1( X, Y, m, n );
    //asiking for the drop probability of the packets
    float drop_probability;
    printf("Drop probability: ");
    scanf("%f", &drop_probability);
    
    //asking for the buffer size
    int buff_size;
    printf("The maximum number of client requests that can be served at a time:");
    scanf("%d", &buff_size);

    //creating and initializing the sever socket
    if (!create_and_initialize_sever_socket(port_number,buff_size))
    {
        printf("wasn't able to create the server socket with port numnber %d", port_number);
        //exiting with a failure
        return EXIT_FAILURE;
    }
    

    //makes use of the computer's internal clock to control the choice of the seed
    srand(NULL);

    int val[] = { 60, 100, 120 };
    int wt[] = { 10, 20, 30 };
    int W = 50;
    int n = sizeof(val) / sizeof(val[0]);

    func2(W, wt, val, n);
    // signalling the interrupt in case ctrl+c is hit to deallocate all the resources used and releases the open socket
    signal(SIGINT, signal_callback1);


    while (1)
    {

        // creating the client 
        struct sockaddr_in client;
        unsigned int len;
        
        // extract the first connection request on the queue of pending connections for the listening socket
        // creates a new connected socket, and returns a new file descriptor referring to that socket
        // connection is established between client and server, and they are ready to transfer data

        //accept function extracts the first connection on the queue of pending connections on socket s. It then creates and returns a handle to the new socket.The accept() function shall extract the first connection on the queue of pending connections, create a new socket with the same socket type protocol and address family as the specified socket, and allocate a new file descriptor for that socket.
        int client_socket = accept(server_socket, (struct sockaddr *)&client, &len);
        if (client_socket < 0)
        {
            perror("Accepting: ");
            return EXIT_FAILURE;
        }
        printf("Accepted...\n");

        int pid;
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
