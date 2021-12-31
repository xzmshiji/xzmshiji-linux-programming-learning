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

struct client_info {
    int fd;
    char ip[1024];
    int port;
};

void sys_err(char *str) {
    perror(str);
    exit(1);
}


int main(int argc, char *argv[]) {

    int maxfd, nready, i, n, maxi;
    fd_set read_set, all_set;
    struct sockaddr_in service_addr, client_addr;
    int service_fd, accept_fd, socket_fd;
    struct client_info client[FD_SETSIZE];
    socklen_t client_add_len;
    char buff[BUFSIZ];
    char str[1024];

    service_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (service_fd == -1) {
        sys_err("socker error");
    }

//    bzero(&service_addr, 0);
    service_addr.sin_port = htons(6666);
    service_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    service_addr.sin_family = AF_INET;
    bind(service_fd, (struct sockaddr *) &service_addr, sizeof(service_addr));

    listen(service_fd, 128);

    FD_ZERO(&all_set);

    for (i = 0; i <= FD_SETSIZE; ++i) {
        client[i].fd = -1;
        client[i].port = 0;
    }
    client[0].fd = service_fd;

    FD_SET(service_fd, &all_set);
    maxi = -1;
    maxfd = service_fd;

    while (1) {

        read_set = all_set;

        nready = select(maxfd + 1, &read_set, NULL, NULL, 0);

        if (nready < 0) {
            sys_err("select error");
        }

        if (FD_ISSET(service_fd, &read_set)) {
            client_add_len = sizeof(client_addr);
            accept_fd = accept(service_fd, (struct sockaddr *) &client_addr, &client_add_len);

            int port = ntohs(client_addr.sin_port);
            printf("received from %s at Port %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, str, sizeof(str)), port);

            if (accept_fd < 0) {
                sys_err("accept error");
            }
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = accept_fd;
                    strcpy(client[i].ip, str);
                    client[i].port = port;
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                perror("many too connnect ....");
            }

            if (accept_fd > maxfd) {
                maxfd = accept_fd;
            }
            if (i > maxi) {
                maxi = i;
            }

            FD_SET(accept_fd, &all_set);

            if (--nready <= 0) {
                continue;
            }

        }


        for (i = 0; i <= maxi; ++i) {
            if ((socket_fd = client[i].fd) < 0)
                continue;
            if (FD_ISSET(socket_fd, &read_set)) {

                if ((n = read(socket_fd, buff, sizeof(buff))) == 0) {
                    printf("%s:%d connnect not run .....\n", client[i].ip, client[i].port);
                    close(socket_fd);
                    client[i].fd = -1;
                    FD_CLR(socket_fd, &all_set);
                } else if (n > 0) {
                    for (int j = 0; j < n; ++j) {
                        buff[j] = toupper(buff[j]);
                    }
                    write(client[i].fd, buff, n);
                }

                if (--nready <= 0) {
                    break;
                }

            }


        }


    }


    return 0;

}