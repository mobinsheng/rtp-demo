#include "z_udp.h"
#include<netdb.h>
/**
 * convert numberic ip in struct src to char *
 */
int inet_generic_ntop(struct sockaddr *src, char *ip, int size)
{
    switch(src->sa_family)
    {
        case AF_INET:
        {
            struct sockaddr_in *inet4 = (struct sockaddr_in *)src;
            if(inet_ntop(AF_INET, (void *)&inet4->sin_addr, ip, size) == NULL)
            {
                return -1;
            }
            return 0;
        }
        case AF_INET6:
        {
            struct sockaddr_in6 *inet6 = (struct sockaddr_in6 *)src;
            if(inet_ntop(AF_INET6, (void *)&inet6->sin6_addr, ip, size) == NULL)
            {
                return -1;
            }
            return 0;
        }
        default:
            return -1;
    }
}

/**
 * convert port in struct src to local machine byte order (little-endian or big-endian)
 */
int inet_generic_port(struct sockaddr *src, int *port)
{
    switch(src->sa_family)
    {
        case AF_INET:
        {
            struct sockaddr_in *inet4 = (struct sockaddr_in *)src;
            if(port)
            {
                *port = ntohs(inet4->sin_port);
            }
            return 0;
        }
        case AF_INET6:
        {
            struct sockaddr_in6 *inet6 = (struct sockaddr_in6 *)src;
            if(port)
            {
                *port = ntohs(inet6->sin6_port);
            }
            return 0;
        }
        default:
            return -1;
    }
}

/**
 * resolve host and service to addrinfo *
 */
int inet_resolve(const char *host, const char *service,
                 struct addrinfo **res)
{
    int ret;
    struct addrinfo hints;
    struct addrinfo *tmp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    ret = getaddrinfo(host, service, &hints, &tmp);
    if(ret != 0)
    {
        //log error;
        printf("%s\n", gai_strerror(ret));
        return -1;
    }

    *res = tmp;
    return 0;
}

/**
 * create a socket
 */
int create_socket(const char *host, const char *service,
                  struct sockaddr *addr, int *addr_len)
{
    int socket_fd;
    int ret;
    struct addrinfo *res, *res_save;

    ret = inet_resolve(host, service, &res);
    if(ret != 0)
    {
        //log error;
        return -1;
    }

    res_save = res;

    assert(res != NULL);

    do
    {
        socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(socket_fd != -1)
        {
            break;
        }
        perror("socket create");
    }while((res = res->ai_next));

    if(res == NULL)
    {
        return -1;
    }

    if(addr)
    {
        *addr = *(res->ai_addr);
    }

    if(addr_len)
    {
        *addr_len = res->ai_addrlen;
    }

    freeaddrinfo(res_save);

    return socket_fd;
}


/**
 * create a listen socket
 */
int udp_server_socket(const char *host, const char *service,
               struct sockaddr *addr, int *addr_len)
{
    return create_socket(host, service, addr, addr_len);
}

/**
 * creat s client socket
 */
int udp_client_socket(const char *host, const char *service,
                      struct sockaddr *addr, int *addr_len)
{
    return create_socket(host, service, addr, addr_len);
}

/**
 * create a client socket
 */
int tcp_client_socket(const char *host, const char *service,
                      struct sockaddr *server_addr, int *addr_len)
{
    struct addrinfo *res, *res_save;
    int ret;
    int socket_fd;

    ret = inet_resolve(host, service, &res);
    if(ret == -1)
    {
        return -1;
    }

    res_save = res;

    do
    {
        socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(socket_fd != -1)
        {
            break;
        }
    }while((res = res->ai_next));

    if(server_addr)
    {
        *server_addr = *(res->ai_addr);
    }

    if(addr_len)
    {
        *addr_len = res->ai_addrlen;
    }

    freeaddrinfo(res_save);

    return socket_fd;
}

int set_socket_nonblocking(int socket_fd)
{
    int flags;

    if((flags = fcntl(socket_fd, F_GETFL)) == -1)
    {
        return -1;
    }

    flags |= O_NONBLOCK;
    if(fcntl(socket_fd, F_SETFL, flags) == -1)
    {
        return -1;
    }
    return socket_fd;
}

/**
 * bind a socket fd
 */
int udp_server_bind(int socket_fd, const struct sockaddr *addr, int addr_len)
{
    if(bind(socket_fd, addr, addr_len) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * connect
 */
int udp_client_connect(int socket_fd, const struct sockaddr *server_addr,
                       int addr_len)
{
    int new_socket_fd;

    new_socket_fd = connect(socket_fd, server_addr, addr_len);
    if(new_socket_fd == -1)
    {
        //log
        return -1;
    }
    return 0;
}

/**
 * create a socket fd and connect to (host, service)
 */
int udp_client(const char *host, const char *service)
{
    int socket_fd;
    struct sockaddr addr;
    int addr_len;
    int ret;

    socket_fd = udp_client_socket(host, service, &addr, &addr_len);
    if(socket_fd == -1)
    {
        return -1;
    }

    ret = udp_client_connect(socket_fd, &addr, addr_len);
    if(ret == -1)
    {
        return -1;
    }

    return socket_fd;
}

/**
 * 1.create a socket fd
 * 2.bind the socket
 */
int udp_server(const char *host, const char *service)
{
    struct sockaddr addr;
    int addr_len;
    int socket_fd;
    int ret;

    socket_fd = udp_server_socket(host, service, &addr, &addr_len);
    if(socket_fd == -1)
    {
        return -1;
    }

    ret = udp_server_bind(socket_fd, &addr, addr_len);
    if(ret == -1)
    {
        return -1;
    }

    return socket_fd;
}

/**
 * send size bytes
 * requirement: must connect before send
 */
int udp_send(int socket_fd, const void *buf, int size)
{
    int ret;

    ret = write(socket_fd, buf, size);
    return ret;
}

/**
 * send size bytes with address info
 */
int udp_sendto(int socket_fd, const void *buf, int size,
               struct sockaddr *to, socklen_t addrlen)
{
    int ret;

    ret = sendto(socket_fd, buf, size, 0, to, addrlen);
    return ret;
}

/**
 * recv size bytes
 * requirement: connect before recv
 */
int udp_recv(int socket_fd, void *buf, int size)
{
    int ret;

    ret = read(socket_fd, buf, size);
    return ret;
}

/**
 * recv size bytes with addrinfo
 */
int udp_recvfrom(int socket_fd, void *buf, int size,
                 struct sockaddr *from, socklen_t *addrlen)
{
    int ret;

    ret = recvfrom(socket_fd, buf, size, 0, from, addrlen);
    return ret;
}

/**
 * close the socket fd
 */
void udp_close(int socket_fd)
{
    close(socket_fd);
}
