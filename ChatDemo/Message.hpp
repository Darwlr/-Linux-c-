/*
聊天时的消息格式
*/
#include <iostream>
#include <string>
#include "jsoncpp/include/json.h"
#include "ProtocolUtil.hpp"

#define NORMAL_TYPE 0
#define LOGIN_TYPE 1
#define LOGOUT_TYPE 2
#define PRIVATE_TYPE 3

class Message{
private:
    string nick_name;
    string school;
    string text;
    unsigned int id;
    unsigned int type;

    unsigned int private_id;   //私聊ID
public:
    Message(){}
    Message(const string &name, const string &_school, const string &_text, const unsigned int &_id, const unsigned int &_type = NORMAL_TYPE)
    {
        nick_name = name;
        school = _school;
        text = _text;
        id = _id;
	type = _type;
    }
    //序列化和反序列化
    void ToSendString(string &sendString)   //将Message对象转成字符串进行发送
    {
        //序列化
        Json::Value root;
        root["nick_name"] = nick_name;
        root["school"] = school;
        root["text"] = text;
        root["id"] = id;
	root["type"] = type;
        Util::Seralizer(root, sendString);
    }
    void ToRecvValue(string &recvString)
    {
        Json::Value root;
        Util::UnSeralizer(recvString, root);

        //反序列化
        nick_name = root["nick_name"].asString();
        school = root["school"].asString();
        text = root["text"].asString();
        id = root["id"].asInt();
	type = root["type"].asInt();

    }
    const string &NickName(){ return nick_name; }
    const string &School(){ return school; }
    const string &Text(){ return text; }
    const unsigned int &Id(){ return id; }
    const unsigned int &Type(){ return type; }
    ~Message(){}

};

