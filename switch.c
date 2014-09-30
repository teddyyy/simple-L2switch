#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <poll.h>
#include <sys/epoll.h>

#include "switch.h"
#include "netutil.h"
#include "hash.h"

PARAM Param = {"eth0", "eth1", "eth2", 1};
DEVICE Device[SOCK];

int EndFlag = 0;

extern int DebugPrintf(char *fmt, ...)
{
	if(Param.DebugOut) {
		va_list args;

		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}

	return 0;

}

extern int DebugPerror(char *msg)
{
	if(Param.DebugOut) {
		fprintf(stderr, "%s : %s\n", msg, strerror(errno));
	}

	return 0;
}

int broadCastFrame(int no, char *buf, int size)                                                                         
{
    int i;

    for (i = 0; i < SOCK; i++) {
        if (i != no) {
            if ((size = write(Device[i].sock, buf, size)) <= 0) {
                perror("write");
            }
        }
    }
    
    return size;

}

static int Switch()
{
	struct epoll_event ev, ev_ret[MAXEVENTS];   
	Hash hash;
	int epfd, nfds, i, j, size;
	u_char buf[4096];

	epfd = epoll_create(MAXEVENTS);
    if (epfd < 0){
        perror("epoll_create: ");
        return -1;
    }

    // register sock to epoll
    for (i = 0; i < SOCK; i++) {
		memset(&ev, 0, sizeof(ev));
    	ev.events = EPOLLIN;
    	ev.data.fd = Device[i].sock;
    	if (epoll_ctl(epfd, EPOLL_CTL_ADD, Device[i].sock, &ev) != 0){
        	perror("epoll_ctl sock0: ");
        	return -1;
		}
    }
	
 	InitHash(&hash, 4096);

	while(EndFlag == 0) {
		DATA	tmp;
		Node	*node;
		int 	ret;

		nfds = epoll_wait(epfd, ev_ret, MAXEVENTS, -1);
        if (nfds <= 0) {
            perror("epoll_wait:");
        	return -1;
        }
	
        for(i = 0; i < nfds; i++) {
			for (j = 0; j < SOCK; j++) {
				if (ev_ret[i].data.fd == Device[j].sock) {
					// frame recv
					if ((size = read(Device[j].sock, buf, sizeof(buf))) <= 0) {
						perror("read");
					}
		
					// get src addr and dst addr and socket
					tmp = AnalyzePacket(j, buf, size);
					tmp.sock = Device[j].sock;

					// check broadcast frame
					if (strcmp(tmp.daddr, "ffffffffffff") == 0) {
						if ((size = broadCastFrame(j, buf, size)) <= 0) {
					   		perror("broadcast write");
						}
						memset(tmp.daddr, 0, sizeof(tmp.daddr));
						InsertNode(&hash, tmp);
					} else {
						// not found dst addr
						node = SearchNode(&hash, tmp);
						if(node == NULL) {
							if ((size = broadCastFrame(j, buf, size)) <= 0) {
					   			perror("broadcast write");
							}
							memset(tmp.daddr, 0, sizeof(tmp.daddr));
							InsertNode(&hash, tmp);
						} else {
						// found dst addr
							if(Device[j].sock != node->data.sock) {
								if ((size = write(node->data.sock, buf, size)) <= 0) {
									perror("write");
								}
							}
							memset(tmp.daddr, 0, sizeof(tmp.daddr));
							InsertNode(&hash, tmp);
						}
					}
				}
			}
		}
		if(Param.DebugOut) {
			DumpHash(&hash);
		}
	}

	TermHash(&hash); 

	return 0;

}

static int DisableIpForward()
{
	FILE *fp;
		
	if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "w")) == NULL) {
		DebugPrintf("cannot write /proc/sys/net/ipv4/ip_forward\n");
		return -1;
	}
	
	fputs("0", fp);
	fclose(fp);

	return 0;

}

static void EndSignal(int sig)
{
	EndFlag = 1;
}

int main(int argc, char *argv[], char *envp[])
{
	
	if ((Device[0].sock = InitRawSocket(Param.Device1)) == -1) {
		DebugPrintf("InitRasSocket:error:%s\n", Param.Device1);
		return -1;
	}
	DebugPrintf("%s OK\n", Param.Device1);

	if ((Device[1].sock = InitRawSocket(Param.Device2)) == -1) {
		DebugPrintf("InitRasSocket:error:%s\n", Param.Device2);
		return -1;
	}
	DebugPrintf("%s OK\n", Param.Device2);

	if ((Device[2].sock = InitRawSocket(Param.Device3)) == -1) {
		DebugPrintf("InitRasSocket:error:%s\n", Param.Device3);
		return -1;
	}
	DebugPrintf("%s OK\n", Param.Device3);

	DisableIpForward();

	signal(SIGINT, EndSignal);
	signal(SIGKILL, EndSignal);
	signal(SIGTERM, EndSignal);
	signal(SIGQUIT, EndSignal);
	signal(SIGTTIN, EndSignal);
	signal(SIGTTOU, EndSignal);

	Switch();

	close(Device[0].sock);
	close(Device[1].sock);
	close(Device[2].sock);

	return 0;

}
