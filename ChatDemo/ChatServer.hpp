
/*
* 服务器
* 登录注册：tcp
* 聊天：udp
*/
#include <iostream>
#include <pthread.h>
#include "ProtocolUtil.hpp"
#include "UserManager.hpp"
#include "Log.hpp"
#include "DataPool.hpp"
#include "Message.hpp"

using namespace std;
class ChatServer;

class Param {
public:
    ChatServer *sp;
    int sock;
    string ip;
    int port;

    Param(ChatServer *_sp, int &_sock, string &_ip, int &_port) : sp(_sp), sock(_sock), ip(_ip), port(_port){};
    ~Param(){}
};

class ChatServer{
private:
    int tcp_listen_sock;           //TCP登陆注册的套接字
    int tcp_port;                       //TCP的端口号

    int udp_work_sock;           //通信的套接字
    int udp_port;

    UserManager um;             //用户信息
    DataPool pool;
public:
        ChatServer(int _tcp_port = 8080, int _udp_port = 8888) :
            tcp_port(_tcp_port),
            tcp_listen_sock(-1),
            udp_port(_udp_port),
            udp_work_sock(-1)
        {

        }
        void InitServer()                  //初始化服务器
        {
            //创建两个套接字
            tcp_listen_sock = SocketApi::Socket(SOCK_STREAM);   //TCP
            udp_work_sock = SocketApi::Socket(SOCK_DGRAM);      //UDP

            //进行绑定
            SocketApi::Bind(tcp_listen_sock, tcp_port);
            SocketApi::Bind(udp_work_sock, udp_port);

            //tcp监听
            SocketApi::Listen(tcp_listen_sock);
        }
        void Start()                           //启动服务器
        {
            //至少得有三个线程
            //线程1(主线程)：处理登录注销
            //线程2：把客户信息写到数据池中
            //线程3：把客户信息从数据池中读出来
            //pthread_t tid;
            //pthread_create(&tid, RecvMessage, );
            //pthread_create(&tid, SendMessage, );

            string ip;
	    int port;
            for(;;)
            {
                int sock = SocketApi::Accept(tcp_listen_sock, ip, port);
                if(sock > 0)
                {
                    //cout << "get a new client " << ip << ":" << port << endl;
                     //处理任务，我们得需要sock套接字，我们怎么获取？— 定义一个param类
                    Param *p = new Param(this, sock, ip, port);
                    pthread_t tid;
                    pthread_create(&tid, NULL, HandlerRequest, p);
                }
            }
        }
        unsigned int RegisterUser(string &nick_name, string &school, string &passwd)  //用户注册
        {
            return um.Insert(nick_name, school, passwd);
        }
        unsigned int LoginUser(const unsigned int &id, const string &passwd, const string &ip, const int &port)  //用户登录
        {
            return um.Check(id, passwd);

                //把登陆成功的用户放到在线用户列表中
                //um.MoveToOnline(id, ip, port);

                //这里我们登录用的是TCP连接，IP地址不变
                //但是进行聊天时，我们用的时UDP连接，ip地址确定，但是端口号有可能随机
                //在这里如何处理端口号？
                //解决方案1：客户端绑定一个UDP端口号，但是这样端口号就写死了，不好
                //解决方案2：先让客户发送一个报文，然后服务器将其端口号记住，插入onlineuser
        }
        //UDP
        void Product()
        {
            string message;
            struct sockaddr_in peer;  //用于接受客户端的信息(端口号)
            Util::RecvMessage(udp_work_sock, message, peer);   //收消息

	    cout << "debug：recv message：" << message << endl;
            if(!message.empty())
            {
		//用户登录成功，把用户的消息(name,school)返回给用户
		
                //将Message进行反序列化，拿到id
                Message m;
                m.ToRecvValue(message);
		//判断用户的输入的消息类型
		if(m.Type() == LOGIN_TYPE)  //登录报文
		{
			um.AddOnlineUser(m.Id(), peer);
			string _name, _school;
			um.GetUserInfo(m.Id(), _name, _school);
			Message new_msg(_name, _school, m.Text(), m.Id(), m.Type());
			new_msg.ToSendString(message);
		}
		else if(m.Type() == LOGOUT_TYPE)
		{
			um.RemoveOnlineUser(m.Id());
			string _name, _school;
			um.GetUserInfo(m.Id(), _name, _school);
			Message new_msg(_name, _school, m.Text(), m.Id(), m.Type());
			new_msg.ToSendString(message);
		}
		pool.PutMessage(message);
                
            }
        }
        //UDP
        void Consume()
        {
            string message;
            //从数据池中拿到数据
            pool.GetMessage(message);
            //发送消息到所有在线客户(online_users)
            auto online = um.OnlineUser();
            for(auto it = online.begin(); it != online.end(); ++it)
            {
                Util::SendMessage(udp_work_sock, message, it->second);
            }

        }
        static void *HandlerRequest(void *arg)        //线程处理函数
        {
            //处理任务，我们得需要sock套接字，我们怎么获取？— 定义一个param类
            Param *p = (Param *)arg;
            int sock = p->sock;
            ChatServer *sp = p->sp;
	    string ip = p->ip;
            int port = p->port;
            delete p;
            //分离线程,将状态改为unjoinable状态，确保资源的释放
            pthread_detach(pthread_self());

            //接收
            Request rq;
            Util::RecvRequest(sock, rq);
            //对正文部分进行反序列化
            Json::Value root;

            /**********************测试**************************/
            LOG(rq.text, NORMAL);
             /**********************测试**************************/

            Util::UnSeralizer(rq.text, root);
            //判断方法：（REGISTER/LOGIN/LOGOUT）
            if(rq.method == "REGISTER")     //注册
            {
                //获取注册用户的信息
                string name = root["nick_name"].asString();  //asInt()
                string school = root["school"].asString();
                string passwd = root["passwd"].asString();

                //将注册用户信息传到服务器，服务器返回一个注册ID
                unsigned int id = sp->RegisterUser(name, school, passwd);
                //unsigned int id = 10000;
                send(sock, (void *)&id, sizeof(id), 0);    //将注册ID返回给客户端
            }
            else if(rq.method == "LOGIN")   //登录
            {
                    //将正文进行反序列化，得到id和密码
                    //unsigned int id = Util::StringToInt(root["id"]);
                    unsigned int id = root["id"].asInt();
                    string passwd = root["passwd"].asString();
                    //用户登录：登陆验证；将登录用户添加到在线用户中
                    unsigned int result = sp->LoginUser(id, passwd, ip, port);
                    //unsigned int result = 10000;
                    //将返回结果返回给客户(登陆成功：result = id)
                    send(sock, &result, sizeof(result), 0);
            }
            else    //退出
            {
                    //可以先检验用户是否在线
            }

            //分析处理
            //响应

            close(sock);
        }
        ~ChatServer();
};















