//
//  main.cpp
//  udpserver
//
//  Created by Folki Bao on 7/11/14.
//  Copyright (c) 2014 wme. All rights reserved.
//

#include <iostream>
#include "util.h"

extern "C" void startTest(const char* bind_addr, uint16_t bind_port);
extern "C" void stopTest();
extern "C" void waitTest();

int main(int argc, const char * argv[])
{
    uint16_t bind_port = 55555;
    if(argc > 1){
        bind_port = atoi(argv[1]);
    }
    char my_ip[64];
    if(wtp_get_local_ip(my_ip, sizeof(my_ip)) == 0){
        my_printf("my ip is %s\n", my_ip);
    }
    startTest("0.0.0.0", bind_port);
    waitTest();
    return 0;
}

