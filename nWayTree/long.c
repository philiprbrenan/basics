//------------------------------------------------------------------------------
// N way tree with long keys
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// Replace Array long
// sde -mix -- ./long
#ifndef NWayTreeLong
#define NWayTreeLong
#define _GNU_SOURCE

#define NWayTreeLongMaxIterations 99                                            /* The maximum number of levels in a tree */
#define f(name) NWayTreeLong##name                                              /* Function name */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include <malloc.h>
#include "array/long.c"
#include "array/void.c"
#include "basics/basics.c"
#include "stack/char.c"

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

//sOptimize
#define static                                                                  /* Simplify debugging by preventing some inline-ing which invalidates the call stack */

typedef long NWayTreeDataType;                                                  // The key and data type used by this tree

typedef struct f(Node)                                                          // A node in a tree
 {long length;                                                                  // The current number of keys in the node
  struct f(Node) *up;
  NWayTreeDataType *keys;
  NWayTreeDataType *data;
  struct f(Node) **down;
  struct f(Tree) *tree;
 } f(Node);

typedef struct f(Tree)                                                          // The root of a tree
 {long NumberOfKeysPerNode;                                                     // Size of a node
  f(Node) *node;                                                                // Root node
  long size;                                                                    // Number of nodes in tree
 } f(Tree);

typedef enum f(FindComparison)                                                  // The results of a comparison
 {f(FindComparison_lower),
  f(FindComparison_equal),
  f(FindComparison_higher),
  f(FindComparison_notFound)
 } f(FindComparison);

typedef struct f(FindResult)                                                    // The results of a find operation
 {f(Tree) *tree;                                                                // Tree searched
  f(Node) *node;                                                                // Node found
  f(FindComparison) cmp;                                                        // Result of the last comparison
  NWayTreeDataType key;                                                         // Key searched for
  long index;                                                                   // Index in the node of equal element
  NWayTreeDataType data;                                                        // Data element at key
 } f(FindResult);

static f(Tree) *f(NewTree)                                                      // Create a new tree
 (const long n)                                                                 // Number of keys in a node
 {f(Tree) * const tree = calloc(sizeof(f(Tree)), 1);
  tree->NumberOfKeysPerNode = n;
  return tree;
 }

static f(Node) *f(NewNode)                                                      //P Create a new node
 (f(Tree) * const tree)                                                         // Tree containing node
 {const long z = tree->NumberOfKeysPerNode;
  const long k = sizeof(long)                      *  z;
  const long d = sizeof(long)                      *  z;
  const long n = sizeof(struct f(Node) *) * (z + 1);
  const long s = sizeof(f(Node));

  f(Node) * const node = calloc(s+k+d+n, 1);
  node->keys = (void *)(((void *)node)+s);
  node->data = (void *)(((void *)node)+s+k);
  node->down = (void *)(((void *)node)+s+k+d);
  node->tree = tree;
  return node;
 }

static void f(FreeNode)                                                         //P Free a node, Wipe the node so it cannot be accidently reused
 (f(Node) * const node)                                                         // Node to free
 {const long z = node->tree->NumberOfKeysPerNode;
  const long k = sizeof(long)                      *  z;
  const long d = sizeof(long)                      *  z;
  const long n = sizeof(struct f(Node) *) * (z + 1);
  const long s = sizeof(f(Node));
  memset(node, -1, s+k+d+n);                                                    // Clear node
  free(node);
 }

static void f(ErrNode)  (f(Node) *node);
static long f(CheckNode)(f(Node) *node, char *name);
static void f(CheckTree)(f(Tree) *tree, char *name);
static long f(IsLeaf)   (f(Node) *node);

static f(FindResult) f(NewFindResult)                                           //P New find result on stack
 (f(Node) * const node, NWayTreeDataType const key,
  f(FindComparison) const cmp, long const index)
 {f(FindResult) r;
  r.tree  = node->tree;
  r.node  = node;
  r.key   = key;
  r.cmp   = cmp;
  r.index = index;
  r.data  = node->data[index];
  return r;
 }

static void f(Free2)                                                            //P Free a node in a tree
 (f(Node) * const node)
 {if (!node) return;
  if (node->length)
   {if (!f(IsLeaf)(node))
      {f(Free2)(node->down[0]);

      for(long i = 1; i <= node->length; ++i)
       {f(Free2)(node->down[i]);
       }
     }
   }
  f(FreeNode)(node);
 }

static void f(Free)                                                             // Free a tree
 (f(Tree) * const tree)
 {if (!tree) return;
  f(Free2)(tree->node);
  memset(tree, -1, sizeof(*tree));
  free(tree);
 }

static void f(ToString2)                                                        //P Print the keys in a tree
 (f(Node) * const node, long const in, StackChar * const p)
 {if (!node || !node->length) return;
  f(ToString2)(node->down[0], in+1, p);
  for(long i = 0; i < node->length; ++i)
   {for(long j = 0; j < in * 3; ++j) StackCharPush(p, ' ');
    char C[100];
    sprintf(C, "%4ld                                %4ld\n",
              (node->keys[i]), (node->data[i]));
    for(char *c = C; *c; ++c) StackCharPush(p, *c);
    f(ToString2)(node->down[i+1], in+1, p);
   }
 }

static StackChar *f(ToString)                                                   // Print a tree as a string
 (f(Tree) * const tree)                                                         // Tree to print as a string
 {StackChar * const p = StackCharNew();
  if (tree->node) f(ToString2)(tree->node, 0, p);
  return p;
 }

static void f(PrintErr)                                                         // Print a tree on stderr
 (f(Tree) * const tree)                                                         // Tree to print
 {StackChar * const s = f(ToString)(tree);
  say("%s", s->arena+s->base);
  free(s);
 }

static void f(ErrAsC)                                                           // Print a tree as C strings on stderr
 (f(Tree) * const tree)                                                         // Tree to print
 {StackChar * const s = f(ToString)(tree);
  const long N = s->next-s->base;                                               // The number of characters to print
  fputs("assert(f(EqText)(t,\n", stderr);
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

static void f(ErrNode)                                                          // Dump a node
 (f(Node) * const node)                                                         // Node
 {say("Node at  %p", node);
  say("  Up     = %p", node->up);
  say("  Length = %ld", node->length);
  fprintf(stderr, "  Keys   : ");
  for(long i = 0; i <  node->length; ++i) fprintf(stderr," %ld", (node->keys[i]));
  fprintf(stderr, "\n  Data   : ");
  for(long i = 0; i <  node->length; ++i) fprintf(stderr," %ld", (node->data[i]));
  fprintf(stderr, "\n  Down   : ");
  for(long i = 0; i <= node->length; ++i) fprintf(stderr," %p",   node->down[i]);
  say("\n");
 }

static long f(EqText)
 (f(Tree) * const tree, char * const text)
 {StackChar *s = f(ToString)(tree);
  return strncmp(s->arena+s->base, text, s->next-s->base) == 0;
 }

static void f(ErrFindResult)                                                    // Print a find result
 (f(FindResult) r)                                                              // Find result
 {char *c;
  switch(r.cmp)
   {case f(FindComparison_equal):  c = "equal";    break;
    case f(FindComparison_lower):  c = "lower";    break;
    case f(FindComparison_higher): c = "higher";   break;
    default:                                c = "notFound"; break;
   }

  say("Find key=%ld Result keys[index]=%ld %s  index=%ld",
      r.key, r.node->keys[r.index], c, r.index);
 }

static long f(MinimumNumberOfKeys)                                              //P Minimum number of keys per node.
 (f(Tree) * const tree)                                                         // Tree
 {return (tree->NumberOfKeysPerNode - 1) / 2;
 }

static long f(MaximumNumberOfKeys)                                              //P Maximum number of keys per node.
 (f(Tree) * const tree)                                                         // Tree
 {return tree->NumberOfKeysPerNode;
 }

static long NWayTreeLong f(MaximumNumberDownPerNode)                            //P Maximum number of children per parent.
 (f(Tree) * const tree)                                                         // Tree
 {return tree->NumberOfKeysPerNode + 1;
 }

static long f(Full)                                                             //P Confirm that a node is full.
 (f(Node) * const node)
 {return node->length == f(MaximumNumberOfKeys)(node->tree);
 }

static long f(HalfFull)                                                         //P Confirm that a node is half full.
 (f(Node) * const node)                                                         // Node
 {const long n = node->length;
  assert(n <= f(MaximumNumberOfKeys)(node->tree)+1);
  return n == f(MinimumNumberOfKeys)(node->tree);
 }

static long f(IsLeaf)                                                           //P Confirm that the tree is a leaf.
 (f(Node) * const node)                                                         // Node to test
 {return node->down[0] == 0;                                                    // No children so it must be a leaf
 }

static void f(ReUp)                                                             //P Reconnect the children to their new parent.
 (f(Node) * const node)                                                         // Node to reconnect
 {for(int i = 0; i <= node->length; ++i)
   {node->down[i]->up = node;
   }
 }

static long f(CheckNode)                                                        //P Check the connections to and from a node
 (f(Node) * const node, char * const name)
 {if (node->length > f(MaximumNumberOfKeys)(node->tree))
   {say("%s: Node %ld is too long at %ld", name, node->keys[0], node->length);
    return 1;
   }
  for(long i = 0; i <= node->length; ++i)                                       // Check that each child has a correct up reference
   {f(Node) * const d = node->down[i];                                          // Step down
    if (d)
     {if (d->length > f(MaximumNumberOfKeys)(node->tree))
       {say("%s: Node %ld under %ld is too long at %ld",
            name, node->keys[0], d->keys[0], d->length);
        return 2;
       }

      if (d->up != node)
       {say("%s: Node %ld(%p) under %ld(%p) has wrong up pointer %ld(%p)",
             name, d->keys[0], d, node->keys[0], node, d->up->keys[0], d->up);
        return 3;
       }
     }
   }

  f(Node) * const p = node->up;                                                 // Check that parent connects to the current node
  if (p)
   {assert(p->length <= f(MaximumNumberOfKeys)(node->tree));
    int c = 0;                                                                  // Check that the parent has a down pointer to the current node
    for(long i = 0; i <= p->length; ++i)
     {if (p->down[i] == node) ++c;                                              // Find the node that points from the parent to the current node
     }
    if (c != 1)                                                                 // We must be able to find the child
     {say("%s: Node %ld has parent %ld that fails to refer back to it",
         name, node->keys[0], p->keys[0]);
      return 4;
     }
   }
  return 0;
 }

static void f(CheckTree2)                                                       //P Check the structure of a tree
 (f(Node) * const node, char * const name)
 {if (!node) return;

  if (f(CheckNode)(node, name))
   {f(PrintErr)(node->tree);
    assert(0);
   }

  f(CheckTree2)(node->down[0], name);
  for(long i = 0; i < node->length; ++i)
   {f(CheckTree2)(node->down[i+1], name);
   }
 }

static void f(CheckTree)                                                        //P Check the structure of a tree
 (f(Tree) * const tree, char * const name)
 {f(CheckTree2)(tree->node, name);
 }

static long f(SplitFullNode)                                                    //P Split a node if it is full. Return true if the node was split else false
 (f(Node) * const node)
 {if (node->length < f(MaximumNumberOfKeys)(node->tree)) return 0;              // Must be a full node

  f(Node)
    * const l = f(NewNode)(node->tree),                                         // New child nodes
    * const r = f(NewNode)(node->tree);

  const long N = f(MaximumNumberOfKeys)(node->tree);                            // Split points
  const long n = N>>1;                                                          // Index of key that will be placed in parent

  const long L = l->length = n;
  const long R = r->length = N - n - 1;

  memcpy(l->keys, node->keys,        L  * sizeof(long));                        // Split left keys and data
  memcpy(l->data, node->data,        L  * sizeof(long));
  memcpy(l->down, node->down,     (1+L) * sizeof(f(Node) *));

  memcpy(r->keys, node->keys+n+1,    R  * sizeof(long));                        // Split right keys and data
  memcpy(r->data, node->data+n+1,    R  * sizeof(long));
  memcpy(r->down, node->down+n+1, (1+R) * sizeof(f(Node) *));

  if (!f(IsLeaf)(node))                                                         // Not a leaf node
   {f(ReUp)(l);
    f(ReUp)(r);
   }

  if (node->up)                                                                 // Not a root node
   {const long L = sizeof(long);
    f(Node) * const p = node->up;                                               // Existing parent node
    l->up = r->up = p;                                                          // Connect children to parent
    if (p->down[0] == node)                                                     // Splitting the first child - move everything up
     {memmove(p->keys+1, p->keys,  p->length    * L);
      memmove(p->data+1, p->data,  p->length    * L);
      memmove(p->down+1, p->down, (++p->length) * L);
      p->keys[0] = node->keys[n];
      p->data[0] = node->data[n];
      p->down[0] = l;
      p->down[1] = r;
      f(FreeNode)(node);
     }
    else if (p->down[p->length] == node)                                        // Splitting the last child - just add it on the end
     {p->keys[  p->length] = node->keys[n];
      p->data[  p->length] = node->data[n];
      p->down[  p->length] = l;
      p->down[++p->length] = r;
      f(FreeNode)(node);
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
          f(FreeNode)(node);
          return 1;
         }
       }
      assert(0);
     }
   }
  else                                                                          // Root node with single key after split
   {l->up = r->up = node;                                                       // Connect children to parent

    node->keys[0] = node->keys[n];                                              // Single key
    node->data[0] = node->data[n];                                              // Data associated with single key
    node->down[0] = l;
    node->down[1] = r;
    node->length  = 1;
   }
  return 1;
 }

static f(FindResult) f(FindAndSplit)                                            //P Find a key in a tree splitting full nodes along the path to the key.
 (f(Tree) * const tree, NWayTreeDataType const key)
 {f(Node) *node = tree->node;
  if (!node) return f(NewFindResult)(node, key,
    f(FindComparison_notFound), -1);

  if (f(SplitFullNode)(node))                                                   // Split the root node if necessary
   {node = tree->node;
   }

  for(long j = 0; j < f(MaxIterations); ++j)                                    // Step down through the tree
   {if (key < (node->keys[0]))                                                  // Less than smallest key in node
     {if (f(IsLeaf)(node)) return f(NewFindResult)
       (node, key, f(FindComparison_lower), 0);                                 // Smallest key in tree
      f(Node) * const n = node->down[0];
      if (!f(SplitFullNode)(n)) node = n;                                       // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
      continue;
     }

    const long last = node->length-1;                                           // Greater than largest key in node
    if (key > (node->keys[last]))                                               // Greater than largest key in node
     {if (f(IsLeaf)(node)) return f(NewFindResult)
       (node, key, f(FindComparison_higher), last);

      f(Node) * const n = node->down[last+1];
      if (!f(SplitFullNode)(n)) node = n;                                       // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
      continue;
     }

    for(long i = 1; i < node->length; ++i)                                      // Search the keys in this node as greater than least key and less than largest key
     {if (key == (node->keys[i]))                                               // Found key
       {return f(NewFindResult)
         (node, key, f(FindComparison_equal), i);
       }
      if (key < (node->keys[i]))                                                // Greater than current key
       {if (f(IsLeaf)(node)) return f(NewFindResult)                            // Leaf
         (node, key, f(FindComparison_lower), i);
        f(Node) * const n = node->down[i];
        if (!f(SplitFullNode)(node)) node = n; else node = n->up;               // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
        break;
       }
     }
   }
  assert(0);
 }
/*
static NWayTreeLongFindResult NWayTreeLongFind22                                // Find a key in a tree returning its associated data or undef if the key does not exist.
 (NWayTreeLongTree *tree, long key)
 {NWayTreeLongNode *node = tree->node;
  if (!node) return NWayTreeLongNewFindResult(tree, node, key,
    NWayTreeLongFindComparison_notFound, -1);                                   // Empty tree

  __m512i Key;
  for(long i = 0; i < NWayTreeLongNumberOfKeysPerNode; ++i) Key[i] = key;

  for(long j = 0; j < NWayTreeLongMaxIterations; ++j)                           // Same code as above
   {__m512i keys;
    memcpy(&keys, node->keys, sizeof(long) * NWayTreeLongNumberOfKeysPerNode);

    __mmask8 eq = _mm512_cmpeq_epi64_mask(Key, keys);
    eq &= (1<<node->length)-1;                                                  // Equality point
    if (eq)                                                                     // Equal to a key
     {int eqP =  __builtin_ctz(eq);                                             // Equality position
      return NWayTreeLongNewFindResult(tree, node, key, NWayTreeLongFindComparison_equal, eqP);
     }

    __mmask8 gt = _mm512_cmpgt_epi64_mask(Key, keys);                           // Find all keys greater
    gt &= (1<<node->length)-1;                                                  // Active keys we are greater than
    int gtP =  __builtin_ctz(gt+1);
    if (NWayTreeLongIsLeaf(node))                                               // Leaf
     {if (gtP > 0)                                                              // Greater than a key
       {return NWayTreeLongNewFindResult(tree, node, key, NWayTreeLongFindComparison_higher, gtP-1);
       }
      else
       {return NWayTreeLongNewFindResult(tree, node, key, NWayTreeLongFindComparison_lower, gtP);
       }
     }
    node = node->down[gtP];                                                     // Continue down
   }
  assert(0);
 }
*/
static f(FindResult) f(Find)                                                    // Find a key in a tree returning its associated data or undef if the key does not exist.
 (f(Tree) * const tree, NWayTreeDataType const key)                             // Tree to search, key to search
 {f(Node) *node = tree->node;                                                   // Current node we are searching
  if (!node) return f(NewFindResult)(node, key,
    f(FindComparison_notFound), -1);                                            // Empty tree

  for(long j = 0; j < NWayTreeLongMaxIterations; ++j)                           // Same code as above
   {if (key > (node->keys[node->length-1]))                                     // Bigger than every  key
     {if (f(IsLeaf)(node)) return f(NewFindResult)                              // Greater than all the keys in the node
       (node, key, f(FindComparison_higher), node->length-1);
      node = node->down[node->length];
     }
    else
     {for(long i = 0; i < node->length; ++i)                                    // Search the keys in this node as less than largest key
       {if (key == (node->keys[i]))                                             // Found key
         {return f(NewFindResult)
           (node, key, f(FindComparison_equal), i);
         }
        else if (key < (node->keys[i]))                                         // Lower than current key
         {if (f(IsLeaf)(node)) return f(NewFindResult)                          // Leaf
           (node, key, f(FindComparison_lower), i);
          node = node->down[i];
          break;
         }
       }
     }
   }
  assert(0);
 }

static long f(IndexInParent)                                                    //P Get the index of a node in its parent.
 (f(Node) * const node)                                                         // Node to locate in parent
 {f(Node) * const p = node->up;
  assert(p);
  for(long i = 0; i < node->length; ++i)
   {if (p->down[i] == node) return i;
   }
  assert(0);
 }

static void f(FillFromLeftOrRight)                                              //P Fill a node from the specified sibling.
 (f(Node) * const n, long const dir)                                            // Node to fill, direction to fill from
 {f(Node) * const p = n->up;                                                    // Parent of leaf
  assert(p);
  const long i = f(IndexInParent)(n);                                           // Index of leaf in parent

  if (dir)                                                                      // Fill from right
   {assert(i < p->length);                                                      // Cannot fill from right
    f(Node) * const r = p->down[i+1];                                           // Right sibling
    n->keys[n->length] = p->keys[i];                                            // Transfer key and data to parent
    n->data[n->length] = p->data[i];

    (p->keys[i]) = ArrayLongShift((r->keys), r->length);                        // Transfer keys and data from right
    (p->data[i]) = ArrayLongShift((r->data), r->length);

    if (!f(IsLeaf)(n))                                                          // Transfer node if not a leaf
     {ArrayVoidPush((void *)n->down, n->length,
       (void *)ArrayVoidShift((void *)r->down, r->length));
      n->down[n->length+1]->up = n;
     }
    r->length--; n->length++;
   }
  else                                                                          // Fill from left
   {assert(i);                                                                  // Cannot fill from left
    long I = i-1;
    f(Node) * const n = p->down[I];                                             // Left sibling
    ArrayLongUnShift((n->keys), n->length, (p->keys[I]));                       // Shift in keys and data from left
    ArrayLongUnShift((n->data), n->length, (p->data[I]));

    (p->keys[I]) = ArrayLongPop((n->keys), n->length);                          // Transfer key and data to parent
    (p->data[I]) = ArrayLongPop((n->data), n->length);

    if (!f(IsLeaf)(n))                                                          // Transfer node if not a leaf
     {ArrayVoidUnShift((void *)n->down, n->length,
       (void *)ArrayVoidPop((void *)n->down, n->length));
      n->down[0]->up = n;
     }
   }
 }

static void f(MergeWithLeftOrRight)                                             //P Merge two adjacent nodes.
 (f(Node) * const n, long const dir)                                            // Node to fill, direction to fill from
 {assert(f(HalfFull)(n));                                                       // Confirm leaf is half full
  f(Node) * const p = n->up;                                                    // Parent of leaf
  assert(p);
  assert(f(HalfFull)(p) && p->up);                                              // Parent must have more than the minimum number of keys because we need to remove one unless it is the root of the tree

  const long i = f(IndexInParent)(n);                                           // Index of leaf in parent

  if (dir)                                                                      // Merge with right hand sibling
   {assert(i < p->length);                                                      // Cannot fill from right
    const long I = i+1;
    f(Node) * const r = p->down[I];                                             // Leaf on right
    assert(f(HalfFull)(r));                                                     // Confirm right leaf is half full

    const NWayTreeDataType k = ArrayLongDelete((p->keys), p->length, I);        // Transfer keys and data from parent
    const NWayTreeDataType d = ArrayLongDelete((p->data), p->length, I);
    ArrayLongPush((n->keys), n->length, k);
    ArrayLongPush((n->data), n->length, d);

    ArrayLongPushArray((n->keys), n->length+1, (r->keys), r->length);           // Transfer keys
    ArrayLongPushArray((n->data), n->length+1, (r->data), r->length);           // Transfer data

    if (!f(IsLeaf)(n))                                                          // Children of merged node
     {ArrayVoidPushArray((void *)n->down, n->length,(void *)r->down, r->length);
      f(ReUp)(n);                                                               // Update parent of children of right node
     }
    ArrayVoidDelete((void *)p->down, p->length, I);                             // Remove link from parent to right child
    n->length += 1 + r->length; --p->length;
    f(FreeNode)(r);
   }
  else                                                                          // Merge with left hand sibling
   {assert(i > 0);                                                              // Cannot fill from left
    const long I = i-1;
    f(Node) * const l = p->down[I];                                             // Node on left
    assert(f(HalfFull)(l));                                                     // Confirm left leaf is half full
    const NWayTreeDataType k = ArrayLongDelete((p->keys), p->length, I);        // Transfer parent key and data
    const NWayTreeDataType d = ArrayLongDelete((p->data), p->length, I);
    ArrayLongUnShift     ((n->keys), n->length,   k);
    ArrayLongUnShift     ((n->data), n->length,   d);
    ArrayLongUnShiftArray((n->keys), n->length+1, (l->keys), l->length);        // Transfer left keys and data
    ArrayLongUnShiftArray((n->data), n->length+1, (l->data), l->length);

    if (!f(IsLeaf)(n))                                                          // Children of merged node
     {ArrayLongUnShiftArray((void *)n->down, n->length,
                            (void *)l->down, l->length);
      f(ReUp)(n);                                                               // Update parent of children of left node
     }
    ArrayVoidDelete((void *)p->down, p->length, I);                             // Remove link from parent to right child
    n->length += 1 + l->length; --p->length;
    f(FreeNode)(l);
   }
 }

static void f(Merge)                                                            //P Merge the current node with its sibling.
 (f(Node) * const node)                                                         // Node to merge into
 {const long i = f(IndexInParent)(node);                                        // Index in parent
  if (i)                                                                        // Merge with left node
   {f(Node) * const l = node->up->down[i-1];                                    // Left node
    f(Node) * const r = node;
    if (f(HalfFull)(r))
     {f(HalfFull)(l) ? f(MergeWithLeftOrRight)(r, 0):
                                f(FillFromLeftOrRight) (r, 0);                  // Merge as left and right nodes are half full
     }
   }
  else
   {f(Node) * const r = node->up->down[1];                                      // Right node
    f(Node) * const l = node;
    if (f(HalfFull)(l))
     {f(HalfFull)(r) ? f(MergeWithLeftOrRight)(l, 1):
                                f(FillFromLeftOrRight) (l, 1);                  // Merge as left and right nodes are half full
     }
   }
 }

static void f(MergeOrFill)                                                      //P Make a node larger than a half node.
 (f(Node) * const node)                                                         // Node to merge or fill
 {if (f(HalfFull)(node)) return;                                                // No need to merge of if not a half node
  f(Node) * const p = node->up;                                                 // Parent exists

  if (p->up)                                                                    // Merge or fill parent which is not the root
   {f(MergeOrFill)(p);
    f(Merge)(node);
   }
  else
   {f(Node) * const l = p->down[0];
    f(Node) * const r = p->down[1];
    if (p->length == 1 && f(HalfFull)(l) && f(HalfFull)(r))                     // Parent is the root and it only has one key - merge into the child if possible
     {const long L = l->length, R = r->length, N = node->length;
      ArrayLongPushArray((node->keys), 0, (l->keys), L);
      ArrayLongPushArray((node->data), 0, (l->data), L);

      ArrayLongPushArray((node->keys), L, (p->keys), 1);
      ArrayLongPushArray((node->data), L, (p->data), 1);

      ArrayLongPushArray((node->keys), L+1, (r->keys), R);
      ArrayLongPushArray((node->data), L+1, (r->data), R);

      ArrayVoidPushArray((void *)node->down, 0,   (void *)l->down, L+1);
      ArrayVoidPushArray((void *)node->down, L+1, (void *)r->down, R);
      node->length = L+R+1;

      ArrayLongPushArray((p->keys), 0, (node->keys), N);
      ArrayLongPushArray((p->data), 0, (node->data), N);
      ArrayVoidPushArray((void *)p->down, 0, (void *)node->down, N+1);

      f(ReUp)(p);                                                               // Reconnect children to parent
     }
    else                                                                        // Parent is the root but it has too may keys to merge into both sibling so merge with a sibling if possible
     {f(Merge)(node);
     }
   }
 }

static void f(Insert)                                                           // Insert a key and its associated data into a tree
 (f(Tree) * const tree,                                                         // Tree to insert into
  NWayTreeDataType const key, NWayTreeDataType const data)                      // Key to insert, data associated with key
 {if (!tree->node)                                                              // Empty tree
   {f(Node) * const n = f(NewNode)(tree);
    (n->keys[0]) = key;
    (n->data[0]) = data;
     n->length   = 1; tree->size++;
    tree->node   = n;
    return;
   }

  f(Node) * const n = tree->node;
  if (n->length < f(MaximumNumberOfKeys)(tree) &&                               // Node is root with no children and room for one more key
     !n->up && f(IsLeaf)(n))
   {for(long i = 0; i < n->length; ++i)                                         // Each key
     {if (key == (n->keys[i]))                                                  // Key already present
       {(n->data[i]) = data;
        return;
       }
      if (key < (n->keys[i]))                                                   // We have reached the insertion point
       {ArrayLongInsert((n->keys), n->length+1, key,  i);
        ArrayLongInsert((n->data), n->length+1, data, i);
        n->length++; tree->size++;
        return;
       }
     }
    ArrayLongPush((n->keys), n->length, key);                                   // Insert the key at the end of the block because it is greater than all the other keys in the block
    ArrayLongPush((n->data), n->length, data);
    n->length++; tree->size++;
   }
  else                                                                          // Insert node
   {f(FindResult)   r = f(FindAndSplit)(tree, key);                             // Check for existing key
    f(Node) * const n = r.node;
    if (r.cmp == f(FindComparison_equal))                                       // Found an equal key whose data we can update
     {(n->data[r.index]) = data;
     }
    else                                                                        // We have room for the insert
     {long index = r.index;
      if (r.cmp == f(FindComparison_higher)) ++index;                           // Position at which to insert new key
      ArrayLongInsert((n->keys), n->length+1, key,  index);
      ArrayLongInsert((n->data), n->length+1, data, index);

      ++n->length;
      f(SplitFullNode)(n);                                                      // Split if the leaf is full to force keys up the tree
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
  for (0..NWayTreeLongMaxIterations)
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
    $iter->pos  = $pos                                         //= 0;
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
    $iter->pos  = $pos                                         //= ($node->keys->@* - 1);
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
    $s[$d]                                         //= '';
    $s[$d]  .= "   ".$i->key;                                                   // Add key at appropriate depth
    long l = length $s[$d];
    for long j(0..$D)                                                           // Pad all strings to the current position
     {long s = $s[$j]                                         //= '';
      $s[$j] = substr($s.(' 'x999), 0, $l) if length($s) < $l;
     }
   }
  for long i(keys @s)                                                           // Clean up trailing blanks so that tests are not affected by spurious white space mismatches
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
void test_3_1()                                                                 // Tests
 {f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < 1; ++i) f(Insert)(t, i, 2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   0                                   2\n"
));
 }

void test_31_1()                                                                // Tests
 {f(Tree) * const t = f(NewTree)(31);
  for(int i = 0; i < 1; ++i) f(Insert)(t, i, 2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   0                                   2\n"
));
 }

void test_3_2()                                                                 // Tests
 {f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < 2; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
));
 }

void test_31_2()                                                                // Tests
 {f(Tree) * const t = f(NewTree)(31);
  for(int i = 0; i < 2; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
));
 }

void test_3_3()                                                                 // Tests
 {f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < 3; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_31_3()                                                                // Tests
 {f(Tree) * const t = f(NewTree)(31);
  for(int i = 0; i < 3; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
"   2                                   4\n"
));
 }

f(Node) *createNode3(f(Tree) * t, long a, long b, long c)                       // Create a test node
 {f(Node) *n = f(NewNode)(t);
  (n->keys[0]) = a; (n->data[0]) = 2*a;
  (n->keys[1]) = b; (n->data[1]) = 2*b;
  (n->keys[2]) = c; (n->data[2]) = 2*c;
  n->length  = 3;
  return n;
 }

void test_3_4a()                                                                // Tree has one node
 {f(Tree) *t = f(NewTree)(3);
  f(Node) *n = createNode3(t, 1, 2, 3);
  t->size = n->length = 3;
  t->node = n;

  long r = f(SplitFullNode)(n);
  assert(r);
                                          //f(ErrAsC)(t);
  assert(f(EqText)(t,
"      1                                   2\n"
"   2                                   4\n"
"      3                                   6\n"
));
 }

void test_3_4b()                                                                // First down
 {f(Tree) *t  = f(NewTree)(3);
  f(Node) *p  = createNode3(t, 10, 20, 30); p->length = 2;
  f(Node) *n0 = createNode3(t, 01, 02, 03);
  f(Node) *n1 = createNode3(t, 11, 12, 13);
  f(Node) *n2 = createNode3(t, 21, 22, 23);
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;
                                          //f(ErrAsC)(t);

  assert(f(EqText)(t,
"      1                                   2\n"
"      2                                   4\n"
"      3                                   6\n"
"  10                                  20\n"
"     11                                  22\n"
"     12                                  24\n"
"     13                                  26\n"
"  20                                  40\n"
"     21                                  42\n"
"     22                                  44\n"
"     23                                  46\n"
));

  long r = f(SplitFullNode)(n0); if (r){}
                                          //f(ErrAsC)(t);
  assert(f(EqText)(t,
"      1                                   2\n"
"   2                                   4\n"
"      3                                   6\n"
"  10                                  20\n"
"     11                                  22\n"
"     12                                  24\n"
"     13                                  26\n"
"  20                                  40\n"
"     21                                  42\n"
"     22                                  44\n"
"     23                                  46\n"
));
//f(ErrAsC)(t);
 }

void test_3_4c()                                                                // Mid down
 {f(Tree) *t  = f(NewTree)(3);
  f(Node) *p  = createNode3(t, 10, 20, 30); p->length = 2;
  f(Node) *n0 = createNode3(t, 01, 02, 03);
  f(Node) *n1 = createNode3(t, 11, 12, 13);
  f(Node) *n2 = createNode3(t, 21, 22, 23);
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;

  assert(p->down[1] == n1);

  long r = f(SplitFullNode)(n1); if (r){}
                                          //f(ErrAsC)(t);
  assert(f(EqText)(t,
"      1                                   2\n"
"      2                                   4\n"
"      3                                   6\n"
"  10                                  20\n"
"     11                                  22\n"
"  12                                  24\n"
"     13                                  26\n"
"  20                                  40\n"
"     21                                  42\n"
"     22                                  44\n"
"     23                                  46\n"
));
 }

void test_3_4d()                                                                // Final node
 {f(Tree) *t  = f(NewTree)(3);
  f(Node) *p  = createNode3(t, 10, 20, 30); p->length = 2;
  f(Node) *n0 = createNode3(t, 01, 02, 03);
  f(Node) *n1 = createNode3(t, 11, 12, 13);
  f(Node) *n2 = createNode3(t, 21, 22, 23);
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;

  assert(p->down[1] == n1);

  long r = f(SplitFullNode)(n2); if (r){}
                                          //f(ErrAsC)(t);
  assert(f(EqText)(t,
"      1                                   2\n"
"      2                                   4\n"
"      3                                   6\n"
"  10                                  20\n"
"     11                                  22\n"
"     12                                  24\n"
"     13                                  26\n"
"  20                                  40\n"
"     21                                  42\n"
"  22                                  44\n"
"     23                                  46\n"
));
//f(ErrAsC)(t);
 }

void test_3_4()                                                                 // Tests
 {test_3_4a();
  test_3_4b();
  test_3_4c();
  test_3_4d();
 }

void test_3_insert1()                                                           // Insert tests
 {const long N = 1;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 1; i <= N; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   1                                   3\n"
));
 }

void test_3_insert2()
 {const long N = 2;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 1; i <= N; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_3_insert3()
 {const long N = 3;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 1; i <= N; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   1                                   3\n"
"   2                                   4\n"
"   3                                   5\n"
));
 }

void test_3_insert4()
 {const long N = 4;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 1; i <= N; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"      1                                   3\n"
"   2                                   4\n"
"      3                                   5\n"
"      4                                   6\n"
));
 }

void test_3_insert5()
 {const long N = 5;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 1; i <= N; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"      1                                   3\n"
"   2                                   4\n"
"      3                                   5\n"
"   4                                   6\n"
"      5                                   7\n"
));
 }

void test_3_insert6()
 {const long N = 6;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 1; i <= N; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"      1                                   3\n"
"   2                                   4\n"
"      3                                   5\n"
"   4                                   6\n"
"      5                                   7\n"
"      6                                   8\n"
));
 }

void test_3_insert7()
 {const long N = 7;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 1; i <= N; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"      1                                   3\n"
"   2                                   4\n"
"      3                                   5\n"
"   4                                   6\n"
"      5                                   7\n"
"   6                                   8\n"
"      7                                   9\n"
));
 }

void test_3_insert8()
 {const long N = 8;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 1; i <= N; ++i) f(Insert)(t, i, i+2);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"         1                                   3\n"
"      2                                   4\n"
"         3                                   5\n"
"   4                                   6\n"
"         5                                   7\n"
"      6                                   8\n"
"         7                                   9\n"
"         8                                  10\n"
));
 }

void test_3_insert2r()
 {const long N = 2;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < N; ++i) f(Insert)(t, N-i, N-i+1);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   1                                   2\n"
"   2                                   3\n"
));
 }

void test_3_insert3r()
 {const long N = 3;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < N; ++i) f(Insert)(t, N-i, N-i+1);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"   1                                   2\n"
"   2                                   3\n"
"   3                                   4\n"
));
 }

void test_3_insert4r()
 {const long N = 4;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < N; ++i) f(Insert)(t, N-i, N-i+1);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"      1                                   2\n"
"      2                                   3\n"
"   3                                   4\n"
"      4                                   5\n"
));
 }

void test_3_insert5r()
 {const long N = 5;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < N; ++i) f(Insert)(t, N-i, N-i+1);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"      1                                   2\n"
"   2                                   3\n"
"      3                                   4\n"
"   4                                   5\n"
"      5                                   6\n"
));
 }

void test_3_insert6r()
 {const long N = 6;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < N; ++i) f(Insert)(t, N-i, N-i+1);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"      1                                   2\n"
"      2                                   3\n"
"   3                                   4\n"
"      4                                   5\n"
"   5                                   6\n"
"      6                                   7\n"
));
 }

void test_3_insert7r()
 {const long N = 7;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < N; ++i) f(Insert)(t, N-i, N-i+1);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"      1                                   2\n"
"   2                                   3\n"
"      3                                   4\n"
"   4                                   5\n"
"      5                                   6\n"
"   6                                   7\n"
"      7                                   8\n"
));
 }

void test_3_insert8r()
 {const long N = 8;
  f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < N; ++i) f(Insert)(t, N-i, N-i+1);
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"         1                                   2\n"
"         2                                   3\n"
"      3                                   4\n"
"         4                                   5\n"
"   5                                   6\n"
"         6                                   7\n"
"      7                                   8\n"
"         8                                   9\n"
));
 }

void test_3_insert12()
 {const long N = 12;
  long     A[N];
  for(int i = 0; i < N; ++i) A[i] = i;
  for(int i = 0; i < N; ++i)
   {long r = i * i % N, R = A[r], I = A[i];
    A[i] = R; A[r] = I;
   }

  f(Tree) * const t = f(NewTree)(3);
  for(int i = 0; i < N; ++i) f(Insert)(t, A[i], i);
  assert(f(EqText)(t,
"         0                                   6\n"
"      1                                   5\n"
"         2                                   8\n"
"         3                                   9\n"
"      4                                   2\n"
"         5                                   7\n"
"   6                                   0\n"
"         7                                  11\n"
"      8                                  10\n"
"         9                                   3\n"
"     10                                   4\n"
"        11                                   1\n"
));
 }

void test_3_insert14()
 {const long N = 14;
  long     A[N];
  for(int i = 0; i < N; ++i) A[i] = i;
  for(int i = 0; i < N; ++i)
   {long r = i * i % N, R = A[r], I = A[i];
    A[i] = R; A[r] = I;
   }

  f(Tree) * const t = f(NewTree)(3);
  for(long i = 0; i < N; ++i)
   {f(Insert)(t, A[i], i);
   }
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"         0                                   0\n"
"         1                                  13\n"
"      2                                  10\n"
"         3                                   9\n"
"      4                                  12\n"
"         5                                  11\n"
"   6                                   8\n"
"         7                                   7\n"
"      8                                   6\n"
"         9                                   3\n"
"  10                                   2\n"
"        11                                   5\n"
"     12                                   4\n"
"        13                                   1\n"
));
 }

void test_3_insert15()
 {const long N = 15;
  long     A[N];
  for(int i = 0; i < N; ++i) A[i] = i;
  for(int i = 0; i < N; ++i)
   {long r = i * i % N, R = A[r], I = A[i];
    A[i] = R; A[r] = I;
   }

  f(Tree) * const t = f(NewTree)(3);
  for(long i = 0; i < N; ++i)
   {f(Insert)(t, A[i], i);
   }
                                          //f(ErrAsC)(tree);
  assert(f(EqText)(t,
"         0                                   0\n"
"      1                                   7\n"
"         2                                  11\n"
"         3                                   6\n"
"   4                                   2\n"
"         5                                  10\n"
"      6                                  12\n"
"         7                                   8\n"
"         8                                  13\n"
"   9                                   3\n"
"        10                                   5\n"
"     11                                  14\n"
"        12                                   9\n"
"     13                                   4\n"
"        14                                   1\n"
));
 }

void test_3_insert63()
 {const long N = 63, NN = 63;
  long     A[N];
  for(int i = 0; i < N; ++i) A[i] = i;
  for(int i = 0; i < N; ++i)
   {long r = i * i % N, R = A[r], I = A[i];
    A[i] = R; A[r] = I;
   }

  f(Tree) * const t = f(NewTree)(3);
  for(long i = 0; i < NN; ++i)
   {f(Insert)(t, A[i], i);
   }
  for(long i = 0; i < NN; ++i)
   {f(FindResult) r = f(Find)(t, A[i]);
    assert(r.data == i);
    assert(r.cmp  == f(FindComparison_equal));
   }
                                          //f(ErrAsC)(tree);
  f(CheckTree)(t, "3/63");
  assert(f(EqText)(t,
"               0                                  21\n"
"            1                                   8\n"
"               2                                  47\n"
"               3                                  12\n"
"         4                                   2\n"
"               5                                  23\n"
"            6                                  15\n"
"               7                                   7\n"
"               8                                  55\n"
"      9                                   3\n"
"              10                                  17\n"
"              11                                  40\n"
"           12                                  24\n"
"              13                                  29\n"
"              14                                  56\n"
"        15                                  27\n"
"              16                                  31\n"
"              17                                  44\n"
"           18                                  30\n"
"              19                                  26\n"
"           20                                  41\n"
"              21                                  42\n"
"  22                                  20\n"
"              23                                  38\n"
"              24                                  39\n"
"           25                                   5\n"
"              26                                  46\n"
"           27                                  48\n"
"              28                                  35\n"
"           29                                  34\n"
"              30                                  33\n"
"        31                                  32\n"
"              32                                  59\n"
"              33                                  51\n"
"           34                                  50\n"
"              35                                  28\n"
"        36                                   6\n"
"              37                                  10\n"
"              38                                  52\n"
"           39                                  45\n"
"              40                                  58\n"
"              41                                  22\n"
"     42                                   0\n"
"              43                                  13\n"
"           44                                  53\n"
"              45                                  60\n"
"        46                                  19\n"
"              47                                  61\n"
"              48                                  57\n"
"           49                                  14\n"
"              50                                  43\n"
"              51                                  54\n"
"     52                                  25\n"
"              53                                  37\n"
"           54                                  18\n"
"              55                                  62\n"
"           56                                  49\n"
"              57                                  36\n"
"        58                                  11\n"
"              59                                  16\n"
"              60                                   9\n"
"           61                                   4\n"
"              62                                   1\n"
));
 }

void test_31_insert163                                                          // Create and free a tree.
 (int test)                                                                     // Warm malloc up until it stabilizes when false.
 {const long N = 163, NN = 1;
  long     A[N];
  for(int i = 0; i < N; ++i) A[i] = i;
  for(int i = 0; i < N; ++i)                                                    // Randomize elements
   {long r = i * i % N, R = A[r], I = A[i];
    A[i] = R; A[r] = I;
   }

  long memory_at_start;                                                         // Memory in use at start
  if (test)
   {struct mallinfo m; m = mallinfo();
    memory_at_start = m.uordblks;
   }

  f(Tree) * const t = f(NewTree)(31);
  for(long i = 0; i < NN; ++i)
   {f(Insert)(t, A[i], i);
   }
  for(long i = 0; i < NN; ++i)
   {f(FindResult) r = f(Find)(t, A[i]);
    assert(r.data == i);
    assert(r.cmp  == f(FindComparison_equal));
   }
                                          //f(ErrAsC)(t);
  f(CheckTree)(t, "31/163");
  assert(1 || f(EqText)(t,
"      0                                   0\n"
"      1                                 162\n"
"      2                                  10\n"
"      3                                  82\n"
"      4                                   2\n"
"      5                                  77\n"
"      6                                 127\n"
"      7                                 114\n"
"      8                                  99\n"
"      9                                   3\n"
"     10                                  57\n"
"     11                                  26\n"
"     12                                  35\n"
"     13                                 150\n"
"     14                                  96\n"
"     15                                  60\n"
"     16                                 161\n"
"     17                                  65\n"
"     18                                 145\n"
"  19                                  47\n"
"     20                                  89\n"
"     21                                  22\n"
"     22                                 141\n"
"     23                                 123\n"
"     24                                  71\n"
"     25                                   5\n"
"     26                                 137\n"
"     27                                  86\n"
"     28                                 126\n"
"     29                                 134\n"
"     30                                  53\n"
"     31                                 135\n"
"     32                                 117\n"
"     33                                  68\n"
"     34                                 129\n"
"     35                                  19\n"
"     36                                  13\n"
"     37                                  98\n"
"     38                                 125\n"
"     39                                 109\n"
"  40                                  23\n"
"     41                                 112\n"
"     42                                 152\n"
"     43                                 107\n"
"     44                                 119\n"
"     45                                  94\n"
"     46                                  32\n"
"     47                                  55\n"
"     48                                 142\n"
"     49                                   7\n"
"     50                                  46\n"
"     51                                 154\n"
"     52                                  88\n"
"     53                                 110\n"
"     54                                 120\n"
"     55                                  50\n"
"     56                                  80\n"
"     57                                 106\n"
"     58                                  58\n"
"     59                                 105\n"
"     60                                 103\n"
"  61                                 102\n"
"     62                                 118\n"
"     63                                 153\n"
"     64                                   8\n"
"     65                                  37\n"
"     66                                  97\n"
"     67                                  83\n"
"     68                                 101\n"
"     69                                  45\n"
"     70                                 147\n"
"     71                                  92\n"
"     72                                 108\n"
"     73                                 116\n"
"     74                                  20\n"
"     75                                  43\n"
"     76                                 139\n"
"     77                                  27\n"
"     78                                 133\n"
"     79                                 128\n"
"     80                                 130\n"
"     81                                 160\n"
"     82                                  41\n"
"     83                                  75\n"
"     84                                 151\n"
"     85                                  30\n"
"     86                                  61\n"
"  87                                  29\n"
"     88                                  67\n"
"     89                                  69\n"
"     90                                  79\n"
"     91                                 113\n"
"     92                                 144\n"
"     93                                 159\n"
"     94                                  34\n"
"     95                                 148\n"
"     96                                  52\n"
"     97                                 143\n"
"     98                                   6\n"
"     99                                  21\n"
"    100                                  70\n"
"    101                                  95\n"
"    102                                 132\n"
"    103                                  14\n"
"    104                                  59\n"
"    105                                 104\n"
" 106                                 121\n"
"    107                                  39\n"
"    108                                  91\n"
"    109                                   4\n"
"    110                                  38\n"
"    111                                 149\n"
"    112                                  49\n"
"    113                                  73\n"
"    114                                  74\n"
"    115                                 155\n"
"    116                                  90\n"
"    117                                   9\n"
"    118                                  66\n"
"    119                                 156\n"
"    120                                  56\n"
"    121                                  11\n"
"    122                                  51\n"
"    123                                  85\n"
"    124                                  54\n"
" 125                                  40\n"
"    126                                  17\n"
"    127                                  64\n"
"    128                                  84\n"
"    129                                  15\n"
"    130                                 111\n"
"    131                                  72\n"
"    132                                  28\n"
"    133                                 140\n"
"    134                                  42\n"
"    135                                 138\n"
"    136                                 158\n"
"    137                                  24\n"
"    138                                 136\n"
"    139                                  87\n"
"    140                                  78\n"
"    141                                  25\n"
"    142                                 115\n"
"    143                                  44\n"
"    144                                  12\n"
" 145                                 124\n"
"    146                                  31\n"
"    147                                  93\n"
"    148                                  62\n"
"    149                                  33\n"
"    150                                 146\n"
"    151                                  76\n"
"    152                                  63\n"
"    153                                 100\n"
"    154                                  81\n"
"    155                                 157\n"
"    156                                 122\n"
"    157                                  36\n"
"    158                                  48\n"
"    159                                  16\n"
"    160                                 131\n"
"    161                                  18\n"
"    162                                   1\n"
));

  f(Free)(t);
  if (test)                                                                     // Memory in use at end
   {struct mallinfo m = mallinfo();
    assert(memory_at_start == m.uordblks);                                      // Confirm that there is no leakage
   }
 }

void test_31x_insert163()
 {for(long i = 0; i < 20; ++i) test_31_insert163(0);
  test_31_insert163(1);
 }

void test_3_Find()
 {const long N = 63;

  f(Tree) * const t = f(NewTree)(3);
  for(long i = 0; i < N;     ++i) f(Insert)(t, i*2, i*2);
                                          //f(ErrAsC)(tree);
  for(long i =-1; i < 2 * N; ++i)
   {//i = 13;
    f(FindResult) r = f(Find)(t, i);
                                            //f(ErrFindResult)(r);
    assert(i % 2 == 0 ? r.cmp == f(FindComparison_equal) :
                        r.cmp != f(FindComparison_equal));

    if (i == -1) assert((r.node->keys[r.index]) == 0 && r.cmp == f(FindComparison_lower)  && r.index == 0);
    if (i ==  0) assert((r.node->keys[r.index]) == 0 && r.cmp == f(FindComparison_equal)  && r.index == 0);
    if (i ==  1) assert((r.node->keys[r.index]) == 0 && r.cmp == f(FindComparison_higher) && r.index == 0);

    if (i == 11) assert((r.node->keys[r.index]) == 12 && r.cmp == f(FindComparison_lower)  && r.index == 0);
    if (i == 12) assert((r.node->keys[r.index]) == 12 && r.cmp == f(FindComparison_equal)  && r.index == 0);
    if (i == 13) assert((r.node->keys[r.index]) == 12 && r.cmp == f(FindComparison_higher) && r.index == 0);
   }
 }

void test_3_insert()                                                            // Tests
 {test_3_insert1();
  test_3_insert2();
  test_3_insert3();
  test_3_insert4();
  test_3_insert5();
  test_3_insert6();
  test_3_insert7();
  test_3_insert8();
  test_3_insert2r();
  test_3_insert3r();
  test_3_insert4r();
  test_3_insert5r();
  test_3_insert6r();
  test_3_insert7r();
  test_3_insert8r();
  test_3_insert12();
  test_3_insert14();
  test_3_insert15();
  test_3_insert63();
 }

void tests3()                                                                   // Tests
 {test_3_1();
  test_3_2();
  test_3_3();
  test_3_4();
  test_3_Find();
  test_3_insert();
 }

void tests31()                                                                  // Tests
 {test_31_1();
  test_31_2();
  test_31_insert163(0);
  test_31x_insert163();
 }

void f(TraceBackHandler)(int sig)
 {void *array[99];
  size_t size = backtrace(array, 99);

  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array+6, size-6, STDERR_FILENO);
  signal(SIGABRT, 0);
  exit(1);
}

int main()                                                                      // Run tests
 {signal(SIGSEGV, f(TraceBackHandler));                                         // Trace back handler
  signal(SIGABRT, f(TraceBackHandler));
  tests3 ();
  tests31();
  return 0;
 }
#endif
#undef f
#endif
