#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "chatmsg.h"
#define SERVER_PORT 8888
#define MAX_MSG_SIZE 1024
int userid, msgid;
struct sockinfo {
    int sockfd;
    struct sockaddr_in* addr;
};
void *heartbeat(void* args) {
    int sockfd = ((struct sockinfo*)args)->sockfd;
    struct sockaddr_in* addr = ((struct sockinfo*)args)->addr;
    struct chatmsg msg;
    while (1) {
        sleep(10);
        msg.userid = userid;
        msg.msgtype = CHATMSG_HEART;
        msg.msgid = msgid++;
        sendto(sockfd, (void*)&msg, sizeof(struct chatmsg), 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
    }
}
void *recvchatmsg(void* args) {
    int sockfd = *(int*)args;
    struct chatmsg msg;
    while(1) {
        int n = recvfrom(sockfd, (void*)&msg, sizeof(struct chatmsg), 0, NULL, NULL);
        if (n < 0) {
            fprintf(stderr, "Recv error%s\n", strerror(errno));
        } else {
            fprintf(stdout, "%d:\t%s", msg.userid, msg.msgbuf);
        }
    }
}
void start_client(int sockfd, struct sockaddr_in *addr, socklen_t len) {
    pthread_t th1, th2;
    struct sockinfo args;
    args.sockfd = sockfd;
    args.addr = addr;
    pthread_create(&th1, NULL, recvchatmsg, (void*)&sockfd);
    pthread_create(&th2, NULL, heartbeat, (void*)&args);
    int n;
    char buffer[MAX_MSG_SIZE];
    struct chatmsg msg;
    msg.userid = userid;
    msg.msgid = msgid++;
    msg.msgtype = CHATMSG_JOIN;
    sendto(sockfd, (void*)&msg, sizeof(struct chatmsg), 0, (struct sockaddr*)addr, len);

    while (fgets(buffer, MAX_MSG_SIZE, stdin)) {
        msg.userid = userid;
        memcpy(msg.msgbuf, buffer, sizeof(buffer));
        msg.msgid = msgid++;
        msg.msgtype = CHATMSG_CHAT;
        sendto(sockfd, (void*)&msg, sizeof(struct chatmsg), 0, (struct sockaddr*)addr, len);
    }
}
int main (int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage ip port userid\n");
        exit(1);
    }
    msgid = 1;
    int sockfd, port;
    struct sockaddr_in addr;
    if ((userid = atoi(argv[3])) < 0) {
        fprintf(stderr, "usage ip port userid\n");
        exit(1);
    }
    if ((port = atoi(argv[2])) < 0) {
        fprintf(stderr, "usage ip port userid\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Socket Error: %s\n", strerror(errno));
        exit(1);
    }
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    if (inet_aton(argv[1], &addr.sin_addr) < 0) {
        fprintf(stderr, "IP error\n");
        exit(1);
    }
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "Connect Error: %s\n", strerror(errno));
        exit(1);
    }    
    start_client(sockfd, &addr, sizeof(struct sockaddr_in));
    close(sockfd);
    return 0;
}