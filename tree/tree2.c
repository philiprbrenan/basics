//------------------------------------------------------------------------------
// Tree in C - Keys are null terminated strings, data is a pointer to anything.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc., 2022
//------------------------------------------------------------------------------
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
  int   l[3];                                                                   // Key lengths
  int   L[3];                                                                   // Data lengths
  int  *k[3];                                                                   // Keys
  int   d[3];                                                                   // Data
  struct TreeNode *n[4];                                                        // Sub trees
 } TreeNode;

void TreePrint(TreeNode **p);

int TreeNodeIsLeaf(TreeNode *p)
 {assert(p != 0);
  return p->n[0] == 0;
 }

int TreeNodeIsFull(TreeNode *p)
 {assert(p != 0);
  return p->size == TreeNode_width;
 }

void TreeClearNode(TreeNode *n)
 {assert(n != 0);
  memset(n, 0, sizeof(TreeNode));
 }

TreeNode *TreeAllocNode()
 {TreeNode *n = malloc(sizeof(TreeNode));
  TreeClearNode(n);
  return n;
 }

void TreeSplitRootNode(TreeNode **root)
 {assert(root != 0);
  TreeNode *p = *root;
  assert(p != 0);
  if (!TreeNodeIsFull(p)) return;
  TreeNode *l = TreeAllocNode(), *r = TreeAllocNode();
  l->k[0] = p->k[0];  l->l[0] = p->l[0];
  l->d[0] = p->d[0];  l->L[0] = p->L[0];
  l->n[0] = p->n[0];
  l->n[1] = p->n[1];

  r->k[0] = p->k[2];  r->l[0] = p->l[2];
  r->d[0] = p->d[2];  r->L[0] = p->L[2];
  r->n[0] = p->n[2];
  r->n[1] = p->n[3];

  p->k[0] = p->k[1];  p->k[1] = 0;  p->k[2] = 0;
  p->l[0] = p->l[1];  p->l[1] = 0;  p->l[2] = 0;

  p->d[0] = p->d[1];  p->d[1] = 0;  p->d[2] = 0;
  p->L[0] = p->L[1];  p->L[1] = 0;  p->L[2] = 0;

  p->n[0] = l;        p->n[1] = r;  p->n[2] = 0; p->n[3] = 0;
  p->size = 1;
  l->size = r->size = 1;
 }

int TreeSplitNonRootNode(TreeNode *p, TreeNode *q)
 {assert(p != 0);
  assert(q != 0);
  if (!TreeNodeIsFull(q)) return 0;
  TreeNode *l = TreeAllocNode(), *r = TreeAllocNode();
  l->k[0] = q->k[0]; l->l[0] = q->l[0];
  l->d[0] = q->d[0]; l->L[0] = q->L[0];
  l->n[0] = q->n[0];
  l->n[1] = q->n[1];

  r->k[0] = q->k[2]; r->l[0] = q->l[2];
  r->d[0] = q->d[2]; r->L[0] = q->L[2];
  r->n[0] = q->n[2];
  r->n[1] = q->n[3];

  if (q == p->n[0])
   {p->k[2] = p->k[1]; p->k[1] = p->k[0]; p->k[0] = q->k[1];
    p->l[2] = p->l[1]; p->l[1] = p->l[0]; p->l[0] = q->l[1];

    p->d[2] = p->d[1]; p->d[1] = p->d[0]; p->d[0] = q->d[1];
    p->L[2] = p->L[1]; p->L[1] = p->L[0]; p->L[0] = q->L[1];

    p->n[3] = p->n[2]; p->n[2] = p->n[1]; p->n[1] = r; p->n[0] = l;
   }
  else if (q == p->n[1])
   {p->k[2] = p->k[1]; p->k[1] = q->k[1];
    p->l[2] = p->l[1]; p->l[1] = q->l[1];

    p->d[2] = p->d[1]; p->d[1] = q->d[1];
    p->L[2] = p->L[1]; p->L[1] = q->L[1];

    p->n[3] = p->n[2]; p->n[2] = r; p->n[1] = l;
   }
  else
   {p->k[2] = q->k[1];
    p->l[2] = q->l[1];

    p->d[2] = q->d[1];
    p->L[2] = q->L[1];

    p->n[3] = r; p->n[2] = l;
   }
  p->size++;
  l->size = r->size = 1;
  TreeClearNode(q);
  free(q);
  return 1;
 }

void TreePut(TreeNode **root, int *key, int keyLength, int data)
 {int dataLength = data;

  if (*root == 0)
   {TreeNode *n = *root = TreeAllocNode();
    n->k[0] = key;   n->l[0] = keyLength;
    n->d[0] = data;  n->L[0] = dataLength;
    n->size = 1;
    return;
   }

  descend:  TreeSplitRootNode(root);

  int loopCount = 0;
  for(TreeNode *p = *root, *q = 0; p != 0; p = q)
   {assert(++loopCount < 999);
    const int c0 = compareMemory((void *)key, keyLength, (void *)(p->k[0]), p->l[0]);
    if (c0 == 0)
     {p->d[0] = data; p->L[0] = dataLength;
      break;
     }
    if (c0 <  0)
     {if (TreeNodeIsLeaf(p))
       {p->k[2] = p->k[1]; p->k[1] = p->k[0]; p->k[0] = key;
        p->l[2] = p->l[1]; p->l[1] = p->l[0]; p->l[0] = keyLength;

        p->d[2] = p->d[1]; p->d[1] = p->d[0]; p->d[0] = data;
        p->L[2] = p->L[1]; p->L[1] = p->L[0]; p->L[0] = dataLength;
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
        p->d[1] = data; p->L[1] = dataLength;
        p->size++;
        break;
       }
      q = p->n[1];
      if (TreeSplitNonRootNode(p, q)) {q = *root; goto descend;}
      continue;
     }

    const int c1 = compareMemory((void *)key, keyLength, (void *)(p->k[1]), p->l[1]);
    if (c1 == 0)
     {p->d[1] = data; p->L[1] = dataLength;
      break;
     }

    if (c1 <  0)
     {if (TreeNodeIsLeaf(p))
       {p->k[2] = p->k[1]; p->k[1] = key;
        p->l[2] = p->l[1]; p->l[1] = keyLength;

        p->d[2] = p->d[1]; p->d[1] = data;
        p->L[2] = p->L[1]; p->L[1] = dataLength;
        p->size++;
        break;
       }
      q = p->n[1];
      if (TreeSplitNonRootNode(p, q)) {q = *root; goto descend;}
      continue;
     }

    if (TreeNodeIsLeaf(p))
     {p->k[2] = key; p->l[2] = keyLength;
      p->d[2] = data;p->L[2] = dataLength;
      p->size++;
      break;
     }
    q = p->n[2];
    if (TreeSplitNonRootNode(p, q)) {q = *root; goto descend;}
    continue;
   }
 }

int TreeCount(TreeNode **P)
 {TreeNode *p = *P;
  return p == 0 ? 0 : p->size + TreeCount(p->n+0) + TreeCount(p->n+1) + TreeCount(p->n+2);
 }

void TreePrint2(TreeNode *p, int d)
 {if (p == 0) return;
  fprintf(stderr, "\n");
  assert(d >= 0 && d < 99);
  for(int i = 0; i < d; ++i) fprintf(stdout, "  ");
  fprintf(stdout, "size=%d\n", p->size);

  if (p->n[0] != 0) TreePrint2(p->n[0], d+1);
  if (p->size >  0) {for(int i = 0; i < d; ++i) fprintf(stdout, "  "); fprintf(stdout, "%d\n", *(p->k[0]));}
  if (p->n[1] != 0) TreePrint2(p->n[1], d+1);
  if (p->size >  1) {for(int i = 0; i < d; ++i) fprintf(stdout, "  "); fprintf(stdout, "%d\n", *(p->k[1]));}
  if (p->n[2] != 0) TreePrint2(p->n[2], d+1);
  if (p->size >  2) {for(int i = 0; i < d; ++i) fprintf(stdout, "  "); fprintf(stdout, "%d\n", *(p->k[2]));}
  if (p->n[3] != 0) TreePrint2(p->n[3], d+1);
 }

void TreePrint(TreeNode **P)
 {TreePrint2(*P, 0);
 }

#pragma GCC diagnostic ignored "-Wunused-parameter"
int TreeFind(TreeNode **root, int *key, int keyLength, int *data)
 {if (root == 0) return 0;

  int loopCount = 0;
  for(TreeNode *p = *root, *q = 0; p != 0; p = q)
   {assert(++loopCount < 999);
    const int c0 = compareMemory((void *)key, keyLength, (void *)(p->k[0]), p->l[0]);
    if (c0 == 0)
     {if (data) *data = p->d[0];
      return 1;
     }
    if (c0 <  0)
     {q = p->n[0];
      continue;
     }
    if (p->size == 1)
     {q = p->n[1];
      continue;
     }
    const int c1 = compareMemory((void *)key, keyLength, (void *)(p->k[1]), p->l[1]);
    if (c1 == 0)
     {if (data) *data = p->d[1];
      return 1;
     }
    if (c1 <  0)
     {q = p->n[1];
      continue;
     }
    if (p->size == 2)
     {q = p->n[2];
      continue;
     }

    const int c2 = compareMemory((void *)key, keyLength, (void *)(p->k[2]), p->l[2]);
    if (c2 == 0)
     {if (data) *data = p->d[2];
      return 1;
     }
    if (c2 <  0)
     {q = p->n[2];
      continue;
     }
    q = p->n[3];
   }
  return 0;
 }

#if (__INCLUDE_LEVEL__ == 0)
//#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
//#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
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
      int dl = 0;
      assert(1 == TreeFind(root, (int *)(t+i), w, &dl));
      assert(dl == i);
     }
    if(1)
     {int k = 926, dl = 0;
      assert(0 == TreeFind(root, &k, w, &dl));
     }
   }
  TreePrint(root);
  return 0;
 }
#endif
#endif
