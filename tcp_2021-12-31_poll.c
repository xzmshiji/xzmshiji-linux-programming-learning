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
#include <poll.h>


#define PORT 6666
#define OPEN_MAX 1024


void sys_err(char *str) {
    perror(str);
    exit(1);
}


int main(int argc, char *argv[]) {

    int service_fd, i, n, maxi, nready, accept_fd, socket_fd;
    struct sockaddr_in service_addr, client_addr;
    socklen_t client_add_len;
    struct pollfd client[OPEN_MAX];
    char buff[1024], str[1024], client_ip[64];

    service_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (service_fd < 0) {
        sys_err("socke error");
    }

    int opt = 1;
    setsockopt(service_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //端口复用

    service_addr.sin_family = AF_INET;
    service_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &service_addr.sin_addr.s_addr);
    bind(service_fd, (struct sockaddr *) &service_addr, sizeof(service_addr));

    listen(service_fd, 128);

    client[0].fd = service_fd;
    client[0].events = POLLIN;

    for (i = 1; i <OPEN_MAX; ++i) {
        client[i].fd = -1;
    }
    maxi = 0;

    while (1) {

        nready = poll(client, maxi + 1, -1);

        if (client[0].revents & POLLIN) {

            client_add_len = sizeof(client_addr);
            accept_fd = accept(client[0].fd, (struct sockaddr *) &client_addr, &client_add_len);
            int port = ntohs(client_addr.sin_port);
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
            printf("connnect client info %s:%d \n", client_ip, port);

            for (i = 0; i < OPEN_MAX; ++i) {
                if (client[i].fd < 0) {
                    client[i].fd = accept_fd;
                    client[i].events = POLLIN;
                    break;
                }
            }

            if (i == OPEN_MAX) {
                sys_err("too many connenct .....");
            }

            if (i > maxi) {
                maxi = i;
            }

            if (--nready <= 0) {
                continue;
            }


        }



        for (int i = 0; i <= maxi; ++i) {
            ;

            if ((socket_fd = client[i].fd) < 0) {
                continue;
            }


            if (client[i].revents & POLLIN) {
                n = read(client[i].fd, buff, sizeof(buff));
                if (n < 0) {
                    if (errno == ECONNRESET) {
                        close(socket_fd);
                        client[i].fd = -1;
                        client[i].events = 0x000;
                    } else {
                        sys_err("read error");
                    }
                } else if (n == 0) {
                    close(socket_fd);
                    client[i].fd = -1;
                    client[i].events = 0x000;
                } else {
                    for (int j = 0; j <n; ++j) {
                        buff[j] = toupper(buff[j]);
                    }
                    write(socket_fd, buff, n);
                }

                if (--nready <= 0) {
                    break;
                }

            }



        }


    }


    return 0;

}