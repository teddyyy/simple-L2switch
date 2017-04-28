#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "switch.h"

int
hash(char *key)
{
	unsigned h = 0;

	while (*key)
		 h += *(key++);

	return (h % HASHSIZE);
}

void
set_node(struct __node *n, struct data  x, struct __node *next)
{
	n->data = x;
	n->next = next;
}

int
init_hash(struct hash *h, int size)
{
	int i;

	h->size = 0;
	if ((h->table = calloc(size, sizeof(struct __node *))) == NULL)
		return 0;

	h->size = size;
	for (i = 0; i < size; i++)
		h->table[i] = NULL;

	return 1;
}

void
term_hash(struct hash *h)
{
	int i;

	for (i = 0; i < h->size; i++) {
		struct __node  *p = h->table[i];
		while (p != NULL) {
			struct __node  *next = p->next;
			free(p);
			p = next;
		}
	}

	free(h->table);
}

struct __node
*search_node(struct hash *h, struct data x)
{
	int key = hash(x.daddr);
	struct __node  *p = h->table[key];

	while (p != NULL) {
		if (!strcmp(p->data.saddr, x.daddr))
			return (p);
		p = p->next;
	}

	return NULL;
}

int
insert_node(struct hash *h, struct data x)
{
	int key = hash(x.saddr);
	struct __node *p = h->table[key];
	struct __node *tmp;

	while (p != NULL) {
		if (p->data.sock == x.sock)
			return 1;
		p = p->next;
	}

	if ((tmp = (struct __node *)calloc(1, sizeof(struct __node))) == NULL)
		return 2;

	set_node(tmp, x, h->table[key]);
	h->table[key] = tmp;

	return 0;
}

void
dump_hash(struct hash *h)
{
	int i;
	FILE *fp;
	char file[] = "debug.log";

	if ((fp = fopen(file, "a+")) == NULL)
		fprintf(stderr, "connot open file\n");

	for (i = 0; i < h->size; i++) {
		struct __node  *p = h->table[i];
		while (p != NULL) {
			fprintf(fp, "→ %d (%s)\n", p->data.sock, p->data.saddr);
			p = p->next;
		}
	}

	fclose(fp);
}

