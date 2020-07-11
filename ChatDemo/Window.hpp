#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>

using namespace std;

#define MESSAGE_SIZE 1024

class Window{
public:
	WINDOW *header;
	WINDOW *output;
	WINDOW *online;
	WINDOW *input;	

	pthread_mutex_t lock;
public:
	Window()
	{
		initscr();	//初始化屏幕
		curs_set(0);	//隐藏光标
		pthread_mutex_init(&lock, NULL);
	}
	void SafeWrefresh(WINDOW *w)  //安全刷新
	{
		pthread_mutex_lock(&lock);
		wrefresh(w);
		pthread_mutex_unlock(&lock);
	}
	void DrawHeader()  //画一个窗口
	{
		//LINES：屏幕高度
		//COLS：屏幕宽度
		int h = LINES * 0.2;
		int w = COLS;
		int y = 0;
		int x = 0;
		//参数：高，宽，第几行(y)，第几列(x)[最后两个参数为左上角坐标]
		header = newwin(h, w, y, x);
		//绘制边框:
		//'-'：左右边框
		//'%'：上下边框
		//box(w, '-', '%');
		box(header, 0, 0);  //缺省
		//刷新窗口
		SafeWrefresh(header);
	}
	void DrawOutput()  //画一个窗口
	{
		int h = LINES * 0.6;
		int w = COLS * 0.75;
		int y = LINES * 0.2;
		int x = 0;
		output = newwin(h, w, y, x);
		box(output, 0, 0);
		SafeWrefresh(output);
	}
	void DrawOnline()  //画一个窗口
	{
		int h = LINES * 0.6;
		int w = COLS * 0.25;
		int y = LINES * 0.2;
		int x = COLS * 0.75;
		online = newwin(h, w, y, x);
		box(online, 0, 0);
		SafeWrefresh(online);
	}
	void DrawInput()  //画一个窗口
	{
		int h = LINES * 0.2;
		int w = COLS;
		int y = LINES * 0.8;
		int x = 0;
		input = newwin(h, w, y, x);
		box(input, 0, 0);

		string tips = "Please Enter:> ";
		PutStringToWin(input, 2, 2, tips);
		SafeWrefresh(input);
	}
	//把字符串显示到窗口上
	void PutStringToWin(WINDOW *w, int y, int x, string &message)
	{
		//mvwaddstr(win, y, x, string_.c_str());
		//参数：字符串放置到哪个窗口，放到窗口的第几行，第几列
		mvwaddstr(w, y, x, message.c_str());
		SafeWrefresh(w);
	}
	//从input窗口获取消息
	void GetStringFormInput(string &message)
	{	
		char buffer[MESSAGE_SIZE];
		memset(buffer, 0, sizeof(buffer));
		//直接从input窗口获取消息
		wgetnstr(input, buffer, sizeof(buffer));
		message = buffer;
		delwin(input);
		DrawInput();
	}
	//把input的内容输出到字符串中
	void PutMessageToOutput(string &message)
	{
		static int line = 1;

		int y, x;
		getmaxyx(output, y, x);
		if(line > y - 2)
		{
			delwin(output);
			DrawOutput();
			line = 1;
		}
		PutStringToWin(output, line++,2, message);
	}
	void PutUserToOnline(vector<string> &online_users)
	{
	    int size = online_users.size();
	    for(int i = 0; i < size; i++)
	    {
		    //此处应考虑万一在线用户很多，窗口溢出
		    PutStringToWin(online, i + 1, 2, online_users[i]);
	    }
	}
	void RefreshUserToOnline(vector<string> &online_users)
	{
	    DrawOnline(); 
	    PutUserToOnline(online_users);
	}
	void Welcome()
	{
		string welcome = "Welcome to my chat system!";
		int num = 1;
		int y, x;
		int dir = 0; //left -> right
		while(1)
		{
			DrawHeader();
			getmaxyx(header, y, x);
			PutStringToWin(header, y / 2, num, welcome);
			if(dir == 0)
			{
				num++;
			}
			else
			{
				num--;
			}
			if(num > x - welcome.size() - 2)
			{
				dir = 1;
			}
			else if(num <= 1)
			{
				dir = 0;
			}
			usleep(100000);
			delwin(header);
			
		}
	}
	~Window()
	{
		
		endwin();
		pthread_mutex_destroy(&lock);
	}

};
