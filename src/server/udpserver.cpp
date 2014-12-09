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

#include "util.h"

static std::thread g_thread;
static bool g_stop = false;
extern "C" void startTest(const char* bind_addr, uint16_t bind_port)
{
    std::string str_bind_addr = bind_addr;
    try
    {
        g_thread = std::thread ([=]{
            int sfd = create_udp_fd(str_bind_addr.c_str(), bind_port);
            if(sfd < 0){
                return;
            }
            sockaddr_storage ss_addr = {0};
            socklen_t addr_len = sizeof(ss_addr);
            if(getsockname(sfd, (struct sockaddr*)&ss_addr, &addr_len) == 0){
                char bind_addr[64] = {0};
                unsigned short bind_port = 0;
                km_get_sock_addr((struct sockaddr*)&ss_addr, addr_len, bind_addr, sizeof(bind_addr), &bind_port);
                my_printf("bind to %s:%d\n", bind_addr, bind_port);
            }
            int buf_size = 64*1024;
            setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (char*)&buf_size, sizeof(buf_size));
            //setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (char*)&buf_size, sizeof(buf_size));
            printf("start to receive data...\n");
            unsigned char buf[65535];
            unsigned int recv_count = 0;
            unsigned int cur_recv_bytes = 0;
            unsigned int cur_recv_count = 0;
            unsigned int cur_lost_count = 0;
            uint16_t last_seq = 0;
            uint64_t start_tick = get_tick_count_ms();
            uint64_t end_tick = start_tick;
            bool first_data = true;
            while (!g_stop) {
                ss_addr = {0};
                addr_len = sizeof(ss_addr);
                long ret = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr *)&ss_addr, &addr_len);
                if(ret <= 0) {
                    printf("failed to call recvfrom, ret=%ld, err=%d\n", ret, errno);
                    close(sfd);
                    return ;
                }
                if(0 == recv_count){
                    start_tick = get_tick_count_ms();
                }
                ++recv_count;
                cur_recv_bytes += ret;
                ++cur_recv_count;
                uint16_t sequence = ntohs(*(uint16_t*)(buf+2));
                if(first_data){
                    first_data = false;
                    last_seq = sequence;
                    char peer_addr[64] = {0};
                    unsigned short peer_port = 0;
                    km_get_sock_addr((struct sockaddr*)&ss_addr, addr_len, peer_addr, sizeof(peer_addr), &peer_port);
                    my_printf("received data from %s:%d, first_sequence=%u\n", peer_addr, peer_port, last_seq);
                }
                if((int16_t(sequence - last_seq)) < 0){
                    my_printf("+++ disorder, seq=%u, last_seq=%u\n", sequence, last_seq);
                } else if(sequence - last_seq > 1){
                    cur_lost_count += sequence-last_seq-1;
                    printf("packet lost, loss_seq=%u, loss_count=%d\n", last_seq+1, sequence-last_seq-1);
                    last_seq = sequence;
                } else {
                    last_seq = sequence;
                }
                end_tick = get_tick_count_ms();
                uint32_t diff = (uint32_t)(end_tick - start_tick);
                if(diff > 5000){
                    uint32_t recv_rate = cur_recv_bytes*1000/diff;
                    printf("sequence=%u, len=%ld, recv_count=%u, cur_recv_bytes=%u, diff=%u, recv_rate=%u"
                           ", cur_recv_count=%u, cur_lost_count=%u, loss_rate=%f\n",
                           sequence, ret, recv_count, cur_recv_bytes, diff, recv_rate,
                           cur_recv_count, cur_lost_count, cur_lost_count*1.0/(cur_recv_count + cur_lost_count));
                    cur_recv_bytes = 0;
                    cur_recv_count = 0;
                    cur_lost_count = 0;
                    start_tick = end_tick;
                }
            }
            close(sfd);
        });
    }
    catch(...)
    {
    }
}

extern "C" void stopTest()
{
    g_stop = true;
}

extern "C" void waitTest()
{
    try
    {
        g_thread.join();
    }
    catch(...)
    {
    }
}

