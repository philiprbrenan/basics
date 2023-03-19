//------------------------------------------------------------------------------
// Generic N way tree
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// Inline  everything possible
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include <malloc.h>
#include "array/void.c"
#include "basics/basics.c"
#include "stack/char.c"

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#ifndef NWayTreeIterate
#define NWayTreeIterate(tree,     find) \
  for(NWayTree(FindResult)        find = NWayTree(IterStart)(tree); \
      NWayTree(IterCheck) (find); find = NWayTree(IterNext) (find))
#endif

#ifndef NWayTree_GetLong                                                        /* Create a long constant */
#define NWayTree_GetLong(l, code) const long l = (long)code;
#endif

#ifndef NWayTree_GetTree                                                        /* Create a constant reference to a tree */
#define NWayTree_GetTree(tree, code) NWayTree(Tree) * const tree = code;
#endif

#ifndef NWayTree_GetNode                                                        /* Create a constant reference to a node */
#define NWayTree_GetNode(node, code) NWayTree(Node) * const node = code;
#endif

#ifndef NWayTree_GetFindResult                                                  /* Create a constant find result */
#define NWayTree_GetFindResult(f, code) const NWayTree(FindResult) f = code;
#endif

#ifndef NWayTree_GetFindComparison                                              /* Create a constant find comparison result */
#define NWayTree_GetFindComparison(f, value) const NWayTree(FindComparison) f = NWayTree(FindComparison_##value)
#endif

//Optimize
//#define static                                                                /* Simplify debugging by preventing some inline-ing which invalidates the call stack */
const long size_of_element = sizeof(NWayTreeDataType);                          // The size of a key, data

//D1 Data structures
//D2 Node

typedef struct NWayTree(Node)                                                   // A node in a tree
 {long length;                                                                  // The current number of keys in the node
  long id;                                                                      // A number identifying this node within this tree
  struct NWayTree(Node) *up;                                                    // Parent node unless at the root node
  NWayTreeDataType *keys;                                                       // Keys associated with this node
  NWayTreeDataType *data;                                                       // Data associated withe each key associated with this node
  struct NWayTree(Node) **down;                                                 // Next layer of nodes down from this node
  struct NWayTree(Tree) *tree;                                                  // The definition of the containing tree
 } NWayTree(Node);

inline static long NWayTree(Node_length)                                        // Get length field from node
 (NWayTree(Node) *node)                                                         // Node
 {return node->length;
 }

inline static long NWayTree(Node_setLength)                                     // Set length field from node
 (NWayTree(Node) *node,                                                         // Node
  long const      length)                                                       // New length
 {return node->length = length;
 }

inline static long NWayTree(Node_id)                                            // Get id field from node
 (NWayTree(Node) *node)                                                         // Node
 {return node->id;
 }

inline static struct NWayTree(Node) * NWayTree(Node_up)                         // Get up field from node
 (NWayTree(Node) *node)                                                         // Node
 {return node->up;
 }

inline static NWayTree(Node) * NWayTree(Node_setUp)                             // Set up field from node
 (NWayTree(Node) *node,                                                         // Node
  NWayTree(Node) *up)                                                           // Upper node
 {return node->up = up;
 }

inline static NWayTreeDataType NWayTree(Node_keys)                              // Get the indicated key from the specified node
 (NWayTree(Node) *node,                                                         // Node
  long const      index)                                                        // Index
 {return node->keys[index];
 }

inline static NWayTreeDataType NWayTree(Node_setKeys)                           // Set the indicated key in specified node
 (NWayTree(Node)        *node,                                                  // Node,
  const long             index,                                                 // Index
  const NWayTreeDataType key)                                                   // Key
 {return node->keys[index] = key;
 }

inline static NWayTreeDataType NWayTree(Node_data)                              // Get the indicated data from the specified node
 (NWayTree(Node) *node,                                                         // Node
  long const      index)                                                        // Index
 {return node->data[index];
 }

inline static NWayTreeDataType NWayTree(Node_setData)                           // Set the indicated data in specified node
 (NWayTree(Node)        *node,                                                  // Node
  const long             index,                                                 // Index
  const NWayTreeDataType data)                                                  // Data
 {return node->data[index] = data;
 }

inline static NWayTree(Node) * NWayTree(Node_down)                              // Get the indicated down from the specified node
 (NWayTree(Node) *node,                                                         // Node
  const long      index)                                                        // Index
 {return node->down[index];
 }

inline static NWayTree(Node) * NWayTree(Node_setDown)                           // Set the indicated down in specified node
 (NWayTree(Node) * node,                                                        // Node
  const long       index,                                                       // Index
  NWayTree(Node) * child)                                                       // Child node
 {return node->down[index] = child;
 }

inline static struct NWayTree(Tree) * NWayTree(Node_tree)                       // Get the tree associated with a node
 (NWayTree(Node) *node)                                                         // Node
 {return node->tree;
 }

//D2 Tree

typedef struct NWayTree(Tree)                                                   // The root of a tree
 {long NumberOfKeysPerNode;                                                     // Size of a node
  NWayTree(Node) *node;                                                         // Root node
  long keys;                                                                    // Number of keys in tree
  long nodes;                                                                   // Number of nodes in tree
  NWayTree(Node) *node_array;                                                   // Array of nodes if tree has been compacted
 } NWayTree(Tree);

inline static long NWayTree(Tree_NumberOfKeysPerNode)                           // Number of keys per node in this tree
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return tree->NumberOfKeysPerNode;
 }

inline static struct NWayTree(Node) * NWayTree(Tree_node)                       // Root node of a tree
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return tree->node;
 }

inline static NWayTree(Node) * NWayTree(Tree_setNode)                           // Set the root node for a tree
 (NWayTree(Tree) * const tree,                                                  // Tree
  NWayTree(Node) * const node)                                                  // Root node
 {return tree->node = node;
 }

inline static long NWayTree(Tree_keys)                                          // Number of keys in a tree
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return tree->keys;
 }

inline static long NWayTree(Tree_setKeys)                                       // Set the number of keys in a tree
 (NWayTree(Tree) * const tree,                                                  // Tree
  const long             keys)                                                  // Number of keys
 {return tree->keys = keys;
 }

inline static long NWayTree(Tree_incKeys)                                       // Increment the number of keys in a tree
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return ++tree->keys;
 }

inline static long NWayTree(Tree_nodes)                                         // Number of nodes in a tree
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return tree->nodes;
 }

inline static long NWayTree(Tree_setNodes)                                      // Set number of nodes in a tree
 (NWayTree(Tree) * const tree,                                                  // Tree
  const long             nodes)                                                 // Number of nodes
 {return tree->nodes = nodes;
 }

inline static long NWayTree(Tree_incNodes)                                      // Increment the number of nodes in a tree
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return ++tree->nodes;
 }

static long NWayTree(MinimumNumberOfKeys)                                       //P Minimum number of keys per node.
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return (NWayTree(Tree_NumberOfKeysPerNode)(tree) - 1) << 1;
 }

static long NWayTree(MaximumNumberOfKeys)                                       //P Maximum number of keys per node.
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return NWayTree(Tree_NumberOfKeysPerNode)(tree);
 }

static long NWayTreeLong NWayTree(MaximumNumberDownPerNode)                     //P Maximum number of children per parent.
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return NWayTree(Tree_NumberOfKeysPerNode)(tree) + 1;
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

static NWayTree(FindResult) NWayTree(NewFindResult)                             //P New find result on stack
 (NWayTree(Node) *         const node,                                          // Node
  NWayTreeDataType         const key,                                           // Search key
  NWayTree(FindComparison) const cmp,                                           // Comparison result
  long                     const index)                                         // Index in node of nearest key
 {NWayTree(FindResult) r;
  r.node  = node;
  r.key   = key;
  r.cmp   = cmp;
  r.index = index;
  return r;
 }

inline static struct NWayTree(Node) * NWayTree(FindResult_node)                 // Get node field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {return f.node;
 }

inline static NWayTreeDataType NWayTree(FindResult_key)                         // Get key field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {return f.key;
 }

inline static long NWayTree(FindResult_cmp)                                     // Get comaprison field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {return f.cmp;
 }

inline static NWayTreeDataType NWayTree(FindResult_index)                       // Get index field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {return f.index;
 }

inline static NWayTreeDataType NWayTree(FindResult_data)                        // Get data field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {NWayTree_GetNode(n, NWayTree(FindResult_node)(f));
  NWayTree_GetLong(i, NWayTree(FindResult_index)(f));
  return NWayTree(Node_data)(n, i);
 }

//Forward declarations

static void NWayTree(ErrNode)       (NWayTree(Node) *node);
static long NWayTree(CheckNode)     (NWayTree(Node) *node, char *name);
static void NWayTree(CheckTree)     (NWayTree(Tree) *tree, char *name);
static long NWayTree(IsLeaf)        (NWayTree(Node) *node);
static long NWayTree(IndexInParent) (NWayTree(Node) * const node);

//D1 Construction
//D2 Tree

static NWayTree(Tree) *NWayTree(NewTree)                                        // Create a new tree
 (const long n)                                                                 // Number of keys in a node
 {NWayTree_GetTree(tree, calloc(sizeof(NWayTree(Tree)), 1));
  tree->NumberOfKeysPerNode = n;
  return tree;
 }

//D2 Node

static NWayTree(Node) *NWayTree(NewNode)                                        //P Create a new node
 (NWayTree(Tree) * const tree)                                                  // Tree containing node
 {NWayTree_GetLong(z, NWayTree(Tree_NumberOfKeysPerNode)(tree));
  NWayTree_GetLong(k, size_of_element                 *  z);
  NWayTree_GetLong(d, size_of_element                 *  z);
  NWayTree_GetLong(n, sizeof(struct NWayTree(Node) *) * (z + 1));
  NWayTree_GetLong(s, sizeof(NWayTree(Node)));

  NWayTree_GetNode(node, calloc(s+k+d+n, 1));
  node->keys = (void *)node+s;
  node->data = (void *)node+s+k;
  node->down = (void *)node+s+k+d;
  node->tree = tree;
  node->id   = NWayTree(Tree_incNodes)(tree);
  return node;
 }

static long NWayTree(SizeOfNode)                                                //P The size of a node in a tree
 (NWayTree(Tree) *tree)                                                         // Tree
 {NWayTree_GetLong(z, NWayTree(Tree_NumberOfKeysPerNode)(tree));
  NWayTree_GetLong(k, size_of_element                 *  z);
  NWayTree_GetLong(d, size_of_element                 *  z);
  NWayTree_GetLong(n, sizeof(struct NWayTree(Node) *) * (z + 1));
  NWayTree_GetLong(s, sizeof(NWayTree(Node)));
  return s+k+d+n;
 }

static void NWayTree(ReUp)                                                      //P Reconnect the children to their new parent.
 (NWayTree(Node) * const node)                                                  // Node to reconnect
 {NWayTree_GetLong(nl, NWayTree(Node_length)(node));
  for(long i = 0; i <= nl; ++i)
   {NWayTree_GetNode(d, NWayTree(Node_down)(node, i));                          // No children so it must be a leaf
    NWayTree(Node_setUp)(d, node);
   }
 }

//D1 Destruction
//D2 Node

static void NWayTree(FreeNode)                                                  //P Free a node, Wipe the node so it cannot be accidently reused
 (NWayTree(Node) * const node)                                                  // Node to free
 {NWayTree_GetTree(t, NWayTree(Node_tree)(node));
  NWayTree_GetLong(w, NWayTree(SizeOfNode)(t));
  memset(node, -1, w);                                                          // Clear node to hinder accidental use
  free(node);
 }

//D2 Tree

static void NWayTree(Free2)                                                     //P Free a node in a tree
 (NWayTree(Node) * const node)
 {if (!node) return;
  NWayTree_GetLong(nl, NWayTree(Node_length)(node));
  if (nl)
   {if (!NWayTree(IsLeaf)(node))
     {NWayTree_GetNode(n, NWayTree(Node_down)(node, 0));
      NWayTree(Free2)(n);
      for(long i = 1; i <= nl; ++i)
       {NWayTree_GetNode(n, NWayTree(Node_down)(node, i));
        NWayTree(Free2)(n);
       }
     }
   }
  NWayTree(FreeNode)(node);
 }

static void NWayTree(Free)                                                      // Free a tree
 (NWayTree(Tree) * const tree)
 {if (!tree) return;
  NWayTree(Free2)(NWayTree(Tree_node)(tree));
  memset(tree, -1, sizeof(*tree));
  free(tree);
 }

//D1 Print

static void NWayTree(ToString2)                                                 //P Print the keys in a tree
 (NWayTree(Node) * const node,                                                  // Node to print
  long             const in,                                                    // Indentation
  StackChar      * const p)                                                     // Stack as a string
 {if (!node) return;
  NWayTree_GetLong(nl, NWayTree(Node_length)(node));
  if (!nl) return;
  NWayTree_GetNode(n, NWayTree(Node_down)(node, 0));
  NWayTree(ToString2)(n, in+1, p);
  for(long i = 0; i < nl; ++i)
   {for(long j = 0; j < in * 3; ++j) StackCharPush(p, ' ');
    char C[100];
    NWayTree_GetLong(k, NWayTree(Node_keys)(node, i));
    NWayTree_GetLong(d, NWayTree(Node_data)(node, i));
    sprintf(C, "%4ld                                %4ld\n", k, d);
    StackCharPushString(p, C);
    NWayTree_GetNode(n, NWayTree(Node_down)(node, i+1));
    NWayTree(ToString2)(n, in+1, p);
   }
 }

static StackChar *NWayTree(ToString)                                            // Print a tree as a string
 (NWayTree(Tree) * const tree)                                                  // Tree to print as a string
 {StackChar * const p = StackCharNew();
  NWayTree_GetNode(n, NWayTree(Tree_node)(tree));
  if (n) NWayTree(ToString2)(n, 0, p);
  return p;
 }

static void NWayTree(PrintErr)                                                  // Print a tree on stderr
 (NWayTree(Tree) * const tree)                                                  // Tree to print
 {StackChar * const s = NWayTree(ToString)(tree);
  StackCharErr(s);
  StackCharFree(s);
 }

static void NWayTree(ToStringWithId2)                                           //P Print the keys in a tree adding the id of each node in the tree and the index of the key within that node
 (NWayTree(Node) * const node,                                                  // Node
  const long             in,                                                    // Indentation
  StackChar      * const p)                                                     // Stack as string
 {NWayTree_GetLong(nl, NWayTree(Node_length)(node));
  if (!node || !nl) return;
  NWayTree_GetNode(n, NWayTree(Node_down)(node, 0));
  NWayTree(ToStringWithId2)(n, in+1, p);
  for(long i = 0; i < nl; ++i)
   {for(long j = 0; j < in; ++j) StackCharPushString(p, "   ");
    char C[100];
    NWayTree_GetLong(k, NWayTree(Node_keys)(node, i));
    sprintf(C, "%4ld", k);
    StackCharPushString(p, C);

    for(long j = 0; j < 10-in; ++j) StackCharPushString(p, "   ");
    char D[100];
    NWayTree_GetLong(di, NWayTree(Node_data)(node, i));
    NWayTree_GetLong(id, NWayTree(Node_id)  (node));
    sprintf(D, "%4ld %4ld %4ld  %p/%4ld=", di, id, i, node, nl);
    sprintf(D, "%4ld %4ld %4ld  %p/%4ld=", di, id, i, node, nl);
    sprintf(D, "%4ld %4ld %4ld  %p/%4ld=", di, id, i, node, nl);
    StackCharPushString(p, D);
    for(long j = 0; j <= nl; ++j)
     {NWayTree_GetNode(d, NWayTree(Node_down)(node, j));
      if (d)
       {NWayTree_GetLong(id, NWayTree(Node_id)(d));
        sprintf(D, " %4ld", id);
       }
      else
       {sprintf(D, " %4ld", 0l);
       }
      StackCharPushString(p, D);
     }
    StackCharPushString(p, "\n");
    NWayTree_GetNode(d, NWayTree(Node_down)(node, i+1));
    NWayTree(ToStringWithId2)(d, in+1, p);
   }
 }

static StackChar *NWayTree(ToStringWithId)                                      //P Print the keys in a tree adding the id of each node in the tree and the index of the key within that node
 (NWayTree(Tree) * const tree)                                                  // Tree to print as a string
 {StackChar * const p = StackCharNew();
  StackCharPushString(p, "                                     Data Node Index Children\n");
  NWayTree_GetNode(n, NWayTree(Tree_node)(tree));
  if (n) NWayTree(ToStringWithId2)(n, 0, p);
  return p;
 }

static void NWayTree(PrintErrWithId)                                            // Print the keys in a tree adding the id of each node in the tree and the index of the key within that node
 (NWayTree(Tree) * const tree)                                                  // Tree to print
 {StackChar * const s = NWayTree(ToStringWithId)(tree);
  StackCharErr(s);
  StackCharFree(s);
 }

static void NWayTree(ErrAsC)                                                    // Print a tree as C strings on stderr
 (NWayTree(Tree) * const tree)                                                  // Tree to print
 {StackChar * const s = NWayTree(ToString)(tree);
  NWayTree_GetLong(N, s->next-s->base);                                         // The number of characters to print
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

static void NWayTree(ErrNode)                                                   // Dump a node
 (NWayTree(Node) * const node)                                                  // Node
 {say("Node at  %p", node);
  NWayTree_GetNode(u, NWayTree(Node_up)(node));
  say("  Up     = %p", u);
  NWayTree_GetLong(nl, NWayTree(Node_length)(node));
  say("  Length = %ld", nl);
  fprintf(stderr, "  Keys   : ");
  for(long i = 0; i <  nl; ++i)
   {NWayTree_GetLong(k, NWayTree(Node_keys)(node, i));
    fprintf(stderr," %ld", k);
   }
  fprintf(stderr, "\n  Data   : ");
  for(long i = 0; i <  nl; ++i)
   {NWayTree_GetLong(d, NWayTree(Node_data)(node, i));
    fprintf(stderr," %ld", d);
   }
  fprintf(stderr, "\n  Down   : ");
  for(long i = 0; i <= nl; ++i)
   {NWayTree_GetNode(n, NWayTree(Node_down)(node, i));
    fprintf(stderr," %p",  n);
   }
  say("\n");
 }

static long NWayTree(EqText)                                                    // Check whether the text representing a tree is the same as the specified text
 (NWayTree(Tree) * const tree,                                                  // Tree to dump
  char           * const text)                                                  // Expected text
 {StackChar * const s = NWayTree(ToString)(tree);
  NWayTree_GetLong(r, strncmp(s->arena+s->base, text, s->next-s->base) == 0);
  StackCharFree(s);
  return r;
 }

static void NWayTree(ErrFindResult)                                             // Print a find result
 (NWayTree(FindResult) const r)                                                 // Find result
 {char *c;
  switch(NWayTree(FindResult_cmp(r)))
   {case NWayTree(FindComparison_equal):  c = "equal";    break;
    case NWayTree(FindComparison_lower):  c = "lower";    break;
    case NWayTree(FindComparison_higher): c = "higher";   break;
    default:                              c = "notFound"; break;
   }

  NWayTree_GetLong(ri, NWayTree(FindResult_index)(r));
  NWayTree_GetNode(n,  NWayTree(FindResult_node)(r));
  const NWayTreeDataType k = NWayTree(Node_keys)(n, ri);
  const NWayTreeDataType K = NWayTree(FindResult_key)(r);

  say("Find key=%ld Result keys[index]=%ld %s  index=%ld", K, k, c, ri);
 }

//D1 Properties

static long NWayTree(Full)                                                      //P Confirm that a node is full.
 (NWayTree(Node) * const node)
 {NWayTree_GetTree(t, NWayTree(Node_tree)(node));
  return NWayTree(Node_length)(node) == NWayTree(MaximumNumberOfKeys)(t);
 }

static long NWayTree(HalfFull)                                                  //P Confirm that a node is half full.
 (NWayTree(Node) * const node)                                                  // Node
 {NWayTree_GetLong(n, NWayTree(Node_length)(node));
  NWayTree(Tree)* const t = NWayTree(Node_tree)(node);
  assert(n <= NWayTree(MaximumNumberOfKeys)(t)+1);
  return n == NWayTree(MinimumNumberOfKeys)(t);
 }

static long NWayTree(IsLeaf)                                                    //P Confirm that the tree is a leaf.
 (NWayTree(Node) * const node)                                                  // Node to test
 {NWayTree_GetNode(d, NWayTree(Node_down)(node, 0));                            // No children so it must be a leaf
  return d == 0;                                                                // No children so it must be a leaf
 }

static long NWayTree(IndexInParent)                                             //P Get the index of a node in its parent.
 (NWayTree(Node) * const node)                                                  // Node to locate in parent
 {NWayTree_GetNode(p, NWayTree(Node_up)(node));
  assert(p);
  NWayTree_GetLong(pl, NWayTree(Node_length)(p));
  for(long i = 0; i <= pl; ++i)
   {if (NWayTree(Node_down)(p, i) == node) return i;
   }
  assert(0);
 }

//D1 Integrity Checking

static long NWayTree(CheckNode)                                                 //P Check the connections to and from a node
 (NWayTree(Node) * const node,                                                  // Node
  char           * const name)                                                  // Name of check
 {NWayTree_GetLong(nl, NWayTree(Node_length)(node));
  NWayTree_GetTree(t, NWayTree(Node_tree)(node));

  if (nl > NWayTree(MaximumNumberOfKeys)(t))
   {const NWayTreeDataType k = NWayTree(Node_keys)(node, 0);
    say("%s: Node %ld is too long at %ld", name, k, nl);
    return 1;
   }

  for(long i = 0; i <= nl; ++i)                                                 // Check that each child has a correct up reference
   {NWayTree_GetNode(d, NWayTree(Node_down)(node, i));                          // Step down
    if (d)
     {NWayTree_GetLong(dl, NWayTree(Node_length)(d));
      if (dl > NWayTree(MaximumNumberOfKeys)(t))
       {NWayTree_GetLong(nk, NWayTree(Node_keys)(node, 0));
        NWayTree_GetLong(dk, NWayTree(Node_keys)(d,    0));
        say("%s: Node %ld under %ld is too long at %ld", name, nk, dk, dl);
        return 2;
       }

      NWayTree_GetNode(u, NWayTree(Node_up)(d));

      if (u != node)
       {const NWayTreeDataType d0 = NWayTree(Node_keys)(d,    0);
        const NWayTreeDataType n0 = NWayTree(Node_keys)(node, 0);
        const NWayTreeDataType u0 = NWayTree(Node_keys)(u,    0);

        say("%s: Node %ld(%p) under %ld(%p) has wrong up pointer %ld(%p)",
             name, d0, d, n0, node, u0, u);
        return 3;
       }
     }
   }

  NWayTree_GetNode(p, NWayTree(Node_up)(node));                                 // Check that parent connects to the current node
  if (p)
   {NWayTree_GetLong(pl, NWayTree(Node_length)(p));
    NWayTree_GetTree(t, NWayTree(Node_tree)(node));                       // Check that parent connects to the current node
    NWayTree_GetLong(m, NWayTree(MaximumNumberOfKeys)(t));
    assert(pl <= m);
    int c = 0;                                                                  // Check that the parent has a down pointer to the current node
    for(long i = 0; i <= pl; ++i)
     {NWayTree_GetNode(d, NWayTree(Node_down)(p, i));
      if (d == node) ++c;                                                       // Find the node that points from the parent to the current node
     }
    if (c != 1)                                                                 // We must be able to find the child
     {NWayTree_GetLong(nk, NWayTree(Node_keys)(node, 0));
      NWayTree_GetLong(pk, NWayTree(Node_keys)(p,    0));
      say("%s: Node %ld has parent %ld that fails to refer back to it",
           name, nk, pk);
      return 4;
     }
   }
  return 0;
 }

static void NWayTree(CheckTree2)                                                //P Check the structure of a tree
 (NWayTree(Node) * const node,                                                  // Node to check
  char           * const name)                                                  // Name of check
 {if (!node) return;

  if (NWayTree(CheckNode)(node, name))
   {NWayTree(PrintErr)(NWayTree(Node_tree)(node));
    assert(0);
   }

  NWayTree_GetNode(d, NWayTree(Node_down)(node, 0));
  NWayTree(CheckTree2)(d, name);
  NWayTree_GetLong(nl, NWayTree(Node_length)(node));
  for(long i = 0; i < nl; ++i)
   {NWayTree_GetNode(d, NWayTree(Node_down)(node, i+1));
    NWayTree(CheckTree2)(d, name);
   }
 }

static void NWayTree(CheckTree)                                                 //P Check the structure of a tree
 (NWayTree(Tree) * const tree,                                                  // Node to check
  char           * const name)                                                  // Name of check
 {NWayTree_GetNode(n, NWayTree(Tree_node)(tree));
  NWayTree(CheckTree2)(n, name);
 }

//D1 Splitting

static long NWayTree(SplitFullNode)                                             //P Split a node if it is full. Return true if the node was split else false
 (NWayTree(Node) * const node)
 {NWayTree_GetLong(nl, NWayTree(Node_length)(node));

  NWayTree_GetTree(t, NWayTree(Node_tree)(node));                         // Associated tree
  NWayTree_GetLong(m, NWayTree(MaximumNumberOfKeys)(t));
  if (nl < m)                                                                   // Must be a full node
   {return 0;
   }

  NWayTree_GetNode(l, NWayTree(NewNode)(t));                                    // New child nodes
  NWayTree_GetNode(r, NWayTree(NewNode)(t));

  NWayTree_GetLong(N, NWayTree(MaximumNumberOfKeys)(t));                        // Split points
  NWayTree_GetLong(n, N>>1);                                                    // Index of key that will be placed in parent

  NWayTree_GetLong(L, NWayTree(Node_setLength)(l, n));
  NWayTree_GetLong(R, NWayTree(Node_setLength)(r, N - n - 1));

  NWayTree_GetLong(LL, L * size_of_element);
  NWayTree_GetLong(RR, R * size_of_element);

  memcpy(l->keys, node->keys,        LL);                                       // Split left keys and data
  memcpy(l->data, node->data,        LL);
  memcpy(l->down, node->down,     (1+L) * sizeof(NWayTree(Node) *));

  memcpy(r->keys, node->keys+n+1,    RR);                                       // Split right keys and data
  memcpy(r->data, node->data+n+1,    RR);
  memcpy(r->down, node->down+n+1, (1+R) * sizeof(NWayTree(Node) *));

  if (!NWayTree(IsLeaf)(node))                                                  // Not a leaf node
   {NWayTree(ReUp)(l);
    NWayTree(ReUp)(r);
   }

  NWayTree_GetNode(p, NWayTree(Node_up)(node));                                 // Existing parent node
  if (p)                                                                        // Not a root node
   {NWayTree_GetLong(pl, NWayTree(Node_length)(p));
    NWayTree(Node_setUp)(l, p);                                                 // Connect children to parent
    NWayTree(Node_setUp)(r, p);
    NWayTree_GetNode(d, NWayTree(Node_down)(p, 0));

    if (d == node)                                                              // Splitting the first child - move everything up
     {NWayTree_GetLong(w, NWayTree(Node_setLength)(p, pl+1));
      memmove(p->keys+1, p->keys,  pl * size_of_element);
      memmove(p->data+1, p->data,  pl * size_of_element);
      memmove(p->down+1, p->down,   w * size_of_element);
      NWayTree_GetLong(nk, NWayTree(Node_keys)(node, n));
      NWayTree_GetLong(nd, NWayTree(Node_data)(node, n));
      NWayTree(Node_setKeys)(p, 0, nk);
      NWayTree(Node_setData)(p, 0, nd);
      NWayTree(Node_setDown)(p, 0, l);
      NWayTree(Node_setDown)(p, 1, r);
      NWayTree(FreeNode)(node);
      return 1;
     }

    NWayTree_GetNode(pd, NWayTree(Node_down)(p, pl));
    if (pd == node)                                                             // Splitting the last child - just add it on the end
     {NWayTree_GetLong(pk, NWayTree(Node_keys)(node, n));
      NWayTree_GetLong(pd, NWayTree(Node_data)(node, n));
      NWayTree(Node_setKeys)  (p, pl, pk);
      NWayTree(Node_setData)  (p, pl, pd);
      NWayTree(Node_setDown)  (p, pl, l);
      NWayTree_GetLong(nd, NWayTree(Node_data)(node, n));
      NWayTree(Node_setData)  (p, pl, nd);
      NWayTree(Node_setDown)  (p, pl, l);
      NWayTree(Node_setLength)(p, pl+1);
      NWayTree(Node_setDown)  (p, pl+1, r);
      NWayTree(FreeNode)      (node);
      return 1;
     }

    for(long i = 1; i < pl; ++i)                                                // Splitting a middle child:
     {if (NWayTree(Node_down)(p, i) == node)                                    // Find the node that points from the parent to the current node
       {memmove(p->keys+i+1, p->keys+i, (pl-i)   * size_of_element);
        memmove(p->data+i+1, p->data+i, (pl-i)   * size_of_element);
        memmove(p->down+i+1, p->down+i, (pl-i+1) * size_of_element);
        NWayTree_GetLong(pk, NWayTree(Node_keys)(node, n));
        NWayTree_GetLong(pd, NWayTree(Node_data)(node, n));
        NWayTree(Node_setKeys)  (p, i,  pk);
        NWayTree(Node_setData)  (p, i,  pd);
        NWayTree(Node_setDown)  (p, i,   l);
        NWayTree(Node_setDown)  (p, i+1, r);
        NWayTree(Node_setLength)(p, pl+1);
        NWayTree(FreeNode)(node);
        return 1;
       }
     }
    assert(0);                                                                  // Could not find the child in the parent
   }

  NWayTree(Node_setUp)(l, node);                                                // Root node with single key after split
  NWayTree(Node_setUp)(r, node);                                                // Connect children to parent

  NWayTree_GetLong(pk, NWayTree(Node_keys)(node, n));                           // Single key
  NWayTree_GetLong(pd, NWayTree(Node_data)(node, n));                           // Data associated with single key
  NWayTree(Node_setKeys)  (node, 0, pk);
  NWayTree(Node_setData)  (node, 0, pd);
  NWayTree(Node_setDown)  (node, 0, l);
  NWayTree(Node_setDown)  (node, 1, r);
  NWayTree(Node_setLength)(node, 1);
  return 1;
 }

static NWayTree(FindResult) NWayTree(FindAndSplit)                              //P Find a key in a tree splitting full nodes along the path to the key.
 (NWayTree(Tree) * const tree,                                                  // Tree to search
  NWayTreeDataType const key)                                                   // Key to locate
 {NWayTree(Node) * node = NWayTree(Tree_node)(tree);

  if (NWayTree(SplitFullNode)(node))                                            // Split the root node if necessary
   {node = NWayTree(Tree_node)(tree);
   }

  for(long j = 0; j < NWayTree(MaxIterations); ++j)                             // Step down through the tree
   {NWayTree_GetLong(nl, NWayTree(Node_length)(node));                          // Length of node
    NWayTree_GetLong(last, nl-1);                                               // Greater than largest key in node. Data often gets inserted in ascending order so we do this check first rather than last.
    if (key > (NWayTree(Node_keys)(node, last)))                                // Leaf
     {if (NWayTree(IsLeaf)(node))
       {NWayTree_GetFindComparison(h, higher);
        return NWayTree(NewFindResult)(node, key, h, last);
       }
      NWayTree_GetNode(n, NWayTree(Node_down)(node, last+1));
      NWayTree_GetLong(s, NWayTree(SplitFullNode)(n));                          // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
      if (!s)                                                                   // No split needed
       {node = n;
       }
      continue;
     }

    for(long i = 0; i < nl; ++i)                                                // Search the keys in this node as greater than least key and less than largest key
     {const NWayTreeDataType k = NWayTree(Node_keys)(node, i);                  // Current key
      if (key == k)                                                             // Found key
       {NWayTree_GetFindComparison(e, equal);
        return NWayTree(NewFindResult)(node, key, e, i);
       }
      if (key < k)                                                              // Greater than current key
       {if (NWayTree(IsLeaf)(node))
         {NWayTree_GetFindComparison(l, lower);
          return NWayTree(NewFindResult)(node, key, l, i);
         }
        NWayTree_GetNode(n, NWayTree(Node_down)(node, i));
        NWayTree_GetLong(s, NWayTree(SplitFullNode)(node));                     // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
        if (s)
         {node = NWayTree(Node_up)(n);
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

static void NWayTree(FillFromLeftOrRight)                                       //P Fill a node from the specified sibling. Not called by find or insert.
 (NWayTree(Node) * const n,                                                     // Node to fill
  const long dir)                                                               // Direction to fill from
 {NWayTree_GetNode(p, NWayTree(Node_up)(n));                                    // Parent of leaf
  assert(p);
  NWayTree_GetLong(i,  NWayTree(IndexInParent)(n));                             // Index of leaf in parent
  NWayTree_GetLong(pl, NWayTree(Node_length)(p));
  NWayTree_GetLong(nl, NWayTree(Node_length)(n));

  if (dir)                                                                      // Fill from right
   {assert(i < pl);                                                             // Cannot fill from right
    NWayTree_GetNode(r, NWayTree(Node_down)(p, i+1));                           // Right sibling
    NWayTree_GetLong(rl, NWayTree(Node_length)(r));
    NWayTree_GetLong(pd, NWayTree(Node_keys)(p, i));                            // Transfer key and data to parent
    NWayTree_GetLong(pk, NWayTree(Node_data)(p, i));
    NWayTree(Node_setKeys)(n, nl, pk);                                          // Transfer key and data to parent
    NWayTree(Node_setData)(n, nl, pd);

    NWayTree_GetLong(rk, ArrayLongShift(r->keys, rl));                          // Transfer keys and data from right
    NWayTree_GetLong(rd, ArrayLongShift(r->data, rl));
    NWayTree(Node_setKeys)(p, i, rk);
    NWayTree(Node_setData)(p, i, rd);

    if (!NWayTree(IsLeaf)(n))                                                   // Transfer node if not a leaf
     {void * const rd = ArrayVoidShift((void *)r->down, rl);
      ArrayVoidPush((void *)n->down, nl, rd);
      NWayTree(Node) * const d = NWayTree(Node_down)(n, nl+1);
      NWayTree(Node_setUp)(d, n);
     }
    NWayTree(Node_setLength)(r, rl-1);
    NWayTree(Node_setLength)(n, nl+1);
   }
  else                                                                          // Fill from left - untested
   {assert(i);                                                                  // Cannot fill from left
    NWayTree_GetLong(I, i-1);
    NWayTree_GetNode(l,  NWayTree(Node_down)(p, I));                            // Left sibling
    NWayTree_GetLong(ll, NWayTree(Node_length)(l));

    NWayTree_GetLong(pk, NWayTree(Node_keys)(p, I));                            // Shift in keys and data from left
    NWayTree_GetLong(pd, NWayTree(Node_data)(p, I));

    ArrayLongUnShift(l->keys, ll, pk);                                          // Shift in keys and data from left
    ArrayLongUnShift(l->data, ll, pd);

    NWayTree_GetLong(lk, ArrayLongPop(l->keys, ll));                            // Transfer key and data to parent
    NWayTree_GetLong(ld, ArrayLongPop(l->data, ll));
    NWayTree(Node_setKeys)(p, I, lk);                                           // Transfer key and data to parent
    NWayTree(Node_setData)(p, I, ld);
    if (!NWayTree(IsLeaf)(l))                                                   // Transfer node if not a leaf
     {void * ld = ArrayVoidPop((void *)l->down, ll);
      ArrayVoidUnShift(        (void *)l->down, ll, ld);

      NWayTree_GetNode(d, NWayTree(Node_down)(l, 0));
      NWayTree(Node_setUp)(d, l);
     }
   }
 }

static void NWayTree(MergeWithLeftOrRight)                                      //P Merge two adjacent nodes.  Not called by find or insert.
 (NWayTree(Node) * const n,                                                     // Node to fill
  const long             dir)                                                   // Direction to fill from
 {assert(NWayTree(HalfFull)(n));                                                // Confirm leaf is half full
  NWayTree_GetNode(p, NWayTree(Node_up)(n));                                    // Parent of leaf
  assert(p);
  NWayTree_GetLong(hf, NWayTree(HalfFull)(p));                                  // Parent must have more than the minimum number of keys because we need to remove one unless it is the root of the tree
  assert(hf);
  NWayTree_GetNode(P, NWayTree(Node_up)(p));                                    // Check that we are not on the root node
  assert(P);

  NWayTree_GetLong(i, NWayTree(IndexInParent)(n));                              // Index of leaf in parent
  NWayTree_GetLong(pl, NWayTree(Node_length)  (p));
  NWayTree_GetLong(nl, NWayTree(Node_length)  (n));

  if (dir)                                                                      // Merge with right hand sibling
   {assert(i < pl);                                                             // Cannot fill from right
    NWayTree_GetLong(I, i+1);
    NWayTree_GetNode(r, NWayTree(Node_down)(p, I));                             // Leaf on right
    assert(NWayTree(HalfFull)(r));                                              // Confirm right leaf is half full
    NWayTree_GetLong(rl, NWayTree(Node_length)(r));

    const NWayTreeDataType k = ArrayLongDelete(p->keys, pl, I);                 // Transfer keys and data from parent
    const NWayTreeDataType d = ArrayLongDelete(p->data, pl, I);
    ArrayLongPush(n->keys, nl, k);
    ArrayLongPush(n->data, nl, d);

    ArrayLongPushArray(n->keys, nl+1, r->keys, rl);                             // Transfer keys
    ArrayLongPushArray(n->data, nl+1, r->data, rl);                             // Transfer data

    if (!NWayTree(IsLeaf)(n))                                                   // Children of merged node
     {ArrayVoidPushArray((void *)n->down, nl,(void *)r->down, rl);
      NWayTree(ReUp)(n);                                                        // Update parent of children of right node
     }
    ArrayVoidDelete((void *)p->down, pl, I);                                    // Remove link from parent to right child
    NWayTree(Node_setLength)(n, nl + rl + 1);
    NWayTree(Node_setLength)(p, pl      - 1);
    NWayTree(FreeNode)(r);
   }
  else                                                                          // Merge with left hand sibling
   {assert(i > 0);                                                              // Cannot fill from left
    NWayTree_GetLong(I, i-1);
    NWayTree_GetNode(l, NWayTree(Node_down)(p, I));                             // Node on left
    assert(NWayTree(HalfFull)(l));                                              // Confirm left leaf is half full
    NWayTree_GetLong(ll, NWayTree(Node_length)(l));
    const NWayTreeDataType k = ArrayLongDelete(p->keys, pl, I);                 // Transfer parent key and data
    const NWayTreeDataType d = ArrayLongDelete(p->data, pl, I);
    ArrayLongUnShift     (n->keys, nl,   k);
    ArrayLongUnShift     (n->data, nl,   d);
    ArrayLongUnShiftArray(n->keys, nl+1, l->keys, ll);                          // Transfer left keys and data
    ArrayLongUnShiftArray(n->data, nl+1, l->data, ll);

    if (!NWayTree(IsLeaf)(n))                                                   // Children of merged node
     {ArrayLongUnShiftArray((void *)n->down, nl,
                            (void *)l->down, ll);
      NWayTree(ReUp)(n);                                                        // Update parent of children of left node
     }
    ArrayVoidDelete((void *)p->down, pl, I);                                    // Remove link from parent to right child
    NWayTree(Node_setLength)(n, nl + ll + 1);
    NWayTree(Node_setLength)(p, pl      - 1);
    NWayTree(FreeNode)(l);
   }
 }

static void NWayTree(Merge)                                                     //P Merge the current node with its sibling. Not called by find or insert
 (NWayTree(Node) * const node)                                                  // Node to merge into
 {NWayTree_GetLong(i, NWayTree(IndexInParent)(node));                           // Index in parent
  NWayTree_GetNode(p, NWayTree(Node_up)(node));                                 // Parent

  if (i)                                                                        // Merge with left node
   {NWayTree_GetNode(l, NWayTree(Node_down)(p, i-1));                           // Left node
    NWayTree_GetNode(r, node);
    if (NWayTree(HalfFull)(r))                                                  // Merge as left and right nodes are half full
     {if (NWayTree(HalfFull)(l))
       {NWayTree(MergeWithLeftOrRight)(r, 0);
       }
      else
       {NWayTree(FillFromLeftOrRight) (r, 0);
       }
     }
   }
  else
   {NWayTree_GetNode(r, NWayTree(Node_down)(p, 1));                             // Right node
    NWayTree_GetNode(l, node);
    if (NWayTree(HalfFull)(l))
     {if (NWayTree(HalfFull)(r))                                                // Merge as left and right nodes are half full
       {NWayTree(MergeWithLeftOrRight)(l, 1);
       }
      else
       {NWayTree(FillFromLeftOrRight) (l, 1);
       }
     }
   }
 }

static void NWayTree(MergeOrFill)                                               //P Make a node larger than a half node. Not called by find or insert.
 (NWayTree(Node) * const node)                                                  // Node to merge or fill
 {if (NWayTree(HalfFull)(node)) return;                                         // No need to merge of if not a half node
  NWayTree_GetNode(p, NWayTree(Node_up)(node));                                 // Parent
  NWayTree_GetNode(P, NWayTree(Node_up)(p));                                    // Parent of parent

  if (P)                                                                        // Merge or fill parent which is not the root
   {NWayTree(MergeOrFill)(p);
    NWayTree(Merge)(node);
    return;
   }

  NWayTree_GetLong(pl, NWayTree(Node_length)(p));

  if (pl == 1)                                                                  // Parent is the root and it only has one key - merge into the child if possible
   {NWayTree_GetNode(l, NWayTree(Node_down)(p, 0));
    NWayTree_GetLong(lh, NWayTree(HalfFull)(l));
    if (lh)
     {NWayTree_GetNode(r, NWayTree(Node_down)(p, 1));
      NWayTree_GetLong(rh, NWayTree(HalfFull)(r));
      if (rh)
       {NWayTree_GetLong(L, NWayTree(Node_length)(l));
        NWayTree_GetLong(R, NWayTree(Node_length)(r));
        NWayTree_GetLong(N, NWayTree(Node_length)(node));
        ArrayLongPushArray(node->keys, 0, l->keys, L);
        ArrayLongPushArray(node->data, 0, l->data, L);

        ArrayLongPushArray(node->keys, L, p->keys, 1);
        ArrayLongPushArray(node->data, L, p->data, 1);

        ArrayLongPushArray(node->keys, L+1, r->keys, R);
        ArrayLongPushArray(node->data, L+1, r->data, R);

        ArrayVoidPushArray((void *)node->down, 0,   (void *)l->down, L+1);
        ArrayVoidPushArray((void *)node->down, L+1, (void *)r->down, R);
        NWayTree(Node_setLength)(node, L+R+1);

        ArrayLongPushArray(p->keys, 0, node->keys, N);
        ArrayLongPushArray(p->data, 0, node->data, N);
        ArrayVoidPushArray((void *)p->down, 0, (void *)node->down, N+1);

        NWayTree(ReUp)(p);                                                      // Reconnect children to parent
        return;
       }
     }
   }

  NWayTree(Merge)(node);                                                        // Parent is the root but it has more than one key
 }

//D1 Find

static NWayTree(FindResult) NWayTree(Find)                                      // Find a key in a tree returning its associated data or undef if the key does not exist.
 (NWayTree(Tree) * const tree,                                                  // Tree to search
  NWayTreeDataType const key)                                                   // Key to search
 {NWayTree(Node) * node = NWayTree(Tree_node)(tree);                            // Current node we are searching
  if (!node)                                                                    // Empty tree
   {NWayTree_GetFindComparison(n, notFound);
    return NWayTree(NewFindResult)(node, key, n, -1);
   }

  for(long j = 0; j < NWayTreeLongMaxIterations; ++j)                           // Same code as above
   {NWayTree_GetLong(nl, NWayTree(Node_length)(node));
    NWayTree_GetLong(nk, NWayTree(Node_keys)(node, nl-1));

    if (key > nk)                                                               // Bigger than every key
     {NWayTree_GetLong(l, NWayTree(IsLeaf)(node));                              // Leaf
      if (l)
       {NWayTree_GetFindComparison(h, higher);
        return NWayTree(NewFindResult)(node, key, h, nl-1);
       }
      node = NWayTree(Node_down)(node, nl);
      continue;
     }

    for(long i = 0; i < nl; ++i)                                                // Search the keys in this node as less than largest key
     {const NWayTreeDataType k = NWayTree(Node_keys)(node, i);                  // Key from tree
      if (key == k)                                                             // Found key
       {NWayTree_GetFindComparison(e, equal);
        return NWayTree(NewFindResult)(node, key, e, i);
       }
      if (key < k)                                                              // Lower than current key
       {if (NWayTree(IsLeaf)(node))                                             // Leaf
         {NWayTree_GetFindComparison(l, lower);
          return NWayTree(NewFindResult)(node, key, l, i);
         }
        node = NWayTree(Node_down)(node, i);
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
 {NWayTree_GetNode(n, NWayTree(Tree_node)(tree));                               // Root node of tree

  if (!n)                                                                       // Empty tree
   {NWayTree_GetNode(n, NWayTree(NewNode)(tree));
    NWayTree(Node_setKeys)(n, 0, key);
    NWayTree(Node_setData)(n, 0, data);
    NWayTree(Node_setLength)(n, 1);
    NWayTree_GetLong(nk, NWayTree(Tree_keys)(tree));
    NWayTree(Tree_setKeys)(tree, nk+1);
    NWayTree(Tree_setNode)(tree, n);
    return;
   }

  NWayTree_GetLong(nl, NWayTree(Node_length)(n));                               // Current length of node
  NWayTree_GetLong(m, NWayTree(MaximumNumberOfKeys)(tree));                     // Maximum number of keys allowed in a node

  if (nl < m)                                                                   // Node is root with no children and room for one more key
   {NWayTree_GetNode(p, NWayTree(Node_up)(n));
    if (!p)
     {NWayTree_GetLong(leaf, NWayTree(IsLeaf)(n));
      if (leaf)
       {for(long i = 0; i < nl; ++i)                                            // Each key
         {NWayTree_GetLong(nk, NWayTree(Node_keys)(n, i));
          if (key == nk)                                                        // Key already present
           {NWayTree(Node_setData)(n, i, data);
            return;
           }
          const NWayTreeDataType k = NWayTree(Node_keys)(n, i);
          if (key < k)                                                          // We have reached the insertion point
           {ArrayLongInsert(n->keys, nl+1, key,  i);
            ArrayLongInsert(n->data, nl+1, data, i);
            NWayTree(Node_setLength)(n, nl+1);
            NWayTree(Tree_incKeys)(tree);
            return;
           }
         }
        ArrayLongPush(n->keys, nl, key);                                        // Insert the key at the end of the block because it is greater than all the other keys in the block
        ArrayLongPush(n->data, nl, data);
        NWayTree(Node_setLength)(n, nl+1);
        NWayTree(Tree_incKeys)(tree);
        return;
       }
     }
   }
                                                                                // Insert node
  NWayTree(FindResult) const r = NWayTree(FindAndSplit)(tree, key);             // Check for existing key
  NWayTree_GetNode(N, NWayTree(FindResult_node)(r));
  NWayTree_GetFindComparison(e, equal);
  NWayTree_GetLong(c, NWayTree(FindResult_cmp)(r));

  if (c == e)                                                                   // Found an equal key whose data we can update
   {NWayTree_GetLong(ri, NWayTree(FindResult_index)(r));
    NWayTree(Node_setData)(N, ri, data);
    return;
   }

  long index = NWayTree(FindResult_index)(r);                                   // We have room for the insert
  NWayTree_GetFindComparison(h, higher);
  if (c == h) ++index;                                                          // Position at which to insert new key
  NWayTree_GetLong(Nl, NWayTree(Node_length)(N));
  ArrayLongInsert(N->keys, Nl+1, key,  index);
  ArrayLongInsert(N->data, Nl+1, data, index);

  NWayTree(Node_setLength)(N, Nl+1);
  NWayTree(SplitFullNode) (N);                                                  // Split if the leaf is full to force keys up the tree
 }

//D1 Iteration

static NWayTree(FindResult) NWayTree(GoAllTheWayLeft)                           // Go as left as possible from the current node
 (NWayTree(Node) * const node)
 {if (!node)                                                                    // Empty tree
   {NWayTree_GetFindComparison(n, notFound);
    return NWayTree(NewFindResult)(node, 0, n, 0);
   }
  NWayTree_GetLong(leaf, NWayTree(IsLeaf)(node));
  if (!leaf)                                                                    // Still some way to go
   {NWayTree_GetNode(d, NWayTree(Node_down)(node, 0));
    return NWayTree(GoAllTheWayLeft)(d);
   }

  const NWayTreeDataType         k = NWayTree(Node_keys)(node, 0);
  NWayTree_GetFindComparison(e, equal);
  return NWayTree(NewFindResult)(node, k, e, 0);                                // Leaf - place us on the first key
 }

static NWayTree(FindResult) NWayTree(GoUpAndAround)                             // Go up until it is possible to go right or we can go no further
 (NWayTree(FindResult) const find)
 {NWayTree(Node) *node = find.node;
  //say("BBBB %ld %ld", find.key, find.index);
  NWayTree_GetLong(leaf, NWayTree(IsLeaf)(node));
  if (leaf)                                                                     // Leaf
   {//say("CCCC %ld", node->id);
    if (NWayTree(FindResult_index)(find) < NWayTree(Node_length)(node)-1)       // More keys in leaf
     {NWayTree_GetLong(i, NWayTree(FindResult_index)(find) + 1);
      //say("DDDD key=%ld %ld", node->keys[i], i);
      NWayTree_GetFindComparison(e, equal);
      const NWayTreeDataType         k = NWayTree(Node_keys)(node, i);
      return NWayTree(NewFindResult)(node, k, e, i);
     }
    //say("DDDD22 %p %ld %ld", node, node->length, node->id);
    NWayTree(Node) *parent = NWayTree(Node_up)(node);                           // Parent
    for(;parent;)                                                               // Not the only node in the tree
     {//say("DDDD33 %p %ld %ld", parent, parent->length, parent->id);
      NWayTree_GetLong(i, NWayTree(IndexInParent)(node));                       // Index in parent
      NWayTree_GetLong(pl, NWayTree(Node_length)(parent));                      // Parent length
      //say("EEEE id=%ld %ld", node->id, i);
      if (i == pl)                                                              // Last key - continue up
       {node = parent;
        //say("EEEE22 id=%id", node->id);
        parent = NWayTree(Node_up)(parent);
        continue;
       }
      //say("FFFF id=%ld %ld parent=%p node=%p", parent->id, i+1, parent, node);
      const NWayTreeDataType         k = NWayTree(Node_keys)(parent, i);
      NWayTree_GetFindComparison(e, equal);
      return NWayTree(NewFindResult)(parent, k, e, i);                          // Not the last key
     }
    //say("GGGG id=%ld", node->id);
    NWayTree_GetFindComparison(n, notFound);
    return NWayTree(NewFindResult)(node, 0, n, 0);                              // Last key of root
   }

  //say("HHHH id=%ld", node->id);
  NWayTree_GetLong(i, NWayTree(FindResult_index)(find));                        // Not a leaf so on an interior key so we can go right then all the way left
  NWayTree_GetNode(d, NWayTree(Node_down)(node, i+1));
  return NWayTree(GoAllTheWayLeft)(d);
 }

static NWayTree(FindResult) NWayTree(IterStart)                                 // Start an iterator
 (NWayTree(Tree) * const tree)                                                  // Tree to iterate
 {NWayTree(Node) * n = NWayTree(Tree_node)(tree);
  NWayTree(FindResult) f = NWayTree(GoAllTheWayLeft)(n);
  return f;
 }

static long NWayTree(IterCheck)                                                 // True if we can continue to iterate
 (NWayTree(FindResult) const find)                                              // Find result of last iteration
 {NWayTree_GetFindComparison(n, notFound);
  NWayTree_GetLong(c, NWayTree(FindResult_cmp)(find));
  return c != n;
 }

static NWayTree(FindResult) NWayTree(IterNext)                                  // Next element of an iteration
 (NWayTree(FindResult) const find)                                              // Find result of last iteration
 {NWayTree(FindResult) f = NWayTree(GoUpAndAround)(find);
  //say("AAAA %ld", f.key);
  return f;
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

//D0 Tests

#if (__INCLUDE_LEVEL__ == 1)
void test_3_0()                                                                 // Tests
 {NWayTree_GetTree(t, NWayTree(NewTree)(3));
  assert(NWayTree(EqText)(t, ""));
  NWayTree_GetFindResult(f, NWayTree(Find)(t, 1));
  NWayTree_GetLong(c, NWayTree(FindResult_cmp)(f));
  if (c) {}
 }

void test_3_1()                                                                 // Tests
 {NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {NWayTree_GetTree(t, NWayTree(NewTree)(31));
  for(int i = 0; i < 1; ++i) NWayTree(Insert)(t, i, 2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
));
 }

void test_3_2()                                                                 // Tests
 {NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(int i = 0; i < 2; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
));
 }

void test_31_2()                                                                // Tests
 {NWayTree_GetTree(t, NWayTree(NewTree)(31));
  for(int i = 0; i < 2; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
));
 }

void test_3_3()                                                                 // Tests
 {NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(int i = 0; i < 3; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_31_3()                                                                // Tests
 {NWayTree_GetTree(t, NWayTree(NewTree)(31));
  for(int i = 0; i < 3; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
"   2                                   4\n"
));
 }

NWayTree(Node) *createNode3(NWayTree(Tree) * t, long a, long b, long c)         // Create a test node
 {NWayTree(Node) *n = NWayTree(NewNode)(t);
  NWayTree(Node_setKeys)(n, 0, a); NWayTree(Node_setData)(n, 0, 2*a);
  NWayTree(Node_setKeys)(n, 1, b); NWayTree(Node_setData)(n, 1, 2*b);
  NWayTree(Node_setKeys)(n, 2, c); NWayTree(Node_setData)(n, 2, 2*c);
  NWayTree(Node_setLength)(n, 3);

  return n;
 }

void test_3_4a()                                                                // Tree has one node
 {NWayTree(Tree) *t = NWayTree(NewTree)(3);
  NWayTree(Node) *n = createNode3(t, 1, 2, 3);
  t->keys = NWayTree(Node_setLength)(n, 3);
  t->node = n;

  long r = NWayTree(SplitFullNode)(n);
  assert(r);
  //NWayTree(ErrAsC)(t);
  assert(NWayTree(EqText)(t,
"      1                                   2\n"
"   2                                   4\n"
"      3                                   6\n"
));
 }

void test_3_4b()                                                                // First down
 {NWayTree(Tree) *t  = NWayTree(NewTree)(3);
  NWayTree(Node) *p  = createNode3(t, 10, 20, 30);
  NWayTree(Node_setLength)(p, 2);
  NWayTree(Node) *n0 = createNode3(t, 01, 02, 03);
  NWayTree(Node) *n1 = createNode3(t, 11, 12, 13);
  NWayTree(Node) *n2 = createNode3(t, 21, 22, 23);
  NWayTree(Node_setDown)(p, 0, n0); NWayTree(Node_setUp)(n0, p);
  NWayTree(Node_setDown)(p, 1, n1); NWayTree(Node_setUp)(n1, p);
  NWayTree(Node_setDown)(p, 2, n2); NWayTree(Node_setUp)(n2, p);
  NWayTree(Tree_setNode)(t, p);
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

  long r = NWayTree(SplitFullNode)(n0); if (r){}
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
 {NWayTree(Tree) *t  = NWayTree(NewTree)(3);
  NWayTree(Node) *p  = createNode3(t, 10, 20, 30);
  NWayTree(Node_setLength)(p, 2);
  NWayTree(Node) *n0 = createNode3(t, 01, 02, 03);
  NWayTree(Node) *n1 = createNode3(t, 11, 12, 13);
  NWayTree(Node) *n2 = createNode3(t, 21, 22, 23);
  NWayTree(Node_setDown)(p, 0, n0); NWayTree(Node_setUp)(n0, p);
  NWayTree(Node_setDown)(p, 1, n1); NWayTree(Node_setUp)(n1, p);
  NWayTree(Node_setDown)(p, 2, n2); NWayTree(Node_setUp)(n2, p);
  NWayTree(Tree_setNode)(t, p);

  assert(NWayTree(Node_down)(p, 1) == n1);

  long r = NWayTree(SplitFullNode)(n1); if (r){}
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
 {NWayTree(Tree) *t  = NWayTree(NewTree)(3);
  NWayTree(Node) *p  = createNode3(t, 10, 20, 30);
  NWayTree(Node_setLength)(p, 2);
  NWayTree(Node) *n0 = createNode3(t, 01, 02, 03);
  NWayTree(Node) *n1 = createNode3(t, 11, 12, 13);
  NWayTree(Node) *n2 = createNode3(t, 21, 22, 23);
  NWayTree(Node_setDown)(p, 0, n0); NWayTree(Node_setUp)(n0, p);
  NWayTree(Node_setDown)(p, 1, n1); NWayTree(Node_setUp)(n1, p);
  NWayTree(Node_setDown)(p, 2, n2); NWayTree(Node_setUp)(n2, p);
  NWayTree(Tree_setNode)(t, p);

  assert(NWayTree(Node_down)(p, 1) == n1);

  long r = NWayTree(SplitFullNode)(n2); if (r){}
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
 {const long N = 1;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   3\n"
));
 }

void test_3_insert2()
 {const long N = 2;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_3_insert3()
 {const long N = 3;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   3\n"
"   2                                   4\n"
"   3                                   5\n"
));
 }

void test_3_insert4()
 {const long N = 4;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 5;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 6;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 7;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 8;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 2;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   2\n"
"   2                                   3\n"
));
 }

void test_3_insert3r()
 {const long N = 3;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   2\n"
"   2                                   3\n"
"   3                                   4\n"
));
 }

void test_3_insert4r()
 {const long N = 4;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 5;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 6;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 7;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
 {const long N = 8;
  NWayTree_GetTree(t, NWayTree(NewTree)(3));
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
   {long r = i * i % N, R = A[r], I = A[i];
    A[i] = R; A[r] = I;
   }
 }

void test_3_insert12()
 {const long N = 12; long A[N]; testLoadArray(A, N);

  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, A[i], i);
  assert(NWayTree(EqText)(t,
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
 {const long N = 14; long A[N]; testLoadArray(A, N);

  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(long i = 0; i < N; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
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
 {const long N = 15; long A[N]; testLoadArray(A, N);

  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(long i = 0; i < N; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
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
 {const long N = 63; long A[N]; testLoadArray(A, N);

  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(long i = 0; i < N; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  for(long i = 0; i < N; ++i)
   {NWayTree(FindResult) r = NWayTree(Find)(t, A[i]);
    assert(NWayTree(FindResult_data(r)) == i);
    NWayTree_GetFindComparison(e, equal);
    const long c = NWayTree(FindResult_cmp)(r);
    assert(c == e);
   }
  //NWayTree(ErrAsC)(tree);
  NWayTree(CheckTree)(t, "3/63");
  assert(NWayTree(EqText)(t,
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

void test_3_iterate63()                                                         // Iterate through a tree
 {const long N = 63, NN = 63; long A[N]; testLoadArray(A, N);

  NWayTree_GetTree(t, NWayTree(NewTree)(3));                              // Create the tree
  for(long i = 0; i < NN; ++i)
   {for(long j = 0; j < 2; ++j)
     {NWayTree(Insert)(t, A[i], i);
     }
   }

  //NWayTree(PrintErrWithId)(t);

  StackChar * const s = StackCharNew();                                         // Extensible string

  NWayTreeIterate(t, f)                                                         // Iterate through the tree
   {char C[100];
    sprintf(C, " %ld", NWayTree(FindResult_key)(f));                            // Key of each iteration
    StackCharPushString(s, C);
   }
  //StackCharErr(s);
  assert(StackCharEqText(s,
    " 0 1 2 3 4 5 6 7 8 9"
    " 10 11 12 13 14 15 16 17 18 19"
    " 20 21 22 23 24 25 26 27 28 29"
    " 30 31 32 33 34 35 36 37 38 39"
    " 40 41 42 43 44 45 46 47 48 49"
    " 50 51 52 53 54 55 56 57 58 59"
    " 60 61 62"));
  StackCharFree(s);
 }

void test_31_insert163                                                          // Create and free a tree.
 (int test)                                                                     // Warm malloc up until it stabilizes when false.
 {long N = 163, NN = N, A[N]; testLoadArray(A, N);

  long memory_at_start;                                                         // Memory in use at start
  if (test)
   {struct mallinfo m; m = mallinfo();
    memory_at_start = m.uordblks;
   }

  NWayTree_GetTree(t, NWayTree(NewTree)(31));
  for(long i = 0; i < NN; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  for(long i = 0; i < NN; ++i)
   {NWayTree(FindResult) r = NWayTree(Find)(t, A[i]);

    const long d = NWayTree(FindResult_data)(r);
    assert(d == i);

    NWayTree_GetFindComparison(e, equal);
    const long c = NWayTree(FindResult_cmp)(r);
    assert(c == e);
   }
  //NWayTree(ErrAsC)(t);
  NWayTree(CheckTree)(t, "31/163");
  if (1) assert(NWayTree(EqText)(t,
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
 {const long N = 63;

  NWayTree_GetTree(t, NWayTree(NewTree)(3));
  for(long i = 0; i < N;     ++i) NWayTree(Insert)(t, i*2, i*2);
  //NWayTree(ErrAsC)(tree);
  for(long i =-1; i < 2 * N; ++i)
   {NWayTree(FindResult) r = NWayTree(Find)(t, i);
    //NWayTree(ErrFindResult)(r);
    const long cmp = NWayTree(FindResult_cmp)(r);
    assert(i % 2 == 0 ? cmp == NWayTree(FindComparison_equal) :
                        cmp != NWayTree(FindComparison_equal));
    const long      ri = NWayTree(FindResult_index)(r);
    NWayTree(Node) *rn = NWayTree(FindResult_node) (r);
    const long      rk = NWayTree(Node_keys)(rn, ri);
    NWayTree_GetFindComparison(l, lower);
    NWayTree_GetFindComparison(e, equal);
    NWayTree_GetFindComparison(h, higher);
    if (i == -1) assert(rk ==  0 && cmp == l && ri == 0);
    if (i ==  0) assert(rk ==  0 && cmp == e && ri == 0);
    if (i ==  1) assert(rk ==  0 && cmp == h && ri == 0);

    if (i == 11) assert(rk == 12 && cmp == l && ri == 0);
    if (i == 12) assert(rk == 12 && cmp == e && ri == 0);
    if (i == 13) assert(rk == 12 && cmp == h && ri == 0);
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
  test_3_iterate63();
  tests3 ();
  tests31();
  return 0;
 }
#endif
// (\A.{80})\s+(//.*\Z) \1\2
