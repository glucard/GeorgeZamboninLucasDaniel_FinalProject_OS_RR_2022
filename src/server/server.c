#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX 122
#define SA struct sockaddr

typedef struct pipesc {
    char** commands;
    int length;
} Pipesc;

Pipesc* splitPipes(char* command) {

    char** all_commands = (char**)malloc(sizeof(char*));

    int i, j, n;

    n = 0;
    i = 0;
    while(n < MAX){
        all_commands = (char**)realloc(all_commands, (i+1)*sizeof(char*));
        all_commands[i] = (char*)malloc(122 * sizeof(char));
        bzero(all_commands[i], MAX);

        printf("n: %d\n", n);
        for (j = 0; j < MAX; j++) {
            if (command[n] == '|' || n == MAX){
                n++;
                break;
            }
            all_commands[i][j] = command[n++];
        }
        i++;
    }

    Pipesc* p = (Pipesc*)malloc(sizeof(Pipesc));
    p->commands = all_commands;
    p->length = i;

    return p;
}

void destroyPipesc(Pipesc* p){
    int i;
    for (i = 0; i < p->length; i++) {
        free(p->commands[i]);
    }
    free(p->commands);
    free(p);
}

void executeCommands(char* command){
    Pipesc* p = splitPipes(command);
    
    int i;
    for (i = 0; i < p->length; i++){
        printf("p[%d]: %s", i, p->commands[i]);
        continue;
    }

    destroyPipesc(p);
}

void func(int connfd){
    char buff[MAX];
    int n;

    for(;;){
        bzero(buff, MAX);

        read(connfd, buff, sizeof(buff));

        printf("From client: %s", buff);
        executeCommands(buff);

        bzero(buff, MAX);
        //n = 0;
        //while ((buff[n++] = getchar()) != '\n');
        
        //write(connfd, buff, sizeof(buff));
        
        if(strncmp("exit", buff, 4) == 0){
            printf("Closing server...\n");
            break;
        }
    }
}

int main(){
    int sockfd, connfd, len, port;
    struct sockaddr_in servaddr, cli;
    port = 8080;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        printf("Socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket created successful..\n");
    }


    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0){
        printf("Failed to bind socket.\n");
        exit(0);
    } else {
        printf("Socket successful binded.\n");
    }

    
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening..\n");
    }

    len = sizeof(cli);

    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    } else {
        printf("server accept the client...\n");
    }
   
    func(connfd);
   
    close(sockfd);
    return 0;
}