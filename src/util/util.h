//
//  util.h
//  udptest
//
//  Created by Jamol Bao on 7/8/14.
//  Copyright (c) 2014. All rights reserved.
//

#ifndef udptest_util_h
#define udptest_util_h

#ifdef WIN32
#include <Ws2tcpip.h>
#include <windows.h>
#include <time.h>
#include <stdint.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef WIN32
#define STRNCPY_S strncpy_s
#define SNPRINTF_S _snprintf_s

#ifndef IN_LOOPBACK
#define IN_LOOPBACK(i)	(((uint32_t)(i) & 0xff000000) == 0x7f000000)
#endif
#else
#define STRNCPY_S(d, dl, s, sl) \
do{ \
    if(0 == dl) \
        break; \
    strncpy(d, s, dl-1); \
    d[dl-1]='\0'; \
}while(0);

#define SNPRINTF_S snprintf
#define closesocket	close
#endif

#define RESOLVE_IPV0		0
#define RESOLVE_IPV4		1
#define RESOLVE_IPV6		2

#ifdef ANDROID
#include <android/log.h>
#define MY_TAG  "udptest"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, MY_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, MY_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MY_TAG, __VA_ARGS__)
#define my_printf LOGI
#else
#define my_printf printf
#endif

#ifdef __cplusplus
extern "C"{
#endif
    int km_resolve_2_ip(const char* host_name, char *ip_buf, int ip_buf_len, int ipv);
    int km_set_sock_addr(const char* addr, unsigned short port,
                                    struct addrinfo* hints, struct sockaddr * sk_addr,
                                    unsigned int sk_addr_len);
    int km_get_sock_addr(const struct sockaddr * sk_addr, unsigned int sk_addr_len,
                                    char* addr, unsigned int addr_len, unsigned short* port);
    bool km_is_ipv6_address(const char* addr);
    bool km_is_ip_address(const char* addr);
    int km_get_local_ip(char* ip, unsigned int ip_len);
    int create_udp_fd(const char* bind_addr, uint16_t bind_port);
#ifdef __cplusplus
}
#endif

#endif
