#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <getopt.h>

// The maximum length of an HTTP message line
#define MAX_LINE 256
// The maximum length of an HTTP response message
#define MAX_LENGTH 16*1024
// The size of a chunk of HTTP response to read from the pipe
#define CHUNK_SIZE 1024
// The keyword to process request
#define KEYWORD "GET"


void printError(char *);
void printServerError(char *str);
void printResponse(char *str);

int debug = 0;


int main(int argc, char **argv) {
    char msg[MAX_LENGTH];
    //int result;// When is result used?

    FILE *fp = stdin; // default is to read from stdin

    // Parse command line options.
    int opt;
    while((opt = getopt(argc, argv, "v")) != -1) {
        switch(opt) {
            case 'v':
                debug = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [filename]\n", argv[0]);
                exit(1);
        }
    }
    if(optind < argc) {
        if((fp = fopen(argv[optind], "r")) == NULL) {
            perror("fopen");
            exit(1);
        }
    }


    //TODO: Complete the program
    while (*(fgets(msg, MAX_LENGTH,fp))!= '\0'){
        if (strncmp(msg, KEYWORD, strlen(KEYWORD)) == 0){ //Check if msg starts with "GET"
            // Isolate the middle part -> can seperate into a function called sperate1
            char *request = strtok(msg, " ");
            int i = 0;
            char req[MAX_LENGTH];
            while ((i < 2) && (request != NULL)){
                 if (i == 1){
                     strncpy(req, request, MAX_LENGTH);
                 }
                 i++;
                 request = strtok(NULL, " ");
            }
            char path[MAX_LENGTH];
            char query[MAX_LENGTH];
            // If '?' is present, sperate path and query string.
            if (strchr(req, '?') != NULL){
                const char s[2] = "?";
                char *temp;
                temp = strtok(req, s);
                int i = 0;
                while (temp != NULL){
                    if (i == 0){
                        strncpy(path, temp, MAX_LENGTH);
                    }
                    else if (i == 1){
                        strncpy(query, temp, MAX_LENGTH);
                    }
                    i++;
                    temp = strtok(NULL, s);
                }
            }
            else{
                strncpy(path, req, MAX_LENGTH);
                query[0] = '\0';
            }
            // Check if the path exist
            char *check_path = path + 1;
            if (access(check_path, F_OK) != 0){
                 printError(path); //Do I need to exit?
                 exit(1);
            }
            else{
                if (setenv("QUERY_STRING", query, 1) == -1){
                    perror("setenv"); //Do I need to exit? 
                    exit(1);
                }
                int fds[2];
                if (pipe(fds) < 0){
                    perror("pipe");
                    exit(1);
                }
                int n = fork();
                if (n < 0){ //Error Check
                   perror( "fork");
                   exit(1);
                }
                if (n == 0){//Child
                   if (close(fds[0]) == -1){ //close read & error check
                       perror("close");
                       exit(1);
                   }
                   if (dup2(fds[1], 1) == -1){ //redirect stdout to writefd
                       perror("dup2");
                       exit(1);
                   }
                   if (close(fds[1]) == -1){ //Can close writefd, because it is redirected
                       perror("close");
                       exit(1);
                   }
                   char exe[strlen(path) + 1];
                   strcpy(exe, "."); //How do I do the strncopy and strncat more safely?
                   strcat(exe, path);
                   execl(exe, exe, NULL); //Execute the program, Why do I do after I execute the program?
                   perror("execl");
                   exit(1);// Do I need to exit?
                }
                if (n > 0){//Parent
                   if (close(fds[1]) == -1){//close write
                       perror("close");
                       exit(1);
                   }
                   int status;
                   wait(&status);
                   if (!(WIFEXITED(status))){
                       printServerError(path); // Do I need to exit?
                       exit(1);
                   }
                   else{
                       char mystr[MAX_LINE];
                       while (read(fds[0], mystr, CHUNK_SIZE) > 0){ //How do I check the error? 0 = EOF -1 = perror, Should it be chunksize or maxsize??
                           printResponse(mystr);
                       }
                       if (read(fds[0], mystr, CHUNK_SIZE) == -1){
                           perror("read");
                       }
                   }
                //How do I close pipe?
                }

            }
        }
    }

  //close pipe and fork


    if(fp != stdin) {
        if(fclose(fp) == EOF) {
            perror("fclose");
            exit(1);
        }
    }
}


/* Print an http error page  
 * Arguments:
 *    - str is the path to the resource. It does not include the question mark
 * or the query string.
 */
void printError(char *str) {
    printf("HTTP/1.1 404 Not Found\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
    printf("<html><head>\n");
    printf("<title>404 Not Found</title>\n");
    printf("</head><body>\n");
    printf("<h1>Not Found</h1>\n");
    printf("The requested resource %s was not found on this server.\n", str);
    printf("<hr>\n</body></html>\n");
}


/* Prints an HTTP 500 error page 
 */
void printServerError(char *str) {
    printf("HTTP/1.1 500 Internal Server Error\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
    printf("<html><head>\n");
    printf("<title>500 Internal Server Error</title>\n");
    printf("</head><body>\n");
    printf("<h1>Internal Server Error</h1>\n");
    printf("The server encountered an internal error or\n");
    printf("misconfiguration and was unable to complete your request.<p>\n");
    printf("</body></html>\n");
}


/* Prints a successful response message
 * Arguments:
 *    - str is the output of the CGI program
 */
void printResponse(char *str) {
    printf("HTTP/1.1 200 OK\r\n\r\n");
    printf("%s", str);
}
