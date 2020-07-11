/*
* 包括第三方方法（功能性函数）
*  socket的封装
*/
#pragma once

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include "jsoncpp/include/json.h"
#include "Log.hpp"

#define BACKLOG     5
#define MESSAGE_SIZE 1024


//请求，采用http
class Request{
public:
    string method;   //REGISTER  LOGIN   LOGOUT
    string content_length;          //"Content-Length: 89"
    string blank;                           //行分隔符
    string text;                               //正文

    Request() : blank("\n"){}
    ~Request(){}
};


class Util{
public:
    //注册界面输入
    static bool RegisterEnter(string &nick_name, string &school, string &passwd)
    {
        cout << "Please input Nickname：" ;
        cin >> nick_name;
        cout << "Please input School：";
        cin >> school;
        cout << "Please input Passwd：";
        cin >> passwd;
        string again;
        cout << "Please input Passwd again：";
        cin >> again;
        if(passwd != again)
        {
            cout << "两次密码不匹配" << endl;
            return false;
        }
        return true;
    }

    //登陆界面进入
     static bool LoginEnter(unsigned int &id, string &passwd)
     {
         cout << "Please input your ID：";
         cin >> id;
         cout << "Please input your Passwd：";
         cin >> passwd;
     }


    //序列化：把一个字符串通过read方法转成value对象
    static void Seralizer(Json::Value &root, string &outString)
    {
        Json::FastWriter w;
        outString = w.write(root);
    }
    //反序列化：把一个value对象通过write方法转成字符串
    static void UnSeralizer(string &inString, Json::Value &root)
    {
        Json::Reader r;
        r.parse(inString, root, false);
    }

    //整型转字符串
    static string IntToString(int value)
    {
        //atoi：字符串转整型，尽量不要用
        stringstream ss;
        ss << value;
        return ss.str();
    }
    //字符串转整型
    static int StringToInt(string &str)
    {
        stringstream ss(str);
        int x;
        ss >> x;
        return x;
    }

    //服务器接受客户端信息(Request)
    static void RecvOneLine(int sock, string &outString)
    {
        char c = 'x';
        while( c != '\n')
        {
            ssize_t s = recv(sock, &c, 1, 0);
            if(s > 0)
            {
                //去除'\n'
                if(c == '\n')
                {
                    break;
                }
                //字符串中有push_back方法
                outString.push_back(c);
            }
            else
            {
                break;
            }
        }
    }

    //服务器接受客户端信息(Request)
    static void RecvRequest(int sock, Request &rq)
    {
        RecvOneLine(sock, rq.method);  //获取request中的方法
        RecvOneLine(sock, rq.content_length);  //获取request中的长度
        RecvOneLine(sock, rq.blank);            //读到空行：说明报头读完

        //获取正文的长度
        string &cl = rq.content_length;   //Content-Length：6  要获取6
        size_t index = cl.find(": ");   //找到": "的下标
        //没有找到
        if(string::npos == index)
        {
            return;
        }
        string sub = cl.substr(index + 2);  //这里获取的正文长度为字符串
        int size = StringToInt(sub);            //将sub转换成整型

        char c;
        for(auto i = 0; i < size; i++)
        {
            recv(sock, &c, 1, 0);
            (rq.text).push_back(c);
        }
    }

     //客户端向服务器发送Request
    static void SendRequest(int sock, Request &rq)
    {
        string &m = rq.method;
        string &cl = rq.content_length;
        string &b = rq.blank;
        string &text = rq.text;
        // send(sockfd, buff, size, 0);
        //为什么不直接吧rq直接send过去？
        //因为Rq是一个类，其中包含的东西太多了
        //send：发送到缓冲区

        send(sock, m.c_str(), m.size(), 0);
        send(sock, cl.c_str(), cl.size(), 0);
        send(sock, b.c_str(), b.size(), 0);
        send(sock, text.c_str(), text.size(), 0);
    }
      //UDP
    static void RecvMessage(int sock, string &message, struct sockaddr_in &peer)
    {
        //UDP收数据函数：recvfrom
        //TCP收数据函数：recv
        char msg[MESSAGE_SIZE];
        socklen_t len = sizeof(peer);
        size_t s = recvfrom(sock, msg, sizeof(msg) - 1, 0,  (struct sockaddr*)&peer, &len);
        if(s < 0)
        {
            LOG("recvfrom message error\n", 1);
        }
        else
        {
            message = msg;
        }
    }

    static void SendMessage(int sock, const string &message, struct sockaddr_in &peer)
    {
        ssize_t s = sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr *)&peer, sizeof(peer));
        if(s < 0)
        {
            LOG("sendto message error\n", 1);
        }
    }
    static void addUser(vector<string> &online, string &f)
    {
        for(auto it = online.begin(); it != online.end(); it++)
        {
            if(*it == f)
            {
                return;
            }
        }
        online.push_back(f);
    }
    static void removeUser(vector<string> &online, string &f)
    {
        for(auto it = online.begin(); it != online.end(); it++)
        {
            if(*it == f)
            {
                online.erase(it);
		return;
            }
        }
        
    }
};



//
class SocketApi{
public:
    //创建套接字
    static int Socket(int type)
    {
         //创建套接字
        int sock = socket(AF_INET, type, 0);
        //创建失败
        if(sock < 0)
        {
            //打印日志
            LOG("socket err!", ERROR);
            exit(2);
        }
        return sock;
    }
    //绑定套接字
    static int Bind(int sock, int port)
    {
        struct sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = htonl(INADDR_ANY); //0
        local.sin_port = htons(port);

        if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0)
        {
            LOG("Bind err!", ERROR);
            exit(3);
        }
    }
    //监听
    static void Listen(int sock)
    {
        //BACKLOG：底层链接队列个数
        if(listen(sock, BACKLOG) < 0)
        {
            LOG("Listen err!", ERROR);
            exit(4);
        }
    }
    //接受连接
    static int Accept(int listen_sock, string &out_ip, int &out_port)
    {
        //客户端信息
        struct sockaddr_in peer;
        //客户端地址信息长度
        socklen_t len = sizeof(peer);
        int sock = accept(listen_sock, (struct sockaddr*)&peer, &len);
        if(sock < 0)
        {
            LOG("Accpet err!", WARNING);
            return -1;
        }
        out_ip = inet_ntoa(peer.sin_addr); //4字节IP转点分十进制
        out_port = htons(peer.sin_port);

        return sock;
    }
    //请求连接
    static bool Connect(int sock, string peer_ip, int port)
    {
         struct sockaddr_in peer;
        peer.sin_family = AF_INET;
        //c_str()函数返回一个指向正规C字符串的指针常量, 内容与本string串相同
        peer.sin_addr.s_addr = inet_addr(peer_ip.c_str()); //inet_addr:将点分十进制地址转换成4字节IP地址
        peer.sin_port = htons(port);

        if(connect(sock, (struct sockaddr*)&peer, sizeof(peer)) < 0)
        {
            LOG("Connect err!", WARNING);
            return false;
        }
        return true;
        }

};















