#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define MAX 256
#define SA struct sockaddr

int is_log = 0;
char log_filename[MAX];

void logger(char* s){
    if (is_log) {
        FILE* log_file = fopen(log_filename, "a+");
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(log_file, "%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        fprintf(log_file, "%s", s);
        fclose(log_file);
    }
}

void func(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string: ");

        n = 0;
        while ((buff[n++] = getchar()) != '\n');
        
        write(sockfd, buff, sizeof(buff));
        // logger(buff);
        
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}


int main(int argsc, char **argsv)
{
    int sockfd, port = 8080;
    struct sockaddr_in servaddr;

    int argsv_i = 1; // ignore ./filename
    if(argsc > 2){
        while(argsv_i < argsc){
            if (strcmp(argsv[argsv_i], "--port") == 0) {
                port = atoi(argsv[++argsv_i]);
                continue;
            }
            if (strcmp(argsv[argsv_i], "--log") == 0) {
                bzero(log_filename, MAX);
                strcpy(log_filename, argsv[++argsv_i]);
                is_log = 1;
                continue;
            }
            if (strcmp(argsv[argsv_i], "--compress") == 0) {
                continue;   
            }
            argsv_i++;
        }
    }
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(port);

    printf("Connecting to port %d\n", port);
   
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
   
    // function for chat
    func(sockfd);
   
    // close the socket
    close(sockfd);
    
    // close log_file, if open.

    return 0;
}