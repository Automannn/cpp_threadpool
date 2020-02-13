//
// Created by 14394 on 2020/2/13.
//

#ifndef AUTOMANNN_THREADPOOL_TASK_H
#define AUTOMANNN_THREADPOOL_TASK_H

class Task {
public:
    static int task1(PVOID param);
};

class TaskCallBack{
public:
    static void taskCallback1(int result);
};


#endif //AUTOMANNN_THREADPOOL_TASK_H
