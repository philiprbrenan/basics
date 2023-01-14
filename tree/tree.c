//------------------------------------------------------------------------------
// Tree in C - Keys are null terminated strings, data is a pointer to anything.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc., 2022
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef Ctree
#define Ctree
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <basics/basics.c>
const int TreeNode_width = 3;

typedef struct TreeNode                                                         // A node in a tree
 {int size;                                                                     // Number of keys in use - 1 to 3
  char *k[3];                                                                   // Keys
  int   l[3];                                                                   // Key lengths
  int   d[3];                                                                   // Data
  struct TreeNode *n[4];                                                        // Sub trees
  struct TreeNode *N;                                                           // Next node
 } TreeNode;

typedef struct TreeFindResult                                                   // Find result
 {int   r;                                                                      // 0 - not found, 1 - found
  char *k;                                                                      // Found key
  int   l;                                                                      // Found length
  int   d;                                                                      // Find data
  TreeNode *R;                                                                  // Root of tree
  TreeNode *n;                                                                  // The node we found
  int i;                                                                        // Index in found node of key
 } TreeFindResult;

typedef struct TreeEntry                                                        // Key/Data entry in a tree
 {char *k;                                                                      // Key
  int   l;                                                                      // Key length
  int   d;                                                                      // Data
 } TreeEntry;

typedef struct TreeEntries                                                      // Key/Data entry in a tree
 {int   n;                                                                      // Number of entries
  int   i;                                                                      // Current entry during build
  TreeEntry *e;                                                                 // Entries
 } TreeEntries;

static void TreePrint(TreeNode **p);

static int TreeNodeIsLeaf(TreeNode *p)
 {assert(p != 0);
  return p->n[0] == 0;
 }

static int TreeNodeIsFull(TreeNode *p)
 {assert(p != 0);
  return p->size == TreeNode_width;
 }

static void TreeClearNode(TreeNode *n)
 {assert(n != 0);
  memset(n, 0, sizeof(TreeNode));
 }

static TreeNode *TreeAllocNode()
 {TreeNode *n = malloc(sizeof(TreeNode));
  TreeClearNode(n);
  return n;
 }

static int compareMemory(const char * a, const int A, const char *b, const int B)
 {const int l = min(A, B);
  const int c = memcmp(a, b, l);
  if (c != 0) return c;
  if (A < B)  return -1;
  if (A > B)  return +1;
  return  0;
 }

static void TreeSplitRootNode(TreeNode **root)
 {assert(root != 0);
  TreeNode *p = *root;
  assert(p != 0);
  if (!TreeNodeIsFull(p)) return;
  TreeNode *l = TreeAllocNode(), *r = TreeAllocNode();
  l->k[0] = p->k[0];  l->l[0] = p->l[0];
  l->d[0] = p->d[0];
  l->n[0] = p->n[0];
  l->n[1] = p->n[1];

  r->k[0] = p->k[2];  r->l[0] = p->l[2];
  r->d[0] = p->d[2];
  r->n[0] = p->n[2];
  r->n[1] = p->n[3];

  p->k[0] = p->k[1];  p->k[1] = 0;  p->k[2] = 0;
  p->l[0] = p->l[1];  p->l[1] = 0;  p->l[2] = 0;

  p->d[0] = p->d[1];  p->d[1] = 0;  p->d[2] = 0;

  p->n[0] = l;        p->n[1] = r;  p->n[2] = 0; p->n[3] = 0;
  p->size = 1;
  l->size = r->size = 1;
 }

static int TreeSplitNonRootNode(TreeNode *p, TreeNode *q)
 {assert(p != 0);
  assert(q != 0);
  if (!TreeNodeIsFull(q)) return 0;
  TreeNode *l = TreeAllocNode(), *r = TreeAllocNode();
  l->k[0] = q->k[0]; l->l[0] = q->l[0];
  l->d[0] = q->d[0];
  l->n[0] = q->n[0];
  l->n[1] = q->n[1];

  r->k[0] = q->k[2]; r->l[0] = q->l[2];
  r->d[0] = q->d[2];
  r->n[0] = q->n[2];
  r->n[1] = q->n[3];

  if (q == p->n[0])
   {p->k[2] = p->k[1]; p->k[1] = p->k[0]; p->k[0] = q->k[1];
    p->l[2] = p->l[1]; p->l[1] = p->l[0]; p->l[0] = q->l[1];

    p->d[2] = p->d[1]; p->d[1] = p->d[0]; p->d[0] = q->d[1];

    p->n[3] = p->n[2]; p->n[2] = p->n[1]; p->n[1] = r; p->n[0] = l;
   }
  else if (q == p->n[1])
   {p->k[2] = p->k[1]; p->k[1] = q->k[1];
    p->l[2] = p->l[1]; p->l[1] = q->l[1];

    p->d[2] = p->d[1]; p->d[1] = q->d[1];

    p->n[3] = p->n[2]; p->n[2] = r; p->n[1] = l;
   }
  else
   {p->k[2] = q->k[1];
    p->l[2] = q->l[1];

    p->d[2] = q->d[1];

    p->n[3] = r; p->n[2] = l;
   }
  p->size++;
  l->size = r->size = 1;
  TreeClearNode(q);
  free(q);
  return 1;
 }

static void TreePut(TreeNode **root, char *key, int keyLength, int data)
 {if (*root == 0)
   {TreeNode *n = *root = TreeAllocNode();
    n->k[0] = key;   n->l[0] = keyLength;
    n->d[0] = data;
    n->size = 1;
    return;
   }

  descend:  TreeSplitRootNode(root);

  int loopCount = 0;
  for(TreeNode *p = *root, *q = 0; p != 0; p = q)
   {assert(++loopCount < 999);
    const int c0 = compareMemory(key, keyLength, p->k[0], p->l[0]);
    if (c0 == 0)
     {p->d[0] = data;
      break;
     }
    if (c0 <  0)
     {if (TreeNodeIsLeaf(p))
       {p->k[2] = p->k[1]; p->k[1] = p->k[0]; p->k[0] = key;
        p->l[2] = p->l[1]; p->l[1] = p->l[0]; p->l[0] = keyLength;

        p->d[2] = p->d[1]; p->d[1] = p->d[0]; p->d[0] = data;
        p->size++;
        break;
       }
      q = p->n[0];
      if (TreeSplitNonRootNode(p, q)) {q = *root; goto descend;}
      continue;
     }

    if (p->size == 1)
     {if (TreeNodeIsLeaf(p))
       {p->k[1] = key;  p->l[1] = keyLength;
        p->d[1] = data;
        p->size++;
        break;
       }
      q = p->n[1];
      if (TreeSplitNonRootNode(p, q)) {q = *root; goto descend;}
      continue;
     }

    const int c1 = compareMemory(key, keyLength, p->k[1], p->l[1]);
    if (c1 == 0)
     {p->d[1] = data;
      break;
     }

    if (c1 <  0)
     {if (TreeNodeIsLeaf(p))
       {p->k[2] = p->k[1]; p->k[1] = key;
        p->l[2] = p->l[1]; p->l[1] = keyLength;
        p->d[2] = p->d[1]; p->d[1] = data;
        p->size++;
        break;
       }
      q = p->n[1];
      if (TreeSplitNonRootNode(p, q)) {q = *root; goto descend;}
      continue;
     }

    if (TreeNodeIsLeaf(p))
     {p->k[2] = key; p->l[2] = keyLength;
      p->d[2] = data;
      p->size++;
      break;
     }
    q = p->n[2];
    if (TreeSplitNonRootNode(p, q)) {q = *root; goto descend;}
    continue;
   }
 }

static void TreePrint2(TreeNode *p, int d)
 {if (p == 0) return;
  fprintf(stderr, "\n");
  assert(d >= 0 && d < 99);
  for(int i = 0; i < d; ++i) fprintf(stdout, "  ");
  fprintf(stdout, "size=%d\n", p->size);

  if (p->n[0] != 0) TreePrint2(p->n[0], d+1);
  if (p->size >  0) {for(int i = 0; i < d; ++i) fprintf(stdout, "  "); printHex(stdout, p->k[0], p->l[0]);}
  if (p->n[1] != 0) TreePrint2(p->n[1], d+1);
  if (p->size >  1) {for(int i = 0; i < d; ++i) fprintf(stdout, "  "); printHex(stdout, p->k[1], p->l[1]);}
  if (p->n[2] != 0) TreePrint2(p->n[2], d+1);
  if (p->size >  2) {for(int i = 0; i < d; ++i) fprintf(stdout, "  "); printHex(stdout, p->k[2], p->l[2]);}
  if (p->n[3] != 0) TreePrint2(p->n[3], d+1);
 }

static void TreePrint(TreeNode **P)
 {TreePrint2(*P, 0);
 }

static int TreeSize(TreeNode **P)
 {TreeNode *p = *P;
  return p == 0 ? 0 : p->size + TreeSize(p->n+0) + TreeSize(p->n+1) + TreeSize(p->n+2) + TreeSize(p->n+3);
 }

static void TreeEntryLoad(TreeNode *p, TreeEntries *e)
 {if (p->n[0] != 0) TreeEntryLoad(p->n[0], e);
  if (p->size >  0) {e->e[e->i].k = p->k[0]; e->e[e->i].l = p->l[0]; e->e[e->i].d = p->d[0]; ++e->i;}

  if (p->n[1] != 0) TreeEntryLoad(p->n[1], e);
  if (p->size >  1) {e->e[e->i].k = p->k[1]; e->e[e->i].l = p->l[1]; e->e[e->i].d = p->d[1]; ++e->i;}

  if (p->n[2] != 0) TreeEntryLoad(p->n[2], e);
  if (p->size >  2) {e->e[e->i].k = p->k[2]; e->e[e->i].l = p->l[2]; e->e[e->i].d = p->d[2]; ++e->i;}
  if (p->n[3] != 0) TreeEntryLoad(p->n[3], e);
 }

static TreeEntries *TreeEntriesLoad(TreeNode **root)
 {const int n = TreeSize(root), s = sizeof(TreeEntries) + sizeof(TreeEntry) * n;
  TreeEntries *e = malloc(s);
  memset(e, 0, s);
  e->n = n;
  e->e = (TreeEntry *)((char *)e + sizeof(TreeEntry));
  if (n == 0) return e;

  TreeEntryLoad(*root, e);
  return e;
 }

static TreeFindResult TreeFind(TreeNode **root, char *key, int keyLength)
 {TreeFindResult r; memset(&r, 0, sizeof(r)); r.R = *root;
  if (root == 0) return r;
  int loopCount = 0;
  for(TreeNode *p = *root, *q = 0; p != 0; p = q)
   {assert(++loopCount < 999);
    const int c0 = compareMemory(key, keyLength, p->k[0], p->l[0]);
    if (c0 == 0)
     {r.r = 1;
      r.k = p->k[0];
      r.l = p->l[0];
      r.d = p->d[0];
      r.n = p;
      r.i = 0;
      return r;
     }
    if (c0 <  0)
     {q = p->n[0];
      continue;
     }
    if (p->size == 1)
     {q = p->n[1];
      continue;
     }
    const int c1 = compareMemory(key, keyLength, p->k[1], p->l[1]);
    if (c1 == 0)
     {r.r = 1;
      r.k = p->k[1];
      r.l = p->l[1];
      r.d = p->d[1];
      r.n = p;
      r.i = 1;
      return r;
     }
    if (c1 <  0)
     {q = p->n[1];
      continue;
     }
    if (p->size == 2)
     {q = p->n[2];
      continue;
     }
    const int c2 = compareMemory(key, keyLength, p->k[2], p->l[2]);
    if (c2 == 0)
     {r.r = 1;
      r.k = p->k[2];
      r.l = p->l[2];
      r.d = p->d[2];
      r.n = p;
      r.i = 2;
      return r;
     }
    if (c2 <  0)
     {q = p->n[2];
      continue;
     }
    q = p->n[3];
   }
  return r;
 }

static TreeFindResult TreeFirst(TreeNode **root)
 {TreeFindResult r; memset(&r, 0, sizeof(r)); r.R = *root;
  if (root == 0) return r;

  int loopCount = 0;
  for(TreeNode *p = *root; p != 0; p = p->n[0])
   {assert(++loopCount < 999);
    if (TreeNodeIsLeaf(p))
     {r.r = 1;
      r.k = p->k[0];
      r.l = p->l[0];
      r.d = p->d[0];
      r.n = p;
      r.i = 0;
      return r;
     }
   }
  return r;
 }

static TreeFindResult TreeLast(TreeNode **root)
 {TreeFindResult r; memset(&r, 0, sizeof(r)); r.R = *root;
  if (root == 0) return r;

  int loopCount = 0;
  for(TreeNode *p = *root; p != 0; p = p->n[p->size-1])
   {assert(++loopCount < 999);
    if (TreeNodeIsLeaf(p))
     {const int n = p->size-1;
      r.r = 1;
      r.k = p->k[n];
      r.l = p->l[n];
      r.d = p->d[n];
      r.n = p;
      r.i = n;
      return r;
     }
   }
  return r;
 }

#if (__INCLUDE_LEVEL__ == 0)
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

int *boxInt(int a)
 {int *m = malloc(sizeof(int));
  *m = a;
  return m;
 }

int main()
 {TreeNode *Root = 0;
  TreeNode **root = &Root;
  const int w = sizeof(int);
  const int t[] = {9,8,9,77,68,976,7298,472,967,925,6926,45,196,196,5,895,468,475,94,27,45,367,4,9,2,3,7,4,9,2,3,67,2,9,3,9,1,65,91,65,196,51,96, 972,987,69276,92769,276,2976,9,2764,8,76569,2,51695,15,26,69067,3954,64568,1347,3,6,5,0};
  int N = 0;
  for(int i = 0;i < 9999; ++i)
   {if (t[i] == 0) break;
    TreePut(root, boxInt(t[i]), w, i);
    ++N;
    for(int j = 0; j <= i; ++j)
     {if (t[i] == 0) break;
      TreeFindResult r = TreeFind(root, (int *)(t+i), w);
      assert(r.r == 1);
      assert(r.d == i);
     }
    if(1)
     {int k = 926;
      TreeFindResult r = TreeFind(root, &k, w);
      assert(r.r == 0);
     }
   }
  TreePrint(root);
  fprintf(stdout, "Size=%d\n", TreeSize(root));
  TreeEntries *e = TreeEntriesLoad(root);

  for(int i = 0; i < e->n; ++i)
   {fprintf(stdout, "%4d\n", *(int *)(e->e[i].k));
   }
  return 0;
 }
#endif

#endif
