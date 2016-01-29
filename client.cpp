#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
using namespace std;
int main(int argc,char *argv[])
{
    struct addrinfo hint, *result;
    int res, fd;
    char buf[20];

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    if(argc!=2){
        perror("argc error\n");
    }

    res = getaddrinfo(argv[1], "8080", &hint, &result);
    if (res == -1)
        return -1;

    fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (fd == -1)
        return -2;
    res = connect(fd, result->ai_addr, result->ai_addrlen);
    cin>>buf;
    write(fd, buf, strlen(buf));
    read(fd, buf, 20);
    printf("data: %s\n", buf);
    return 0;

}
