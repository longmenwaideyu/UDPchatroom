#define CHATMSG_JOIN 0
#define CHATMSG_EXIT 1
#define CHATMSG_CHAT 2
#define CHATMSG_HEART 3
struct chatmsg
{
    int userid;
    int msgid;
    int msgtype;
    char msgbuf[1024];
};