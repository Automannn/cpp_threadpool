//
// Created by 14394 on 2020/2/13.
//

#include "lib/include/ThreadPool.h"
#include "lib/include/Task.h"



int main(){
    ThreadPool threadPool(2,6);
    for(size_t i=0;i<10;i++){
        threadPool.QueueTaskItem(Task::task1,NULL,TaskCallBack::taskCallback1);
    }
    threadPool.QueueTaskItem(Task::task1,NULL,TaskCallBack::taskCallback1,TRUE);

    //system("pause");
    return 0;
}
