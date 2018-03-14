#ifndef Z_UDP_H
#define Z_UDP_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"{
#endif

int inet_generic_ntop(struct sockaddr *src, char *ip, int size);
int inet_generic_port(struct sockaddr *src, int *port);
int inet_resolve(const char *host, const char *service,
                 struct addrinfo **res);
int create_socket(const char *host, const char *service,
                  struct sockaddr *addr, int *addr_len);
int udp_server_socket(const char *host, const char *service,
               struct sockaddr *addr, int *addr_len);
int udp_client_socket(const char *host, const char *service,
                      struct sockaddr *addr, int *addr_len);
int tcp_client_socket(const char *host, const char *service,
                      struct sockaddr *server_addr, int *addr_len);
int set_socket_nonblocking(int socket_fd);
int udp_server_bind(int socket_fd, const struct sockaddr *addr, int addr_len);
int udp_client_connect(int socket_fd, const struct sockaddr *server_addr,
                       int addr_len);
int udp_client(const char *host, const char *service);
int udp_server(const char *host, const char *service);
int udp_send(int socket_fd, const void *buf, int size);
int udp_sendto(int socket_fd, const void *buf, int size,
               struct sockaddr *to, socklen_t addrlen);
int udp_recv(int socket_fd, void *buf, int size);
int udp_recvfrom(int socket_fd, void *buf, int size,
                 struct sockaddr *from, socklen_t *addrlen);
void udp_close(int socket_fd);

#ifdef __cplusplus
}
#endif

#endif // Z_NET_H
