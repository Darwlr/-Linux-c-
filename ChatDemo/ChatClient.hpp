#include <iostream>
#include <string>
#include <pthread.h>
#include <vector>
#include "ProtocolUtil.hpp"
#include "Message.hpp"
#include "Window.hpp"

using namespace std;

#define TCP_PORT    8080
#define UDP_PORT    8888
class ChatClient;

struct ParamPair
{
	Window *wp;
	ChatClient *cp;
};

class ChatClient{
private:
    int tcp_sock;
    int udp_sock;
    string peer_ip;
    struct sockaddr_in peer;
	
    string passwd;
public:
    unsigned int id;    //用户id
    string nick_name;
    string school;
public:
        ChatClient(string ip) : peer_ip(ip), tcp_sock(-1), udp_sock(-1), id(0)
	{
	    peer.sin_family = AF_INET;
            peer.sin_addr.s_addr = inet_addr(peer_ip.c_str());
            peer.sin_port = htons(UDP_PORT);
	}
	void InitClient()
	{
	    //创建套接字
	    tcp_sock = SocketApi::Socket(SOCK_STREAM);
	    udp_sock = SocketApi::Socket(SOCK_DGRAM);
	}
	bool ConnectServer()
	{
	    //在注册完成之后，都会close(tcp_sock)，所以每次连接时，都要重新建立
	    tcp_sock = SocketApi::Socket(SOCK_STREAM);
	    return SocketApi::Connect(tcp_sock, peer_ip, TCP_PORT);
	}
	//用户注册
	bool Register()
	{
	    //检查是否已连接
	    if(ConnectServer() && Util::RegisterEnter(nick_name, school, passwd))
	    {
		Request rq;
		rq.method = "REGISTER\n";

		 //构建一个value对象,write将value对象转换成字符串
		 //效果 re.text = {"nick_name" : nick_name, "school" : school, "passwd" : passwd}
		Json::Value root;
		root["nick_name"] = nick_name;
		root["school"] = school;
		root["passwd"] = passwd;
		Util::Seralizer(root, rq.text);   //正文

		 rq.content_length = "Content-Length: " + Util::IntToString((rq.text).size()) + "\n";

		 //将自己的而信息发送给服务器
		Util::SendRequest(tcp_sock, rq);

		//接受服务器返回的状态码
		recv(tcp_sock, &id, sizeof(id), 0);
		bool res = false; //注册是否成功
		//如果id大于10000，表示注册成功
		if(id >= 10000)
		{
		    cout << "Resgister Success ! Your Login ID is：" << id << endl;
		    res = true;
		}
		else
		{
		    cout << "Register Failed! Code is : " << id << endl;
		}

		close(tcp_sock);
		return res;
	    }
	}
	//用户登录
	bool Login()
	{
	    //检查是否已连接
	    if(ConnectServer() && Util::LoginEnter(id, passwd))
	    {
		Request rq;
		rq.method = "LOGIN\n";

		 //构建一个value对象,write将value对象转换成字符串
		 //效果 re.text = {"nick_name" : nick_name, "school" : school, "passwd" : passwd}
		Json::Value root;
		root["id"] = id;
		root["passwd"] = passwd;
		Util::Seralizer(root, rq.text);   //正文

		 rq.content_length = "Content-Length: " + Util::IntToString((rq.text).size()) + "\n";

		 //将自己的而信息发送给服务器
		Util::SendRequest(tcp_sock, rq);

		//接受服务器返回的状态码
		unsigned int result = 0;
		recv(tcp_sock, &result, sizeof(result), 0);
		bool res = false;
		//如果id大于10000，表示登录成功，返回登录ID
		if(result >= 10000)
		{
		    cout << "Login Success ! " << endl;
		    res = true;
		     //先向服务器发送一个消息，为的就是让服务器知道此处用户上线，并将其添加到用户列表中
		        string name = "None";
		        string school = "None";
		        string text = "I am login! talk with me";
			unsigned int type = LOGIN_TYPE;
		        string sendString;
		        unsigned int id = result;
		        Message m(name, school, text, id, type);
		        m.ToSendString(sendString);
		        UdpSend(sendString);
		}
		else
		{
		    cout << "Login Failed! Code is : " << result << endl;
		}

		close(tcp_sock);
		return res;
	    }
	}
	void UdpSend(string &msg)
	{
		Util::SendMessage(udp_sock, msg, peer);
	}
	void UdpRecv(string &msg)
	{
		struct sockaddr_in server;
		Util::RecvMessage(udp_sock, msg, server);
	}
	static void *Welcome(void *arg)
	{
		pthread_detach(pthread_self());
		Window *wp = (Window *)arg;
		wp->Welcome();
	}
	static void *Input(void *arg)
	{
		pthread_detach(pthread_self());
		struct ParamPair *pptr = (struct ParamPair *)arg;
		Window *wp = pptr->wp;
		ChatClient *cp = pptr->cp;	

		wp->DrawInput();
		string text;
		while(1)
		{
			wp->GetStringFormInput(text);  //拿到数据
			Message msg(cp->nick_name, cp->school, text, cp->id);
			string sendString;
			msg.ToSendString(sendString);
			cp->UdpSend(sendString);
		}
	
	}
	//用户聊天
	void Chat()
	{
		Window w;
		//创建两个线程，分别描绘header(h)，Intput(l)
		//主线程output
		pthread_t h, l;

		//结构体不能被整体赋值，但能够被整体初始化
		struct ParamPair pp = {&w, this};
	

		pthread_create(&h, NULL, Welcome, &w);  //进行welcome的刷新
		pthread_create(&l, NULL, Input, &pp);  //进行的input窗口刷新

		//绘制窗口
		w.DrawOutput();
		w.DrawOnline();

		string recvString;
		string showString;
		vector<string> online;  //在线用户

		//主线程
		while(1)
		{
			Message msg;
			//收消息
			UdpRecv(recvString);
			msg.ToRecvValue(recvString);

			if(msg.Id() == id && msg.Type() == LOGIN_TYPE)
			{
				nick_name = msg.NickName();
				school = msg.School();
			}
			if(msg.Id() == id && msg.Text() == "bye")
			{
				Logout();
			}
			string f =msg.NickName() + "(" + msg.School() + ")";
			

			showString = msg.NickName() + "(" + msg.School() + ") Say: ";
			showString += msg.Text(); // zhangsan(SWPU) Say: hello

			
			w.PutMessageToOutput(showString);
			if(msg.Type() == LOGIN_TYPE || msg.Type() == NORMAL_TYPE)
			{
				Util::addUser(online, f);
				w.PutUserToOnline(online);	
			}
			else if(msg.Type() == LOGOUT_TYPE)
			{
				Util::removeUser(online, f);
				w.RefreshUserToOnline(online);	
			}
			
			
		}

	}
        //退出
	void Logout()
	{
		string content = "I am logout! See you!";
		Message msg(nick_name, school, content, id, LOGOUT_TYPE);
		string sendString;
		msg.ToSendString(sendString);
		UdpSend(sendString);
		
	}
        ~ChatClient(){}
};




