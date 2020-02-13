//
// Created by 14394 on 2020/2/13.
//

#ifndef CPP_THREADPOOL_THREADPOOL_H
#define CPP_THREADPOOL_THREADPOOL_H

#include <Windows.h>
#include <list>
#include <queue>
#include <memory>

#define THRESHOLE_OF_WAIT_TAST 20

typedef int(*TaskFun) (PVOID param);
typedef void(*TaskCallbackFun) (int result);

class ThreadPool {
private:
    //线程内部类
    class Thread{
    public:
        Thread(ThreadPool *threadPool);
        ~Thread();

        BOOL  isBusy(); //是否有任务在执行
        void executeTask(TaskFun task,PVOID param,TaskCallbackFun taskCallback); //执行任务
    private:
        ThreadPool *threadPool;  //所属线程池
        BOOL  busy; //是否有任务在执行
        BOOL  exit;//是否退出
        HANDLE  thread; //线程句柄
        TaskFun  task;  //要执行的任务
        PVOID param; //任务参数
        TaskCallbackFun  taskCb; //回调任务
        static unsigned int __stdcall ThreadProc(PVOID pM);  //线程函数
    };

    //IOCP的通知种类
    enum WAIT_OPERATION_TYPE{
        GET_TASK, //获取到任务
        EXIT //退出
    };

    //待执行的任务类
    class WaitTask{
    public:
        WaitTask(TaskFun task,PVOID param,TaskCallbackFun taskCb,BOOL bLong);
        ~WaitTask();

        TaskFun  task;//要执行的任务
        PVOID param;//任务参数
        TaskCallbackFun taskCb;// 回调的任务
        BOOL bLong; //是否时长任务
    };

    //从任务列表获取任务的线程数
    static unsigned __stdcall GetTaskThreadProc(PVOID pM);

    //线程临界区锁
    class CriticalSectionLock{
    private:
        CRITICAL_SECTION  cs;//临界区
    public:
        CriticalSectionLock();
        ~CriticalSectionLock();

        void Lock();
        void UnLock();
    };

public:
    ThreadPool(size_t minNumOfThread =2,size_t maxNumOfThread =10);
    ~ThreadPool();

    BOOL QueueTaskItem(TaskFun task,PVOID param,TaskCallbackFun taskCb = NULL,BOOL longFun = FALSE); //任务入队

private:
    size_t  getCurNumOfThread(); //获取线程池中的当前线程数
    size_t getMaxNumOfThread(); //获取线程池中的最大线程数
    void setMaxNumOfThread(size_t size); //设置线程池中的最大线程数
    size_t getMinNumOfThread(); //获取线程池中的最小线程数
    void setMinNumOfThread(size_t size); //设置线程池中的最小线程数

    size_t getIdleThreadNum(); //或许线程池中的空闲线程数
    size_t getBusyThreadNum(); //获取线程池中的运行线程数
    void createIdleThread(size_t size); //创建空闲线程
    void deleteIdleThread(size_t size); //删除空闲线程
    Thread *getIdleThread(); //或许空闲线程
    void moveBusyThreadToIdleList(Thread *busyThread); //忙碌线程加入空闲队列
    void moveThreadToBusyList(Thread *thread); //线程加入忙碌列表
    void getTaskExecute(); //从任务队列中取任务执行

    WaitTask *getTask(); //从任务队列中取任务

    CriticalSectionLock idleThreadLock; //空闲线程列表锁
    std::list<Thread *> idleThreadList; //空闲线程列表
    CriticalSectionLock busyThreadLock; //忙碌线程列表锁
    std::list<Thread *> busyThreadList; //忙碌线程列表


    CriticalSectionLock waitTaskLock; //任务锁
    std::list<WaitTask*> waitTaskList; //任务列表

    HANDLE dispatchThread; //分发任务线程
    HANDLE stopEvent; //通知线程退出的事件
    HANDLE completionPort; //完成端口

    size_t maxNumOfThread; //线程池中最大的线程数
    size_t minNumOfThread; //线程池中最小的线程数
    size_t numOfLongFun; //线程池中时常 线程数
};


#endif //CPP_THREADPOOL_THREADPOOL_H
