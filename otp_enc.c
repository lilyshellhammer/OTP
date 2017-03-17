#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

struct cipher{
	char *text;
	char *key;
};

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void saveCipherInfo(char** argv, struct cipher *c){
	int file_descriptor_text, file_descriptor_key, nread;
	char readBuffer[256];
	char buffer[256];

	/****GET TEXT*/
	file_descriptor_text = open(argv[1], O_RDONLY);
	if (file_descriptor_text == -1) 
	{ printf("Hull breach - open() failed on \"%s\"\n", argv[1]); perror("In main()");
		exit(1); 
	}
	memset(readBuffer, '\0', sizeof(readBuffer)); // Clear out the array before using it 
	lseek(file_descriptor_text, 0, SEEK_SET); // Reset the file pointer to the beginning of the file 
	nread = read(file_descriptor_text, readBuffer, sizeof(readBuffer));
	close(file_descriptor_text);
	/***GET KEY*/
	file_descriptor_key = open(argv[2], O_RDONLY);
	if (file_descriptor_key == -1) 
	{ printf("Hull breach - open() failed on \"%s\"\n", argv[2]); perror("In main()");
		exit(1); 
	}
	memset(buffer, '\0', sizeof(buffer)); // Clear out the array before using it 
	lseek(file_descriptor_key, 0, SEEK_SET); // Reset the file pointer to the beginning of the file 
	nread = read(file_descriptor_key, buffer, sizeof(buffer));
	close(file_descriptor_key);
	/*get rid of newlines*/
	readBuffer[strcspn(readBuffer, "\n")] = '\0';
	buffer[strcspn(buffer, "\n")] = '\0';

	/*****COPY OVER TO CIPHER STRUCT*/
	c->text = malloc(strlen(readBuffer)*sizeof(char));
	strcpy(c->text, readBuffer);
	c->key = malloc(strlen(buffer)*sizeof(char));
	strcpy(c->key, buffer);
}


/*****************************************************************
* Function name: catchSIGINT
* Description: 
* Input: 
* Output: 
*****************************************************************/
void sendTextKey(int socketFD, struct cipher *c){
	int charsWritten, charsRead;

	//SEND TEXT
	charsWritten = send(socketFD, c->text, strlen(c->text), 0); // Write TEXT to the server
	if (charsWritten < 0) error("ENC: ERROR writing to socket");
	if (charsWritten < strlen(c->text)) printf("ENC: WARNING: Not all data written to socket!\n");
	
	//interim, "check" from server
	char readBuffer[256];
	memset(readBuffer, '\0', sizeof(readBuffer));
	charsRead = recv(socketFD, readBuffer, sizeof(readBuffer) - 1, 0);
	if (charsRead < 0) 
		error("ERROR reading from socket");
	if(readBuffer[0] == '0')
		printf("moving on\n");

	//SEND KEY
	charsWritten = send(socketFD, c->key, strlen(c->key), 0); // Write KEY to the server
	if (charsWritten < 0) error("ENC: ERROR writing to socket");
	if (charsWritten < strlen(c->key)) printf("ENC: WARNING: Not all data written to socket!\n");
	
	// Get return message from server
	memset(readBuffer, '\0', sizeof(readBuffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, readBuffer, sizeof(readBuffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("ENC: ERROR reading from socket");
	printf("Recieved from daemon: \"%s\"\n", readBuffer);
}


void beginConnect(int argc, char** argv, int* socketFD){
	int portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	ssize_t nread;
	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "ENC: ERROR, no such host\n"); 
		exit(0); 
	}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
	// Copy in the address

	// Set up the socket
	*socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (*socketFD < 0) error("ENC: ERROR opening socket");

	// Connect to server
	if (connect(*socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to addy
	error("ENC: ERROR connecting");

}

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void readEncoded(int socketFD){
	int charsRead;
	char readBuffer[256];
	memset(readBuffer, '\0', sizeof(readBuffer));
	charsRead = recv(socketFD, readBuffer, sizeof(readBuffer) - 1, 0);
	if (charsRead < 0) 
		error("ERROR reading from socket");
	/*printf("Receiving from daemon, encoded message is:\n");*/
	printf("%s\n", readBuffer);
}

/*****************************************************************
* Function name: 
* Description: 
* Input: 
* Output: 
*****************************************************************/
void firstContact(int socketFD){
	/*SEND TYPE*/
	int charsWritten = send(socketFD, "e", 1, 0); // Write TEXT to the server
	if (charsWritten < 0) error("ENC: ERROR writing to socket");
	if (charsWritten < 1) printf("ENC: WARNING: Not all CODE data written to socket!\n");
	
	/*RECIEVE YES OR NO*/
	char buffer[10];
	memset(buffer, '\0', sizeof(buffer));
	int charsRead = recv(socketFD, buffer, 9, 0); // Read the client's message from the socket
	if (charsRead < 0) 
		error("ERROR reading from socket");
	if(buffer[0] == 'X'){
		printf("ERROR: cannot connec to decoding servers! Exiting.\n");
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
	int socketFD;
	beginConnect(argc, argv, &socketFD);

	firstContact(socketFD);
	/*AFTER SETTING UP SERVER AND CLIENT CONNECTION, ACCEPT DATA AND SEND IT*/
	struct cipher c;
	saveCipherInfo(argv, &c);
	sendTextKey(socketFD, &c);

	readEncoded(socketFD);

	close(socketFD); // Close the socket
	return 0;
}
