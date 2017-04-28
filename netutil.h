static char *ether_ntoa_r(u_char *hwaddr, char *buf, socklen_t size);
struct data  analyze_packet(int device_no, u_char *data, int size);
int init_rawsocket(char *device);
