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

struct param prm = {"eth0", "eth1", "eth2", 1};
struct device dev[SOCK];

int end_flag = 0;

extern int
debug_printf(char *fmt, ...)
{
	if (prm.debug) {
		va_list args;

		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}

	return 0;

}

extern int
debug_perror(char *msg)
{
	if (prm.debug)
		fprintf(stderr, "%s : %s\n", msg, strerror(errno));

	return 0;
}

int
send_broadcast(int no, char *buf, int size)
{
	int i;

	for (i = 0; i < SOCK; i++) {
		if (i != no) {
			if ((size = write(dev[i].sock, buf, size)) <= 0)
				perror("write");
		}
	}

	return size;
}

static int
switch_loop()
{
	struct epoll_event ev, ev_ret[MAXEVENTS];
	struct hash hash;
	struct data tmp;
	struct __node *node;
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
		ev.data.fd = dev[i].sock;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, dev[i].sock, &ev) != 0){
			perror("epoll_ctl sock0: ");
			return -1;
		}
	}

	init_hash(&hash, 4096);

	while(end_flag == 0) {
		int	ret;

		nfds = epoll_wait(epfd, ev_ret, MAXEVENTS, -1);
		if (nfds <= 0) {
			perror("epoll_wait:");
			return -1;
		}
        }

        for(i = 0; i < nfds; i++) {
		for (j = 0; j < SOCK; j++) {
			if (ev_ret[i].data.fd == dev[j].sock) {
				// frame recv
				if ((size = read(dev[j].sock, buf, sizeof(buf))) <= 0)
					perror("read");

				// get src addr and dst addr and socket
				tmp = analyze_packet(j, buf, size);
				tmp.sock = dev[j].sock;

				// check broadcast frame
				if (strcmp(tmp.daddr, "ffffffffffff") == 0) {
					if ((size = send_broadcast(j, buf, size)) <= 0)
						perror("broadcast write");

					memset(tmp.daddr, 0, sizeof(tmp.daddr));
					insert_node(&hash, tmp);
				// unicast frame
				} else {
					// not found dst addr
					node = search_node(&hash, tmp);
					if(node == NULL) {
						if ((size = send_broadcast(j, buf, size)) <= 0)
							perror("broadcast write");

						memset(tmp.daddr, 0, sizeof(tmp.daddr));
						insert_node(&hash, tmp);
					} else {
					// found dst addr
						if(dev[j].sock != node->data.sock) {
							if ((size = write(node->data.sock, buf, size)) <= 0)
								perror("write");
						}

						memset(tmp.daddr, 0, sizeof(tmp.daddr));
						insert_node(&hash, tmp);
					}
				}
			}
		}

		if (prm.debug)
			dump_hash(&hash);
	}

	term_hash(&hash);

	return 0;

}

static int
disable_ip_forward()
{
	FILE *fp;

	if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "w")) == NULL) {
		debug_printf("cannot write /proc/sys/net/ipv4/ip_forward\n");
		return -1;
	}

	fputs("0", fp);
	fclose(fp);

	return 0;

}

static void
end_signal(int sig)
{
	end_flag = 1;
}

int
main(int argc, char *argv[], char *envp[])
{

	if ((dev[0].sock = init_rawsocket(prm.device1)) == -1) {
		debug_printf("init_rawsocket:error:%s\n", prm.device1);
		return -1;
	}

	debug_printf("%s OK\n", prm.device1);

	if ((dev[1].sock = init_rawsocket(prm.device2)) == -1) {
		debug_printf("init_rawsocket:error:%s\n", prm.device2);
		return -1;
	}

	debug_printf("%s OK\n", prm.device2);

	if ((dev[2].sock = init_rawsocket(prm.device3)) == -1) {
		debug_printf("init_rawsocket:error:%s\n", prm.device3);
		return -1;
	}

	debug_printf("%s OK\n", prm.device3);

	disable_ip_forward();

	signal(SIGINT, end_signal);
	signal(SIGKILL, end_signal);
	signal(SIGTERM, end_signal);
	signal(SIGQUIT, end_signal);
	signal(SIGTTIN, end_signal);
	signal(SIGTTOU, end_signal);

	switch_loop();

	close(dev[0].sock);
	close(dev[1].sock);
	close(dev[2].sock);

	return 0;

}
