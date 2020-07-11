/*
* 日志打印
* 功能：通过打印日志信息，排查代码bug
*/
#pragma once
#include <iostream>
#include <string>
#include <stdio.h>

using namespace std;

//宏定义日志级别
#define NORMAL  0
#define WARNING 1
#define ERROR   2

//日志级别和字符串进行映射
const char *log_level[] = {
    "Normal",
    "Warning",
    "Error",
    nullptr
};

//日志信息
/*
* msg：日志信息
* level：日志级别(error, warning, normal)
* file：在哪个文件中
* line：在哪一行
*/
/*
__FILE__：输入的是哪个文件
__LINE__：当前是第几行
*/
void Log(const string msg, int level, string file, int line)
{
    cout << "[" << msg << "]" << "[" << log_level[level] << "]" << " : " << file << " : " << line << endl;
}

#define LOG(msg, level) Log(msg, level, __FILE__, __LINE__)














