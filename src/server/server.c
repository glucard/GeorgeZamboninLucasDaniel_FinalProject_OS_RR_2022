#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define die(e) do { fprintf(stderr, "%s", e); exit(EXIT_FAILURE); } while (0);

#define MAX 256
#define SA struct sockaddr

typedef struct subprocess_t {
	pid_t pid;
	int supplyfd;
} subprocess_t;

subprocess_t subprocess(const char *command, char* output) {
	int fds[2];
    int fds_out[2];

	pipe(fds);
    pipe(fds_out);
	subprocess_t process = { fork(), fds[1] };
	if (process.pid == 0) {
        close(fds_out[0]);
        dup2(fds_out[1], STDOUT_FILENO);
        close(fds_out[1]);
		dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
		char *argv[] = {"/bin/sh", "-c", (char*) command, NULL};
		execvp(argv[0], argv);
        printf("execvp error...\n");
		exit(1);
	}
    close(fds_out[1]);

    bzero(output, MAX);
    int status;
    int nbytes = read(fds_out[0], output, sizeof(output));
    int i, last_enter_pos = 0;
    for (i = 0; output[i] != '\0'; i++);
    printf("%d length\n, ", i);
	close(fds[0]);
	return process;
}

typedef struct pipesc {
    char** commands;
    int length;
} pipesc;

pipesc* splitPipes(char* command) {

    char** all_commands = (char**)malloc(sizeof(char*));

    int i, j, n;

    n = 0;
    i = 0;
    while(n < MAX){
        all_commands = (char**)realloc(all_commands, (i+1)*sizeof(char*));
        all_commands[i] = (char*)malloc(MAX * sizeof(char));
        bzero(all_commands[i], MAX);

        for (j = 0; j < MAX; j++) {
            if (command[n] == '[' || n == MAX){
                n++;
                break;
            }
            all_commands[i][j] = command[n++];
        }
        i++;
    }

    pipesc* p = (pipesc*)malloc(sizeof(pipesc));
    p->commands = all_commands;
    p->length = i;

    return p;
}

void destroyPipesc(pipesc* p){
    int i;
    for (i = 0; i < p->length; i++) {
        free(p->commands[i]);
    }
    free(p->commands);
    free(p);
}

void executePipes(pipesc* p) {
	int status;
    char output[4096];
	if (p->length > 0) {
		subprocess_t sp[p->length];
		sp[0] = subprocess(p->commands[0], output);
		
		int i = 1;
		for (; i < p->length; i++){
			int next_next_supply_fd;
			sp[i] = subprocess(p->commands[i], output);
			close(sp[i-1].supplyfd);
            dprintf(sp->supplyfd, "%s", output);
			pid_t pid = waitpid(sp[i-1].pid, &status, 0);
		}
		close(sp[i-1].supplyfd);
		pid_t pid = waitpid(sp[i-1].pid, &status, 0);
	}
    printf("Shell: %s\n", output);
}

void executeCommands(char* command){
    pipesc* p = splitPipes(command);
    executePipes(p);

    destroyPipesc(p);
}

void func(int connfd){
    char buff[MAX];
    int n = 0;

    for(;;){
        bzero(buff, MAX);
        read(connfd, buff, sizeof(buff));
        printf("From client: %s\n", buff);
        
        if(strncmp("exit", buff, 4) == 0){
            printf("Closing server...\n");
            break;
        }
        executeCommands(buff);
    }
}

int main(int argsc, char **argsv){
    int sockfd, connfd, len, port;
    struct sockaddr_in servaddr, cli;
    port = 8080;

    int argsv_i = 1; // ignore ./filename
    if(argsc > 2){
        while(argsv_i < argsc){
            if (strcmp(argsv[argsv_i], "--port") == 0) {
                port = atoi(argsv[++argsv_i]);
                continue;
            }
            argsv_i++;
        }
    }

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