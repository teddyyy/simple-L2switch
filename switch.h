struct param {
	char	*device1;
	char	*device2;
	char	*device3;
	int	debug;
};

struct device {
	int	sock;
};

struct data {
	char    saddr[13];
	char    daddr[13];
	int     sock;
};

struct __node {
	struct	data	data;
	struct	__node	*next;
};

struct hash {
	int   size;
	struct	__node  **table;
};

extern int debug_printf(char *fmt, ...);
extern int debug_perror(char *msg);

#define MAXEVENTS 16
#define SOCK 3
#define HASHSIZE 4096
