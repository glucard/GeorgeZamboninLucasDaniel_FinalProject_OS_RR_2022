#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX 80

void func(int connfd){
    char buff[MAX];
    int n;

    for(;;){
        bzero(buff, MAX);

        read(connfd, buff, sizeof(buff));

        printf("%s \n", buff);
        
        bzero(buff, MAX);
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
        
        write(connfd, buff, sizeof(buff));
        
        if(strncmp("exit", buff, 4) == 0){
            printf("Closing server...\n");
            break;
        }
    }
}

int main(){
    int socketfd, port;
    port = 8080;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1){
        printf("Socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket created successful..\n");
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    close(socketfd);
    return 0;
}