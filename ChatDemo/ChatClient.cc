#include "ChatClient.hpp"
#include <iostream>
#include <string>

using namespace std;

static void Usage(string proc)
{
    cout << "Usage: " << proc << " peer_ip" << endl;
}

//提示用户进行登录界面
static void Menu(int &s)
{
    cout << "**********************************************" << endl;
    cout << "**************    1.   Register   *****************" << endl;
    cout << "**************    2.   Login       *****************" << endl;
    cout << "**************    3.  Exit           *****************" << endl;
    cout << "Please Select：> " << endl;
    cin >> s;
}

// ./ChatClient ip
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(0);
    }
    //创建一个客户端
    ChatClient *cp = new ChatClient(argv[1]);
    cp->InitClient();
    int select = 0;
    while(1)
    {
        Menu(select);
        switch(select)
        {
            case 1:     //Register
                cp->Register();
                break;
            case 2:     //Login
                if(cp->Login())
                {
                    cp->Chat();
		    cp->Logout();
                }
                break;
            case 3:     //exit
                exit(0);
            default:
                exit(1);
                break;
        }
    }



   // if(cp->ConnectServer())
    //{
     //   cout << "connect success!" << endl;
    //}
    return 0;
}

