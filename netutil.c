#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netpacket/packet.h>
#include <netinet/if_ether.h>

#include "switch.h"

static char
*ether_ntoa_r(u_char *hwaddr, char *buf, socklen_t size)
{
	snprintf(buf, size, "%02x%02x%02x%02x%02x%02x",
		hwaddr[0],hwaddr[1],hwaddr[2],hwaddr[3],hwaddr[4],hwaddr[5]);

	return buf;
}

struct data
analyze_packet(int device_no, u_char *data, int size)
{
	u_char *ptr;
	int lest;
	struct ether_header *eh;
	char buf[80];
	char *dummy;
	struct data d;

	ptr = data;
	lest = size;

	if (lest < sizeof(struct ether_header))
		fprintf(stderr, "[%d]:lest(%d) < sizeof(struct ether_header)\n",
			device_no, lest);

	eh = (struct ether_header *)ptr;
	ptr += sizeof(struct ether_header);
	lest -= sizeof(struct ether_header);

	dummy = ether_ntoa_r(eh->ether_shost, buf, sizeof(buf));
	memcpy(d.saddr, dummy, 13);
	dummy = ether_ntoa_r(eh->ether_dhost, buf, sizeof(buf));
	memcpy(d.daddr, dummy, 13);

	return d;
}

int
init_rawsocket(char *device)
{
	struct ifreq ifreq;
	struct sockaddr_ll sa;
	int sock;

	if((sock = socket(PF_PACKET, SOCK_RAW,htons(ETH_P_ALL))) < 0) {
		perror("socket");
		return -1;
	}

	memset(&ifreq, 0, sizeof(struct ifreq));
	strncpy(ifreq.ifr_name, device, sizeof(ifreq.ifr_name) - 1);
	if (ioctl(sock, SIOCGIFINDEX, &ifreq) < 0) {
		perror("ioctl");
		close(sock);
		return -1;
	}

	sa.sll_family = PF_PACKET;
	sa.sll_protocol = htons(ETH_P_ALL);
	sa.sll_ifindex = ifreq.ifr_ifindex;

	if (bind(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
		perror("bind");
		close(sock);
		return -1;
	}

	if (ioctl(sock, SIOCGIFFLAGS, &ifreq) < 0) {
		perror("ioctl");
		close(sock);
		return -1;
	}

	return sock;
}

