static char *ether_ntoa_r(u_char *hwaddr, char *buf, socklen_t size);
DATA AnalyzePacket(int deviceNo, u_char *data, int size);
int InitRawSocket(char *device);
