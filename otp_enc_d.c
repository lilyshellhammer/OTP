#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

struct cipher{
	char *text;
	char *key;
	char *code;
};

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void checkValid(struct cipher *c){
	int i;
	for(i = 0; i < strlen(c->text); i++){
		if(c->text[i] > 'Z' || (c->text[i] < 'A' && c->text[i]!= ' ')){
			printf("otp_enc_d error: input contains bad characters\n");
			exit(1);
		}
	}
}

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void encode(struct cipher *c){
	int i, j, math, letter, key;
	c->code = malloc(strlen(c->text)*sizeof(char));
	memset(c->code, '\0', sizeof(c->code));
	char alpha[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	for(i = 0; i < strlen(c->text); i++){
		for(j = 0; j < 27; j++){
			if(c->text[i] == alpha[j])
				letter = j;
			if(c->text[i] == alpha[j])
				key = j;
		}
		math = (letter + key) %27;
		c->code[i] = alpha[math];
	}
	printf("%s\n",c->code);
}

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void getTextKey(int establishedConnectionFD, struct cipher *c){
	int charsWritten, charsRead;
	char buffer[1000];

	/*****FIRST GET PLAINTEXT*/
	memset(buffer, '\0', sizeof(buffer));
	charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); // Read the client's message from the socket
	if (charsRead < 0) 
		error("ERROR reading from socket");
	/*printf("ENC DEAMON: Recieved plaintext: \"%s\"\n", buffer);*/

	c->text = malloc(sizeof(buffer)*sizeof(char));
	strcpy(c->text, buffer);

	/*PAUSE TO SAY RECIEVED*/
	charsRead = send(establishedConnectionFD, "0", 1, 0); // Send success back
	if (charsRead < 0) error("ENC: ERROR writing to socket");
	if (charsRead < 1) printf("ENC: WARNING: Not all data written to socket!\n");
	
	/******GET KEY*/
	memset(buffer, '\0', sizeof(buffer));
	charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); // Read the client's message from the socket
	if (charsRead < 0) 
		error("ERROR reading from socket");
	/*printf("ENC DEA: Received key: \"%s\"\n", buffer);*/

	c->key = malloc(sizeof(buffer)*sizeof(char));
	strcpy(c->key, buffer);
	
	// Send a Success message back to the client
	charsRead = send(establishedConnectionFD, "ENC DEA: done", 21, 0); // Send success back
	if (charsRead < 0) error("ERROR writing to socket");
}

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void spawning(struct cipher *c, int establishedConnectionFD){
	int charsWritten;
	pid_t spawnPid;
	spawnPid = fork();
	int childExitMethod = -5;
	if (spawnPid == -1) 
	{
		perror("Hull Breach!\n");
		exit(1);
	}
	else if (spawnPid == 0) /*SPAWN CODE*/
	{
		//printf("SPAWN CODE\n");
		/*DO ACTUAL ENCODING, SAVE VALUES*/
		checkValid(c);
		encode(c);
		charsWritten = send(establishedConnectionFD, c->code, strlen(c->code), 0); // Write KEY to the server
		if (charsWritten < 0) error("ENC: ERROR writing to socket");
		if (charsWritten < strlen(c->code)) printf("ENC: WARNING: Not all data written to socket!\n");
		exit(0);
	}

	pid_t childPID = waitpid(spawnPid, &childExitMethod, 0); 
}

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void createSocketBegin(int argc, char** argv, int* listenSocketFD, struct sockaddr_in *serverAddress, struct sockaddr_in *clientAddress){
	int portNumber;
	char buffer[256];
	if (argc < 2) { 
		fprintf(stderr,"USAGE: %s port\n", argv[0]); 
		exit(1); 
	} // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)serverAddress, '\0', sizeof(*serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress->sin_family = AF_INET; // Create a network-capable socket
	serverAddress->sin_port = htons(portNumber); // Store the port number
	serverAddress->sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
	// Set up the socket
	*listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket

	if (*listenSocketFD < 0) 
		error("ERROR opening socket");
	// Enable the socket to begin listening
	if (bind(*listenSocketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
}

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void firstContact(int establishedConnectionFD){
	/*RECIEVE TYPE*/
	int charsWritten;
	char buffer[10];
	memset(buffer, '\0', sizeof(buffer));
	int charsRead = recv(establishedConnectionFD, buffer, 9, 0); // Read the client's message from the socket
	if (charsRead < 0) 
		error("ERROR reading from socket");
	if(buffer[0] == 'd'){
		/*IF WRONG, SEND ENDING MESSAGE THEN EXIT*/
		printf("ERROR: cannot connec to encoding servers! Exiting.\n");
		charsWritten = send(establishedConnectionFD, "X", 1, 0); // Write TEXT to the server
	}
	else{
		charsWritten = send(establishedConnectionFD, "0", 1, 0); // Write TEXT to the server
	}
	if (charsWritten < 0) error("ENC: ERROR writing to socket");
	if (charsWritten < 1) printf("ENC: WARNING: Not all CODE data written to socket!\n");
	
	/*EXIT IF BAD CONNECTION*/
	if(buffer[0] == 'd') { exit(1); }
}

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void checkLength(struct cipher *c){
	if(strlen(c->key) < strlen(c->text)){
		printf("ERROR: key shorter than message. Enter longer key\n");
		exit(1);
	}
}


/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	/*START SOCKET TO BEGIN LISTENING*/
	createSocketBegin(argc, argv, &listenSocketFD, &serverAddress, &clientAddress);
	int i = 0;
	do{
		i++;
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	
	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0) error("ERROR on accept");
	
	firstContact(establishedConnectionFD);
	struct cipher c;
	getTextKey(establishedConnectionFD, &c);
	checkLength(&c);
	
	/*OK NOW WE HAVE CODE AND KEY*/
	spawning(&c, establishedConnectionFD);
	}while(i < 5);

	close(establishedConnectionFD); // Close the existing socket which is connected to the client
	close(listenSocketFD); // Close the listening socket
	return 0;
}
