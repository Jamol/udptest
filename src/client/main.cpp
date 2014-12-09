//
//  main.cpp
//  udpclient
//
//  Created by Folki Bao on 7/11/14.
//  Copyright (c) 2014 wme. All rights reserved.
//

#include <iostream>
#include "util.h"

extern "C" void startTest(const char* bind_addr, uint16_t bind_port, const char* peer_addr,
                          uint16_t peer_port, uint32_t bw, uint32_t slice_ms, uint32_t burst_bytes);
extern "C" void stopTest();
extern "C" void waitTest();

void usage()
{
    printf("udpclient peer_addr peer_port bw burst\n");
}

int main(int argc, const char * argv[])
{
    const char* peer_addr = "10.140.49.144";
    uint16_t peer_port = 55555;
    
    if(argc >= 2 && strcmp(argv[1], "-h") == 0){
        usage();
        return 0;
    }
    if(argc > 2){
        peer_addr = argv[1];
        peer_port = atoi(argv[2]);
    }
    uint32_t bw = 200 * 1024;
    uint32_t slice_ms = 10;
    uint32_t burst_bytes = 30720;
    if (argc > 3) {
        bw = atoi(argv[3]);
    }
    if (argc > 4) {
        burst_bytes = atoi(argv[4]);
    }
    
#ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(1, 1);
    int nResult = WSAStartup(wVersionRequested, &wsaData);
    if (nResult != 0)
    {
        return 1;
    }
#endif
    
    startTest("0.0.0.0", 0, peer_addr, peer_port, bw, slice_ms, burst_bytes);
    waitTest();
    
#ifdef WIN32
    WSACleanup();
#endif
    return 0;
}

