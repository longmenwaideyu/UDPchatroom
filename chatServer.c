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
#include <time.h>
#include <pthread.h>
#include "chatmsg.h"
#define SERVER_PORT 8888
#define MAX_MSG_SIZE 1024
struct sockaddr_in addrlist[1000];
int useridlist[1000];
time_t islive[10000];
void *checklive(void *args) {
    int i;
    while(1) {
        sleep(10);
        for (i = 0; i < 1000; i++) {
            if (useridlist[i] != -1) {
                if (time(NULL) - islive[useridlist[i]] >= 20) {
                useridlist[i] = -1;
                fprintf(stdout, "%d lost connect\n", useridlist[i]);
                }
            }
        }
    }
}
int adduser(struct sockaddr_in addr, int userid){
    int i;
    for (i = 0; i < 1000; i++) {
        if (useridlist[i] == -1) {
            addrlist[i] = addr;
            useridlist[i] = userid;
            islive[userid] = time(NULL);
            return 0;
        }
    }
    return -1;
}
void sendtouser(int sockfd, struct chatmsg msg) {
    int i;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    for (i = 0; i < 1000; i++) {
        if (useridlist[i] != -1 && useridlist[i] != msg.userid) {
            sendto(sockfd, (void*)&msg, sizeof(msg), 0, (struct sockaddr*) &addrlist[i], addrlen);
        }
    }
}
void processMsg(int sockfd, struct sockaddr_in addr, struct chatmsg msg) {
    if (msg.msgtype == CHATMSG_JOIN) {
        adduser(addr, msg.userid);
    } else if (msg.msgtype == CHATMSG_CHAT) {
        sendtouser(sockfd, msg);
    } else if (msg.msgtype == CHATMSG_HEART) {
        islive[msg.userid] = time(NULL);
    }
}
void start_server(int sockfd) {
    struct sockaddr_in addr;
    int n;
    socklen_t addrlen;
    struct chatmsg msg;
    pthread_t th;
    pthread_create(&th, NULL, checklive, NULL);
    while (1) {
        addrlen = sizeof(struct sockaddr_in);
        n = recvfrom(sockfd, (void *)&msg, sizeof(struct chatmsg), 0, (struct sockaddr*)&addr, &addrlen);
        if (msg.msgtype == CHATMSG_CHAT)
            fprintf(stdout, "receved msg from %d: msgid: %d msg: %s", msg.userid, msg.msgid, msg.msgbuf);
        else if (msg.msgtype == CHATMSG_JOIN)
            fprintf(stdout, "%d join the room\n", msg.userid);
        processMsg(sockfd, addr, msg);
    }
}

int main () {
    int i;
    for (i = 0; i < 1000; i++) {
        useridlist[i] = -1;
    }
    for (i = 0; i < 10000; i++) {
        islive[i] = time(NULL);
    }
    int sockfd;
    struct sockaddr_in addr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Socket Error: %s\n", strerror(errno));
        exit(1);
    }
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "Bind Error: %s\n", strerror(errno));
        exit(1);
    }
    start_server(sockfd);
    close(sockfd);
    return 0;
}