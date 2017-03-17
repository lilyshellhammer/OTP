#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char *argv[]) {
	time_t t;
	srand((unsigned) time(&t));

	if(argc != 2){
		printf("ERROR: Usage <exec> <length>\n");
		exit(1);
	}
	/*GET AMNT FROM ARGC*/
	int count = atoi(argv[1]);

	int i;
	int letter;
	for(i = 0; i < count; i++){
		letter = rand() % 27;
		if((char)(letter+65) == '[')
			printf(" ");
		else
			printf("%c", (char)(letter+65));
	}
	printf("\n");

	return 0;
}