typedef struct {
	char	*Device1;
	char 	*Device2;
	char 	*Device3;
	int		DebugOut;
} PARAM;

typedef struct {
	int 	sock;
} DEVICE;

typedef struct {
    char    saddr[13];
    char    daddr[13];
    int     sock;
} DATA;

typedef struct __node {
    DATA     data; 
    struct __node  *next; 
} Node;

typedef struct {
    int   size;                     
	Node  **table;              
} Hash;

extern int DebugPrintf(char *fmt, ...);
extern int DebugPerror(char *msg);

#define MAXEVENTS 16 
#define SOCK 3 
#define HASHSIZE 4096
