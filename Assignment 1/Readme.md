2_________________________________________
CS 342 Assignment 2
Group 17
######################################
Team members:
Mansi(200101064)
Mihir Sagar(200101065)
Minali Manhar(200101066)
Mohit Kumar(200101067)
######################################
Here, I have created a makefile to make compilation and running of the code more efficient and easy. 
Run the following command to run the makefile on the terminal
make
Then, to run the server, run 
./server [PORT NUMBER]
it would ask you for the following things:
Drop Probability (the probability with which the packets are dropped)
The buffer size (As this is a concurrent server, it will be taking multiple client requests on the same time, so the buffer size indicates the maximum number of client requestes it can keep in the buffer)

Then, the server will wait for a request from the cilent
Then, to activate the cilent, type the following command on a saperate terminal window
./client [IP ADDRESS OF THE SERVER] [PORT NUMBER]
note, this port number must be the same as the one used for the server
Then, on the client side, you will be asked for the following things:
BER(Bit error rate) (the probability with which the orignal message will be changed)
Timeout value (the time in ms for which the client will wait for the ACK/NACK from the server, before declaring timeout)
Message to be sent (it is the message  that will be sent to the server, it should not cross 128 bytes(hard coded) in size)

Then, the message will be sent to the server and the there, the server will check for error in the message, if error is found, it will send back an NACK, and the client will keep sending the message until the correct message is received.
