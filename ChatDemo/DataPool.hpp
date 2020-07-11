/*
数据池：里面放的全是客户发的消息：string
服务器只起转发作用：不需要知道消息是什么
生产者消费者模型中的缓冲区：环形队列【数组】
使用vector
取模运算：防止下标越界

描述：
1. 当一个生产者和一个消费者操作环形队列中不同数据时，就可以同时进行
2. 当一个生产者和一个消费者操作环形队列中相同数据时
    两种情况：空或满

信号量：描述临界资源的个数，实际上就是一个计数器
【注意】：只要把信号量申请成功，表明这个资源就是你的，不管你使不使用，都是你的
1. 信号量是一个计数器，那可不可以用一个整型表示？不可以
    要想访问临界i资源，必须先访问信号量，相当于信号量也是一个临界资源，要保证信号量本身是安全的，才能保证临界资源的安全
    P、V操作是原子操作，信号量是原子的，才能保证自己不出错  一个整型变量不是原子（在++和--中可能会被中断）的（在内存中）

2. PutMessage/GetMessage
 - 当生产者和消费者同时访问空的缓冲区时，绝对是生产者先运行
 - 不管是空和满，product_step = 0;当满的时候，只能消费者先运行
 - 生产者绝对不能将消费者套一个圈，生产者已经生产满了，还在继续生产（信号量约束，blank_sem）
 - 消费者绝对不能超过生产者
 - 只有为空或为满，生产者和消费者指向同一位置

 生产者和生产者之间是竞争关系，消费者和消费者之间也是竞争互斥关系
 生产者和消费者之间是互斥或同步关系

 3. 在代码中，只设置了一个线程来生产，一个线程来消费
    所以只需要维护生产者和消费者之间的关系即可
*/
#include <iostream>
#include <vector>
#include <string>
#include <semaphore.h>

class DataPool{
private:
    vector<string> pool;
    int cap;        //环形队列的大小
    sem_t data_sem;
    sem_t blank_sem;
    int product_step;
    int consume_step;
public:
    DataPool(int _cap = 512) : cap(_cap), pool(_cap)
    {
        sem_init(&data_sem, 0, 0);        //最开始的时候，数据缓冲区中没有数据
        sem_init(&blank_sem, 0, cap);  //最开始的时候，数据缓冲区中空的个数为环形队列的大小
        product_step = 0;       //缓冲队列的头
        consume_step = 0;       //缓冲队列的尾
    }
    //往数据池put数据
    void PutMessage(const string &msg)
    {
        //生产者先看循环缓冲区中的空格数量
        sem_wait(&blank_sem);
        pool[product_step] = msg;
        product_step++;
        product_step %= cap;
        sem_post(&data_sem);
    }
    //从数据池中拿数据
    void GetMessage(string &msg)
    {
        sem_wait(&data_sem);
        msg = pool[consume_step];
        consume_step++;
        consume_step %= cap;
        sem_post(&blank_sem);
    }
    ~DataPool()
    {
        sem_destroy(&data_sem);
        sem_destroy(&blank_sem);
    }
};

