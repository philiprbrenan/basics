//------------------------------------------------------------------------------
// Generic N way tree
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#include "array/void.c"
#include "basics/basics.c"
#include "stack/char.c"

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#define NWayTree_MaxIterations 99                                               /* The maximum number of levels in a tree */

#ifndef NWayTree_Included
#define NWayTree_Included
#define NWayTreeIterate(tree,     find) \
  for(NWayTree(FindResult)        find = NWayTree(IterStart)(tree); \
      NWayTree(IterCheck) (find); find = NWayTree(IterNext) (find))

#define NWayTree_constLong(l, code)            const long l = (long)code;

#define NWayTree_new(tree, n)                  NWayTree(Tree) * const tree = NWayTree(New)(n);
#define NWayTree_constNode(node, code)         NWayTree(Node) *       node = code;
#define NWayTree_maximumNumberOfKeys(n, tree)  const long n = tree->NumberOfKeysPerNode;
//#define NWayTree_minimumNumberOfKeys(n, tree)  const long n = (tree->NumberOfKeysPerNode - 1) >> 1;

#define NWayTree_root(n, tree)                 NWayTree(Node) *       n = tree->root;
#define NWayTree_setRoot(tree, n)              tree->root = n;
#define NWayTree_incKeys(tree)               ++tree->keys;
#define NWayTree_incNodes(tree)              ++tree->nodes;

#define NWayTree_Node_length(l, node)          const long l = node->length;
#define NWayTree_Node_setLength(node, n)       node->length = n;
#define NWayTree_Node_id(i, node)              const long i = node->id;
#define NWayTree_Node_up(u, node)              NWayTree(Node) * const u = node->up;
#define NWayTree_Node_setUp(node, n)           node->up = n;
#define NWayTree_Node_tree(t, node)            NWayTree(Tree) * const t = node->tree;

#define NWayTree_Node_keys(k, node, index)     const NWayTreeDataType k = node->keys[index];
#define NWayTree_Node_data(d, node, index)     const NWayTreeDataType d = node->data[index];
#define NWayTree_Node_down(n, node, index)     NWayTree(Node) * const n = node->down[index];
#define NWayTree_Node_isLeaf(l, node)          const long l = node->down[0] == 0;

#define NWayTree_Node_setKeys(node, index, k)  node->keys[index] = k;
#define NWayTree_Node_setData(node, index, d)  node->data[index] = d;
#define NWayTree_Node_setDown(node, index, n)  node->down[index] = n;

#define NWayTree_FindResult(f, code)           const NWayTree(FindResult) f = code;
#define NWayTree_FindResult_key(k, f)          const NWayTreeDataType k = f.key;
#define NWayTree_FindResult_data(d, f)         const NWayTreeDataType d = NWayTree(FindResult_data)(f);
#define NWayTree_FindResult_cmp(c, f)          const long c = f.cmp;
#define NWayTree_FindResult_index(i, f)        const long i = f.index;
#define NWayTree_FindResult_node(n, f)         NWayTree(Node) * const n = f.node;
#define NWayTree_FindComparison(f, value)      const NWayTree(FindComparison) f = NWayTree(FindComparison_##value)
#endif

//Optimize
//#define static                                                                /* Simplify debugging by preventing some inline-ing which invalidates the call stack */
const long size_of_element = sizeof(NWayTreeDataType);                          // The size of a key, data field

//D1 Data structures
//D2 Node

typedef struct NWayTree(Node)                                                   // A node in a tree
 {long length;                                                                  // The current number of keys in the node
  long id;                                                                      // A number identifying this node within this tree
  struct NWayTree(Node) *up;                                                    // Parent node unless at the root node
  NWayTreeDataType *keys;                                                       // Keys associated with this node
  NWayTreeDataType *data;                                                       // Data associated with each key associated with this node
  struct NWayTree(Node) **down;                                                 // Next layer of nodes down from this node
  struct NWayTree(Tree) *tree;                                                  // The definition of the containing tree
 } NWayTree(Node);

inline static void NWayTree(Node_open)                                          // Open a gap in a node
 (NWayTree(Node) * const node,                                                  // Node
  const long offset,                                                            // Offset in node at which to open the gap
  const long length)                                                            // Number of items to move to open the gap
 {const long l = offset+length, L = l + 1;
  NWayTree_Node_down(n, node, l);
  NWayTree_Node_setDown(node, L, n);
  for(long i = length; i > 0; --i)
   {const long p = offset + i, q = p - 1;
    NWayTree_Node_keys(k, node, q);
    NWayTree_Node_data(d, node, q);
    NWayTree_Node_down(n, node, q);
    NWayTree_Node_setKeys(node, p, k);
    NWayTree_Node_setData(node, p, d);
    NWayTree_Node_setDown(node, p, n);
   }
 }

inline static void NWayTree(Node_copy)                                          // Copy part of one node into another going down.
 (NWayTree(Node) * const t,                                                     // Target node
  NWayTree(Node) * const s,                                                     // Source node
  const long to,                                                                // Target offset
  const long so,                                                                // Source offset
  const long length)                                                            // Number of items to copy
 {for(long i = 0; i < length; ++i)
   {const long S = so+i, T = to+i;
    NWayTree_Node_keys(k, s, S);
    NWayTree_Node_data(d, s, S);
    NWayTree_Node_down(n, s, S);
    NWayTree_Node_setKeys(t, T, k);
    NWayTree_Node_setData(t, T, d);
    NWayTree_Node_setDown(t, T, n);
   }
  NWayTree_Node_down(n,    s, so+length);
  NWayTree_Node_setDown(t, to+length, n);
 }

inline static void NWayTree(Node_free)                                          // Free a node, Wipe the node so it cannot be accidently reused
 (NWayTree(Node) * const node)                                                  // Node to free
 {free(node);
 }

//D2 Tree

typedef struct NWayTree(Tree)                                                   // The root of a tree
 {long keys;                                                                    // Number of keys in tree
  long nodes;                                                                   // Number of nodes in tree
  long NumberOfKeysPerNode;                                                     // Size of a node
  NWayTree(Node) *root;                                                         // Root node
 } NWayTree(Tree);

static NWayTree(Tree) *NWayTree(New)                                            // Create a new tree
 (const long n)                                                                 // Number of keys in a node
 {NWayTree(Tree) * const tree = calloc(sizeof(NWayTree(Tree)), 1);
  tree->NumberOfKeysPerNode = n;
  return tree;
 }

inline static void NWayTree(Free2)                                              // Free a node in a tree
 (NWayTree(Node) * const node)
 {if (!node) return;
  NWayTree_Node_length(nl, node);
  if (nl)
   {for(long i = 0; i <= nl; ++i)                                               // Free each sub node
     {NWayTree_Node_down(n, node, i);
      NWayTree(Free2)(n);
     }
   }
  NWayTree(Node_free)(node);                                                    // Free node now sub nodes have been freed
 }

inline static void NWayTree(Free)                                               // Free a tree
 (NWayTree(Tree) * const tree)
 {if (!tree) return;
  NWayTree_root(n, tree);
  NWayTree(Free2)(n);
  free(tree);
 }

//D2 Find Result

typedef enum NWayTree(FindComparison)                                           // The results of a comparison
 {NWayTree(FindComparison_lower),
  NWayTree(FindComparison_equal),
  NWayTree(FindComparison_higher),
  NWayTree(FindComparison_notFound)
 } NWayTree(FindComparison);

typedef struct NWayTree(FindResult)                                             // The results of a find operation
 {NWayTree(Node) *node;                                                         // Node found
  NWayTree(FindComparison) cmp;                                                 // Result of the last comparison
  NWayTreeDataType key;                                                         // Key searched for
  long index;                                                                   // Index in the node of equal element
 } NWayTree(FindResult);

inline static NWayTree(FindResult) NWayTree(FindResult_new)                     // New find result on stack
 (NWayTree(Node) *         const node,                                          // Node
  NWayTreeDataType         const key,                                           // Search key
  NWayTree(FindComparison) const cmp,                                           // Comparison result
  long                     const index)                                         // Index in node of nearest key
 {NWayTree(FindResult) f;
  f.node  = node;
  f.key   = key;
  f.cmp   = cmp;
  f.index = index;
  return f;
 }

inline static NWayTreeDataType NWayTree(FindResult_data)                        // Get data field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {NWayTree_FindResult_node (n, f);
  NWayTree_FindResult_index(i, f);
  NWayTree_Node_data(d, n, i);
  return d;
 }

//D0 Forward declarations

static void NWayTree(ErrNode)       (NWayTree(Node) * const node);
static long NWayTree(CheckNode)     (NWayTree(Node) * const node, char * const name);
static void NWayTree(CheckTree)     (NWayTree(Tree) * const tree, char * const name);
static long NWayTree(Node_indexInParent) (NWayTree(Node) * const node);

//D1 Node Construction

inline static NWayTree(Node) *NWayTree(Node_new)                                // Create a new node
 (NWayTree(Tree) * const tree)                                                  // Tree containing node
 {NWayTree_maximumNumberOfKeys(z, tree);
  NWayTree_constLong(k, size_of_element                 *  z);
  NWayTree_constLong(d, size_of_element                 *  z);
  NWayTree_constLong(n, sizeof(struct NWayTree(Node) *) * (z + 1));
  NWayTree_constLong(s, sizeof(NWayTree(Node)));

  NWayTree_constNode(node, calloc(s+k+d+n, 1));
  node->keys = (void *)node+s;
  node->data = (void *)node+s+k;
  node->down = (void *)node+s+k+d;
  node->tree = tree;
  node->id   = NWayTree_incNodes(tree);
  return node;
 }

inline static long NWayTree(SizeOfNode)                                         // The size of a node in a tree
 (NWayTree(Tree) *tree)                                                         // Tree
 {NWayTree_maximumNumberOfKeys(z, tree);
  NWayTree_constLong(k, size_of_element                 *  z);
  NWayTree_constLong(d, size_of_element                 *  z);
  NWayTree_constLong(n, sizeof(struct NWayTree(Node) *) * (z + 1));
  NWayTree_constLong(s, sizeof(NWayTree(Node)));
  return s+k+d+n;
 }

inline static void NWayTree(ReUp)                                               // Reconnect the children to their new parent.
 (NWayTree(Node) * const node)                                                  // Node to reconnect
 {NWayTree_Node_length(nl, node);
  for(long i = 0; i <= nl; ++i)
   {NWayTree_Node_down(d, node, i);                                             // No children so it must be a leaf
    NWayTree_Node_setUp(d, node);
   }
 }

//D1 Print

inline static void NWayTree(ToString2)                                          // Print the keys in a tree
 (NWayTree(Node) * const node,                                                  // Node to print
  long             const in,                                                    // Indentation
  StackChar      * const p)                                                     // Stack as a string
 {if (!node) return;
  NWayTree_Node_length(nl, node);
  if (!nl) return;
  NWayTree_Node_down(n, node, 0);
  NWayTree(ToString2)(n, in+1, p);
  for(long i = 0; i < nl; ++i)
   {for(long j = 0; j < in * 3; ++j) StackCharPush(p, ' ');
    char C[100];
    NWayTree_Node_keys(k, node, i);
    NWayTree_Node_data(d, node, i);
    sprintf(C, "%4ld                                %4ld\n", k, d);
    StackCharPushString(p, C);
    NWayTree_Node_down(n, node, i+1);
    NWayTree(ToString2)(n, in+1, p);
   }
 }

inline static StackChar *NWayTree(ToString)                                     // Print a tree as a string
 (NWayTree(Tree) * const tree)                                                  // Tree to print as a string
 {StackChar * const p = StackCharNew();
  NWayTree_root(n, tree);
  if (n) NWayTree(ToString2)(n, 0, p);
  return p;
 }

inline static void NWayTree(PrintErr)                                           // Print a tree on stderr
 (NWayTree(Tree) * const tree)                                                  // Tree to print
 {StackChar * const s = NWayTree(ToString)(tree);
  StackCharErr(s);
  StackCharFree(s);
 }

inline static void NWayTree(ToStringWithId2)                                    // Print the keys in a tree adding the id of each node in the tree and the index of the key within that node
 (NWayTree(Node) * const node,                                                  // Node
  const long             in,                                                    // Indentation
  StackChar      * const p)                                                     // Stack as string
 {if (!node) return;
  NWayTree_Node_length(nl, node);
  if (!node || !nl) return;
  NWayTree_Node_down(n, node, 0);
  NWayTree(ToStringWithId2)(n, in+1, p);

  for(long i = 0; i < nl; ++i)
   {for(long j = 0; j < in; ++j) StackCharPushString(p, "   ");
    char C[100];
    NWayTree_Node_keys(k, node, i);
    sprintf(C, "%4ld", k);
    StackCharPushString(p, C);

    for(long j = 0; j < 10-in; ++j) StackCharPushString(p, "   ");
    char D[100];
    NWayTree_Node_data(di, node, i);
    NWayTree_Node_id(id, node);
    sprintf(D, "%4ld %4ld %4ld  %p/%4ld=", di, id, i, node, nl);
    sprintf(D, "%4ld %4ld %4ld  %p/%4ld=", di, id, i, node, nl);
    sprintf(D, "%4ld %4ld %4ld  %p/%4ld=", di, id, i, node, nl);
    StackCharPushString(p, D);
    for(long j = 0; j <= nl; ++j)
     {NWayTree_Node_down(d, node, j);
      if (d)
       {NWayTree_Node_id(id, d);
        sprintf(D, " %4ld", id);
       }
      else
       {sprintf(D, " %4ld", 0l);
       }
      StackCharPushString(p, D);
     }
    StackCharPushString(p, "\n");
    NWayTree_Node_down(d, node, i+1);
    NWayTree(ToStringWithId2)(d, in+1, p);
   }
 }

inline static StackChar *NWayTree(ToStringWithId)                               // Print the keys in a tree adding the id of each node in the tree and the index of the key within that node
 (NWayTree(Tree) * const tree)                                                  // Tree to print as a string
 {StackChar * const p = StackCharNew();
  StackCharPushString(p, "                                     Data Node Index Children\n");
  NWayTree_root(n, tree);
  if (n) NWayTree(ToStringWithId2)(n, 0, p);
  return p;
 }

inline static void NWayTree(PrintErrWithId)                                     // Print the keys in a tree adding the id of each node in the tree and the index of the key within that node
 (NWayTree(Tree) * const tree)                                                  // Tree to print
 {StackChar * const s = NWayTree(ToStringWithId)(tree);
  StackCharErr(s);
  StackCharFree(s);
 }

inline static void NWayTree(ErrAsC)                                             // Print a tree as C strings on stderr
 (NWayTree(Tree) * const tree)                                                  // Tree to print
 {StackChar * const s = NWayTree(ToString)(tree);
  NWayTree_constLong(N, s->next-s->base);                                       // The number of characters to print
  fputs("assert(NWayTree(EqText)(t,\n", stderr);
  fputc('\"', stderr);

  for(long i = s->base; i < N; ++i)
   {const char c = s->arena[i];
    if (c == '\n')
     {fputs("\\n\"\n", stderr);
      if (i + 1 < N) putc('"', stderr);
     }
    else putc(c, stderr);
   }
  if (!N)
   {fputc('\"', stderr);
   }
  StackCharFree(s);
  fputs("));\n", stderr);
 }

inline static void NWayTree(ErrNode)                                            // Dump a node
 (NWayTree(Node) * const node)                                                  // Node
 {say("Node at  %p", node);
  NWayTree_Node_up(u, node);
  say("  Up     = %p", u);
  NWayTree_Node_length(nl, node);
  say("  Length = %ld", nl);
  fprintf(stderr, "  Keys   : ");
  for(long i = 0; i <  nl; ++i)
   {NWayTree_Node_keys(k, node, i);
    fprintf(stderr," %ld", k);
   }
  fprintf(stderr, "\n  Data   : ");
  for(long i = 0; i <  nl; ++i)
   {NWayTree_Node_data(d, node, i);
    fprintf(stderr," %ld", d);
   }
  fprintf(stderr, "\n  Down   : ");
  for(long i = 0; i <= nl; ++i)
   {NWayTree_Node_down(n, node, i);
    fprintf(stderr," %p",  n);
   }
  say("\n");
 }

inline static long NWayTree(EqText)                                             // Check whether the text representing a tree is the same as the specified text
 (NWayTree(Tree) * const tree,                                                  // Tree to dump
  char           * const text)                                                  // Expected text
 {StackChar * const s = NWayTree(ToString)(tree);
  NWayTree_constLong(r, strncmp(s->arena+s->base, text, s->next-s->base) == 0);
  StackCharFree(s);
  return r;
 }

inline static void NWayTree(ErrFindResult)                                      // Print a find result
 (NWayTree(FindResult) const r)                                                 // Find result
 {char *c;
  NWayTree_FindResult_cmp(cmp, r);
  switch(cmp)
   {case NWayTree(FindComparison_equal):  c = "equal";    break;
    case NWayTree(FindComparison_lower):  c = "lower";    break;
    case NWayTree(FindComparison_higher): c = "higher";   break;
    default:                              c = "notFound"; break;
   }

  NWayTree_FindResult_index(ri, r);
  NWayTree_FindResult_node(n, r);
  NWayTree_Node_keys(k,  n, ri);
  NWayTree_FindResult_key(K, r);

  say("Find key=%ld Result keys[index]=%ld %s  index=%ld", K, k, c, ri);
 }

//D1 Properties

inline static long NWayTree(Full)                                               // Confirm that a node is full. Not used by insert or find.
 (NWayTree(Node) * const node)
 {NWayTree_Node_tree(t, node);
  NWayTree_Node_length(nl, node);
  NWayTree_maximumNumberOfKeys(m, t);
  return nl == m;
 }

//inline static long NWayTree(HalfFull)                                         // Confirm that a node is half full.
// (NWayTree(Node) * const node)                                                // Node
// {NWayTree_Node_length(n, node);
//  NWayTree_Node_tree(t, node);
//  NWayTree_maximumNumberOfKeys(M, t);
//  assert(n <= M+1);
//  NWayTree_minimumNumberOfKeys(m, t);
//  return n == m;
// }

inline static long NWayTree(Node_indexInParent)                                 // Get the index of a node in its parent.
 (NWayTree(Node) * const node)                                                  // Node to locate in parent
 {NWayTree_Node_up(p, node);
  assert(p);
  NWayTree_Node_length(l, p);
  for(long i = 0; i <= l; ++i)
   {NWayTree_Node_down(d, p, i);
    if (d == node) return i;
   }
  assert(0);
 }

//D1 Integrity Checking

inline static long NWayTree(CheckNode)                                          // Check the connections to and from a node
 (NWayTree(Node) * const node,                                                  // Node
  char           * const name)                                                  // Name of check
 {NWayTree_Node_length(nl, node);
  NWayTree_Node_tree(t, node);

  NWayTree_maximumNumberOfKeys(m, t);
  if (nl > m)
   {NWayTree_Node_keys(k, node, 0);
    say("%s: Node %ld is too long at %ld", name, k, nl);
    return 1;
   }

  for(long i = 0; i <= nl; ++i)                                                 // Check that each child has a correct up reference
   {NWayTree_Node_down(d, node, i);                                             // Step down
    if (d)
     {NWayTree_Node_length(dl, d);
      if (dl > m)
       {NWayTree_Node_keys(nk, node, 0);
        NWayTree_Node_keys(dk, d,    0);
        say("%s: Node %ld under %ld is too long at %ld", name, nk, dk, dl);
        return 2;
       }

      NWayTree_Node_up(u, d);

      if (u != node)
       {NWayTree_Node_keys(d0, d,    0);
        NWayTree_Node_keys(n0, node, 0);
        NWayTree_Node_keys(u0, u,    0);

        say("%s: Node %ld(%p) under %ld(%p) has wrong up pointer %ld(%p)",
             name, d0, d, n0, node, u0, u);
        return 3;
       }
     }
   }

  NWayTree_Node_up(p, node);                                                    // Check that parent connects to the current node
  if (p)
   {NWayTree_Node_length(pl, p);
    NWayTree_Node_tree(t, node);                                                // Check that parent connects to the current node
    NWayTree_maximumNumberOfKeys(m, t);
    assert(pl <= m);
    int c = 0;                                                                  // Check that the parent has a down pointer to the current node
    for(long i = 0; i <= pl; ++i)
     {NWayTree_Node_down(d, p, i);
      if (d == node) ++c;                                                       // Find the node that points from the parent to the current node
     }
    if (c != 1)                                                                 // We must be able to find the child
     {NWayTree_Node_keys(nk, node, 0);
      NWayTree_Node_keys(pk, p,    0);
      say("%s: Node %ld has parent %ld that fails to refer back to it",
           name, nk, pk);
      return 4;
     }
   }
  return 0;
 }

inline static void NWayTree(CheckTree2)                                         // Check the structure of a tree
 (NWayTree(Node) * const node,                                                  // Node to check
  char           * const name)                                                  // Name of check
 {if (!node) return;

  if (NWayTree(CheckNode)(node, name))
   {NWayTree_Node_tree(t, node);
    NWayTree(PrintErr)(t);
    assert(0);
   }

  NWayTree_Node_down(d, node, 0);
  NWayTree(CheckTree2)(d, name);
  NWayTree_Node_length(nl, node);
  for(long i = 0; i < nl; ++i)
   {NWayTree_Node_down(d, node, i+1);
    NWayTree(CheckTree2)(d, name);
   }
 }

inline static void NWayTree(CheckTree)                                          // Check the structure of a tree
 (NWayTree(Tree) * const tree,                                                  // Node to check
  char           * const name)                                                  // Name of check
 {NWayTree_root(n, tree);
  NWayTree(CheckTree2)(n, name);
 }

//D1 Splitting

inline static long NWayTree(Node_SplitIfFull)                                   // Split a node if it is full. Return true if the node was split else false
 (NWayTree(Node) * const node)
 {NWayTree_Node_length(nl, node);

  NWayTree_Node_tree(t, node);                                                  // Associated tree
  NWayTree_maximumNumberOfKeys(m, t);
  if (nl < m)                                                                   // Must be a full node
   {return 0;
   }

  NWayTree_constNode(l, NWayTree(Node_new)(t));                                 // New child nodes
  NWayTree_constNode(r, NWayTree(Node_new)(t));

  NWayTree_maximumNumberOfKeys(N, t);                                           // Split points
  NWayTree_constLong(n, N>>1);                                                  // Index of key that will be placed in parent

  NWayTree_constLong(L, n + 1);
  NWayTree_constLong(R, N - L);

  NWayTree_Node_setLength(l, n);
  NWayTree_Node_setLength(r, R);

  NWayTree(Node_copy)(l, node, 0, 0, n);    // n was  L                         // Split left node
  NWayTree(Node_copy)(r, node, 0, L, R);                                        // Split right node

  NWayTree_Node_isLeaf(leaf, node);                                             // Leaf node
  if (!leaf)                                                                    // Not a leaf node
   {NWayTree(ReUp)(l);
    NWayTree(ReUp)(r);
   }

  NWayTree_Node_up(p, node);                                                    // Existing parent node
  if (p)                                                                        // Not a root node
   {NWayTree_Node_length(pl, p);
    NWayTree_Node_setUp(l, p);                                                  // Connect children to parent
    NWayTree_Node_setUp(r, p);
    NWayTree_Node_down(d, p, 0);

    if (d == node)                                                              // Splitting the first child - move everything up
     {NWayTree_Node_setLength(p, pl+1);
      NWayTree(Node_open)(p, 0, pl);
      NWayTree_Node_keys(nk, node, n);
      NWayTree_Node_data(nd, node, n);
      NWayTree_Node_setKeys(p, 0, nk);
      NWayTree_Node_setData(p, 0, nd);
      NWayTree_Node_setDown(p, 0, l);
      NWayTree_Node_setDown(p, 1, r);
      NWayTree(Node_free)(node);
      return 1;
     }

    NWayTree_Node_down(pd, p, pl);
    if (pd == node)                                                             // Splitting the last child - just add it on the end
     {NWayTree_Node_keys(pk,  node, n);
      NWayTree_Node_data(pd,  node, n);
      NWayTree_Node_setKeys  (p, pl, pk);
      NWayTree_Node_setData  (p, pl, pd);
      NWayTree_Node_setDown  (p, pl, l);
      NWayTree_Node_data(nd,  node, n);
      NWayTree_Node_setData  (p, pl, nd);
      NWayTree_Node_setDown  (p, pl, l);
      NWayTree_Node_setLength(p, pl+1);
      NWayTree_Node_setDown  (p, pl+1, r);
      NWayTree(Node_free)     (node);
      return 1;
     }

    for(long i = 1; i < pl; ++i)                                                // Splitting a middle child:
     {NWayTree_Node_down(d, p, i);
      if (d == node)                                                            // Find the node that points from the parent to the current node
       {NWayTree(Node_open)(p, i, pl-i);
        NWayTree_Node_keys(pk, node, n);
        NWayTree_Node_data(pd, node, n);
        NWayTree_Node_setKeys  (p, i,  pk);
        NWayTree_Node_setData  (p, i,  pd);
        NWayTree_Node_setDown  (p, i,   l);
        NWayTree_Node_setDown  (p, i+1, r);
        NWayTree_Node_setLength(p, pl+1);
        NWayTree(Node_free)(node);
        return 1;
       }
     }
    assert(0);                                                                  // Could not find the child in the parent
   }

  NWayTree_Node_setUp(l, node);                                                 // Root node with single key after split
  NWayTree_Node_setUp(r, node);                                                 // Connect children to parent

  NWayTree_Node_keys(pk,  node, n);                                             // Single key
  NWayTree_Node_data(pd,  node, n);                                             // Data associated with single key
  NWayTree_Node_setKeys  (node, 0, pk);
  NWayTree_Node_setData  (node, 0, pd);
  NWayTree_Node_setDown  (node, 0, l);
  NWayTree_Node_setDown  (node, 1, r);
  NWayTree_Node_setLength(node, 1);
  return 1;
 }

inline static NWayTree(FindResult) NWayTree(FindAndSplit)                       // Find a key in a tree splitting full nodes along the path to the key.
 (NWayTree(Tree) * const tree,                                                  // Tree to search
  NWayTreeDataType const key)                                                   // Key to locate
 {NWayTree_root(node, tree);

  NWayTree(Node_SplitIfFull)(node);                                             // Split the root node if necessary

  for(long j = 0; j < NWayTree_MaxIterations; ++j)                              // Step down through the tree
   {NWayTree_Node_length(nl, node);                                             // Length of node
    NWayTree_constLong(last, nl-1);                                             // Greater than largest key in node. Data often gets inserted in ascending order so we do this check first rather than last.
    NWayTree_Node_keys(K, node, last);
    if (key > K)                                                                // Key greater than current key
     {NWayTree_Node_isLeaf(leaf, node);
      if (leaf)                                                                 // Leaf
       {NWayTree_FindComparison(h, higher);
        return NWayTree(FindResult_new)(node, key, h, last);
       }
      NWayTree_Node_down(n, node, last+1);
      NWayTree_constLong(s, NWayTree(Node_SplitIfFull)(n));                     // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
      if (!s)                                                                   // No split needed
       {node = n;
       }
      continue;
     }

    for(long i = 0; i < nl; ++i)                                                // Search the keys in this node as greater than least key and less than largest key
     {NWayTree_Node_keys(k, node, i);                                           // Current key
      if (key == k)                                                             // Found key
       {NWayTree_FindComparison(e, equal);
        return NWayTree(FindResult_new)(node, key, e, i);
       }
      if (key < k)                                                              // Greater than current key
       {NWayTree_Node_isLeaf(leaf, node);
        if (leaf)
         {NWayTree_FindComparison(l, lower);
          return NWayTree(FindResult_new)(node, key, l, i);
         }
        NWayTree_Node_down(n, node, i);
        NWayTree_constLong(s, NWayTree(Node_SplitIfFull)(node));                // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
        if (s)
         {NWayTree_Node_up(N, n);
          node = N;
         }
        else
         {node = n;
         }
        break;
       }
     }
   }
  assert(0);
 }

//inline static void NWayTree(FillFromLeftOrRight)                              // Fill a node from the specified sibling. Not called by find or insert.
 //(NWayTree(Node) * const n,                                                   // Node to fill
  //const long dir)                                                             // Direction to fill from
 //{NWayTree_Node_up(p, n);                                                     // Parent of leaf
  //assert(p);
  //NWayTree_constLong(i,  NWayTree(Node_indexInParent)(n));                    // Index of leaf in parent
  //NWayTree_Node_length(pl, p);
  //NWayTree_Node_length(nl, n);

  //if (dir)                                                                    // Fill from right
   //{assert(i < pl);                                                           // Cannot fill from right
    //NWayTree_Node_down(r, p, i+1);                                            // Right sibling
    //NWayTree_Node_length(rl, r);
    //NWayTree_Node_keys(pd, p, i);                                             // Transfer key and data to parent
    //NWayTree_Node_data(pk, p, i);
    //NWayTree_Node_setKeys(n, nl, pk);                                         // Transfer key and data to parent
    //NWayTree_Node_setData(n, nl, pd);

    //NWayTree_constLong(rk, ArrayLongShift(r->keys, rl));                      // Transfer keys and data from right
    //NWayTree_constLong(rd, ArrayLongShift(r->data, rl));
    //NWayTree_Node_setKeys(p, i, rk);
    //NWayTree_Node_setData(p, i, rd);

    //NWayTree_Node_isLeaf(leaf, n);                                            // Leaf
    //if (!leaf)                                                                // Transfer node if not a leaf
     //{void * const rd = ArrayVoidShift((void *)r->down, rl);
      //ArrayVoidPush((void *)n->down, nl, rd);
      //NWayTree_Node_down(d, n, nl+1);
      //NWayTree_Node_setUp(d, n);
     //}
    //NWayTree_Node_setLength(r1, r, rl-1); if (r1) {}
    //NWayTree_Node_setLength(n1, n, nl+1); if (n1) {}
   //}
  //else                                                                        // Fill from left - untested
   //{assert(i);                                                                // Cannot fill from left
    //NWayTree_constLong(I, i-1);
    //NWayTree_Node_down(l, p, I);                                              // Left sibling
    //NWayTree_Node_length(ll, l);

    //NWayTree_Node_keys(pk, p, I);                                             // Shift in keys and data from left
    //NWayTree_Node_data(pd, p, I);

    //ArrayLongUnShift(l->keys, ll, pk);                                        // Shift in keys and data from left
    //ArrayLongUnShift(l->data, ll, pd);

    //NWayTree_constLong(lk, ArrayLongPop(l->keys, ll));                        // Transfer key and data to parent
    //NWayTree_constLong(ld, ArrayLongPop(l->data, ll));
    //NWayTree_Node_setKeys(p, I, lk);                                          // Transfer key and data to parent
    //NWayTree_Node_setData(p, I, ld);
    //NWayTree_Node_isLeaf(leaf, l);                                            // Leaf
    //if (!leaf)                                                                // Transfer node if not a leaf
     //{void * ld = ArrayVoidPop((void *)l->down, ll);
      //ArrayVoidUnShift(        (void *)l->down, ll, ld);

      //NWayTree_Node_down(d, l, 0);
      //NWayTree_Node_setUp(d, l);
     //}
   //}
 //}

//inline static void NWayTree(MergeWithLeftOrRight)                             // Merge two adjacent nodes.  Not called by find or insert.
 //(NWayTree(Node) * const n,                                                   // Node to fill
  //const long             dir)                                                 // Direction to fill from
 //{assert(NWayTree(HalfFull)(n));                                              // Confirm leaf is half full
  //NWayTree_Node_up(p, n);                                                     // Parent of leaf
  //assert(p);
  //NWayTree_constLong(hf, NWayTree(HalfFull)(p));                              // Parent must have more than the minimum number of keys because we need to remove one unless it is the root of the tree
  //assert(hf);
  //NWayTree_Node_up(P, p);                                                     // Check that we are not on the root node
  //assert(P);

  //NWayTree_constLong(i, NWayTree(Node_indexInParent)(n));                     // Index of leaf in parent
  //NWayTree_Node_length(pl, p);
  //NWayTree_Node_length(nl, n);

  //if (dir)                                                                    // Merge with right hand sibling
   //{assert(i < pl);                                                           // Cannot fill from right
    //NWayTree_constLong(I, i+1);
    //NWayTree_Node_down(r, p, I);                                              // Leaf on right
    //assert(NWayTree(HalfFull)(r));                                            // Confirm right leaf is half full
    //NWayTree_Node_length(rl, r);

    //const NWayTreeDataType k = ArrayLongDelete(p->keys, pl, I);               // Transfer keys and data from parent
    //const NWayTreeDataType d = ArrayLongDelete(p->data, pl, I);
    //ArrayLongPush(n->keys, nl, k);
    //ArrayLongPush(n->data, nl, d);

    //ArrayLongPushArray(n->keys, nl+1, r->keys, rl);                           // Transfer keys
    //ArrayLongPushArray(n->data, nl+1, r->data, rl);                           // Transfer data

    //NWayTree_Node_isLeaf(leaf, n);                                            // Leaf
    //if (!leaf)                                                                // Children of merged node
     //{ArrayVoidPushArray((void *)n->down, nl,(void *)r->down, rl);
      //NWayTree(ReUp)(n);                                                      // Update parent of children of right node
     //}
    //ArrayVoidDelete((void *)p->down, pl, I);                                  // Remove link from parent to right child
    //NWayTree_Node_setLength(n1, n, nl + rl + 1); if (n1) {}
    //NWayTree_Node_setLength(p1, p, pl      - 1); if (p1) {}
    //NWayTree(Node_free)(r);
   //}
  //else                                                                        // Merge with left hand sibling
   //{assert(i > 0);                                                            // Cannot fill from left
    //NWayTree_constLong(I, i-1);
    //NWayTree_Node_down(l, p, I);                                              // Node on left
    //assert(NWayTree(HalfFull)(l));                                            // Confirm left leaf is half full
    //NWayTree_Node_length(ll, l);
    //const NWayTreeDataType k = ArrayLongDelete(p->keys, pl, I);               // Transfer parent key and data
    //const NWayTreeDataType d = ArrayLongDelete(p->data, pl, I);
    //ArrayLongUnShift     (n->keys, nl,   k);
    //ArrayLongUnShift     (n->data, nl,   d);
    //ArrayLongUnShiftArray(n->keys, nl+1, l->keys, ll);                        // Transfer left keys and data
    //ArrayLongUnShiftArray(n->data, nl+1, l->data, ll);

    //NWayTree_Node_isLeaf(leaf, n);                                            // Leaf
    //if (!leaf)                                                                // Children of merged node
     //{ArrayLongUnShiftArray((void *)n->down, nl,
                            //(void *)l->down, ll);
      //NWayTree(ReUp)(n);                                                      // Update parent of children of left node
     //}
    //ArrayVoidDelete((void *)p->down, pl, I);                                  // Remove link from parent to right child
    //NWayTree_Node_setLength(n1, n, nl + ll + 1); if (n1) {}
    //NWayTree_Node_setLength(p1, p, pl      - 1); if (p1) {}
    //NWayTree(Node_free)(l);
   //}
 //}

//inline static void NWayTree(Merge)                                            // Merge the current node with its sibling. Not called by find or insert
 //(NWayTree(Node) * const node)                                                // Node to merge into
 //{NWayTree_constLong(i, NWayTree(Node_indexInParent)(node));                  // Index in parent
  //NWayTree_Node_up(p, node);                                                  // Parent

  //if (i)                                                                      // Merge with left node
   //{NWayTree_Node_down(l, p, i-1);                                            // Left node
    //NWayTree_constNode(r, node);
    //if (NWayTree(HalfFull)(r))                                                // Merge as left and right nodes are half full
     //{if (NWayTree(HalfFull)(l))
       //{NWayTree(MergeWithLeftOrRight)(r, 0);
       //}
      //else
       //{NWayTree(FillFromLeftOrRight) (r, 0);
       //}
     //}
   //}
  //else
   //{NWayTree_Node_down(r, p, 1);                                              // Right node
    //NWayTree_constNode(l, node);
    //if (NWayTree(HalfFull)(l))
     //{if (NWayTree(HalfFull)(r))                                              // Merge as left and right nodes are half full
       //{NWayTree(MergeWithLeftOrRight)(l, 1);
       //}
      //else
       //{NWayTree(FillFromLeftOrRight) (l, 1);
       //}
     //}
   //}
 //}

//inline static void NWayTree(MergeOrFill)                                      // Make a node larger than a half node. Not called by find or insert.
 //(NWayTree(Node) * const node)                                                // Node to merge or fill
 //{if (NWayTree(HalfFull)(node)) return;                                       // No need to merge of if not a half node
  //NWayTree_Node_up(p, node);                                                  // Parent
  //NWayTree_Node_up(P, p);                                                     // Parent of parent

  //if (P)                                                                      // Merge or fill parent which is not the root
   //{NWayTree(MergeOrFill)(p);
    //NWayTree(Merge)(node);
    //return;
   //}

  //NWayTree_Node_length(pl, p);

  //if (pl == 1)                                                                // Parent is the root and it only has one key - merge into the child if possible
   //{NWayTree_Node_down(l, p, 0);
    //NWayTree_constLong(lh, NWayTree(HalfFull)(l));
    //if (lh)
     //{NWayTree_Node_down(r, p, 1);
      //NWayTree_constLong(rh, NWayTree(HalfFull)(r));
      //if (rh)
       //{NWayTree_Node_length(L, l);
        //NWayTree_Node_length(R, r);
        //NWayTree_Node_length(N, node);
        //ArrayLongPushArray(node->keys, 0, l->keys, L);
        //ArrayLongPushArray(node->data, 0, l->data, L);

        //ArrayLongPushArray(node->keys, L, p->keys, 1);
        //ArrayLongPushArray(node->data, L, p->data, 1);

        //ArrayLongPushArray(node->keys, L+1, r->keys, R);
        //ArrayLongPushArray(node->data, L+1, r->data, R);

        //ArrayVoidPushArray((void *)node->down, 0,   (void *)l->down, L+1);
        //ArrayVoidPushArray((void *)node->down, L+1, (void *)r->down, R);
        //NWayTree_Node_setLength(lr1, node, L+R+1); if (lr1) {}

        //ArrayLongPushArray(p->keys, 0, node->keys, N);
        //ArrayLongPushArray(p->data, 0, node->data, N);
        //ArrayVoidPushArray((void *)p->down, 0, (void *)node->down, N+1);

        //NWayTree(ReUp)(p);                                                    // Reconnect children to parent
        //return;
       //}
     //}
   //}

  //NWayTree(Merge)(node);                                                      // Parent is the root but it has more than one key
 //}

//D1 Find

static NWayTree(FindResult) NWayTree(Find)                                      // Find a key in a tree returning its associated data or undef if the key does not exist.
 (NWayTree(Tree) * const tree,                                                  // Tree to search
  NWayTreeDataType const key)                                                   // Key to search
 {NWayTree_root(N, tree);
  NWayTree(Node) * node = N;                                                    // Current node we are searching
  if (!node)                                                                    // Empty tree
   {NWayTree_FindComparison(n, notFound);
    return NWayTree(FindResult_new)(node, key, n, -1);
   }

  for(long j = 0; j < NWayTree_MaxIterations; ++j)                                  // Same code as above
   {NWayTree_Node_length(nl, node);
    const long nl1 = nl - 1;
    NWayTree_Node_keys(nk, node, nl1);

    if (key > nk)                                                               // Bigger than every key
     {NWayTree_Node_isLeaf(l, node);                                            // Leaf
      if (l)
       {NWayTree_FindComparison(h, higher);
        return NWayTree(FindResult_new)(node, key, h, nl1);
       }
      NWayTree_Node_down(n, node, nl);
      node = n;
      continue;
     }

    for(long i = 0; i < nl; ++i)                                                // Search the keys in this node as less than largest key
     {NWayTree_Node_keys(k, node, i);                                           // Key from tree
      if (key == k)                                                             // Found key
       {NWayTree_FindComparison(e, equal);
        return NWayTree(FindResult_new)(node, key, e, i);
       }
      if (key < k)                                                              // Lower than current key
       {NWayTree_Node_isLeaf(leaf, node);                                       // Leaf
        if (leaf)                                                               // Leaf
         {NWayTree_FindComparison(l, lower);
          return NWayTree(FindResult_new)(node, key, l, i);
         }
        NWayTree_Node_down(n, node, i);
        node = n;
        break;
       }
     }
   }
  assert(0);
 }

//D1 Insert

static void NWayTree(Insert)                                                    // Insert a key and its associated data into a tree
 (NWayTree(Tree) * const tree,                                                  // Tree to insert into
  NWayTreeDataType const key,                                                   // Key to insert
  NWayTreeDataType const data)                                                  // Data associated with key
 {NWayTree_root(n, tree);                                                       // Root node of tree

  if (!n)                                                                       // Empty tree
   {NWayTree_constNode(n, NWayTree(Node_new)(tree));
    NWayTree_Node_setKeys(n, 0, key);
    NWayTree_Node_setData(n, 0, data);
    NWayTree_Node_setLength(n, 1);
    NWayTree_incKeys(tree);
    NWayTree_setRoot(tree, n);
    return;
   }

  NWayTree_Node_length(nl, n);                                                  // Current length of node
  NWayTree_maximumNumberOfKeys(m, tree);                                        // Maximum number of keys allowed in a node

  if (nl < m)                                                                   // Node is root with no children and room for one more key
   {NWayTree_Node_up(p, n);
    if (!p)
     {NWayTree_Node_isLeaf(leaf, n);
      if (leaf)
       {for(long i = 0; i < nl; ++i)                                            // Each key
         {NWayTree_Node_keys(nk, n, i);
          if (key == nk)                                                        // Key already present
           {NWayTree_Node_setData(n, i, data);
            return;
           }
          NWayTree_Node_keys(k, n, i);
          if (key < k)                                                          // We have reached the insertion point
           {NWayTree(Node_open)(n, i, nl - i);
            NWayTree_Node_setKeys(n, i, key);
            NWayTree_Node_setData(n, i, data);
            NWayTree_Node_setLength(n, nl+1);
            NWayTree_incKeys(tree);
            return;
           }
         }
        NWayTree_Node_setKeys(n, nl, key);                                       // Insert the key at the end of the block because it is greater than all the other keys in the block
        NWayTree_Node_setData(n, nl, data);
        NWayTree_Node_setLength(n, nl+1);
        NWayTree_incKeys(tree);
        return;
       }
     }
   }
                                                                                // Insert node
  NWayTree(FindResult) const r = NWayTree(FindAndSplit)(tree, key);             // Check for existing key
  NWayTree_FindResult_node(N, r);
  NWayTree_FindComparison(e, equal);
  NWayTree_FindResult_cmp(c, r);
  NWayTree_FindResult_index(i, r);

  if (c == e)                                                                   // Found an equal key whose data we can update
   {NWayTree_Node_setData(N, i, data);
    return;
   }

  NWayTree_FindComparison(h, higher);
  NWayTree_Node_length(Nl, N);
  const long Nl1 = Nl+1;
  if (c == h)
   {const long i1 = i+1;
    NWayTree(Node_open)(N, i1, Nl - i1);

    NWayTree_Node_setKeys(N, i1, key);
    NWayTree_Node_setData(N, i1, data);
   }
  else
   {NWayTree(Node_open)(N, i, Nl - i);

    NWayTree_Node_setKeys(N, i, key);
    NWayTree_Node_setData(N, i, data);
   }

  NWayTree_Node_setLength(N, Nl1);
  NWayTree(Node_SplitIfFull)(N);                                                // Split if the leaf is full to force keys up the tree
 }

//D1 Iteration

inline static NWayTree(FindResult) NWayTree(GoAllTheWayLeft)                    // Go as left as possible from the current node
 (NWayTree(Node) * node)
 {if (!node)                                                                    // Empty tree
   {NWayTree_FindComparison(n, notFound);
    return NWayTree(FindResult_new)(node, 0, n, 0);
   }

  for(long i = 0; i < NWayTree_MaxIterations; ++i)                              // Step down through tree
   {NWayTree_Node_isLeaf(leaf, node);
    if (leaf) break;                                                            // Reached leaf
    NWayTree_Node_down(d, node, 0);
    node = d;
   }

  NWayTree_Node_keys(k, node, 0);                                               // Update find result
  NWayTree_FindComparison(e, equal);
  return NWayTree(FindResult_new)(node, k, e, 0);                               // Leaf - place us on the first key
 }

inline static NWayTree(FindResult) NWayTree(GoUpAndAround)                      // Go up until it is possible to go right or we can go no further
 (NWayTree(FindResult) const find)
 {NWayTree_FindResult_node(Node, find);
  NWayTree(Node) * node = Node;
  NWayTree_Node_isLeaf(leaf, node);
  if (leaf)                                                                     // Leaf
   {NWayTree_FindResult_index(I, find);
    NWayTree_Node_length(L, node);
    if (I < L - 1)                                                              // More keys in leaf
     {NWayTree_constLong(i, I + 1);
      NWayTree_FindComparison(e, equal);
      NWayTree_Node_keys(k, node, i);
      return NWayTree(FindResult_new)(node, k, e, i);
     }
    NWayTree_Node_up(Parent, node);                                             // Parent
    if (Parent)
     {NWayTree(Node) *parent = Parent;
      for(long j = 0; j < NWayTree_MaxIterations; ++j)                          // Not the only node in the tree
       {NWayTree_constLong(i, NWayTree(Node_indexInParent)(node));              // Index in parent
        NWayTree_Node_length(pl, parent);                                       // Parent length
        if (i == pl)                                                            // Last key - continue up
         {node = parent;
          NWayTree_Node_up(Parent, parent);                                     // Parent
          parent = Parent;
          if (!parent) break;
         }
        else
         {NWayTree_Node_keys(k, parent, i);
          NWayTree_FindComparison(e, equal);
          return NWayTree(FindResult_new)(parent, k, e, i);                     // Not the last key
         }
       }
     }
    NWayTree_FindComparison(n, notFound);
    return NWayTree(FindResult_new)(node, 0, n, 0);                             // Last key of root
   }

  NWayTree_FindResult_index(i, find);                                           // Not a leaf so on an interior key so we can go right then all the way left
  NWayTree_Node_down(d, node, i+1);
  return NWayTree(GoAllTheWayLeft)(d);
 }

inline static NWayTree(FindResult) NWayTree(IterStart)                          // Start an iterator
 (NWayTree(Tree) * const tree)                                                  // Tree to iterate
 {NWayTree_root(n, tree);
  NWayTree(FindResult) f = NWayTree(GoAllTheWayLeft)(n);
  return f;
 }

inline static long NWayTree(IterCheck)                                          // True if we can continue to iterate
 (NWayTree(FindResult) const find)                                              // Find result of last iteration
 {NWayTree_FindComparison(n, notFound);
  NWayTree_FindResult_cmp(c, find);
  return c != n;
 }

inline static NWayTree(FindResult) NWayTree(IterNext)                           // Next element of an iteration
 (NWayTree(FindResult) const find)                                              // Find result of last iteration
 {NWayTree(FindResult) f = NWayTree(GoUpAndAround)(find);
  return f;
 }

/*
inline static void NWayTreeLong deleteLeafKey($$)                               // Delete a key in a leaf.
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

inline static void NWayTreeLong deleteKey($$)                                   // Delete a key.
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

inline static void NWayTreeLong delete($$)                                      // Find a key in a tree, delete it and return any associated data.
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

inline static void NWayTreeLong insert($$$)                                     // Insert the specified key and data into a tree.
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

inline static void NWayTreeLong iterator($)                                     // Make an iterator for a tree.
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

inline static void NWayTreeLong Tree::Multi::Iterator::next($)                  // Find the next key.
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

inline static void NWayTreeLong reverseIterator($)                              // Create a reverse iterator for a tree.
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

inline static void NWayTreeLong Tree::Multi::ReverseIterator::prev($)           // Find the previous key.
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

inline static void NWayTreeLong flat($@)                                        // Print the keys in a tree from left right to make it easier to visualize the structure of the tree.
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

inline static void NWayTreeLong size($)                                         // Count the number of keys in a tree.
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

//D0 Tests

#if (__INCLUDE_LEVEL__ == 1)
void test_node_open()                                                           // Tests
 {NWayTree_new(t, 7);
  NWayTree(Node) *n = NWayTree(Node_new)(t);                                    // Create a new node
  for(long i = 0; i < 7; ++i)
   {n->keys[i] = i+1; n->data[i] = 11+i; n->down[i] = (void *)(21+i);
   }
  StackChar * const p = StackCharNew();
  char C[100];
  StackCharPushString(p, "Before:\n");
  for(long i = 0; i < 7; ++i)
   {sprintf(C, "%4ld %4ld  %4ld  %4ld\n", i, n->keys[i], n->data[i], (long)n->down[i]);
    StackCharPushString(p, C);
   }
  NWayTree(Node_open)(n, 2, 4);
  StackCharPushString(p, "After:\n");
  for(long i = 0; i < 7; ++i)
   {sprintf(C, "%4ld %4ld  %4ld  %4ld\n", i, n->keys[i], n->data[i], (long)n->down[i]);
    StackCharPushString(p, C);
   }
  //StackCharErr(p);
  assert(StackCharEqText(p,
"Before:\n"
"   0    1    11    21\n"
"   1    2    12    22\n"
"   2    3    13    23\n"
"   3    4    14    24\n"
"   4    5    15    25\n"
"   5    6    16    26\n"
"   6    7    17    27\n"
"After:\n"
"   0    1    11    21\n"
"   1    2    12    22\n"
"   2    3    13    23\n"
"   3    3    13    23\n"
"   4    4    14    24\n"
"   5    5    15    25\n"
"   6    6    16    26\n"
  ));
  StackCharFree(p);
 }

void test_3_0()                                                                 // Tests
 {NWayTree_new(t, 3);
  assert(NWayTree(EqText)(t, ""));
  NWayTree_FindResult(f, NWayTree(Find)(t, 1));
  NWayTree_FindResult_cmp(c, f);
  if (c) {}
 }

void test_3_1()                                                                 // Tests
 {NWayTree_new(t, 3);
  for(int i = 0; i < 1; ++i) NWayTree(Insert)(t, i, 2);
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
));
  NWayTree(Insert)(t, 0, 1);
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"   0                                   1\n"
));
 }

void test_31_1()                                                                // Tests
 {NWayTree_new(t, 31);
  for(int i = 0; i < 1; ++i) NWayTree(Insert)(t, i, 2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
));
 }

void test_3_2()                                                                 // Tests
 {NWayTree_new(t, 3);
  for(int i = 0; i < 2; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
));
 }

void test_31_2()                                                                // Tests
 {NWayTree_new(t, 31);
  for(int i = 0; i < 2; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
));
 }

void test_3_3()                                                                 // Tests
 {NWayTree_new(t, 3);
  for(int i = 0; i < 3; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_3_4z()                                                                // Tests
 {NWayTree_new(t, 3);
  for(int i = 0; i < 4; ++i) NWayTree(Insert)(t, i, i+2);
  NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_31_3()                                                                // Tests
 {NWayTree_new(t, 31);
  for(int i = 0; i < 3; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
"   2                                   4\n"
));
 }

NWayTree(Node) *createNode3(NWayTree(Tree) * t, long a, long b, long c)         // Create a test node
 {NWayTree(Node) *n = NWayTree(Node_new)(t);
  NWayTree_Node_setKeys(n, 0, a); NWayTree_Node_setData(n, 0, 2*a);
  NWayTree_Node_setKeys(n, 1, b); NWayTree_Node_setData(n, 1, 2*b);
  NWayTree_Node_setKeys(n, 2, c); NWayTree_Node_setData(n, 2, 2*c);
  NWayTree_Node_setLength(n, 3);

  return n;
 }

void test_3_4a()                                                                // Tree has one node
 {NWayTree(Tree) *t = NWayTree(New)(3);
  NWayTree(Node) *n = createNode3(t, 1, 2, 3);
  NWayTree_Node_setLength(n, 3);
  NWayTree_setRoot(t, n);

  long r = NWayTree(Node_SplitIfFull)(n);
  assert(r);
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"      1                                   2\n"
"   2                                   4\n"
"      3                                   6\n"
));
 }

void test_3_4b()                                                                // First down
 {NWayTree(Tree) *t  = NWayTree(New)(3);
  NWayTree(Node) *p  = createNode3(t, 10, 20, 30);
  NWayTree_Node_setLength(p, 2);
  NWayTree(Node) *n0 = createNode3(t, 01, 02, 03);
  NWayTree(Node) *n1 = createNode3(t, 11, 12, 13);
  NWayTree(Node) *n2 = createNode3(t, 21, 22, 23);
  NWayTree_Node_setDown(p, 0, n0); NWayTree_Node_setUp(n0, p);
  NWayTree_Node_setDown(p, 1, n1); NWayTree_Node_setUp(n1, p);
  NWayTree_Node_setDown(p, 2, n2); NWayTree_Node_setUp(n2, p);
  NWayTree_setRoot(t, p);
  //NWayTree(ErrAsC)(t);

  assert(NWayTree(EqText)(t,
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

  long r = NWayTree(Node_SplitIfFull)(n0); if (r){}
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
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
  //NWayTree(ErrAsC)(t);
 }

void test_3_4c()                                                                // Mid down
 {NWayTree(Tree) *t  = NWayTree(New)(3);
  NWayTree(Node) *p  = createNode3(t, 10, 20, 30);
  NWayTree_Node_setLength(p, 2);
  NWayTree(Node) *n0 = createNode3(t, 01, 02, 03);
  NWayTree(Node) *n1 = createNode3(t, 11, 12, 13);
  NWayTree(Node) *n2 = createNode3(t, 21, 22, 23);
  NWayTree_Node_setDown(p, 0, n0); NWayTree_Node_setUp(n0, p);
  NWayTree_Node_setDown(p, 1, n1); NWayTree_Node_setUp(n1, p);
  NWayTree_Node_setDown(p, 2, n2); NWayTree_Node_setUp(n2, p);
  NWayTree_setRoot(t, p);

  NWayTree_Node_down(n, p, 1);
  assert(n == n1);

  long r = NWayTree(Node_SplitIfFull)(n1); if (r){}
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
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
 {NWayTree(Tree) *t  = NWayTree(New)(3);
  NWayTree(Node) *p  = createNode3(t, 10, 20, 30);
  NWayTree_Node_setLength(p, 2);
  NWayTree(Node) *n0 = createNode3(t, 01, 02, 03);
  NWayTree(Node) *n1 = createNode3(t, 11, 12, 13);
  NWayTree(Node) *n2 = createNode3(t, 21, 22, 23);
  NWayTree_Node_setDown(p, 0, n0); NWayTree_Node_setUp(n0, p);
  NWayTree_Node_setDown(p, 1, n1); NWayTree_Node_setUp(n1, p);
  NWayTree_Node_setDown(p, 2, n2); NWayTree_Node_setUp(n2, p);
  NWayTree_setRoot(t, p);

  NWayTree_Node_down(n, p, 1);
  assert(n == n1);

  long r = NWayTree(Node_SplitIfFull)(n2); if (r){}
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
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

  NWayTree(FindAndSplit)(t, 12);
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"         1                                   2\n"
"         2                                   4\n"
"         3                                   6\n"
"     10                                  20\n"
"        11                                  22\n"
"     12                                  24\n"
"        13                                  26\n"
"  20                                  40\n"
"        21                                  42\n"
"     22                                  44\n"
"        23                                  46\n"
));
 }

void test_3_4()                                                                 // Tests
 {test_3_4a();
  test_3_4b();
  test_3_4c();
  test_3_4d();
 }

void test_3_insert1()                                                           // Insert tests
 {NWayTree_constLong(N, 1);
  NWayTree_new(t, 3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   3\n"
));
 }

void test_3_insert2()
 {NWayTree_constLong(N, 2);
  NWayTree_new(t, 3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_3_insert3()
 {NWayTree_constLong(N, 3);
  NWayTree_new(t, 3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   3\n"
"   2                                   4\n"
"   3                                   5\n"
));
 }

void test_3_insert4()
 {NWayTree_constLong(N, 4);
  NWayTree_new(t, 3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"      1                                   3\n"
"   2                                   4\n"
"      3                                   5\n"
"      4                                   6\n"
));
 }

void test_3_insert5()
 {NWayTree_constLong(N, 5);
  NWayTree_new(t, 3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"      1                                   3\n"
"   2                                   4\n"
"      3                                   5\n"
"   4                                   6\n"
"      5                                   7\n"
));
 }

void test_3_insert6()
 {NWayTree_constLong(N, 6);
  NWayTree_new(t, 3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"      1                                   3\n"
"   2                                   4\n"
"      3                                   5\n"
"   4                                   6\n"
"      5                                   7\n"
"      6                                   8\n"
));
 }

void test_3_insert7()
 {NWayTree_constLong(N, 7);
  NWayTree_new(t, 3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
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
 {NWayTree_constLong(N, 8);
  NWayTree_new(t, 3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
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
 {NWayTree_constLong(N, 2);
  NWayTree_new(t, 3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   2\n"
"   2                                   3\n"
));
 }

void test_3_insert3r()
 {NWayTree_constLong(N, 3);
  NWayTree_new(t, 3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   2\n"
"   2                                   3\n"
"   3                                   4\n"
));
 }

void test_3_insert4r()
 {NWayTree_constLong(N, 4);
  NWayTree_new(t, 3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"      1                                   2\n"
"      2                                   3\n"
"   3                                   4\n"
"      4                                   5\n"
));
 }

void test_3_insert5r()
 {NWayTree_constLong(N, 5);
  NWayTree_new(t, 3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"      1                                   2\n"
"   2                                   3\n"
"      3                                   4\n"
"   4                                   5\n"
"      5                                   6\n"
));
 }

void test_3_insert6r()
 {NWayTree_constLong(N, 6);
  NWayTree_new(t, 3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"      1                                   2\n"
"      2                                   3\n"
"   3                                   4\n"
"      4                                   5\n"
"   5                                   6\n"
"      6                                   7\n"
));
 }

void test_3_insert7r()
 {NWayTree_constLong(N, 7);
  NWayTree_new(t, 3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
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
 {NWayTree_constLong(N, 8);
  NWayTree_new(t, 3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
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

void testLoadArray(long *A, long const N)
 {for(int i = 0; i < N; ++i) A[i] = i;
  for(int i = 0; i < N; ++i)
   {const long a = rand() % N, b = rand() % N;
    const long R = A[a], I = A[b];
                   A[b] = R; A[a] = I;
   }
 }

void testLoadArray33(long *A, long const N)
 {for(int i = 0; i < N; ++i) A[i] = i;
  for(int i = 0; i < N; ++i)
   {long r = i * i % N, R = A[r], I = A[i];
    A[i] = R; A[r] = I;
   }
 }

void test_3_insert12()
 {NWayTree_constLong(N, 12); long A[N]; testLoadArray(A, N);

  NWayTree_new(t, 3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, A[i], i);
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"         0                                   8\n"
"         1                                   3\n"
"      2                                   2\n"
"         3                                   9\n"
"      4                                   4\n"
"         5                                   7\n"
"   6                                   0\n"
"         7                                   6\n"
"         8                                  11\n"
"      9                                   5\n"
"        10                                   1\n"
"        11                                  10\n"
));
 }

void test_3_insert14()
 {NWayTree_constLong(N, 14); long A[N]; testLoadArray(A, N);

  NWayTree_new(t, 3);
  for(long i = 0; i < N; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"         0                                   7\n"
"      1                                  11\n"
"         2                                  13\n"
"      3                                   6\n"
"         4                                   4\n"
"   5                                   0\n"
"         6                                  12\n"
"      7                                   5\n"
"         8                                   9\n"
"      9                                   2\n"
"        10                                   3\n"
"     11                                   1\n"
"        12                                   8\n"
"        13                                  10\n"
));
 }

void test_3_insert15()
 {NWayTree_constLong(N, 15); long A[N]; testLoadArray(A, N);

  NWayTree_new(t, 3);
  for(long i = 0; i < N; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"         0                                   9\n"
"         1                                  13\n"
"      2                                  11\n"
"         3                                   7\n"
"      4                                   4\n"
"         5                                  14\n"
"         6                                   6\n"
"   7                                   3\n"
"         8                                   8\n"
"      9                                   1\n"
"        10                                  10\n"
"     11                                   5\n"
"        12                                  12\n"
"     13                                   2\n"
"        14                                   0\n"
));
 }

void test_3_insert63()
 {NWayTree_constLong(N, 63); long A[N]; testLoadArray(A, N);

  NWayTree_new(t, 3);
  for(long i = 0; i < N; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  for(long i = 0; i < N; ++i)
   {NWayTree_FindResult(r, NWayTree(Find)(t, A[i]));
    assert(NWayTree(FindResult_data(r)) == i);
    NWayTree_FindComparison(e, equal);
    NWayTree_FindResult_cmp(c, r);
    assert(c == e);
   }
  //NWayTree(ErrAsC)(t);
  NWayTree(CheckTree)(t, "3/63");
  assert(NWayTree(EqText)(t,
"               0                                  11\n"
"            1                                   5\n"
"               2                                  41\n"
"               3                                  19\n"
"            4                                   7\n"
"               5                                  43\n"
"            6                                   6\n"
"               7                                  45\n"
"               8                                  18\n"
"         9                                   9\n"
"              10                                  10\n"
"           11                                  12\n"
"              12                                  47\n"
"              13                                  28\n"
"     14                                  33\n"
"              15                                  44\n"
"              16                                  46\n"
"           17                                  22\n"
"              18                                  36\n"
"              19                                  32\n"
"        20                                  15\n"
"              21                                  21\n"
"           22                                  29\n"
"              23                                  62\n"
"              24                                  42\n"
"           25                                  27\n"
"              26                                  60\n"
"              27                                  23\n"
"  28                                   1\n"
"              29                                  34\n"
"              30                                  31\n"
"           31                                  30\n"
"              32                                   4\n"
"        33                                  20\n"
"              34                                  17\n"
"           35                                  24\n"
"              36                                  40\n"
"           37                                  37\n"
"              38                                  52\n"
"           39                                  59\n"
"              40                                  53\n"
"     41                                   0\n"
"              42                                  14\n"
"              43                                  58\n"
"           44                                  16\n"
"              45                                  56\n"
"              46                                  35\n"
"        47                                  13\n"
"              48                                  48\n"
"              49                                  49\n"
"           50                                   8\n"
"              51                                  51\n"
"              52                                  39\n"
"     53                                   3\n"
"              54                                  54\n"
"           55                                   2\n"
"              56                                  38\n"
"        57                                  25\n"
"              58                                  26\n"
"              59                                  57\n"
"           60                                  55\n"
"              61                                  61\n"
"              62                                  50\n"
));
 }

long iterateAndTestTree                                                         // Check the keys in a tree by iterating through a tree and seeing that the keys increase sequentially by one from zero. Return 1 if all such checks pass else 0
 (NWayTree(Tree) * const t)                                                     // Tree to test
 {long n = 0;
  NWayTreeIterate(t, f)                                                         // Iterate through the tree
   {NWayTree_FindResult_key(k, f);                                              // Key of each iteration
    if (k != n++) return 0;
   }
  return 1;
 }

void test_3_iterate63()                                                         // Iterate through a tree
 {const long N = 63, NN = 63; long A[N]; testLoadArray(A, N);

  NWayTree_new(t, 3);                                                           // Create the tree
  for(long i = 0; i < NN; ++i)
   {for(long j = 0; j < 2; ++j)
     {NWayTree(Insert)(t, A[i], i);
     }
   }
  //NWayTree(ErrAsC)(t);
  //NWayTree(PrintErrWithId)(t);
  assert(iterateAndTestTree(t));
 }

void test_31_insert163                                                          // Create and free a tree.
 (int test)                                                                     // Warm malloc up until it stabilizes when false.
 {long N = 163, NN = N, A[N]; testLoadArray(A, N);

  long memory_at_start;                                                         // Memory in use at start
  if (test)
   {struct mallinfo m; m = mallinfo();
    memory_at_start = m.uordblks;
   }

  NWayTree_new(t, 31);
  for(long i = 0; i < NN; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  for(long i = 0; i < NN; ++i)
   {NWayTree(FindResult) r = NWayTree(Find)(t, A[i]);

    const long d = NWayTree(FindResult_data)(r);
    assert(d == i);

    NWayTree_FindComparison(e, equal);
    NWayTree_FindResult_cmp(c, r);
    assert(c == e);
   }
  //NWayTree(ErrAsC)(t);
  NWayTree(CheckTree)(t, "31/163");
  assert(NWayTree(EqText)(t,
"      0                                 155\n"
"      1                                  62\n"
"      2                                  90\n"
"      3                                  75\n"
"      4                                   2\n"
"      5                                  85\n"
"      6                                 134\n"
"      7                                 107\n"
"      8                                  51\n"
"      9                                  22\n"
"     10                                  73\n"
"     11                                 142\n"
"     12                                  12\n"
"     13                                  95\n"
"     14                                  98\n"
"  15                                  60\n"
"     16                                  16\n"
"     17                                  17\n"
"     18                                 112\n"
"     19                                  52\n"
"     20                                  20\n"
"     21                                  21\n"
"     22                                  42\n"
"     23                                  23\n"
"     24                                 119\n"
"     25                                  44\n"
"     26                                  26\n"
"     27                                  27\n"
"     28                                  28\n"
"     29                                  83\n"
"     30                                  80\n"
"  31                                  34\n"
"     32                                  33\n"
"     33                                  19\n"
"     34                                  37\n"
"     35                                 151\n"
"     36                                 144\n"
"     37                                  84\n"
"     38                                  38\n"
"     39                                 149\n"
"     40                                  47\n"
"     41                                  77\n"
"     42                                 157\n"
"     43                                  43\n"
"     44                                  45\n"
"     45                                 160\n"
"     46                                  40\n"
"     47                                  70\n"
"     48                                 114\n"
"     49                                 121\n"
"     50                                 145\n"
"     51                                  30\n"
"     52                                  53\n"
"     53                                 105\n"
"     54                                 139\n"
"     55                                 101\n"
"  56                                 102\n"
"     57                                  56\n"
"     58                                  61\n"
"     59                                  54\n"
"     60                                 109\n"
"     61                                  11\n"
"     62                                 135\n"
"     63                                  71\n"
"     64                                 115\n"
"     65                                 132\n"
"     66                                  66\n"
"     67                                  13\n"
"     68                                  48\n"
"     69                                  31\n"
"     70                                  92\n"
"     71                                  39\n"
"     72                                 141\n"
"     73                                 140\n"
"     74                                  74\n"
"     75                                  49\n"
"     76                                  76\n"
"  77                                   0\n"
"     78                                 128\n"
"     79                                  79\n"
"     80                                 153\n"
"     81                                  86\n"
"     82                                  82\n"
"     83                                   6\n"
"     84                                 104\n"
"     85                                  68\n"
"     86                                  72\n"
"     87                                  87\n"
"     88                                  59\n"
"     89                                  32\n"
"     90                                  55\n"
"     91                                   9\n"
"     92                                  88\n"
"     93                                 118\n"
"     94                                 136\n"
"     95                                 158\n"
"     96                                 152\n"
"     97                                 120\n"
"     98                                   1\n"
"  99                                  99\n"
"    100                                 100\n"
"    101                                 106\n"
"    102                                  25\n"
"    103                                 162\n"
"    104                                 154\n"
"    105                                  69\n"
"    106                                 137\n"
"    107                                 130\n"
"    108                                 108\n"
"    109                                  96\n"
"    110                                  36\n"
"    111                                 111\n"
"    112                                 123\n"
"    113                                 113\n"
"    114                                   5\n"
"    115                                 126\n"
"    116                                  46\n"
"    117                                  29\n"
"    118                                  67\n"
"    119                                 131\n"
"    120                                  50\n"
"    121                                  81\n"
" 122                                  35\n"
"    123                                  10\n"
"    124                                 124\n"
"    125                                 125\n"
"    126                                  78\n"
"    127                                 138\n"
"    128                                 122\n"
"    129                                 129\n"
"    130                                 127\n"
"    131                                 148\n"
"    132                                 103\n"
"    133                                 133\n"
"    134                                  89\n"
"    135                                  58\n"
"    136                                 146\n"
"    137                                   4\n"
"    138                                  14\n"
"    139                                 116\n"
"    140                                   8\n"
"    141                                 159\n"
"    142                                 161\n"
" 143                                 110\n"
"    144                                 143\n"
"    145                                  15\n"
"    146                                 156\n"
"    147                                 147\n"
"    148                                  41\n"
"    149                                  57\n"
"    150                                  65\n"
"    151                                  97\n"
"    152                                   7\n"
"    153                                  94\n"
"    154                                  24\n"
"    155                                   3\n"
"    156                                 150\n"
"    157                                  18\n"
"    158                                  91\n"
"    159                                  93\n"
"    160                                  63\n"
"    161                                 117\n"
"    162                                  64\n"
));

  NWayTree(Free)(t);
  struct mallinfo m = mallinfo();
  //say("Memory at end: %ld", m.uordblks);
  if (test) assert(memory_at_start == m.uordblks);                              // Confirm that there is no leakage
 }

void test_31x_insert163()
 {for(long i = 0; i < 40; ++i) test_31_insert163(0);
  test_31_insert163(1);
 }

void test_3_Find()
 {NWayTree_constLong(N, 63);

  NWayTree_new(t, 3);
  for(long i = 0; i < N;     ++i) NWayTree(Insert)(t, i*2, i*2);
  //NWayTree(ErrAsC)(t);
  for(long i =-1; i < 2 * N; ++i)
   {NWayTree(FindResult) r = NWayTree(Find)(t, i);
    //NWayTree(ErrFindResult)(r);
    NWayTree_FindResult_cmp(cmp, r);
    assert(i % 2 == 0 ? cmp == NWayTree(FindComparison_equal) :
                        cmp != NWayTree(FindComparison_equal));
    NWayTree_FindResult_index(ri, r);
    NWayTree_FindResult_node(rn, r);
    NWayTree_Node_keys(rk, rn, ri);
    NWayTree_FindComparison(l, lower);
    NWayTree_FindComparison(e, equal);
    NWayTree_FindComparison(h, higher);
    if (i == -1) assert(rk ==  0 && cmp == l && ri == 0);
    if (i ==  0) assert(rk ==  0 && cmp == e && ri == 0);
    if (i ==  1) assert(rk ==  0 && cmp == h && ri == 0);

    if (i == 11) assert(rk == 12 && cmp == l && ri == 0);
    if (i == 12) assert(rk == 12 && cmp == e && ri == 0);
    if (i == 13) assert(rk == 12 && cmp == h && ri == 0);
   }
 }

void test_reverse()                                                             // Create a tree in reverse order
 {long N = 121;

  NWayTree_new(t, 5);
  for(long i = N; i >= 0; --i)
   {NWayTree(Insert)(t, i, i);
   }
  //NWayTree(ErrAsC)(t);
  assert(iterateAndTestTree(t));
 }

void test_asm4()
 {NWayTree_new(t, 3);
  NWayTree(Insert)(t, 4, 4);
  NWayTree(Insert)(t, 2, 2);
  NWayTree(Insert)(t, 3, 3);
  NWayTree(Insert)(t, 1, 1);
//NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"      1                                   1\n"
"      2                                   2\n"
"   3                                   3\n"
"      4                                   4\n"
));
 }

void test_asm5()
 {NWayTree_new(t, 3);
  NWayTree(Insert)(t, 1, 1);
  NWayTree(Insert)(t, 4, 4);
  NWayTree(Insert)(t, 6, 6);
  NWayTree(Insert)(t, 2, 2);
  NWayTree(Insert)(t, 5, 5);
  NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"      1                                   1\n"
"      2                                   2\n"
"   4                                   4\n"
"      5                                   5\n"
"      6                                   6\n"
));
 }

void test_asm6()
 {NWayTree_new(t, 3);
  NWayTree(Insert)(t, 1, 1);
  NWayTree(Insert)(t, 4, 4);
  NWayTree(Insert)(t, 6, 6);
  NWayTree(Insert)(t, 2, 2);
  NWayTree(Insert)(t, 5, 5);
  NWayTree(Insert)(t, 3, 3);
 //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"      1                                   1\n"
"   2                                   2\n"
"      3                                   3\n"
"   4                                   4\n"
"      5                                   5\n"
"      6                                   6\n"
));
 }

void test_asm()
 {test_asm4();
  test_asm5();
  test_asm6();
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
 {test_3_0();
  test_3_1();
  test_3_2();
  test_3_3();
  test_3_4();
  test_3_Find();
  test_3_insert();
  test_3_iterate63();
 }

void tests31()                                                                  // Tests
 {test_31_1();
  test_31_2();
  test_31_insert163(0);
//test_31x_insert163();
 }

void testsReverse()                                                             // Tests
 {test_reverse();
 }

NWayTree(Tree) *test_3_iterate_load_n(int N)                                    // Tests
 {NWayTree_new(t, 3);
  for(long i = 0; i < N; ++i) NWayTree(Insert)(t, i, i);
  return t;
 }

void test_3_iterate_1()                                                         // Tests
 {NWayTree(Tree) *t = test_3_iterate_load_n(1);
  assert(iterateAndTestTree(t));
  NWayTree(Free)(t);
 }

void test_3_iterate_2()                                                         // Tests
 {NWayTree(Tree) *t = test_3_iterate_load_n(2);
  assert(iterateAndTestTree(t));
  NWayTree(Free)(t);
 }

void test_3_iterate_3()                                                         // Tests
 {NWayTree(Tree) *t = test_3_iterate_load_n(3);
  assert(iterateAndTestTree(t));
  NWayTree(Free)(t);
 }

void test_3_iterate_4()                                                         // Tests
 {NWayTree(Tree) *t = test_3_iterate_load_n(4);
  assert(iterateAndTestTree(t));
  NWayTree(Free)(t);
 }

void test_3_iterate_5()                                                         // Tests
 {NWayTree(Tree) *t = test_3_iterate_load_n(5);
  assert(iterateAndTestTree(t));
  NWayTree(Free)(t);
 }

void test_3_iterate_6()                                                         // Tests
 {NWayTree(Tree) *t = test_3_iterate_load_n(6);
  assert(iterateAndTestTree(t));
  NWayTree(Free)(t);
 }

void test_3_iterate_7()                                                         // Tests
 {NWayTree(Tree) *t = test_3_iterate_load_n(7);
  assert(iterateAndTestTree(t));
  NWayTree(Free)(t);
 }

void test_3_iterate_8()                                                         // Tests
 {NWayTree(Tree) *t = test_3_iterate_load_n(8);
  assert(iterateAndTestTree(t));
  NWayTree(Free)(t);
 }

void test_3_iterate()                                                           // Tests
 {test_3_iterate_1();
  test_3_iterate_2();
  test_3_iterate_3();
  test_3_iterate_4();
  test_3_iterate_5();
  test_3_iterate_6();
  test_3_iterate_7();
  test_3_iterate_8();
 }

void NWayTree(TraceBackHandler)(int sig)
 {void *array[99];
  size_t size = backtrace(array, 99);

  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array+6, size-6, STDERR_FILENO);
  signal(SIGABRT, 0);
  exit(1);
}

int main()                                                                      // Run tests
 {signal(SIGSEGV, NWayTree(TraceBackHandler));                                  // Trace back handler
  signal(SIGABRT, NWayTree(TraceBackHandler));
  test_asm();
  test_node_open();
  tests3 ();
  tests31();
  testsReverse();
  test_3_iterate();
  return 0;
 }
#endif
// (\A.{80})\s+(//.*\Z) \1\2
