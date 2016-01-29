#include <stdio.h>
#include <event.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
/**
 * [create_socket 创建一个监听套接字，设置次套接字为非阻塞]
 * @param  port [服务器监听的的端口号]
 * @return      [返回监听的套接字描述符]
 */
int create_socket(char *port)
{
    struct addrinfo hint, *result;
    int res, fd, flags;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    res = getaddrinfo(NULL, port, &hint, &result);
    if (res == -1)
        return -1;

    fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (fd == -1)
        goto err;

    // 设置非阻塞
    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    flags = fcntl(fd, F_SETFL, flags);
    if (flags == -1)
        goto err;

    res = bind(fd, result->ai_addr, result->ai_addrlen);
    if (res == -1)
        goto err;

    res = listen(fd, 100);
    if (res == -1)
        goto err;

    return fd;

err:
    free(result);
    return -1;

}
/**
 * [write_cb description]
 * @param fd    [description]
 * @param event [description]
 * @param arg   [description]
 */
void write_cb(int fd, short event, void *arg)
{
    char buf[100];
    strcpy(buf, "Hello Client\n");

    write(fd, buf, strlen(buf));

    // 释放写event结构
    free(arg);
}
/**
 * [read_cb description]
 * @param fd    [description]
 * @param event [description]
 * @param arg   [description]
 */
void read_cb(int fd, short event, void *arg)
{
    char buf[100];
    struct event *ev = (struct event*)arg;

    read(fd, buf, 100);
    printf("data: %s\n", buf);

    free(ev);

    /* 有写需求，加入 写事件 */
    ev = (struct event*)malloc(sizeof(struct event));
    event_set(ev, fd, EV_WRITE, write_cb, ev);
    event_add(ev, NULL);
}
/**
 * [accept_cb description]
 * @param fd    [description]
 * @param event [description]
 * @param arg   [description]
 */
void accept_cb(int fd, short event, void *arg)
{
    int sfd, addrlen;
    struct sockaddr addr;
    while (1)
    {
        addrlen = sizeof(struct sockaddr);
        sfd = accept(fd, &addr, (socklen_t*)(&addrlen));

        if (sfd == -1)
        {
            if (errno == EAGAIN)
                break;
            else
                continue;
        }

        struct event *ev = (struct event*)malloc(sizeof(struct event));
        if (ev == NULL)
            break;

        event_set(ev, sfd, EV_READ, read_cb, ev);
        event_add(ev, NULL);
    }
}
/**
 * [main description]
 * @return [description]
 */
int main()
{
    struct event ev;
    int fd, res;
    struct event_base *base = event_init();
    if (base == NULL)
        return -1;
    fd = create_socket("8080");
    if (fd == -1)
        return -2;
    /* 监听套接字，用于建立连接，取得连接套接字 */
    event_set(&ev, fd, EV_PERSIST | EV_READ, accept_cb, NULL);
    event_add(&ev, NULL);
    event_base_dispatch(base);
    return 0;
}
