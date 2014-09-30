int hash(char *key);
void SetNode(Node *n, DATA x, Node *next);
int InitHash(Hash *h, int size);
void TermHash(Hash *h);
Node *SearchNode(Hash *h, DATA x);
int InsertNode(Hash *h, DATA x);
