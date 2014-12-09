//
//  util.c
//  udptest
//
//  Created by Folki Bao on 7/8/14.
//  Copyright (c) 2014 wme. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#ifdef MACOS
#include <net/if.h>
#include <ifaddrs.h>
#endif

#include "util.h"


#ifdef WIN32
typedef int (WSAAPI *pf_getaddrinfo)(
    _In_opt_  PCSTR pNodeName,
    _In_opt_  PCSTR pServiceName,
    _In_opt_  const ADDRINFOA *pHints,
    _Out_     PADDRINFOA *ppResult
    );

typedef int (WSAAPI *pf_getnameinfo)(
    __in   const struct sockaddr FAR *sa,
    __in   socklen_t salen,
    __out  char FAR *host,
    __in   DWORD hostlen,
    __out  char FAR *serv,
    __in   DWORD servlen,
    __in   int flags
    );

typedef void (WSAAPI *pf_freeaddrinfo)(
    __in  struct addrinfo *ai
    );

static HMODULE s_hmod_ws2_32 = NULL;
static bool s_ipv6_support = false;
static bool s_ipv6_inilized = false;
pf_getaddrinfo wtp_getaddrinfo = NULL;
pf_getnameinfo wtp_getnameinfo = NULL;
pf_freeaddrinfo wtp_freeaddrinfo = NULL;
#else
typedef int(*pf_getaddrinfo)(
    const char *node,
    const char *service,
    const struct addrinfo *hints,
struct addrinfo **res
    );

typedef int(*pf_getnameinfo)(
    const struct sockaddr *sa,
    socklen_t salen,
    char *host,
#ifdef LINUX
    size_t hostlen,
#else
    socklen_t hostlen,
#endif
    char *serv,
#ifdef LINUX
    size_t servlen,
#else
    socklen_t servlen,
#endif
#ifdef LINUX
    int flags
#else
    int flags
#endif
    );

typedef void(*pf_freeaddrinfo)(
struct addrinfo *res
    );

pf_getaddrinfo wtp_getaddrinfo = getaddrinfo;
pf_getnameinfo wtp_getnameinfo = getnameinfo;
pf_freeaddrinfo wtp_freeaddrinfo = freeaddrinfo;
#endif

bool ipv6_api_init()
{
#ifdef WIN32
    if (!s_ipv6_inilized)
    {
        s_ipv6_inilized = true;

#define GET_WS2_FUNCTION(f) wtp_##f = (pf_##f)GetProcAddress(s_hmod_ws2_32, #f);\
        if (NULL == wtp_##f)\
        {\
        FreeLibrary(s_hmod_ws2_32); \
        s_hmod_ws2_32 = NULL; \
        break; \
        }

        s_hmod_ws2_32 = LoadLibrary("Ws2_32.dll");
        do
        {
            if (NULL == s_hmod_ws2_32)
                break;
            GET_WS2_FUNCTION(getaddrinfo);
            GET_WS2_FUNCTION(getnameinfo);
            GET_WS2_FUNCTION(freeaddrinfo);
            s_ipv6_support = true;
        } while (0);
    }

    return s_ipv6_support;
#else
    return true;
#endif
}

int wtp_resolve_2_ip_v4(const char* host_name, char *ip_buf, int ip_buf_len)
{
    const char* ptr = host_name;
    bool is_digit = true;
    while (*ptr)
    {
        if (*ptr != ' ' &&  *ptr != '\t')
        {
            if (*ptr != '.' && !(*ptr >= '0' && *ptr <= '9'))
            {
                is_digit = false;
                break;
            }
        }
        ++ptr;
    }

    if (is_digit)
    {
        STRNCPY_S(ip_buf, ip_buf_len, host_name, strlen(host_name));
        return 0;
    }

    struct hostent* he = NULL;
#ifdef LINUX
    int nError = 0;
    char szBuffer[1024] = { 0 };
    struct hostent *pheResultBuf = reinterpret_cast<struct hostent *>(szBuffer);

    if (::gethostbyname_r(
        host_name,
        pheResultBuf,
        szBuffer + sizeof(struct hostent),
        sizeof(szBuffer)-sizeof(struct hostent),
        &pheResultBuf,
        &nError) == 0)
    {
        he = pheResultBuf;
    }
#else
    he = gethostbyname(host_name);
#endif

    if (he && he->h_addr_list && he->h_addr_list[0])
    {
#ifndef WIN32
        char tmp[INET_ADDRSTRLEN] = { 0 };
        ::inet_ntop(AF_INET, he->h_addr_list[0], tmp, sizeof(tmp));
#else
        char* tmp = (char*)inet_ntoa((in_addr&)(*he->h_addr_list[0]));
#endif
        if (tmp)
        {
            STRNCPY_S(ip_buf, ip_buf_len, tmp, strlen(tmp));
            return 0;
        }
    }

    ip_buf[0] = 0;
    return -1;
}

extern "C" WBXTP_API int wtp_resolve_2_ip(const char* host_name, char *ip_buf, int ip_buf_len, int ipv)
{
    if (NULL == host_name || NULL == ip_buf)
    {
        return -1;
    }

    ip_buf[0] = '\0';
#ifdef WIN32
    if (!ipv6_api_init())
    {
        if (RESOLVE_IPV6 == ipv)
            return -1;
        return wtp_resolve_2_ip_v4(host_name, ip_buf, ip_buf_len);
    }
#endif

    struct addrinfo* ai = NULL;
    int ret = wtp_getaddrinfo(host_name, NULL, NULL, &ai);
    if (ret != 0 || NULL == ai)
    {
        my_printf("wtp_resolve_2_ip, failed to resolve host, host=%s, ret=%d, err=%s\n", host_name, ret, gai_strerror(ret));
        return -1;
    }

    for (struct addrinfo *aii = ai; aii; aii = aii->ai_next)
    {
        if (AF_INET6 == aii->ai_family && (RESOLVE_IPV6 == ipv || RESOLVE_IPV0 == ipv))
        {
            sockaddr_in6 *sa6 = (sockaddr_in6*)aii->ai_addr;
            if (IN6_IS_ADDR_LINKLOCAL(&(sa6->sin6_addr)))
                continue;
            if (IN6_IS_ADDR_SITELOCAL(&(sa6->sin6_addr)))
                continue;
            if (wtp_getnameinfo(aii->ai_addr, aii->ai_addrlen, ip_buf, ip_buf_len, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
                continue;
            else
                break; // found a ipv6 address
        }
        else if (AF_INET == aii->ai_family && (RESOLVE_IPV4 == ipv || RESOLVE_IPV0 == ipv))
        {
            if (wtp_getnameinfo(aii->ai_addr, aii->ai_addrlen, ip_buf, ip_buf_len, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
                continue;
            else
                break; // found a ipv4 address
        }
    }
    if ('\0' == ip_buf[0] && RESOLVE_IPV0 == ipv &&
        wtp_getnameinfo(ai->ai_addr, ai->ai_addrlen, ip_buf, ip_buf_len, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
    {
        wtp_freeaddrinfo(ai);
        return -1;
    }
    wtp_freeaddrinfo(ai);
    return 0;
}

extern "C" WBXTP_API int wtp_set_sock_addr(const char* addr, unsigned short port,
struct addrinfo* hints,
struct sockaddr * sk_addr,
    unsigned int sk_addr_len)
{
#ifdef WIN32
    if (!ipv6_api_init())
    {
        struct sockaddr_in *sa = (struct sockaddr_in*)sk_addr;
        sa->sin_family = AF_INET;
        sa->sin_port = htons(port);
        if (NULL == addr || addr[0] == '\0')
        {
            sa->sin_addr.s_addr = INADDR_ANY;
        }
        else
        {
#ifndef WIN32
            inet_pton(sa->sin_family, addr, &sa->sin_addr);
#else
            sa->sin_addr.s_addr = inet_addr(addr);
#endif
        }
        return 0;
    }
#endif
    char service[128] = { 0 };
    struct addrinfo* ai = NULL;
    if (NULL == addr && hints)
    {
        hints->ai_flags |= AI_PASSIVE;
    }
    SNPRINTF_S(service, sizeof(service)-1, "%d", port);
    if (wtp_getaddrinfo(addr, service, hints, &ai) != 0 || NULL == ai || ai->ai_addrlen > sk_addr_len)
    {
        if (ai) wtp_freeaddrinfo(ai);
        return -1;
    }
    if (sk_addr)
        memcpy(sk_addr, ai->ai_addr, ai->ai_addrlen);
    wtp_freeaddrinfo(ai);
    return 0;
}

extern "C" WBXTP_API int wtp_get_sock_addr(const struct sockaddr * sk_addr, unsigned int sk_addr_len,
    char* addr, unsigned int addr_len, unsigned short* port)
{
#ifdef WIN32
    if (!ipv6_api_init())
    {
        struct sockaddr_in *sa = (struct sockaddr_in*)sk_addr;
#ifndef WIN32
        inet_ntop(sa->sin_family, &sa->sin_addr, addr, addr_len);
#else
        STRNCPY_S(addr, addr_len, inet_ntoa(sa->sin_addr), addr_len - 1);
#endif
        if (port)
            *port = ntohs(sa->sin_port);
        return 0;
    }
#endif

    char service[16] = { 0 };
    if (wtp_getnameinfo(sk_addr, sk_addr_len, addr, addr_len, service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV) != 0)
        return -1;
    if (port)
        *port = atoi(service);
    return 0;
}

extern "C" WBXTP_API bool wtp_is_ipv6_address(const char* addr)
{
    sockaddr_storage ss_addr = { 0 };
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
    if (wtp_set_sock_addr(addr, 0, &hints, (struct sockaddr *)&ss_addr, sizeof(ss_addr)) != 0)
    {
        return false;
    }
    return AF_INET6 == ss_addr.ss_family;
}

extern "C" WBXTP_API bool wtp_is_ip_address(const char* addr)
{
    sockaddr_storage ss_addr = { 0 };
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
    return wtp_set_sock_addr(addr, 0, &hints, (struct sockaddr *)&ss_addr, sizeof(ss_addr)) == 0;
}

extern "C" WBXTP_API int wtp_get_local_ip(char* ip_buf, unsigned int ip_buf_len)
{
#ifdef MACOS
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;

    if (getifaddrs(&myaddrs) != 0) {
        return -1;
    }

    int ret = -1;
    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        switch (ifa->ifa_addr->sa_family)
        {
        case AF_INET:
        {
                        struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                        in_addr = &s4->sin_addr;
                        if (IN_LOOPBACK(ntohl(s4->sin_addr.s_addr)))
                            continue;
                        break;
        }

        case AF_INET6:
        {
                         struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                         in_addr = &s6->sin6_addr;
                         if (IN6_IS_ADDR_LOOPBACK(&s6->sin6_addr))
                             continue;
                         if (IN6_IS_ADDR_LINKLOCAL(&s6->sin6_addr))
                             continue;
                         if (IN6_IS_ADDR_SITELOCAL(&s6->sin6_addr))
                             continue;
                         break;
        }

        default:
            continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, ip_buf, ip_buf_len)) {
            continue;
        }
        else {
            ret = 0;
            break;
        }
    }

    freeifaddrs(myaddrs);

    return ret;
#else
    struct addrinfo* ai = NULL;
    int ret = wtp_getaddrinfo(NULL, NULL, NULL, &ai);
    if (ret != 0 || NULL == ai)
    {
        my_printf("wtp_get_local_ip, failed to resolve host, ret=%d, err=%s\n", ret, gai_strerror(ret));
        return -1;
    }

    for (struct addrinfo *aii = ai; aii; aii = aii->ai_next)
    {
        if (AF_INET6 == aii->ai_family)
        {
            sockaddr_in6 *sa6 = (sockaddr_in6*)aii->ai_addr;
            if (IN6_IS_ADDR_LOOPBACK(&sa6->sin6_addr))
                continue;
            if (IN6_IS_ADDR_LINKLOCAL(&(sa6->sin6_addr)))
                continue;
            if (IN6_IS_ADDR_SITELOCAL(&(sa6->sin6_addr)))
                continue;
            if (wtp_getnameinfo(aii->ai_addr, aii->ai_addrlen, ip_buf, ip_buf_len, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
                continue;
            else
                break; // found a ipv6 address
        }
        else if (AF_INET == aii->ai_family)
        {
            sockaddr_in *sa4 = (sockaddr_in*)aii->ai_addr;
            if (IN_LOOPBACK(ntohl(sa4->sin_addr.s_addr)))
                continue;
            if (wtp_getnameinfo(aii->ai_addr, aii->ai_addrlen, ip_buf, ip_buf_len, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
                continue;
            else
                break; // found a ipv4 address
        }
    }
    if ('\0' == ip_buf[0] &&
        wtp_getnameinfo(ai->ai_addr, ai->ai_addrlen, ip_buf, ip_buf_len, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
    {
        wtp_freeaddrinfo(ai);
        return -1;
    }
    wtp_freeaddrinfo(ai);
    return 0;
#endif
}

extern "C" uint64_t get_tick_count_us()
{
#ifdef WIN32
    return GetTickCount64() * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    uint64_t tick = tv.tv_sec * 1000000 + tv.tv_usec;
    return tick;
#endif
}

extern "C" uint64_t get_tick_count_ms()
{
    return get_tick_count_us() / 1000;
}

extern "C" int create_udp_fd(const char* bind_addr, uint16_t bind_port)
{
    sockaddr_storage ss_addr = { 0 };
    socklen_t addr_len = sizeof(ss_addr);
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_ADDRCONFIG; // will block 10 seconds in some case if not set AI_ADDRCONFIG

    if (wtp_set_sock_addr(bind_addr, bind_port, &hints, (struct sockaddr *)&ss_addr, addr_len) < 0){
        my_printf("failed to set socket address\n");
        return -1;
    }
    if (AF_INET == ss_addr.ss_family) {
        addr_len = sizeof(sockaddr_in);
    }
    else {
        addr_len = sizeof(sockaddr_in6);
    }
    int sfd = socket(ss_addr.ss_family, SOCK_DGRAM, 0);
    if (sfd < 0){
        my_printf("failed to new socket, err=%d\n", errno);
        return -1;
    }
    if (bind(sfd, (struct sockaddr *)&ss_addr, addr_len) < 0){
        my_printf("failed to bind socket, err=%d\n", errno);
        return -1;
    }

    return sfd;
}



