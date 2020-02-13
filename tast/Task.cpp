//
// Created by 14394 on 2020/2/13.
//

#include "Task.h"

int Task::task1(PVOID param) {
    int i=10;
    while (i>=0){
        printf("this is: %d\n",i);
        Sleep(3000);
        i--;
    }
    return i;
}

void TaskCallBack::taskCallback1(int result) {
    printf("this is a callbackFun!\n");
}
