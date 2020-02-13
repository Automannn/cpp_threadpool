//
// Created by 14394 on 2020/2/13.
//

#include "TestTask.h"
#include <stdio.h>
int Task::task1(PVOID param) {
    int i=10;
    while (i>=0){
        printf("this is: %d\n",i);
        Sleep(100);
        i--;
    }
    return i;
}

void TaskCallBack::taskCallback1(int result) {
    printf("this is thread call back function;\n");
}