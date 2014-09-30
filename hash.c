#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "switch.h"

int hash(char *key)
{
    unsigned h = 0;

    while (*key)
        h += *(key++);
    return (h % HASHSIZE);
}

void SetNode(Node *n, DATA x, Node *next)
{
    n->data = x;                        
    n->next = next;                    
}

int InitHash(Hash *h, int size)
{
    int  i;

    h->size = 0;
    if ((h->table = calloc(size, sizeof(Node *))) == NULL)
        return (0);

    h->size = size;
    for (i = 0; i < size; i++)
        h->table[i] = NULL;
    return (1);
}

void TermHash(Hash *h)
{
    int  i;

    for (i = 0; i < h->size; i++) {
        Node  *p = h->table[i];
        while (p != NULL) {
            Node  *next = p->next;
            free(p);
            p = next;
        }
    }
    free(h->table);
}

Node *SearchNode(Hash *h, DATA x)
{
    int   key = hash(x.daddr);               
	Node  *p = h->table[key];        

    while (p != NULL) {
        if (!strcmp(p->data.saddr, x.daddr)) 
            return (p);
        p = p->next;                    
    }
    return (NULL);                     
}

int InsertNode(Hash *h, DATA x)
{
    int   key = hash(x.saddr);           
    Node  *p = h->table[key];      
    Node  *temp;

    while (p != NULL) {
        if (p->data.sock == x.sock)       
            return (1);
        p = p->next;             
    }
    if ((temp = (Node *)calloc(1, sizeof(Node))) == NULL)
        return (2);
    SetNode(temp, x, h->table[key]);   
    h->table[key] = temp;
    return (0);
}

void DumpHash(Hash *h)
{
    int  i;                                                                                              
	FILE *fp;
	char file[] = "debug.log";

	if((fp = fopen(file, "a+")) == NULL) {
		fprintf(stderr, "connot open file\n");
    }

    for (i = 0; i < h->size; i++) {
        Node  *p = h->table[i];
        while (p != NULL) {
            fprintf(fp, "→ %d (%s)\n", p->data.sock, p->data.saddr);
            p = p->next;
        }
    }
	
	fprintf(fp,"============\n");
	fclose(fp);
}

