#include "ChatServer.hpp"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

using namespace std;

static void Usage(string proc)
{
    cout << "Usage：" << proc << " tcp_port  udp_port" << endl;
}
//生产线程函数
void *RunProduct(void *arg)
{
    pthread_detach(pthread_self());
    ChatServer *sp = (ChatServer *)arg;
    while(1)
    {
        sp->Product();
    }
}
//消费线程函数
void *RunConsume(void *arg)
{
    pthread_detach(pthread_self());
    ChatServer *sp = (ChatServer *)arg;
    while(1)
    {
            sp->Consume();
    }

}
// ./ChatServer tcp_port udp_port
int main(int argc, char *argv[])
{
    //检查参数
    if(argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }
    int tcp_port = atoi(argv[1]);
    int udp_port = atoi(argv[2]);

    //建立服务器
    ChatServer *sp = new ChatServer(tcp_port, udp_port);
    //初始化服务器
    sp->InitServer();
    //创建两个线程，一个用于生产，一个用于消费
    pthread_t c, p;
    pthread_create(&p, NULL, RunProduct, (void *)sp);
    pthread_create(&c, NULL, RunConsume, (void *)sp);

    //启动服务器
    sp->Start();

    return 0;
}

