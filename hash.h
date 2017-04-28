int hash(char *key);
void set_node(struct __node *n, struct data x, struct __node *next);
int init_hash(struct hash *h, int size);
void term_hash(struct hash *h);
struct __node *search_node(struct hash *h, struct data x);
int insert_node(struct hash *h, struct data x);
void dump_hash(struct hash *h);
