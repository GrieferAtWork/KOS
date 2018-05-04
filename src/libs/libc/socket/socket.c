/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBS_LIBC_INET_SOCKET_C
#define GUARD_LIBS_LIBC_INET_SOCKET_C 1
#define _GNU_SOURCE 1

#include "../libc.h"
#include "../system.h"
#include "../errno.h"
#include "socket.h"
#include <hybrid/compiler.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <hybrid/timespec.h>
#include <net/if.h>
#include <assert.h>

DECL_BEGIN


DEFINE_PUBLIC_ALIAS(ntohl,libc_ntohl);
DEFINE_PUBLIC_ALIAS(ntohs,libc_ntohs);
DEFINE_PUBLIC_ALIAS(htonl,libc_htonl);
DEFINE_PUBLIC_ALIAS(htons,libc_htons);

#if __BYTE_ORDER == __BIG_ENDIAN
CRT_NET uint32_t LIBCCALL libc_ntohl(uint32_t netlong) { return netlong; }
CRT_NET uint16_t LIBCCALL libc_ntohs(uint16_t netshort) { return netshort; }
DEFINE_INTERN_ALIAS(libc_htonl,libc_ntohl);
DEFINE_INTERN_ALIAS(libc_htons,libc_ntohs);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
CRT_NET uint32_t LIBCCALL libc_ntohl(uint32_t netlong) { return __bswap_32(netlong); }
CRT_NET uint16_t LIBCCALL libc_ntohs(uint16_t netshort) { return __bswap_16(netshort); }
DEFINE_INTERN_ALIAS(libc_htonl,libc_ntohl);
DEFINE_INTERN_ALIAS(libc_htons,libc_ntohs);
#else
#error FIXME
#endif


DEFINE_PUBLIC_ALIAS(socket,libc_socket);
CRT_NET int LIBCCALL
libc_socket(int domain, int type, int protocol) {
 return FORWARD_SYSTEM_ERROR(sys_socket(domain,type,protocol));
}

DEFINE_PUBLIC_ALIAS(socketpair,libc_socketpair);
CRT_NET int LIBCCALL
libc_socketpair(int domain, int type, int protocol, int fds[2]) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(bind,libc_bind);
CRT_NET int LIBCCALL
libc_bind(fd_t fd, struct sockaddr const *addr, socklen_t len) {
 return FORWARD_SYSTEM_ERROR(sys_bind(fd,addr,len));
}

DEFINE_PUBLIC_ALIAS(getsockname,libc_getsockname);
CRT_NET int LIBCCALL
libc_getsockname(fd_t fd, struct sockaddr *addr,
                 socklen_t *__restrict len) {
 return FORWARD_SYSTEM_ERROR(sys_getsockname(fd,addr,len));
}
DEFINE_PUBLIC_ALIAS(connect,libc_connect);
CRT_NET int LIBCCALL
libc_connect(fd_t fd, struct sockaddr const *addr, socklen_t len) {
 return FORWARD_SYSTEM_ERROR(sys_connect(fd,addr,len));
}
DEFINE_PUBLIC_ALIAS(getpeername,libc_getpeername);
CRT_NET int LIBCCALL
libc_getpeername(fd_t fd, struct sockaddr *addr, socklen_t *__restrict len) {
 return FORWARD_SYSTEM_ERROR(sys_getpeername(fd,addr,len));
}

DEFINE_PUBLIC_ALIAS(send,libc_send);
CRT_NET ssize_t LIBCCALL
libc_send(fd_t fd, void const *buf, size_t n, int flags) {
 return FORWARD_SYSTEM_ERROR(sys_send(fd,buf,n,flags));
}

DEFINE_PUBLIC_ALIAS(recv,libc_recv);
CRT_NET ssize_t LIBCCALL
libc_recv(fd_t fd, void *buf, size_t n, int flags) {
 return FORWARD_SYSTEM_ERROR(sys_recv(fd,buf,n,flags));
}

DEFINE_PUBLIC_ALIAS(sendto,libc_sendto);
CRT_NET ssize_t LIBCCALL
libc_sendto(fd_t fd, void const *buf, size_t n, int flags,
            struct sockaddr const *addr, socklen_t addr_len) {
 return FORWARD_SYSTEM_ERROR(sys_sendto(fd,buf,n,flags,addr,addr_len));
}

DEFINE_PUBLIC_ALIAS(recvfrom,libc_recvfrom);
CRT_NET ssize_t LIBCCALL
libc_recvfrom(fd_t fd, void *__restrict buf, size_t n, int flags,
              struct sockaddr *addr, socklen_t *__restrict addr_len) {
 return FORWARD_SYSTEM_ERROR(sys_recvfrom(fd,buf,n,flags,addr,addr_len));
}

DEFINE_PUBLIC_ALIAS(getsockopt,libc_getsockopt);
CRT_NET int LIBCCALL
libc_getsockopt(fd_t fd, int level, int optname,
                void *__restrict optval,
                socklen_t *__restrict optlen) {
 return FORWARD_SYSTEM_ERROR(sys_getsockopt(fd,level,optname,optval,optlen));
}

DEFINE_PUBLIC_ALIAS(setsockopt,libc_setsockopt);
CRT_NET int LIBCCALL
libc_setsockopt(fd_t fd, int level, int optname,
                void const *optval, socklen_t optlen) {
 return FORWARD_SYSTEM_ERROR(sys_setsockopt(fd,level,optname,optval,optlen));
}

DEFINE_PUBLIC_ALIAS(listen,libc_listen);
CRT_NET int LIBCCALL libc_listen(fd_t fd, int n) {
 return FORWARD_SYSTEM_ERROR(sys_listen(fd,n));
}

DEFINE_PUBLIC_ALIAS(accept,libc_accept);
CRT_NET int LIBCCALL
libc_accept(fd_t fd, struct sockaddr *addr, socklen_t *__restrict addr_len) {
 return FORWARD_SYSTEM_ERROR(sys_accept(fd,addr,addr_len));
}

DEFINE_PUBLIC_ALIAS(shutdown,libc_shutdown);
CRT_NET int LIBCCALL
libc_shutdown(fd_t fd, int how) {
 return FORWARD_SYSTEM_ERROR(sys_shutdown(fd,how));
}

DEFINE_PUBLIC_ALIAS(accept4,libc_accept4);
CRT_NET int LIBCCALL
libc_accept4(fd_t fd, struct sockaddr *addr,
             socklen_t *__restrict addr_len,
             int flags) {
 return FORWARD_SYSTEM_ERROR(sys_accept4(fd,addr,addr_len,flags));
}

DEFINE_PUBLIC_ALIAS(sockatmark,libc_sockatmark);
CRT_NET int LIBCCALL libc_sockatmark(fd_t fd) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(isfdtype,libc_isfdtype);
CRT_NET int LIBCCALL
libc_isfdtype(fd_t fd, int fdtype) {
 assertf(0,"TODO");
 return -1;
}




DEFINE_PUBLIC_ALIAS(sendmsg,libc_sendmsg);
CRT_NET ssize_t LIBCCALL
libc_sendmsg(fd_t fd, struct msghdr const *message, int flags) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(recvmsg,libc_recvmsg);
CRT_NET ssize_t LIBCCALL
libc_recvmsg(fd_t fd, struct msghdr *message, int flags) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(sendmmsg,libc_sendmmsg);
CRT_NET int LIBCCALL
libc_sendmmsg(fd_t fd, struct mmsghdr *vmessages,
              unsigned int vlen, int flags) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(recvmmsg,libc_recvmmsg);
CRT_NET int LIBCCALL
libc_recvmmsg(fd_t fd, struct mmsghdr *vmessages,
              unsigned int vlen, int flags,
              struct timespec *tmo) {
 assertf(0,"TODO");
 return -1;
}

DECL_END

#endif /* !GUARD_LIBS_LIBC_INET_SOCKET_C */
