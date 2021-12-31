#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <sys/wait.h>


void sys_err(char *str) {
    perror(str);
    exit(1);
}


void do_well(int signo){
    while (waitpid(0,NULL,WNOHANG)>0);
    return ;
}


int main(int argc, char *argv[]) {

    int link_fd ,client_fd;
    struct sockaddr_in service_addr,client_addr;
    socklen_t client_addr_len;
    char buff[BUFSIZ];
    int n;
    pid_t  pid;

    link_fd = socket(AF_INET,SOCK_STREAM,0);
    if(link_fd==-1){
        printf("socke error");
        exit(1);
    }

    service_addr.sin_family = AF_INET;
    inet_pton(AF_INET,argv[1],&service_addr.sin_addr.s_addr);
    service_addr.sin_port = htons(atoi(argv[2]));

    bind(link_fd,(struct sockaddr*)&service_addr,sizeof(service_addr));

    listen(link_fd,128);

    client_addr_len = sizeof(client_addr);

    while (1){

        client_fd = accept(link_fd,(struct sockaddr *)&client_addr,&client_addr_len);
        pid = fork();
        if(pid>0){
            close(client_fd);
            signal(SIGCHLD,do_well);
        }
        if(pid==0){
            close(link_fd);
            break;
        }
        if(pid==-1){
            perror("fork error");
            exit(1);
        }

    }

    if(pid==0){
        while (1){
            n = read(client_fd,buff,sizeof(buff));
            printf("n = %d\n",n);
            if(n==0){
                close(client_fd);
                return 0 ;
            }else if(n==-1){
                perror("read error");
                exit(1);
            }else{
                for (int i = 0; i <n ; ++i) {
                    buff[i] = toupper(buff[i]);
                }
                write(client_fd,buff,n);
            }
        }
    }

    if(pid>0){
        close(link_fd);
    }
    close(client_fd);
    return 0;

}