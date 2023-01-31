//------------------------------------------------------------------------------
// N way tree with 8 long keys
//Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// 1,057,170 instructions executed

#define _GNU_SOURCE
#ifndef NWayTreeLong
#define NWayTreeLong
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "array/long.c"
#include "array/void.c"
#include "basics/basics.c"
#include "stack/char.c"

#define NWayTreeLongNumberOfKeysPerNode 3

typedef struct NWayTreeLongNode                                                 // A node in a tree
 {long length;
  long keys[NWayTreeLongNumberOfKeysPerNode];
  long data[NWayTreeLongNumberOfKeysPerNode];
  struct NWayTreeLongNode *down[NWayTreeLongNumberOfKeysPerNode+1];
  struct NWayTreeLongNode *up;
 } NWayTreeLongNode;

typedef struct NWayTreeLongTree                                                 // The root of a tree
 {NWayTreeLongNode *node;
  long size;
 } NWayTreeLongTree;

typedef enum NWayTreeLongFindComparison                                         // The results of a comparison
 {lower, equal, higher, notFound
 } NWayTreeLongFindComparison;

typedef struct NWayTreeLongFindResult                                           // The results of a find operation
 {NWayTreeLongTree *tree;                                                       // Tree searched
  NWayTreeLongNode *node;                                                       // Node found
  NWayTreeLongFindComparison cmp;                                               // Result of the last comparison
  long key;                                                                     // Key searched for
  long index;                                                                   // Index in the node of equal element
 } NWayTreeLongFindResult;


static NWayTreeLongTree *NWayTreeLongNewTree()                                  // Create a new tree
 {NWayTreeLongTree * const tree = calloc(sizeof(NWayTreeLongTree), 1);
  return tree;
 }

static NWayTreeLongNode *NWayTreeLongNewNode()                                  // Create a new node
 {NWayTreeLongNode *node = calloc(sizeof(NWayTreeLongNode), 1);
  return node;
 }

static NWayTreeLongFindResult NWayTreeLongNewFindResult                         // New find result on stack
 (NWayTreeLongTree *tree, NWayTreeLongNode *node, long key,
  NWayTreeLongFindComparison cmp, long index)
 {NWayTreeLongFindResult r;
  r.tree  = tree;
  r.node  = node;
  r.key   = key;
  r.cmp   = cmp;
  r.index = index;
  return r;
 }

static void NWayTreeLongToString2                                               // Print the keys in a tree
 (NWayTreeLongNode *node, long in, StackChar *p)
 {if (!node || !node->length) return;

  NWayTreeLongToString2(node->down[0], in+1, p);
  for(long i = 0; i < node->length; ++i)
   {for(long j = 0; j < in * 3; ++j) StackCharPush(p, ' ');
    char C[100];
    sprintf(C, "%4ld  %4ld\n", node->keys[i], node->data[i]);
    for(char *c = C; *c; ++c) StackCharPush(p, *c);
    NWayTreeLongToString2(node->down[i+1], in+1, p);
   }
 }

static StackChar *NWayTreeLongToString(NWayTreeLongTree *tree)                  // Print a tree as a string
 {StackChar *p = StackCharNew();
  if (tree->node) NWayTreeLongToString2(tree->node, 0, p);
  return p;
 }

static void NWayTreeLongPrintErr(NWayTreeLongTree *tree)                        // Print a tree on stderr
 {StackChar *s = NWayTreeLongToString(tree);
  say("%s", s->arena+s->base);
  free(s);
 }

static void NWayTreeLongErrAsC(NWayTreeLongTree *tree)                          // Print a tree as C strings on stderr
 {StackChar *s = NWayTreeLongToString(tree);
  const long N = s->next-s->base;                                               // The number of characters to print
  fputs("assert(NWayTreeLongEqText(tree,\n", stderr);
  fputc('\"', stderr);

  for(long i = s->base; i < N; ++i)
   {const char c = s->arena[i];
    if (c == '\n')
     {fputs("\\n\"\n", stderr);
      if (i + 1 < N) putc('"', stderr);
     }
    else putc(c, stderr);
   }
  free(s);
  fputs("));\n", stderr);
 }

static long NWayTreeLongEqText(NWayTreeLongTree *tree, char * text)
 {StackChar *s = NWayTreeLongToString(tree);
  return strncmp(s->arena+s->base, text, s->next-s->base) == 0;
 }

static void NWayTreeLongErrFindResult(NWayTreeLongFindResult r)                 // Print a find result
 {char *c;
  switch(r.cmp)
   {case equal:     c = "equal";    break;
    case lower:     c = "lower";    break;
    case higher:    c = "higher";   break;
    case notFound:  c = "notFound"; break;
   }

  say("Find key=%ld Result key[0]=%ld %s  index=%ld",
      r.key, r.node->keys[0], c, r.index);
 }

static long NWayTreeLongMinimumNumberOfKeys()                                   //P Minimum number of keys per node.
 {return (NWayTreeLongNumberOfKeysPerNode - 1) / 2;
 }

static long NWayTreeLongMaximumNumberOfKeys()                                   //P Maximum number of keys per node.
 {return NWayTreeLongNumberOfKeysPerNode;
 }

static long NWayTreeLong NWayTreeLongMaximumNumberDownPerNode()                 //P Maximum number of children per parent.
 {return NWayTreeLongNumberOfKeysPerNode + 1;
 }

static long NWayTreeLongFull(NWayTreeLongNode *node)                            //P Confirm that a node is full.
 {return node->length == NWayTreeLongMaximumNumberOfKeys();
 }

static long NWayTreeLongHalfFull(NWayTreeLongNode *node)                                                                 //P Confirm that a node is half full.
 {const long n = node->length;
  assert(n <= NWayTreeLongMaximumNumberOfKeys()+1);
  return n == NWayTreeLongMinimumNumberOfKeys();
 }
//
//static void NWayTreeLong root(NWayTreeLongTree *tree)                                                                     // Return the root node of a tree.
// {my ($tree) = @_;                                                            // Tree
// confess unless $tree;
// for(; $tree->up; $tree = $tree->up) {}
// $tree
// }

static long NWayTreeLongIsLeaf(NWayTreeLongNode *node)                          // Confirm that the tree is a leaf.
 {return node->down[0] == 0;                                                    // No children so it must be a leaf
 }

static void NWayTreeLongReUp(NWayTreeLongNode *node)                            //P Reconnect the children to their new parent.
 {for(int i = 0; i < node->length; ++i)
   {node->down[i]->up = node;
   }
 }

static int NWayTreeLongSplitFullNode                                            //P Split a node if it is full. Return true if the node was split else false
 (NWayTreeLongTree *tree, NWayTreeLongNode *node)
 {if (node->length < NWayTreeLongMaximumNumberOfKeys()) return 0;               // Must be a full node

  NWayTreeLongNode
    * const l = NWayTreeLongNewNode(),                                          // New child nodes
    * const r = NWayTreeLongNewNode();

  long N = NWayTreeLongMaximumNumberOfKeys();                                   // Split points
  long n = (N - N % 2)>>1;                                                      // Index of key that will be placed in parent

  l->length = n;
  r->length = N - n - 1;

  memcpy(l->keys, node->keys,        l->length  * sizeof(long));                // Split left keys and data
  memcpy(l->data, node->data,        l->length  * sizeof(long));
  memcpy(l->down, node->down,     (1+l->length) * sizeof(long));

  memcpy(r->keys, node->keys+n+1,    r->length  * sizeof(long));                // Split right keys and data
  memcpy(r->data, node->data+n+1,    r->length  * sizeof(long));
  memcpy(r->down, node->down+n+1, (1+r->length) * sizeof(long));

  if (!NWayTreeLongIsLeaf(node))                                                // Not a leaf node
   {NWayTreeLongReUp(l);
    NWayTreeLongReUp(r);
   }

  if (node->up)                                                                 // Not a root node
   {const long L = sizeof(long);
    NWayTreeLongNode *p = node->up;                                             // Existing parent node
    l->up = r->up = p;                                                          // Connect children to parent
    if (p->down[0] == node)                                                     // Splitting the first child - move everything up
     {memmove(p->keys+1, p->keys,  p->length    * L);
      memmove(p->data+1, p->data,  p->length    * L);
      memmove(p->down+1, p->down, (++p->length) * L);
      p->keys[0] = node->keys[n];
      p->data[0] = node->data[n];
      p->down[0] = l;
      p->down[1] = r;
     }
    else if (p->down[p->length] == node)                                        // Splitting the last child - just add it on the end
     {p->keys[  p->length] = node->keys[n];
      p->data[  p->length] = node->data[n];
      p->down[  p->length] = l;
      p->down[++p->length] = r;
     }
    else                                                                        // Splitting a middle child:
     {for(long i = 1; i < p->length; ++i)
       {if (p->down[i] == node)                                                 // Find the node that points from the parent to the current node
         {memmove(p->keys+i+1, p->keys+i, (p->length-i)   * L);
          memmove(p->data+i+1, p->data+i, (p->length-i)   * L);
          memmove(p->down+i+1, p->down+i, (p->length-i+1) * L);
          p->keys[i]   = node->keys[n];
          p->data[i]   = node->data[n];
          p->down[i]   = l;
          p->down[i+1] = r;
          ++p->length;
          return 1;
         }
       }
      assert(0);
     }
   }
  else                                                                          // Root node with single key after split
   {NWayTreeLongNode *p = tree->node = NWayTreeLongNewNode();                   // Create a new parent node
    l->up = r->up = p;                                                          // Connect children to parent

    p->keys[0] = node->keys[n];                                                 // Single key
    p->data[0] = node->data[n];                                                 // Data associated with single key
    p->down[0] = l;
    p->down[1] = r;
    p->length  = 1;
   }
  return 1;
 }

static NWayTreeLongFindResult NWayTreeLongFindAndSplit                          //P Find a key in a tree splitting full nodes along the path to the key.
 (NWayTreeLongTree *tree, long key)
 {for(long pass = 0; pass < 999; ++pass)                                        // Restart the traversal if we are forced to split a node - we can probably do better by restarting at the parent
   {NWayTreeLongNode *node = tree->node;
    if (!node) return NWayTreeLongNewFindResult(tree, node, key, notFound, -1);

    if (NWayTreeLongSplitFullNode(tree, node))                                  // Split the root node if necessary
     {continue;
     }

    for(long j = 0; j < 999; ++j)                                               // Step down through the tree
     {if (key < node->keys[0])                                                  // Less than smallest key in node
       {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult
         (tree, node, key, lower, 0);                                           // Smallest key in tree
        node = node->down[0];
        continue;
       }

      const long last = node->length-1;                                         // Greater than largest key in node
      if (key > node->keys[last])                                               // Greater than largest key in node
       {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult
         (tree, node, key, higher, last);
        node = node->down[last+1];
        continue;
       }

      for(long i = 1; i < last; ++i)                                            // Search the keys in this node as greater than least key and less than largest key
       {if (key == node->keys[i])                                               // Found key
         {return NWayTreeLongNewFindResult(tree, node, key, equal, i);
         }
        else if (key < node->keys[i])                                           // Greater than current key
         {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult        // Leaf
           (tree, node, key, lower, i);
          node = node->down[i+1];
         }
       }
      if (NWayTreeLongSplitFullNode(tree, node)) break;                         // Split the node we have stepped to if necessary - if the node does need to be split we restart the descent to pick up the new tree. No doubt with some ingenuity we could restart at the parent rather than at the root
     }
   }
  assert(0);
 }

static NWayTreeLongFindResult NWayTreeLongFind                                  // Find a key in a tree returning its associated data or undef if the key does not exist.
 (NWayTreeLongTree *tree, long key)
 {NWayTreeLongNode *node = tree->node;
  if (!node) return NWayTreeLongNewFindResult(tree, node, key, notFound, -1);   // Empty tree

  for(long i = 0; i < 999; ++i)                                                 // Same code as above
   {if (key < node->keys[0])                                                    // Less than smallest key in node
     {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult
       (tree, node, key, lower, 0);
      node = node->down[0];
     }

    const long last = node->length-1;                                           // Index of last key
    if (key > node->keys[last])                                                 // Greater than largest key in node
     {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult
       (tree, node, key, higher, last);
      node = node->down[last+1];
      continue;
     }

    for(long i = 1; i < last; ++i)                                              // Search the keys in this node as greater than least key and less than largest key
     {if (key == node->keys[i])                                                 // Found key
       {return NWayTreeLongNewFindResult(tree, node, key, equal, i);
       }
      else if (key > node->keys[i])                                             // Greater than current key
       {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult          // Leaf
         (tree, node, key, higher, i);
        node = node->down[i+1];
       }
     }
   }
  assert(0);
 }

static long NWayTreeLongIndexInParent                                          //P Get the index of a node in its parent.
 (NWayTreeLongNode *node)
 {NWayTreeLongNode *p = node->up;
  assert(p);
  for(long i = 0; i < node->length; ++i)
   {if (p->down[i] == node) return i;
   }
  assert(0);
 }

static void NWayTreeLongFillFromLeftOrRight                                     //P Fill a node from the specified sibling.
 (NWayTreeLongNode *n, long dir)
 {NWayTreeLongNode * const p = n->up;                                           // Parent of leaf
  assert(p);
  const long i = NWayTreeLongIndexInParent(n);                                  // Index of leaf in parent

  if (dir)                                                                      // Fill from right
   {assert(i < p->length);                                                      // Cannot fill from right
    NWayTreeLongNode * const r = p->down[i+1];                                  // Right sibling
    n->keys[n->length] = p->keys[i];                                            // Transfer key and data to parent
    n->data[n->length] = p->data[i];

    p->keys[i] = ArrayLongShift(r->keys, r->length);                            // Transfer keys and data from right
    p->data[i] = ArrayLongShift(r->data, r->length);

    if (!NWayTreeLongIsLeaf(n))                                                 // Transfer node if not a leaf
     {ArrayVoidPush((void *)n->down, n->length, (void *)ArrayVoidShift((void *)r->down, r->length));
      n->down[n->length+1]->up = n;
     }
    r->length--; n->length++;
   }
  else                                                                          // Fill from left
   {assert(i);                                                                  // Cannot fill from left
    long I = i-1;
    NWayTreeLongNode *n = p->down[I];                                           // Left sibling
    ArrayLongUnShift(n->keys, n->length, p->keys[I]);                           // Shift in keys and data from left
    ArrayLongUnShift(n->data, n->length, p->data[I]);

    p->keys[I] = ArrayLongPop(n->keys, n->length);                              // Transfer key and data to parent
    p->data[I] = ArrayLongPop(n->data, n->length);

    if (!NWayTreeLongIsLeaf(n))                                                 // Transfer node if not a leaf
     {ArrayVoidUnShift((void *)n->down, n->length, (void *)ArrayVoidPop((void *)n->down, n->length));
      n->down[0]->up = n;
     }
   }
 }

static void NWayTreeLongMergeWithLeftOrRight                                    //P Merge two adjacent nodes.
 (NWayTreeLongNode *n, long dir)
 {assert(NWayTreeLongHalfFull(n));                                              // Confirm leaf is half full
  NWayTreeLongNode *p = n->up;                                                  // Parent of leaf
  assert(p);
  assert(NWayTreeLongHalfFull(p) && p->up);                                     // Parent must have more than the minimum number of keys because we need to remove one unless it is the root of the tree

  long i = NWayTreeLongIndexInParent(n);                                        // Index of leaf in parent

  if (dir)                                                                      // Merge with right hand sibling
   {assert(i < p->length);                                                      // Cannot fill from right
    long I = i+1;
    NWayTreeLongNode *r = p->down[I];                                           // Leaf on right
    assert(NWayTreeLongHalfFull(r));                                            // Confirm right leaf is half full

    ArrayLongPush(n->keys, n->length, ArrayLongDelete(p->keys, p->length, I));  // Transfer keys and data from parent
    ArrayLongPush(n->data, n->length, ArrayLongDelete(p->data, p->length, I));

    ArrayLongPushArray(n->keys, n->length+1, r->keys, r->length);               // Transfer ketys and
    ArrayLongPushArray(n->data, n->length+1, r->data, r->length);

    if (!NWayTreeLongIsLeaf(n))                                                 // Children of merged node
     {ArrayVoidPushArray((void *)n->down, n->length, (void *)r->down, r->length);
      NWayTreeLongReUp(n);                                                      // Update parent of children of right node
     }
    ArrayVoidDelete((void *)p->down, p->length, I);                             // Remove link from parent to right child
    n->length += 1 + r->length; --p->length;
    free(r);
   }
  else                                                                          // Merge with left hand sibling
   {assert(i > 0);                                                              // Cannot fill from left
    const long I = i-1;
    NWayTreeLongNode * const l = p->down[I];                                    // Node on left
    assert(NWayTreeLongHalfFull(l));                                            // Confirm left leaf is half full
    const long k = ArrayLongDelete(p->keys, p->length, I);                      // Transfer parent key and data
    const long d = ArrayLongDelete(p->data, p->length, I);
    ArrayLongUnShift     (n->keys, n->length,   k);
    ArrayLongUnShift     (n->data, n->length,   d);
    ArrayLongUnShiftArray(n->keys, n->length+1, l->keys, l->length);            // Transfer left keys and data
    ArrayLongUnShiftArray(n->data, n->length+1, l->data, l->length);

    if (!NWayTreeLongIsLeaf(n))                                                 // Children of merged node
     {ArrayLongUnShiftArray((void *)n->down, n->length,
                            (void *)l->down, l->length);
      NWayTreeLongReUp(n);                                                      // Update parent of children of left node
     }
    ArrayVoidDelete((void *)p->down, p->length, I);                             // Remove link from parent to right child
    n->length += 1 + l->length; --p->length;
    free(l);
   }
 }

static void NWayTreeLongMerge(NWayTreeLongNode *node)                           //P Merge the current node with its sibling.
 {long i = NWayTreeLongIndexInParent(node);                                     // Index in parent
  if (i)                                                                        // Merge with left node
   {NWayTreeLongNode *l = node->up->down[i-1];                                  // Left node
    NWayTreeLongNode *r = node;
    if (NWayTreeLongHalfFull(r))
     {NWayTreeLongHalfFull(l) ? NWayTreeLongMergeWithLeftOrRight(r, 0):
                                NWayTreeLongFillFromLeftOrRight (r, 0);         // Merge as left and right nodes are half full
     }
   }
  else
   {NWayTreeLongNode *r = node->up->down[1];                                    // Right node
    NWayTreeLongNode *l = node;
    if (NWayTreeLongHalfFull(l))
     {NWayTreeLongHalfFull(r) ? NWayTreeLongMergeWithLeftOrRight(l, 1):
                                NWayTreeLongFillFromLeftOrRight (l, 1);         // Merge as left and right nodes are half full
     }
   }
 }

static void NWayTreeLongMergeOrFill(NWayTreeLongNode *node)                     //P Make a node larger than a half node.
 {if (NWayTreeLongHalfFull(node)) return;                                       // No need to merge of if not a half node
  NWayTreeLongNode *p = node->up;                                               // Parent exists

  if (p->up)                                                                    // Merge or fill parent which is not the root
   {NWayTreeLongMergeOrFill(p);
    NWayTreeLongMerge(node);
   }
  else
   {NWayTreeLongNode *l = p->down[0];
    NWayTreeLongNode *r = p->down[1];
    if (p->length == 1 && NWayTreeLongHalfFull(l) && NWayTreeLongHalfFull(r))   // Parent is the root and it only has one key - merge into the child if possible
     {const long L = l->length, R = r->length, N = node->length;
      ArrayLongPushArray(node->keys, 0, l->keys, L);
      ArrayLongPushArray(node->data, 0, l->data, L);

      ArrayLongPushArray(node->keys, L, p->keys, 1);
      ArrayLongPushArray(node->data, L, p->data, 1);

      ArrayLongPushArray(node->keys, L+1, r->keys, R);
      ArrayLongPushArray(node->data, L+1, r->data, R);

      ArrayVoidPushArray((void *)node->down, 0,   (void *)l->down, L+1);
      ArrayVoidPushArray((void *)node->down, L+1, (void *)r->down, R);
      node->length = L+R+1;

      ArrayLongPushArray(p->keys, 0, node->keys, N);
      ArrayLongPushArray(p->data, 0, node->data, N);
      ArrayVoidPushArray((void *)p->down, 0, (void *)node->down, N+1);

      NWayTreeLongReUp(p);                                                      // Reconnect children to parent
     }
    else                                                                        // Parent is the root but it has too may keys to merge into both sibling so merge with a sibling if possible
     {NWayTreeLongMerge(node);
     }
   }
 }

static NWayTreeLongNode *NWayTreeLongLeftMost(NWayTreeLongNode *node)           // Return the left most node below the specified one.
 {for(long i = 0; i < 999; ++i)
   {if (NWayTreeLongIsLeaf(node)) return node;                                  // We are on a leaf so we have arrived at the left most node
    node = node->down[0];                                                       // Go left
   }
  assert(0);
 }

static NWayTreeLongNode *NWayTreeLongRightMost(NWayTreeLongNode *node)          // Return the right most node below the specified one.
 {for(long i = 0; i < 999; ++i)
   {if (NWayTreeLongIsLeaf(node)) return node;                                  // We are on a leaf so we have arrived at the left most node
    node = node->down[node->length];                                            // Go Right
   }
  assert(0);
 }

static long NWayTreeLongDepth(NWayTreeLongNode *node)                           // Return the depth of a node within a tree.
 {if (!node->up && !node->length) return 0;                                     // We are at the root and it is empty
  for(long n = 0; n < 999; ++n)
   {if (!node->up) return n;
    node = node->up;
   }
  assert(0);
 }

static void NWayTreeLongInsert                                                  // Insert a key and its associated data into a tree
 (NWayTreeLongTree *tree, long key, long data)
 {if (!tree->node)                                                              // Empty tree
   {NWayTreeLongNode *n = NWayTreeLongNewNode();
    n->keys[0] = key; n->data[0] = data; n->length = 1; tree->size++;
    tree->node = n;
    return;
   }

  NWayTreeLongNode *n = tree->node;
  if (n->length < NWayTreeLongMaximumNumberOfKeys() && !n->up && NWayTreeLongIsLeaf(n)) // Node is root with no children and room for one more key
   {for(long i = 0; i < n->length; ++i)                                         // Each key
     {if (key == n->keys[i])                                                    // Key already present
       {n->data[i]= data;
        return;
       }
      if (key < n->keys[i])                                                     // We have reached the insertion point
       {ArrayLongInsert(n->keys, n->length, i, key);
        ArrayLongInsert(n->data, n->length, i, data);
        n->length++; tree->size++;
        return;
       }
     }
    ArrayLongPush(n->keys, n->length, key);                                     // Insert the key at the end of the block because it is greater than all the other keys in the block
    ArrayLongPush(n->data, n->length, data);
    n->length++; tree->size++;
   }
  else                                                                          // Insert node
   {NWayTreeLongFindResult r = NWayTreeLongFindAndSplit(tree, key);             // Check for existing key
    NWayTreeLongNode      *n = r.node;
    if (r.cmp == equal)                                                         // Found an equal key whose data we can update
     {n->data[r.index] = data;
     }
    else                                                                        // We have room for the insert
     {long index = r.index;
      if (r.cmp == higher) ++index;                                             // Position at which to insert new key
      ArrayLongInsert(n->keys, n->length, key,  index);
      ArrayLongInsert(n->data, n->length, data, index);
      ++n->length;
      NWayTreeLongSplitFullNode(tree, n);                                       // Split if the leaf is full to force keys up the tree
     }
   }
 }

/*
static void NWayTreeLong deleteLeafKey($$)                                      //P Delete a key in a leaf.
 {my ($tree, $i) = @_;                                                          // Tree, index to delete at
  @_ == 2 or confess;
  confess "Not a leaf" unless leaf $tree;
  long key = $tree->keys->[$i];
  mergeOrFill $tree if $tree->up;                                               // Merge and fill unless we are on the root and the root is a leaf
  long k = $tree->keys;
  for long j(keys @$k)                                                          // Search for key to delete
   {if ($$k[$j] == $key)
     {splice $tree->keys->@*, $j, 1;                                            // Remove keys
      splice $tree->data->@*, $j, 1;                                            // Remove data
      return;
     }
   }
 }

static void NWayTreeLong deleteKey($$)                                          //P Delete a key.
 {my ($tree, $i) = @_;                                                          // Tree, index to delete at
  @_ == 2 or confess;
  if (leaf $tree)                                                               // Delete from a leaf
   {deleteLeafKey($tree, $i);
   }
  elsif ($i > 0)                                                                // Delete from a node
   {long l = rightMost $tree->node->[$i];                                       // Find previous node
    splice  $tree->keys->@*, $i, 1, $l->keys->[-1];
    splice  $tree->data->@*, $i, 1, $l->data->[-1];
    deleteLeafKey $l, -1 + scalar $l->keys->@*;                                 // Remove leaf key
   }
  else                                                                          // Delete from a node
   {long r = leftMost $tree->node->[1];                                         // Find previous node
    splice  $tree->keys->@*,  0, 1, $r->keys->[0];
    splice  $tree->data->@*,  0, 1, $r->data->[0];
    deleteLeafKey $r, 0;                                                        // Remove leaf key
   }
 }

static void NWayTreeLong delete($$)                                             // Find a key in a tree, delete it and return any associated data.
 {my ($root, $key) = @_;                                                        // Tree root, key
  @_ == 2 or confess;

  long tree = $root;
  for (0..999)
   {long k = $tree->keys;

    if ($key < $$k[0])                                                          // Less than smallest key in node
     {return undef unless $tree = $tree->node->[0];
     }
    elsif ($key > $$k[-1])                                                      // Greater than largest key in node
     {return undef unless $tree = $tree->node->[-1];
     }
    else
     {for long i(keys @$k)                                                      // Search the keys in this node
       {if ((my  $s = $key <=> $$k[$i]) == 0)                                   // Delete found key
         {long d = $tree->data->[$i];                                           // Save data
          deleteKey $tree, $i;                                                  // Delete the key
          return $d;                                                            // Return data associated with key
         }
        elsif ($s < 0)                                                          // Less than current key
         {return undef unless $tree = $tree->node->[$i];
          last;
         }
       }
     }
   }
  confess "Should not happen";
 }

static void NWayTreeLong insert($$$)                                            // Insert the specified key and data into a tree.
 {my ($tree, $key, $data) = @_;                                                 // Tree, key, data
  @_ == 3 or confess;

  if (!(long n = $tree->keys->@*))                                              // Empty tree
   {push $tree->keys->@*, $key;
    push $tree->data->@*, $data;
    return $tree;
   }
  elsif ($n < NWayTreeLongMaximumNumberOfKeys and $tree->node->@* == 0)         // Node is root with no children and room for one more key
   {long k = $tree->keys;
    for long i(reverse keys @$k)                                                // Each key - in reverse due to the preponderance of already sorted data
     {if ((long s = $key <=> $$k[$i]) == 0)                                     // Key already present
       {$tree->data->[$i]= $data;
        return;
       }
      elsif ($s > 0)                                                            // Insert before greatest smaller key
       {long I = $i + 1;
        splice $tree->keys->@*, $I, 0, $key;
        splice $tree->data->@*, $I, 0, $data;
        return;
       }
     }
    unshift $tree->keys->@*, $key;                                              // Insert the key at the start of the block because it is less than all the other keys in the block
    unshift $tree->data->@*, $data;
   }
  else                                                                          // Insert node
   {my ($compare, $node, $index) = findAndSplit $tree, $key;                    // Check for existing key

    if ($compare == 0)                                                          // Found an equal key whose data we can update
     {$node->data->[$index] = $data;
     }
    else                                                                        // We have room for the insert
     {++$index if $compare > 0;                                                 // Position at which to insert new key
      splice $node->keys->@*, $index, 0, $key;
      splice $node->data->@*, $index, 0, $data;
      splitFullNode 0, 1, $node                                                 // Split if the leaf is full to force keys up the tree
     }
   }
 }

static void NWayTreeLong iterator($)                                            // Make an iterator for a tree.
 {my ($tree) = @_;                                                              // Tree
  @_ == 1 or confess;
  long i = genHash(__PACKAGE__.'::Iterator',                                    // Iterator
    tree  => $tree,                                                             // Tree we are iterating over
    node  => $tree,                                                             // Current node within tree
    pos   => undef,                                                             // Current position within node
    key   => undef,                                                             // Key at this position
    data  => undef,                                                             // Data at this position
    count => 0,                                                                 // Counter
    more  => 1,                                                                 // Iteration not yet finished
   );
  $i->next;                                                                     // First element if any
  $i                                                                            // Iterator
 }

static void NWayTreeLong Tree::Multi::Iterator::next($)                         // Find the next key.
 {my ($iter) = @_;                                                              // Iterator
  @_ == 1 or confess;
  confess unless long C = $iter->node;                                          // Current node required

  ++$iter->count;                                                               // Count the calls to the iterator

  long new  = static void NWayTreeLong                                          // Load iterator with latest position
   {my ($node, $pos) = @_;                                                      // Parameters
    $iter->node = $node;
    $iter->pos  = $pos //= 0;
    $iter->key  = $node->keys->[$pos];
    $iter->data = $node->data->[$pos]
   };

  long done = static void NWayTreeLong {$iter->more = undef};                   // The tree has been completely traversed

  if (!defined($iter->pos))                                                     // Initial descent
   {long l = $C->node->[0];
    return $l ? &$new($l->leftMost) : $C->keys->@* ? &$new($C) : &$done;        // Start node or done if empty tree
   }

  long up = static void NWayTreeLong                                            // Iterate up to next node that has not been visited
   {for(long n = $C; long p = $n->up; $n = $p)
     {long i = indexInParent $n;
      return &$new($p, $i) if $i < $p->keys->@*;
     }
    &$done                                                                      // No nodes not visited
   };

  long i = ++$iter->pos;
  if (leaf $C)                                                                  // Leaf
   {$i < $C->keys->@* ? &$new($C, $i) : &$up;
   }
  else                                                                          // Node
   {&$new($C->node->[$i]->leftMost)
   }
 }

static void NWayTreeLong reverseIterator($)                                     // Create a reverse iterator for a tree.
 {my ($tree) = @_;                                                              // Tree
  @_ == 1 or confess;
  long i = genHash(__PACKAGE__.'::ReverseIterator',                             // Iterator
    tree  => root($tree),                                                       // Tree we are iterating over
    node  => $tree,                                                             // Current node within tree
    pos   => undef,                                                             // Current position within node
    key   => undef,                                                             // Key at this position
    data  => undef,                                                             // Data at this position
    count => 0,                                                                 // Counter
    less  => 1,                                                                 // Iteration not yet finished
   );
  $i->prev;                                                                     // Last element if any
  $i                                                                            // Iterator
 }

static void NWayTreeLong Tree::Multi::ReverseIterator::prev($)                  // Find the previous key.
 {my ($iter) = @_;                                                              // Iterator
  @_ == 1 or confess;
  confess unless long C = $iter->node;                                          // Current node required

  ++$iter->count;                                                               // Count the calls to the iterator

  long new  = static void NWayTreeLong                                          // Load iterator with latest position
   {my ($node, $pos) = @_;                                                      // Parameters
    $iter->node = $node;
    $iter->pos  = $pos //= ($node->keys->@* - 1);
    $iter->key  = $node->keys->[$pos];
    $iter->data = $node->data->[$pos]
   };

  long done = static void NWayTreeLong {$iter->less = undef};                   // The tree has been completely traversed

  if (!defined($iter->pos))                                                     // Initial descent
   {long l = $C->node->[-1];
    return $l ? &$new($l->rightMost) : $C->keys->@* ? &$new($C) : &$done;       // Start node or done if empty tree
    return;
   }

  long up = static void NWayTreeLong                                            // Iterate up to next node that has not been visited
   {for(long n = $C; long p = $n->up; $n = $p)
     {long i = indexInParent $n;
      return &$new($p, $i-1) if $i > 0;
     }
    &$done                                                                      // No nodes not visited
   };

  long i = $iter->pos;
  if (leaf $C)                                                                  // Leaf
   {$i > 0 ?  &$new($C, $i-1) : &$up;
   }
  else                                                                          // Node
   {$i >= 0 ? &$new($C->node->[$i]->rightMost) : &$up
   }
 }

static void NWayTreeLong flat($@)                                               // Print the keys in a tree from left right to make it easier to visualize the structure of the tree.
 {my ($tree, @title) = @_;                                                      // Tree, title
  confess unless $tree;
  my @s;                                                                        // Print
  long D;                                                                       // Deepest
  for(long i = iterator root $tree; $i->more; $i->next)                         // Traverse tree
   {long d = depth $i->node;
    $D = $d unless $D and $D > $d;
    $s[$d] //= '';
    $s[$d]  .= "   ".$i->key;                                                   // Add key at appropriate depth
    long l = length $s[$d];
    for long j(0..$D)                                                           // Pad all strings to the current position
     {long s = $s[$j] //= '';
      $s[$j] = substr($s.(' 'x999), 0, $l) if length($s) < $l;
     }
   }
  for long i(keys @s)                                                            // Clean up trailing blanks so that tests are not affected by spurious white space mismatches
   {$s[$i] =~ s/\s+\n/\n/gs;
    $s[$i] =~ s/\s+\Z//gs;
   }
  unshift @s, join(' ', @title) if @title;                                      // Add title
  join "\n", @s, '';
 }

static void NWayTreeLong size($)                                                // Count the number of keys in a tree.
 {my ($tree) = @_;                                                              // Tree
  @_ == 1 or confess;
  long n = 0;                                                                   // Print

  long count = static void NWayTreeLong                                         // Print a node
   {my ($t) = @_;
    return unless $t and $t->keys and my @k = $t->keys->@*;
    $n += @k;
    if (long nodes = $t->node)                                                  // Each key
     {__SUB__->($_) for $nodes->@*;
     }
   };

  &$count(root $tree);                                                          // Count nodes in tree

  $n;
 }
*/

#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 1; ++i) NWayTreeLongInsert(tree, i, 2);
  assert(NWayTreeLongEqText(tree,
"   0     2\n"
));
 }

void test2()                                                                    // Tests
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 2; ++i) NWayTreeLongInsert(tree, i, i+2);
  assert(NWayTreeLongEqText(tree,
"   0     2\n"
"   1     3\n"
));
 }

void test3()                                                                    // Tests
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 3; ++i) NWayTreeLongInsert(tree, i, i+2);
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"      0     2\n"
"   1     3\n"
"      2     4\n"
));
 }

NWayTreeLongNode *createNode3(long a, long b, long c)                           // Create a test node
 {NWayTreeLongNode *n = NWayTreeLongNewNode();
  n->keys[0] = a; n->data[0] = 2*a;
  n->keys[1] = b; n->data[1] = 2*b;
  n->keys[2] = c; n->data[2] = 2*c;
  n->length  = 3;
  return n;
 }

void test4a()                                                                   // Tree has one node
 {NWayTreeLongTree *t = NWayTreeLongNewTree();
  NWayTreeLongNode *n = createNode3(1, 2, 3);
  t->size = n->length = 3;
  t->node = n;

  long r = NWayTreeLongSplitFullNode(t, n);
  assert(r);
  assert(NWayTreeLongEqText(t,
"      1     2\n"
"   2     4\n"
"      3     6\n"
));
 }

void test4b()                                                                   // First down
 {NWayTreeLongTree *t  = NWayTreeLongNewTree();
  NWayTreeLongNode *p  = createNode3(10, 20, 30); p->length = 2;
  NWayTreeLongNode *n0 = createNode3(01, 02, 03);
  NWayTreeLongNode *n1 = createNode3(11, 12, 13);
  NWayTreeLongNode *n2 = createNode3(21, 22, 23);
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;

  assert(NWayTreeLongEqText(t,
"      1     2\n"
"      2     4\n"
"      3     6\n"
"  10    20\n"
"     11    22\n"
"     12    24\n"
"     13    26\n"
"  20    40\n"
"     21    42\n"
"     22    44\n"
"     23    46\n"
));

  long r = NWayTreeLongSplitFullNode(t, n0); if (r){}
  assert(NWayTreeLongEqText(t,
"      1     2\n"
"   2     4\n"
"      3     6\n"
"  10    20\n"
"     11    22\n"
"     12    24\n"
"     13    26\n"
"  20    40\n"
"     21    42\n"
"     22    44\n"
"     23    46\n"
));
//NWayTreeLongErrAsC(t);
 }

void test4c()                                                                   // Mid down
 {NWayTreeLongTree *t  = NWayTreeLongNewTree();
  NWayTreeLongNode *p  = createNode3(10, 20, 30); p->length = 2;
  NWayTreeLongNode *n0 = createNode3(01, 02, 03);
  NWayTreeLongNode *n1 = createNode3(11, 12, 13);
  NWayTreeLongNode *n2 = createNode3(21, 22, 23);
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;

  assert(p->down[1] == n1);

  long r = NWayTreeLongSplitFullNode(t, n1); if (r){}
  assert(NWayTreeLongEqText(t,
"      1     2\n"
"      2     4\n"
"      3     6\n"
"  10    20\n"
"     11    22\n"
"  12    24\n"
"     13    26\n"
"  20    40\n"
"     21    42\n"
"     22    44\n"
"     23    46\n"
));
 }

void test4d()                                                                   // Final node
 {NWayTreeLongTree *t  = NWayTreeLongNewTree();
  NWayTreeLongNode *p  = createNode3(10, 20, 30); p->length = 2;
  NWayTreeLongNode *n0 = createNode3(01, 02, 03);
  NWayTreeLongNode *n1 = createNode3(11, 12, 13);
  NWayTreeLongNode *n2 = createNode3(21, 22, 23);
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;

  assert(p->down[1] == n1);

  long r = NWayTreeLongSplitFullNode(t, n2); if (r){}
  assert(NWayTreeLongEqText(t,
"      1     2\n"
"      2     4\n"
"      3     6\n"
"  10    20\n"
"     11    22\n"
"     12    24\n"
"     13    26\n"
"  20    40\n"
"     21    42\n"
"  22    44\n"
"     23    46\n"
));
//NWayTreeLongErrAsC(t);
 }

void test4()                                                                    // Tests
 {test4a();
  test4b();
  test4c();
  test4d();
 }

void testInsert1()                                                              // Insert tests
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  NWayTreeLongErrAsC(tree);
  for(int i = 0; i < 1; ++i)
   {NWayTreeLongInsert(tree, i, i+2);
   }
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"   0     2\n"
));
 }

void testInsert2()
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 2; ++i)
   {NWayTreeLongInsert(tree, i, i+2);
   }
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"   0     2\n"
"   1     3\n"
));
 }

void testInsert3()
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 3; ++i)
   {NWayTreeLongInsert(tree, i, i+2);
   }
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"      0     2\n"
"   1     3\n"
"      2     4\n"
));
 }

void testInsert4()
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 4; ++i)
   {NWayTreeLongInsert(tree, i, i+2);
   }
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"      0     2\n"
"   1     3\n"
"      2     4\n"
"      3     5\n"
));
 }

void testInsert5()
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 5; ++i)
   {NWayTreeLongInsert(tree, i, i+2);
   }
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"      0     2\n"
"   1     3\n"
"      2     4\n"
"   3     5\n"
"      4     6\n"
));
 }

void testInsert6()
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 6; ++i)
   {NWayTreeLongInsert(tree, i, i+2);
   }
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"      0     2\n"
"   1     3\n"
"      2     4\n"
"   3     5\n"
"      4     6\n"
"      5     7\n"
));
 }

void testInsert7()
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 7; ++i)
   {NWayTreeLongInsert(tree, i, i+2);
   }
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"      0     2\n"
"   1     3\n"
"      2     4\n"
"   3     5\n"
"      4     6\n"
"   5     7\n"
"      6     8\n"
));
 }

void testInsert8()
 {NWayTreeLongTree * const tree = NWayTreeLongNewTree();
  for(int i = 0; i < 8; ++i)
   {NWayTreeLongInsert(tree, i, i+2);
   }
  //NWayTreeLongErrAsC(tree);
  assert(NWayTreeLongEqText(tree,
"         0     2\n"
"      1     3\n"
"         2     4\n"
"   3     5\n"
"         4     6\n"
"      5     7\n"
"         6     8\n"
"         7     9\n"
));
 }

void testInsert()                                                               // Tests
 {testInsert1();
  testInsert2();
  testInsert3();
  testInsert4();
  testInsert5();
  testInsert6();
  testInsert7();
  testInsert8();
 }

void tests()                                                                    // Tests
 {testInsert8(); exit(1);
  test1();
  test2();
  test3();
  test4();
  testInsert();
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
#endif
