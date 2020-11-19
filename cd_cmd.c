/*6 Library*/
#include <stdlib.h>		//malloc(), realloc(), freee(), exit(), execvp()
#include <stdio.h>		//printf(), stderr, getchar(), perror(), popen()
#include <string.h>		//strcmp(), strcpy(), strcat(), strtok()
#include <sys/wait.h>		//waitpid()
#include <unistd.h>		//getcwd(), chdir(), fork(), exec(), pid_t
#include <limits.h>		//PATH_MAX, CHAR_MAX

/*Define 11 Marcos*/
#define KEY  0			//first argument from user	- cd(), available_cmd(), validation(), main()
#define ARGV 1			//second argument from user	- cd()
#define OVER 2			//third argument from user	- cd()

//For statement comparing true or false
#define TRUE  0			
#define FALSE 1

//Function keywords - validation()
#define EXIT "exit"
#define CD   "cd" 

//Infinity loop - get_cmd()
#define INFINITY for( ; ; )

//For strtok() to split the word - cmd_arr()
#define SPACE   " "
#define NEWLINE "\n"

//Available commands to those found in the /usr/bin directory
#define CMDBIN  "cd /usr/bin && ls"

/*6 Functions*/
char *get_cmd(void);			//Called from main() - input in string
char **cmd_arr(char *input);	//Called from main() - args to array
int validation(char **args);	//Called from main() - status [0:Terminate/1:Continue]

int cd(char **args);			//Called from validation() - cmd key is 'cd'
char *user_path();				//Called from cd() - user enter 'cd' only
int available_cmd(char **args);	//Called from validation() - cmd key in usr/bin

/*Global variable*/
char cwd[PATH_MAX];		//User current path


int main(int argc, char **argv)
{
	char *input, **args; //get user input
	int status;			 //control the loop
	pid_t pid;			 //process identification store to represent process id

	system("clear");	 //clear the screen

						 // begin the simple command shell program with do-while loop
	do {
		int stat;
		if (getcwd(cwd, sizeof(cwd)) != NULL) {		//if current path is not null

			printf("%s >>", cwd);		//print the current path
			input = get_cmd();			//get the user input eg: cd ..
			args = cmd_arr(input);		//get the user input to array eg: |cd| |..|
			status = validation(args);	//validate the user input and execute it

										//Use fork and exec to spawn a new child process to perform that command
			pid = fork();				//Run two process in same time
			signal(SIGINT, SIG_DFL);	//Handle the signal interupt by user [^C]
			if (pid == TRUE) {			//Child process will be 0
				exit(execvp(args[KEY], args));
			}
			else {//Parent process will be any positive number
				signal(SIGINT, SIG_IGN);	//Ignore the signal interupt by user
				wait(&stat);				//Wait the parent process completed
				WIFEXITED(stat);			//Child exited normally = 1
			}
			//Both process terminated

			//free memory block in both pointers
			free(input);
			free(args);
		}
		else {		//get_cwd() error handling
			perror("Expected current path");
			return EXIT_FAILURE;
		}

	} while (status);

	printf("\n-cd_cmd finished\n\n");
	return EXIT_SUCCESS;
	//end of program
}

char *get_cmd(void) //Called from main() - input
{
	int size = CHAR_MAX;	//allocate CHAR_MAX, 127
	int count = 0;			//counter
	int user_input;

	char *input_buffer = malloc(size * sizeof(char)); //set aside the requested memory [ size * sizeof(char),1]

	//malloc() error handling
	if (input_buffer == NULL) {
		printf("Memory allocation failed\n\n");
		exit(EXIT_FAILURE);
	}

	INFINITY{ //loop user input
		user_input = getchar(); //get an unsigned char [0-255]
	if (user_input == '\n' || user_input == EOF) {	//return user input when user press enter '\n' or EOF
		input_buffer[count] = '\0';	//assign end of the char to null
		return input_buffer;		//return string eg: input = "cd ..";
	}
	else {
		input_buffer[count] = user_input; //allocate every single character into input_buffer
	}
	count++; //address to next address

	if (count >= size) { //reach maximum size from user input

		//increase the size of input_buffer
		size += CHAR_MAX;
		input_buffer = realloc(input_buffer, size);	//set aside the requested memory [ size *input_buffer]
		//realloc() error handling
		if (input_buffer == NULL) {
			printf("Memory allocation failed\n\n");
			exit(EXIT_FAILURE); // terminate the program
		}
	}
	}
}

char **cmd_arr(char *input)	//Called from main() - args to array
{
	int size = CHAR_MAX;	//allocate CHAR_MAX, 127
	int count = 0;			//counter
	char *argv;
	char **args = malloc(size * sizeof(char*));	//set aside the requested memory [ size * sizeof(char),1]	

	//malloc() error handling
	if (!args) {
		printf("Memory allocation failed\n\n");
		exit(EXIT_FAILURE);
	}

	argv = strtok(input, SPACE);//break string by space
	while (argv != NULL) {		//not null = '\0'
		args[count] = argv;		//args[] allocate word from user command
		count++;

		if (count >= size) {	//reach maximum size from each words
			size += CHAR_MAX;	
			args = realloc(args, size * sizeof(char*));//set aside the requested memory [ size *sizeof(char*)]
			//realloc() error handling
			if (!args) {
				printf("Memory allocation failed\n\n"); 
				exit(EXIT_FAILURE); 
			}
		}

		argv = strtok(NULL, SPACE); //break NULL by space
	}
	args[count] = NULL;	//last address set as null
	return args;	//return array eg: args =[cd,...];
}

int validation(char **args)	//Called from main() - status [0:Terminate/1:Continue]
{
	if (args[KEY] == NULL) {	//No cmd key
		return FALSE;
	}
	if (strcmp(args[KEY], EXIT) == TRUE) {	//cmd key is 'exit'
		return TRUE;
	}
	if (strcmp(args[KEY], CD) == TRUE) {	//cmd key is 'cd'
		cd(args);	//call cd() to execute the cmd
	}
	else if (!available_cmd(args)) {		//call available_cmd() to verify cmd key in usr/bin (1)
		printf("-cd_cmd: %s command not found\n", args[KEY]);
	}

	return FALSE;
}

int cd(char **args)	//Called from validation() - cmd key is 'cd'
{
	if (args[OVER] != NULL) {	//Arguments more than 1: error handling
		printf("-cd_cmd: %s: too many arguments\n", args[KEY]);
	}
	else if (args[ARGV] != NULL) {	//Arguments available
		if (chdir(args[ARGV]) != TRUE) { //chdir() error handling
			perror("-cd_cmd: cd");
		}
	}
	else {
		//Set 'cd' only to user directory
		args[ARGV] = user_path();	//call user_path()
		if (chdir(args[ARGV]) != TRUE) { //chdir() error handling
			perror("-cd_cmd: cd");
		}

	}
	return EXIT_FAILURE; //1
}

char *user_path() {	//Called from cd() - user enter 'cd' only
	char *name;			//char pointer
	name = getlogin();	//get logged in username
	char *user_path = malloc(CHAR_MAX * sizeof(char));	//set aside the requested memory [ size * sizeof(char),1]
	if (user_path == NULL) { //malloc success or not
		printf("Memory allocation failed\n\n"); //
		exit(EXIT_FAILURE); // exit
	}
	//declare user path combine with username
	strcpy(user_path, "/nfs/pihome/");
	strcat(user_path, name);
	strcat(user_path, "/");
	return user_path;
}

int available_cmd(char **args) {//Called from validation() - cmd key in usr/bin
	FILE *valid;				//For file
	char valid_cmd[CHAR_MAX];	//Store valid key in /bin
	int invalid = TRUE;			//Check valid or not
	char *argv;					//Store key in right format by strtok() break by New line

	valid = popen(CMDBIN, "r");	//Start another process "cd /usr/bin && ls" and read

	if (valid == NULL) {		//popen() error handling
		printf("Failed to run command\n");
		exit(EXIT_FAILURE);
	}
	//Read the output screen - 'ls'
	while (fgets(valid_cmd, sizeof(valid_cmd), valid) != NULL) { //Read the line from stream and store into valid_cmd
		argv = strtok(valid_cmd, NEWLINE);		//Break the key value from /bin by New line
		if (strcmp(args[KEY], argv) == TRUE) {	//Compare is user input key and key in /bin
			invalid = FALSE;					//Found invalid to false = 1
			break;								//Break the loop
		}
	}

	pclose(valid); // close the file
	
	//Allocate user back to previous path
	if (chdir(cwd) != TRUE) {	//chdir() error handling
		perror("-cd_cmd: ");
	}
	if (!invalid) {	 //if false(1) go here VALID
		return TRUE; //0
	}
	else {	//INVALID
		return FALSE; //1
	}
	
}
