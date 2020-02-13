//
// Created by 14394 on 2020/2/13.
//

#include "ThreadPool.h"
#include <process.h>

ThreadPool::ThreadPool(size_t minNumOfThread,size_t maxNumOfThread){
    if(minNumOfThread<2)
        this->minNumOfThread =2;
    else
        this->minNumOfThread=minNumOfThread;

    if(maxNumOfThread<this->minNumOfThread*2)
        this->maxNumOfThread = this->minNumOfThread*2;
    else
        this->maxNumOfThread = maxNumOfThread;

    //停止事件
    stopEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    //实例化完成端口,分配内存IO
    completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);

    idleThreadList.clear();
    createIdleThread(this->minNumOfThread);

    busyThreadList.clear();

    //实例化分发线程
    dispatchThread = (HANDLE) _beginthreadex(0,0,GetTaskThreadProc,this,0,0);
    numOfLongFun =0;
}

ThreadPool::~ThreadPool() {
    SetEvent(stopEvent);
    //触发完成端口通知
    PostQueuedCompletionStatus(completionPort,0,EXIT,NULL);

    CloseHandle(stopEvent);
}

BOOL ThreadPool::QueueTaskItem(TaskFun task, PVOID param, TaskCallbackFun taskCb, BOOL longFun) {
    waitTaskLock.Lock();
    WaitTask* waitTask = new WaitTask(task,param,taskCb,longFun);
    waitTaskList.push_back(waitTask);
    waitTaskLock.UnLock();
    //通知完成端口
    PostQueuedCompletionStatus(completionPort,0,GET_TASK,NULL);
    return TRUE;
}

void ThreadPool::createIdleThread(size_t size) {
    idleThreadLock.Lock();
    for(size_t i=0;i<size;i++){
        idleThreadList.push_back(new Thread(this));
    }
    idleThreadLock.UnLock();
}

void ThreadPool::deleteIdleThread(size_t size) {
    idleThreadLock.Lock();
    size_t t = idleThreadList.size();
    if(t>= size){
        for(size_t i=0;i<size;i++){
            auto thread = idleThreadList.back();
            delete thread;
            idleThreadList.pop_back();
        }
    }else{
        for(size_t i=0;i<t;i++){
            auto thread = idleThreadList.back();
            delete  thread;
            idleThreadList.pop_back();
        }
    }
    idleThreadLock.UnLock();
}

ThreadPool::Thread* ThreadPool::getIdleThread() {
    Thread * thread = NULL;
    idleThreadLock.Lock();

    if(idleThreadList.size()>0){
        thread = idleThreadList.front();
        idleThreadList.pop_front();
    }
    idleThreadLock.UnLock();

    if(thread == NULL && getCurNumOfThread()<maxNumOfThread){
        thread = new Thread(this);
        InterlockedIncrement(&maxNumOfThread);
    }
    return thread;
}

void ThreadPool::moveBusyThreadToIdleList(ThreadPool::Thread *busyThread) {
    idleThreadLock.Lock();
    idleThreadList.push_back(busyThread);
    idleThreadLock.UnLock();

    busyThreadLock.Lock();
    for(auto it=busyThreadList.begin();it!=busyThreadList.end();it++){
        if(*it == busyThread){
            busyThreadList.erase(it);
            break;
        }
    }
    busyThreadLock.UnLock();

    if(maxNumOfThread != 0 && idleThreadList.size()>maxNumOfThread*0.8){
        deleteIdleThread(idleThreadList.size()/2);
    }
    PostQueuedCompletionStatus(completionPort,0,GET_TASK,NULL);
}

void ThreadPool::moveThreadToBusyList(ThreadPool::Thread *thread) {
    busyThreadLock.Lock();
    busyThreadList.push_back(thread);
    busyThreadLock.UnLock();
}

void ThreadPool::getTaskExecute() {
    Thread *thread = NULL;
    WaitTask *waitTask = NULL;

    waitTask = getTask();

    if(waitTask ==NULL){
        return;
    }
    //如果是时常任务
    if(waitTask->bLong){
        if(idleThreadList.size()>minNumOfThread){
            thread = getIdleThread();
        }else{
            thread = new Thread(this);
            InterlockedIncrement(&numOfLongFun);
            InterlockedIncrement(&maxNumOfThread);
        }
    } else{//若不是时长任务
        thread = getIdleThread();
    }
    if(thread!=NULL){
        thread->executeTask(waitTask->task,waitTask->param,waitTask->taskCb);
        delete waitTask;
        moveThreadToBusyList(thread);
    } else{
        waitTaskLock.Lock();
        waitTaskList.push_front(waitTask);
        waitTaskLock.UnLock();
    }
}

ThreadPool::WaitTask* ThreadPool::getTask() {
    WaitTask *waitTask =NULL;
    waitTaskLock.Lock();
    if (waitTaskList.size()>0){
        waitTask = waitTaskList.front();
        waitTaskList.pop_front();
    }
    waitTaskLock.UnLock();
    return waitTask;
}

ThreadPool::Thread::Thread(ThreadPool *threadPool):busy(FALSE),thread(INVALID_HANDLE_VALUE),task(NULL),taskCb(NULL),exit(FALSE),threadPool(threadPool)
{
    thread = (HANDLE)_beginthreadex(0, 0, ThreadProc, this, CREATE_SUSPENDED, 0);
}

ThreadPool::Thread::~Thread() {
    exit = TRUE;
    task = NULL;
    taskCb = NULL;
    ResumeThread(thread);
    WaitForSingleObject(thread,INFINITE);
    CloseHandle(thread);
}

BOOL ThreadPool::Thread::isBusy() {
    return busy;
}

void ThreadPool::Thread::executeTask(TaskFun task, PVOID param, TaskCallbackFun taskCallback) {
    busy = TRUE;
    this->task = task;
    this->param = param;
    this->taskCb = taskCallback;
    ResumeThread(thread);
}

unsigned int ThreadPool::Thread::ThreadProc(PVOID pM) {
    Thread *pThread = (Thread*)pM;
    while (true){
        if(pThread->exit)break; //线程退出
        //首次检查
        if(pThread->task==NULL&&pThread->taskCb==NULL){
            pThread->busy = FALSE;
            pThread->threadPool->moveBusyThreadToIdleList(pThread);
            SuspendThread(pThread->thread);
            continue;
        }
        //构造方法
        int result =pThread->task(pThread->param);
        if(pThread->taskCb)pThread->taskCb(result);

        WaitTask *waitTask = pThread->threadPool->getTask();
        //取到了任务就继续执行
        if(waitTask!=NULL){
            pThread->task = waitTask->task;
            pThread->taskCb = waitTask->taskCb;
            delete waitTask;
            continue;
        } else{
            pThread->task =NULL;
            pThread->param = NULL;
            pThread->taskCb =NULL;
            pThread->busy = FALSE;
            pThread->threadPool->moveBusyThreadToIdleList(pThread);
            SuspendThread(pThread->thread);
        }
    }
    return 0;
}

ThreadPool::WaitTask::WaitTask(TaskFun task,PVOID param,TaskCallbackFun taskCb,BOOL bLong){
    this->task= task;
    this->param = param;
    this->taskCb = taskCb;
    this->bLong = bLong;
}

ThreadPool::WaitTask::~WaitTask() {
    task=NULL;
    taskCb = NULL;
    bLong= FALSE;
    param = NULL;
}

ThreadPool::CriticalSectionLock::CriticalSectionLock() {
    InitializeCriticalSection(&cs);
}
ThreadPool::CriticalSectionLock::~CriticalSectionLock() {
    DeleteCriticalSection(&cs);
}
void ThreadPool::CriticalSectionLock::Lock() {
    EnterCriticalSection(&cs);
}
void ThreadPool::CriticalSectionLock::UnLock() {
    LeaveCriticalSection(&cs);
}

//从任务队列取任务的线程函数
 unsigned ThreadPool::GetTaskThreadProc(PVOID pM){
    ThreadPool* threadPool = (ThreadPool*)pM;
    BOOL bRet = FALSE;
    DWORD dwBytes = 0;
    WAIT_OPERATION_TYPE  opType;
    OVERLAPPED* ol;
    while(WAIT_OBJECT_0!=WaitForSingleObject(threadPool->stopEvent,0)){
        BOOL  bRet = GetQueuedCompletionStatus(threadPool->completionPort,&dwBytes,(PULONG_PTR)&opType,&ol,INFINITE);
        if(EXIT ==opType){
            break;
        } else if(GET_TASK ==opType){
            threadPool->getTaskExecute();
        }
    }
    return 0;
}

size_t ThreadPool::getCurNumOfThread() {
    return getIdleThreadNum()+getBusyThreadNum();
}

size_t ThreadPool::getMaxNumOfThread() {
    return maxNumOfThread-numOfLongFun;
}

void ThreadPool::setMaxNumOfThread(size_t size) {
    if(size<numOfLongFun){
        maxNumOfThread = size+numOfLongFun;
    } else{
        maxNumOfThread =size;
    }
}

size_t ThreadPool::getMinNumOfThread() {
    return minNumOfThread;
}

void ThreadPool::setMinNumOfThread(size_t size) {
    minNumOfThread = size;
}

size_t ThreadPool::getIdleThreadNum() {
    return idleThreadList.size();
}

size_t ThreadPool::getBusyThreadNum() {
    return busyThreadList.size();
}





