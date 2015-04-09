//
//  udptest.cpp
//  udptest
//
//  Created by Jamol Bao on 7/8/14.
//  Copyright (c) 2014. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <chrono>

#include "util.h"

using namespace std::chrono;

#define now_tick_ms() duration_cast<milliseconds>(steady_clock::now().time_since_epoch())

static std::thread s_thread;
static bool s_stop = false;
extern "C" void startTest(const char* bind_addr, uint16_t bind_port, const char* peer_addr,
                          uint16_t peer_port, uint32_t bw, uint32_t slice_ms, uint32_t burst_bytes)
{
    std::string str_bind_addr = bind_addr;
    std::string str_peer_addr = peer_addr;
    try
    {
        s_thread = std::thread ([=] () mutable {
            my_printf("bind_addr=%s, bind_port=%d, peer_addr=%s, peer_port=%d, bw=%u, slice=%u, burst_bytes=%u\n",
                      str_bind_addr.c_str(), bind_port, str_peer_addr.c_str(), peer_port, bw, slice_ms,
                      burst_bytes);
            
            int sfd = create_udp_fd(str_bind_addr.c_str(), bind_port);
            if(sfd < 0){
                return;
            }
            int buf_size = 64*1024;
            //setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (char*)&buf_size, sizeof(buf_size));
            setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (char*)&buf_size, sizeof(buf_size));

            sockaddr_storage ss_addr = {0};
            socklen_t addr_len = sizeof(ss_addr);
            if(getsockname(sfd, (struct sockaddr*)&ss_addr, &addr_len) == 0){
                char bind_addr[64] = {0};
                unsigned short bind_port = 0;
                km_get_sock_addr((struct sockaddr*)&ss_addr, addr_len, bind_addr, sizeof(bind_addr), &bind_port);
                my_printf("bind to %s:%d\n", bind_addr, bind_port);
            }
            struct addrinfo hints = {0};
            addr_len = sizeof(ss_addr);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_flags = AI_ADDRCONFIG;
            
            km_set_sock_addr(str_peer_addr.c_str(), peer_port, &hints, (struct sockaddr *)&ss_addr, addr_len);
            if(AF_INET == ss_addr.ss_family) {
                addr_len = sizeof(sockaddr_in);
            } else {
                addr_len = sizeof(sockaddr_in6);
            }
            unsigned char buf[65535];
            
            // construct RTP header
            buf[0] = 2 << 6;
            buf[1] = 0x80|101; // marker & payload type
            *(uint32_t*)(buf + 4) = 0; // timestamp
            *(uint32_t*)(buf + 8) = htonl(12345678); // ssrc
            
            my_printf("start to send data...\n");
            unsigned short sequence = 0;
            unsigned short* p_seq = (unsigned short*)(buf + 2);
            uint32_t loop_count = 1000/slice_ms;
            uint32_t average_bytes_per_slice = bw/loop_count;
            uint32_t bytes_sent_in_this_second = 0;
            milliseconds start_tick = now_tick_ms();
            uint32_t cur_send_bytes = 0;
            if(burst_bytes < average_bytes_per_slice){
                burst_bytes = average_bytes_per_slice;
            }
            srand((unsigned int)time(NULL));
            while (!s_stop) {
                milliseconds second_start_tick = now_tick_ms();
                bytes_sent_in_this_second = 0;
                for (uint32_t i=0; i<loop_count; ++i) {// this second
                    milliseconds slice_start_tick = now_tick_ms();
                    uint32_t cur_slice_bytes_to_send = average_bytes_per_slice;
                    uint32_t left_bytes_in_this_second = bw - bytes_sent_in_this_second;
                    if(left_bytes_in_this_second < burst_bytes){
                        cur_slice_bytes_to_send = left_bytes_in_this_second;
                    }
                    
                    //int r = rand();
                    
                    uint32_t slice_loop_count = cur_slice_bytes_to_send/1024;
                    if(cur_slice_bytes_to_send%1024 != 0){
                        ++slice_loop_count;
                    }
                    uint32_t bytes_sent_in_this_slice = 0;
                    uint32_t cur_bytes_to_send = cur_slice_bytes_to_send/slice_loop_count;
                    for (uint32_t j=0; j<slice_loop_count; ++j) {
                        //*(unsigned int*)buf = htonl(++sequence);
                        *p_seq = htons(++sequence);
                        long ret = sendto(sfd, (const char*)buf, cur_bytes_to_send, 0, (struct sockaddr*)&ss_addr, addr_len);
                        if(ret < 0){
                            my_printf("failed to call sendto, ret=%ld, err=%d\n", ret, errno);
							closesocket(sfd);
                            return ;
                        }
                        if(ret != cur_bytes_to_send){
                            my_printf("+++ ret=%ld, cur_bytes_to_send=%d\n", ret, cur_bytes_to_send);
                        }
                        cur_send_bytes += ret;
                        bytes_sent_in_this_slice += cur_bytes_to_send;
                    }
                    
                    bytes_sent_in_this_second += bytes_sent_in_this_slice;
                    
                    milliseconds slice_end_tick = now_tick_ms();
                    milliseconds diff = slice_end_tick - slice_start_tick;
                    if(diff.count() < slice_ms - 1){
                        std::this_thread::sleep_for(milliseconds(slice_ms-diff.count()-1));
                    }
                    
                    if(bytes_sent_in_this_second >= bw){
                        break;
                    }
                }
                milliseconds end_tick = now_tick_ms();
                milliseconds diff = end_tick - second_start_tick;
                if(diff.count() < 999){
                    std::this_thread::sleep_for(milliseconds(1000 - diff.count()));
                    end_tick += milliseconds(1000-diff.count());
                }
                diff = end_tick - start_tick;
                if(diff.count() > 5000){
                    uint32_t send_rate = cur_send_bytes*1000/diff.count();
                    my_printf("sequence=%u, send_bytes=%u, diff=%lld, send_rate=%u\n",
                              sequence, cur_send_bytes, diff.count(), send_rate);
                    cur_send_bytes = 0;
                    start_tick = end_tick;
                }
            }
			closesocket(sfd);
        });
    }
    catch(...)
    {
    }
}

extern "C" void stopTest()
{
    my_printf("stopTest ...\n");
    s_stop = true;
}

extern "C" void waitTest()
{
    try
    {
        s_thread.join();
    }
    catch(...)
    {
    }
}

