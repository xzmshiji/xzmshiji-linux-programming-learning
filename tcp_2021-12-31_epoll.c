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
#include <sys/select.h>
#include <sys/epoll.h>


#define PORT 6666
#define OPEN_MAX 128

void sys_err(char *str) {
    perror(str);
    exit(1);
}


int main(int argc, char *argv[]) {

    int service_fd,accept_fd,socket_fd;
    struct sockaddr_in service_addr,client_addr;
    socklen_t client_addr_len;
    char buff[BUFSIZ],client_ip[64];
    int nready,n;

    service_fd = socket(AF_INET,SOCK_STREAM,0);

    service_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    service_addr.sin_port = htons(PORT);
    service_addr.sin_family = AF_INET;
    bind(service_fd,(struct sockaddr *) &service_addr,sizeof(service_addr));

    listen(service_fd,128);

    struct epoll_event listen_epevent[OPEN_MAX];
    struct epoll_event epevent;

    epevent.events = EPOLLIN;
    epevent.data.fd = service_fd;

    int epfd = epoll_create(OPEN_MAX);

    epoll_ctl(epfd,EPOLL_CTL_ADD,service_fd,&epevent);

    while (1){

        nready = epoll_wait(epfd,listen_epevent,OPEN_MAX,-1);

        if(nready<0){
            sys_err("epoll_waite error");
        }
        for (int i = 0; i <nready ; ++i) {

            socket_fd = listen_epevent[i].data.fd;

            if(socket_fd==service_fd){
                client_addr_len = sizeof(client_addr);
                accept_fd =  accept(service_fd,(struct sockaddr *)&client_addr,&client_addr_len);

                inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,client_ip,sizeof(client_ip));
                printf("connect on from %s:%d\n",client_ip, ntohs(client_addr.sin_port));

                epevent.events= EPOLLIN;
                epevent.data.fd = accept_fd;
                epoll_ctl(epfd,EPOLL_CTL_ADD,accept_fd,&epevent);
                continue;
            }
            n = read(socket_fd,buff,sizeof(buff));
            if(n<=0){
                close(socket_fd);
                epevent.events = EPOLLIN;
                epevent.data.fd = socket_fd;
                epoll_ctl(epfd,EPOLL_CTL_DEL,socket_fd,&epevent);
            }else{
                for (int j = 0; j <n ; ++j) {
                    buff[j] = toupper(buff[j]);
                }
                write(socket_fd,buff,n);
            }

        }
    }

    close(service_fd);
    return 0;

}