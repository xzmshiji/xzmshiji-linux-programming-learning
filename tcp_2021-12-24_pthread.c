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
#include <ctype.h>
#include <sys/socket.h>
#include <pthread.h>


#define SERVICE_PORT 6666
#define SERVICE_IP "192.168.3.45"


void sys_err(char *str) {
    perror(str);
    exit(1);
}


struct cinfo{
    int fd;
    struct sockaddr_in *client_ddr;
};

void *run_thread(void *arg){

    char buff[BUFSIZ];
    int n;
    struct cinfo *client_info  = (struct cinfo *)arg;
    char client_ip[1024];

    int client_fd = client_info->fd;

    inet_ntop(AF_INET,&client_info->client_ddr->sin_addr.s_addr,client_ip,sizeof(client_ip));
    int client_port = ntohs(client_info->client_ddr->sin_port);
    printf("客户端IP：%s，端口：%d\n", client_ip, client_port);


    while (1) {
        n = read(client_fd, buff, sizeof(buff));
        if (n == 0) {
            printf("client %s:%d disconnect......\n",client_ip,client_port);
            break;
        } else if (n < 0) {
            sys_err("read error");
        } else {
            for (int i = 0; i < n; ++i) {
                buff[i] = toupper(buff[i]);
            }
            write(client_fd, buff, n);
        }
    }
    close(client_fd);
}


int main(int argc, char *argv[]) {

    int link_fd, client_fd;
    struct sockaddr_in service_addr, client_addr;
    socklen_t socklen;
    char buff[BUFSIZ];
    int n;
    char client_ip[BUFSIZ];
    pthread_t tid;
    struct cinfo ts[1024];
    int i = 0;


    link_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (link_fd == -1) {
        sys_err("socke error");
    }

    bzero(&service_addr,sizeof(service_addr));
    service_addr.sin_family = AF_INET;
    service_addr.sin_port = htons(SERVICE_PORT);
    inet_pton(AF_INET, SERVICE_IP, &service_addr.sin_addr.s_addr);

    bind(link_fd, (struct sockaddr *) &service_addr, sizeof(service_addr));

    listen(link_fd, 128);




    while (1){
        socklen = sizeof(client_addr);
        client_fd = accept(link_fd, (struct sockaddr *) &client_addr, &socklen);
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
        int client_port = ntohs(client_addr.sin_port);
        ts[i].fd = client_fd;
        ts[i].client_ddr = &client_addr;
        pthread_create(&tid,NULL,run_thread,(void*)&ts[i]);
        pthread_detach(tid);
        i++;
    }


    close(link_fd);
    close(client_fd);
    return 0;

}