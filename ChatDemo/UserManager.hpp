/*
* 用户信息管理
*/
/*
* 注册过程：
 1. 用户发送自己的昵称，学校和密码
 2. 服务器根据客户端传入的信息生成一个id，并封装成一个User对象
 3. 将生成的User对象插入到users中
 4. 将生成的id返回给客户端
 */
 /*
 * 登录认证过程
 1. 将id作为users的key值，进行查找，若没有，则登陆失败
 2. 若找到，则将登陆密码和对应user对象的密码比对，若相等，则登陆成功，不相同，登陆失败
 3. 登陆成功，则将其id插入到online_user中
 */
#include <iostream>
#include <string>
#include <unordered_map>   //方便对用户信息进行查询
#include <pthread.h>
using namespace std;

//用户信息类
class User{
private:
    string nick_name;       //用户昵称
    string school;              //用户学校
    string passwd;              //登陆密码
public:
    User(){}
    User(const string &_nick_name, const string &_school, const string &_passwd) :
        nick_name(_nick_name), school(_school), passwd(_passwd){}
    ~User(){}
    //检验passwd
    bool IsPasswdOk(const string &pwd)
    {
        return passwd == pwd ? true : false;
    }
    string &GetNickName(){ return nick_name; }
    string &GetSchool(){ return school; }
};
class UserManager{
private:
    unordered_map<unsigned int, User> users;         //所有用户,便于查询，所以用map <id, User>
    unordered_map<unsigned int, struct sockaddr_in> online_users;  //所有在线用户，便于删除插入<id, addr>
    unsigned int assign_id;              //返回给用户的id
    pthread_mutex_t lock;

    void Lock(){ pthread_mutex_lock(&lock); } //加锁
    void Unlock(){ pthread_mutex_unlock(&lock); }  //解锁
public:
    UserManager() : assign_id(10000)
    {
        //初始化锁
       pthread_mutex_init(&lock, NULL);
    }
    ~UserManager()
    {
        //销毁锁
        pthread_mutex_destroy(&lock);
    }
    unsigned int Insert(const string &nick_name, const string &school, const string &passwd)
    {
        //在此时考虑到，在你注册的时候，别人也在注册，那么id怎么分配？
        //那么考虑到unordered_map是个临界资源,加锁

        //加锁
        Lock();
        //分配一个id
        //如果设置以下语句直接++分配id，那么别人可以通过注册一个用户推算出这个服务器的注册人数，不安全
        unsigned int id = assign_id++;  //assign_id：无符号整数，可达42亿

        //构造一个User对象
        User u(nick_name, school, passwd);
        //将u插入到users表,先检验合法性(恶意注册)，查看user是否在users中
        if(users.find(id) == users.end())
        {
            //没有找到，则插入
            //users.insert(make_pair(id, u));
            users.insert({id, u});
            //解锁
            Unlock();
            return id;
        }
        Unlock();
        return 1;   //注册错误
    }
    
    unsigned int Check(const int &id, const string &passwd)
    {
        //在查的时候别人正在注册
        Lock();
        auto it = users.find(id);  //it是一个unordered_map，里面有key，和value
        if(it != users.end())
        {
            //用户存在，检查密码
            // it->first 取到key值
            // it->second 取到value值
            User &u = it->second;
            if(u.IsPasswdOk(passwd))
            {
                Unlock();
                return id;
            }
        }
        Unlock(); 
        return 2;
    }
    //将用户插入到在线用户列表中
    void AddOnlineUser(unsigned int id, struct sockaddr_in &peer)
    {
        Lock();
        auto it = online_users.find(id);
        if(it == online_users.end())
        {
            online_users.insert({id, peer});
        }
        Unlock();
    }
    //将用户从在线用户列表中删除
    void RemoveOnlineUser(unsigned int id)
    {
	Lock();
        auto it = online_users.find(id);
        online_users.erase(it);
        Unlock();
    }
    void GetUserInfo(const unsigned int &id, string &_name, string &_school)
    {
	Lock();
        _name = users[id].GetNickName();
	_school = users[id].GetSchool();
        Unlock();
    }
    unordered_map<unsigned int, struct sockaddr_in> OnlineUser()
    {
        Lock();
        auto online = online_users;
        Unlock();
        return online;
    }
};

