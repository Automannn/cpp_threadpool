//
// Created by 14394 on 2020/2/13.
//
#include "ThreadPool.h"
#include "TestTask.h"
int main(){
    ThreadPool threadPool(2,6);
    for(size_t i=0;i<5;i++){
        threadPool.QueueTaskItem(Task::task1,NULL,TaskCallBack::taskCallback1);
    }
    threadPool.QueueTaskItem(Task::task1,NULL,TaskCallBack::taskCallback1,TRUE);
    printf("run away!");
    system("pause");
    return 0;
}
