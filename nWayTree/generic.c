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
#define NWayTreeIterate(tree, find) \
  for(NWayTree(FindResult) find = NWayTree(IterStart)(tree); \
      NWayTree(IterCheck) (find); \
                           find = NWayTree(IterNext)(find))
#endif

#define DeR(type_target, target, type_source, source, field) type_target target = ((type_source)source)->field; /* Deref a field in a source structure to produce a target of a known */
#define Add(t, s1, s2) const long t = (s1) + (s2);                              /* Add two longs together and save the result */
#define Mul(t, s1, s2) const long t = (s1) * (s2);                              /* Multiply two longs together and save the result */

//Optimize
//#define static                                                                /* Simplify debugging by preventing some inline-ing which invalidates the call stack */
const long size_of_element = sizeof(NWayTreeDataType);                          // The size of a key, data

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
 (NWayTree(Node) *node, long const length)                                      // Node, new length
 {return node->length = length;
 }

inline static NWayTreeDataType NWayTree(Node_keys)                              // Get the indicated key from the specified node
 (NWayTree(Node) *node, long const index)                                       // Node, index
 {return node->keys[index];
 }

inline static NWayTreeDataType NWayTree(Node_setKeys)                           // Set the indicated key in specified node
 (NWayTree(Node) *node, long const index, const NWayTreeDataType key)           // Node, index, key
 {return node->keys[index] = key;
 }

inline static NWayTreeDataType NWayTree(Node_data)                              // Get the indicated data from the specified node
 (NWayTree(Node) *node, long const index)                                       // Node, index
 {return node->data[index];
 }

inline static NWayTreeDataType NWayTree(Node_setData)                           // Set the indicated data in specified node
 (NWayTree(Node) *node, long const index, const NWayTreeDataType data)           // Node, index, data
 {return node->data[index] = data;
 }

inline static NWayTree(Node) * NWayTree(Node_down)                              // Get the indicated down from the specified node
 (NWayTree(Node) *node, long const index)                                       // Node, index
 {return node->down[index];
 }

inline static NWayTree(Node) * NWayTree(Node_setDown)                           // Set the indicated down in specified node
 (NWayTree(Node) *node, long const index, NWayTree(Node) * child)               // Node, index, child node
 {return node->down[index] = child;
 }

inline static struct NWayTree(Tree) * NWayTree(Node_tree)                       // Get the tree associated with a node
 (NWayTree(Node) *node)                                                         // Node
 {return node->tree;
 }

typedef struct NWayTree(Tree)                                                   // The root of a tree
 {long NumberOfKeysPerNode;                                                     // Size of a node
  NWayTree(Node) *node;                                                         // Root node
  long keys;                                                                    // Number of keys in tree
  long nodes;                                                                   // Number of nodes in tree
  NWayTree(Node) *node_array;                                                   // Array of nodes if tree has been compacted
 } NWayTree(Tree);

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
 (NWayTree(Node) * const node, NWayTreeDataType const key,
  NWayTree(FindComparison) const cmp, long const index)
 {NWayTree(FindResult) r;
  r.node  = node;
  r.key   = key;
  r.cmp   = cmp;
  r.index = index;
  return r;
 }

inline static NWayTreeDataType NWayTree(FindResult_key)                         // Get key field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {return f.key;
 }

inline static long NWayTree(FindResult_cmp)                                     // Get comaprison field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {return f.cmp;
 }

inline static NWayTreeDataType NWayTree(FindResult_data)                        // Get data field from find results
 (NWayTree(FindResult) f)                                                       // The results of a find operation
 {return f.node->data[f.index];
 }

static void NWayTree(ErrNode)       (NWayTree(Node) *node);
static long NWayTree(CheckNode)     (NWayTree(Node) *node, char *name);
static void NWayTree(CheckTree)     (NWayTree(Tree) *tree, char *name);
static long NWayTree(IsLeaf)        (NWayTree(Node) *node);
static long NWayTree(IndexInParent) (NWayTree(Node) * const node);

static NWayTree(Tree) *NWayTree(NewTree)                                        // Create a new tree
 (const long n)                                                                 // Number of keys in a node
 {NWayTree(Tree) * const tree = calloc(sizeof(NWayTree(Tree)), 1);
  tree->NumberOfKeysPerNode = n;
  return tree;
 }

static NWayTree(Node) *NWayTree(NewNode)                                        //P Create a new node
 (NWayTree(Tree) * const tree)                                                  // Tree containing node
 {const long z = tree->NumberOfKeysPerNode;
  const long k = size_of_element                    *  z;
  const long d = size_of_element                    *  z;
  const long n = sizeof(struct NWayTree(Node) *) * (z + 1);
  const long s = sizeof(NWayTree(Node));

  NWayTree(Node) * const node = calloc(s+k+d+n, 1);
  node->keys = (void *)(((void *)node)+s);
  node->data = (void *)(((void *)node)+s+k);
  node->down = (void *)(((void *)node)+s+k+d);
  node->tree = tree;
  node->id   = tree->nodes++;
  return node;
 }

static long NWayTree(SizeOfNode)                                                //P The size of a node in a tree
 (NWayTree(Tree) *t)                                                            // Tree
 {const long z = t->NumberOfKeysPerNode;
  const long k = size_of_element                    *  z;
  const long d = size_of_element                    *  z;
  const long n = sizeof(struct NWayTree(Node) *) * (z + 1);
  const long s = sizeof(NWayTree(Node));
  return s+k+d+n;
 }

static void NWayTree(FreeNode)                                                  //P Free a node, Wipe the node so it cannot be accidently reused
 (NWayTree(Node) * const node)                                                  // Node to free
 {memset(node, -1, NWayTree(SizeOfNode)(NWayTree(Node_tree)(node)));            // Clear node to hinder accidental use
  free(node);
 }

static NWayTree(Tree) *NWayTree(Compact)                                        // Compact a tree into a single block of memory
 (NWayTree(Tree) * const tree)                                                  // Tree to compact
 {long const size_tree  = sizeof(NWayTree(Tree));
  long const size_node  = NWayTree(SizeOfNode)(tree);
  long const size       = size_tree + tree->nodes * size_node;
  NWayTree(Tree) * const t = calloc(size, 1);
  memcpy(t, tree, size_tree);
  t->node_array = ((void *)t)+size_tree;                                        // Start of nodes
  for(long i = 0; i < tree->nodes; ++i)
   {
   }
  return tree;
 }

static NWayTree(FindResult) NWayTree(GoAllTheWayLeft)                           // Go as left as possible from the current node
 (NWayTree(Node) * const node)
 {if (!node) return NWayTree(NewFindResult)                                     // Empty tree
   (node, 0, NWayTree(FindComparison_notFound), 0);

  if (!NWayTree(IsLeaf)(node)) return NWayTree(GoAllTheWayLeft)(node->down[0]); // Still some way to go

  return NWayTree(NewFindResult)                                                // Leaf - place us on the first key
   (node, NWayTree(Node_keys)(node, 0),  NWayTree(FindComparison_equal), 0);
 }

static NWayTree(FindResult) NWayTree(GoUpAndAround)                             // Go up until it is possible to go right or we can go no further
 (NWayTree(FindResult) const find)
 {NWayTree(Node) *node = find.node;
  //say("BBBB %ld %ld", find.key, find.index);
  if (NWayTree(IsLeaf)(node))                                                   // Leaf
   {//say("CCCC %ld", node->id);
    if (find.index < NWayTree(Node_length)(node)-1)                             // More keys in leaf
     {const long i = find.index + 1;
      //say("DDDD key=%ld %ld", node->keys[i], i);
      return NWayTree(NewFindResult)
       (node, NWayTree(Node_keys)(node, i), NWayTree(FindComparison_equal), i);
     }
    //say("DDDD22 %p %ld %ld", node, node->length, node->id);
    for(NWayTree(Node) *parent = node->up; parent; parent = parent->up)         // Not the only node in the tree
     {//say("DDDD33 %p %ld %ld", parent, parent->length, parent->id);
      const long i = NWayTree(IndexInParent)(node);                             // Index in parent
      //say("EEEE id=%ld %ld", node->id, i);
      if (i == NWayTree(Node_length)(parent))                                   // Last key - continue up
       {node = parent;
        //say("EEEE22 id=%id", node->id);
        continue;
       }
      //say("FFFF id=%ld %ld parent=%p node=%p", parent->id, i+1, parent, node);
      return NWayTree(NewFindResult)                                            // Not the last key
       (parent, parent->keys[i], NWayTree(FindComparison_equal), i);
     }
    //say("GGGG id=%ld", node->id);
    return NWayTree(NewFindResult)                                              // Last key of root
     (node, 0, NWayTree(FindComparison_notFound), 0);
   }

  //say("HHHH id=%ld", node->id);
  return NWayTree(GoAllTheWayLeft)(node->down[find.index+1]);                   // Not a leaf so on an interior key so we can go right then all the way left
 }

static NWayTree(FindResult) NWayTree(FindNext)                                  // Find the next key after the one referenced by a find result
 (const NWayTree(FindResult) f)
 {return f;
 }

static void NWayTree(Free2)                                                     //P Free a node in a tree
 (NWayTree(Node) * const node)
 {if (!node) return;
  const long nl = NWayTree(Node_length)(node);
  if (nl)
   {if (!NWayTree(IsLeaf)(node))
     {NWayTree(Free2)(node->down[0]);
      for(long i = 1; i <= nl; ++i)
       {NWayTree(Free2)(node->down[i]);
       }
     }
   }
  NWayTree(FreeNode)(node);
 }

static void NWayTree(Free)                                                      // Free a tree
 (NWayTree(Tree) * const tree)
 {if (!tree) return;
  NWayTree(Free2)(tree->node);
  memset(tree, -1, sizeof(*tree));
  free(tree);
 }

static void NWayTree(ToString2)                                                 //P Print the keys in a tree
 (NWayTree(Node) * const node, long const in, StackChar * const p)
 {if (!node) return;
  const long nl = NWayTree(Node_length)(node);
  if (!nl) return;
  NWayTree(ToString2)(node->down[0], in+1, p);
  for(long i = 0; i < nl; ++i)
   {for(long j = 0; j < in * 3; ++j) StackCharPush(p, ' ');
    char C[100];
    sprintf(C, "%4ld                                %4ld\n",
              (NWayTree(Node_keys)(node, i)), (NWayTree(Node_data)(node, i)));
    StackCharPushString(p, C);
    NWayTree(ToString2)(node->down[i+1], in+1, p);
   }
 }

static StackChar *NWayTree(ToString)                                            // Print a tree as a string
 (NWayTree(Tree) * const tree)                                                  // Tree to print as a string
 {StackChar * const p = StackCharNew();
  if (tree->node) NWayTree(ToString2)(tree->node, 0, p);
  return p;
 }

static void NWayTree(PrintErr)                                                  // Print a tree on stderr
 (NWayTree(Tree) * const tree)                                                  // Tree to print
 {StackChar * const s = NWayTree(ToString)(tree);
  StackCharErr(s);
  StackCharFree(s);
 }

static void NWayTree(ToStringWithId2)                                           //P Print the keys in a tree adding the id of each node in the tree and the index of the key within that node
 (NWayTree(Node) * const node, long const in, StackChar * const p)
 {const long nl = NWayTree(Node_length)(node);
  if (!node || !nl) return;
  NWayTree(ToStringWithId2)(node->down[0], in+1, p);
  for(long i = 0; i < nl; ++i)
   {for(long j = 0; j < in; ++j) StackCharPushString(p, "   ");
    char C[100];
    sprintf(C, "%4ld", NWayTree(Node_keys)(node, i));
    StackCharPushString(p, C);

    for(long j = 0; j < 10-in; ++j) StackCharPushString(p, "   ");
    char D[100];
    sprintf(D, "%4ld %4ld %4ld  %p/%4ld=", NWayTree(Node_data)(node, i), node->id, i, node, nl);
    StackCharPushString(p, D);
    for(long j = 0; j <= nl; ++j)
     {NWayTree(Node) *d = node->down[j];
      sprintf(D, " %4ld", d ? d->id : 0l);
      StackCharPushString(p, D);
     }
    StackCharPushString(p, "\n");
    NWayTree(ToStringWithId2)(node->down[i+1], in+1, p);
   }
 }

static StackChar *NWayTree(ToStringWithId)                                      //P Print the keys in a tree adding the id of each node in the tree and the index of the key within that node
 (NWayTree(Tree) * const tree)                                                  // Tree to print as a string
 {StackChar * const p = StackCharNew();
  StackCharPushString(p, "                                     Data Node Index Children\n");
  if (tree->node) NWayTree(ToStringWithId2)(tree->node, 0, p);
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
  const long N = s->next-s->base;                                               // The number of characters to print
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
  StackCharFree(s);
  fputs("));\n", stderr);
 }

static void NWayTree(ErrNode)                                                   // Dump a node
 (NWayTree(Node) * const node)                                                  // Node
 {say("Node at  %p", node);
  say("  Up     = %p", node->up);
  const long nl = NWayTree(Node_length)(node);
  say("  Length = %ld", nl);
  fprintf(stderr, "  Keys   : ");
  for(long i = 0; i <  nl; ++i) fprintf(stderr," %ld", (NWayTree(Node_keys)(node, i)));
  fprintf(stderr, "\n  Data   : ");
  for(long i = 0; i <  nl; ++i) fprintf(stderr," %ld", (NWayTree(Node_data)(node, i)));
  fprintf(stderr, "\n  Down   : ");
  for(long i = 0; i <= nl; ++i) fprintf(stderr," %p",   node->down[i]);
  say("\n");
 }

static long NWayTree(EqText)
 (NWayTree(Tree) * const tree, char * const text)
 {StackChar * const s = NWayTree(ToString)(tree);
  const long r = strncmp(s->arena+s->base, text, s->next-s->base) == 0;
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

  say("Find key=%ld Result keys[index]=%ld %s  index=%ld",
      NWayTree(FindResult_key)(r), r.node->keys[r.index], c, r.index);
 }

static long NWayTree(MinimumNumberOfKeys)                                       //P Minimum number of keys per node.
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return (tree->NumberOfKeysPerNode - 1) << 1;
 }

static long NWayTree(MaximumNumberOfKeys)                                       //P Maximum number of keys per node.
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return tree->NumberOfKeysPerNode;
 }

static long NWayTreeLong NWayTree(MaximumNumberDownPerNode)                     //P Maximum number of children per parent.
 (NWayTree(Tree) * const tree)                                                  // Tree
 {return tree->NumberOfKeysPerNode + 1;
 }

static long NWayTree(Full)                                                      //P Confirm that a node is full.
 (NWayTree(Node) * const node)
 {return NWayTree(Node_length)(node) == NWayTree(MaximumNumberOfKeys)(NWayTree(Node_tree)(node));
 }

static long NWayTree(HalfFull)                                                  //P Confirm that a node is half full.
 (NWayTree(Node) * const node)                                                  // Node
 {const long n = NWayTree(Node_length)(node);
  assert(n <= NWayTree(MaximumNumberOfKeys)(NWayTree(Node_tree)(node))+1);
  return n == NWayTree(MinimumNumberOfKeys)(NWayTree(Node_tree)(node));
 }

static long NWayTree(IsLeaf)                                                    //P Confirm that the tree is a leaf.
 (NWayTree(Node) * const node)                                                  // Node to test
 {return node->down[0] == 0;                                                    // No children so it must be a leaf
 }

static void NWayTree(ReUp)                                                      //P Reconnect the children to their new parent.
 (NWayTree(Node) * const node)                                                  // Node to reconnect
 {const long nl = NWayTree(Node_length)(node);
  for(int i = 0; i <= nl; ++i)
   {node->down[i]->up = node;
   }
 }

static long NWayTree(CheckNode)                                                 //P Check the connections to and from a node
 (NWayTree(Node) * const node, char * const name)
 {const long nl = NWayTree(Node_length)(node);
  if (nl > NWayTree(MaximumNumberOfKeys)(NWayTree(Node_tree)(node)))
   {say("%s: Node %ld is too long at %ld", name, NWayTree(Node_keys)(node, 0), nl);
    return 1;
   }
  for(long i = 0; i <= nl; ++i)                                                 // Check that each child has a correct up reference
   {NWayTree(Node) * const d = node->down[i];                                   // Step down
    if (d)
     {const long dl = NWayTree(Node_length)(d);
      if (dl > NWayTree(MaximumNumberOfKeys)(NWayTree(Node_tree)(node)))
       {say("%s: Node %ld under %ld is too long at %ld",
            name, NWayTree(Node_keys)(node, 0), d->keys[0], dl);
        return 2;
       }

      if (d->up != node)
       {say("%s: Node %ld(%p) under %ld(%p) has wrong up pointer %ld(%p)",
             name, d->keys[0], d, NWayTree(Node_keys)(node, 0), node, d->up->keys[0], d->up);
        return 3;
       }
     }
   }

  NWayTree(Node) * const p = node->up;                                          // Check that parent connects to the current node
  if (p)
   {const long pl = NWayTree(Node_length)(p);
    assert(pl <= NWayTree(MaximumNumberOfKeys)(NWayTree(Node_tree)(node)));
    int c = 0;                                                                  // Check that the parent has a down pointer to the current node
    for(long i = 0; i <= pl; ++i)
     {if (p->down[i] == node) ++c;                                              // Find the node that points from the parent to the current node
     }
    if (c != 1)                                                                 // We must be able to find the child
     {say("%s: Node %ld has parent %ld that fails to refer back to it",
         name, NWayTree(Node_keys)(node, 0), p->keys[0]);
      return 4;
     }
   }
  return 0;
 }

static void NWayTree(CheckTree2)                                                //P Check the structure of a tree
 (NWayTree(Node) * const node, char * const name)
 {if (!node) return;

  if (NWayTree(CheckNode)(node, name))
   {NWayTree(PrintErr)(NWayTree(Node_tree)(node));
    assert(0);
   }

  NWayTree(CheckTree2)(node->down[0], name);
  const long nl = NWayTree(Node_length)(node);
  for(long i = 0; i < nl; ++i)
   {NWayTree(CheckTree2)(node->down[i+1], name);
   }
 }

static void NWayTree(CheckTree)                                                 //P Check the structure of a tree
 (NWayTree(Tree) * const tree, char * const name)
 {NWayTree(CheckTree2)(tree->node, name);
 }

static long NWayTree(SplitFullNode)                                             //P Split a node if it is full. Return true if the node was split else false
 (NWayTree(Node) * const node)
 {const long nl = NWayTree(Node_length)(node);

  if (nl < NWayTree(MaximumNumberOfKeys)(NWayTree(Node_tree)(node))) return 0;                 // Must be a full node

  NWayTree(Node) * const l = NWayTree(NewNode)(NWayTree(Node_tree)(node));                     // New child nodes
  NWayTree(Node) * const r = NWayTree(NewNode)(NWayTree(Node_tree)(node));

  const long N = NWayTree(MaximumNumberOfKeys)(NWayTree(Node_tree)(node));                     // Split points
  const long n = N>>1;                                                          // Index of key that will be placed in parent

  const long L = NWayTree(Node_setLength)(l, n);
  const long R = NWayTree(Node_setLength)(r, N - n - 1);

  const long LL = L * size_of_element;
  const long RR = R * size_of_element;

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

  if (node->up)                                                                 // Not a root node
   {NWayTree(Node) * const p = node->up;                                        // Existing parent node
    const long pl = NWayTree(Node_length)(p);
    l->up = r->up = p;                                                          // Connect children to parent
    if (p->down[0] == node)                                                     // Splitting the first child - move everything up
     {memmove(p->keys+1, p->keys,     pl * size_of_element);
      memmove(p->data+1, p->data,     pl * size_of_element);
      memmove(p->down+1, p->down, NWayTree(Node_setLength)(p, pl+1) * size_of_element);
      p->keys[0] = NWayTree(Node_keys)(node, n);
      p->data[0] = NWayTree(Node_data)(node, n);
      p->down[0] = l;
      p->down[1] = r;
      NWayTree(FreeNode)(node);
     }
    else if (p->down[pl] == node)                                               // Splitting the last child - just add it on the end
     {p->keys[  pl] = NWayTree(Node_keys)(node, n);
      p->data[  pl] = NWayTree(Node_data)(node, n);
      p->down[  pl] = l;
      p->down[NWayTree(Node_setLength)(p, pl+1)] = r;
      NWayTree(FreeNode)(node);
     }
    else                                                                        // Splitting a middle child:
     {for(long i = 1; i < pl; ++i)
       {if (p->down[i] == node)                                                 // Find the node that points from the parent to the current node
         {memmove(p->keys+i+1, p->keys+i, (pl-i)   * size_of_element);
          memmove(p->data+i+1, p->data+i, (pl-i)   * size_of_element);
          memmove(p->down+i+1, p->down+i, (pl-i+1) * size_of_element);
          p->keys[i]   = NWayTree(Node_keys)(node, n);
          p->data[i]   = NWayTree(Node_data)(node, n);
          p->down[i]   = l;
          p->down[i+1] = r;
          NWayTree(Node_setLength)(p, pl+1);
          NWayTree(FreeNode)(node);
          return 1;
         }
       }
      assert(0);
     }
   }
  else                                                                          // Root node with single key after split
   {l->up = r->up = node;                                                       // Connect children to parent

    node->keys[0] = NWayTree(Node_keys)(node, n);                               // Single key
    node->data[0] = NWayTree(Node_data)(node, n);                               // Data associated with single key
    node->down[0] = l;
    node->down[1] = r;
    NWayTree(Node_setLength)(node, 1);
   }
  return 1;
 }

static NWayTree(FindResult) NWayTree(FindAndSplit)                              //P Find a key in a tree splitting full nodes along the path to the key.
 (NWayTree(Tree) * const tree, NWayTreeDataType const key)
 {NWayTree(Node) * node = tree->node;
  if (!node) return NWayTree(NewFindResult)(node, key,
    NWayTree(FindComparison_notFound), -1);

  if (NWayTree(SplitFullNode)(node))                                            // Split the root node if necessary
   {node = tree->node;
   }

  for(long j = 0; j < NWayTree(MaxIterations); ++j)                             // Step down through the tree
   {if (key < (NWayTree(Node_keys)(node, 0)))                                                  // Less than smallest key in node
     {if (NWayTree(IsLeaf)(node)) return NWayTree(NewFindResult)
       (node, key, NWayTree(FindComparison_lower), 0);                          // Smallest key in tree
      NWayTree(Node) * const n = node->down[0];
      if (!NWayTree(SplitFullNode)(n)) node = n;                                // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
      continue;
     }

    const long nl = NWayTree(Node_length)(node);                                // Length of node
    const long last = nl-1;                                                     // Greater than largest key in node
    if (key > (NWayTree(Node_keys)(node, last)))                                               // Greater than largest key in node
     {if (NWayTree(IsLeaf)(node)) return NWayTree(NewFindResult)
       (node, key, NWayTree(FindComparison_higher), last);

      NWayTree(Node) * const n = node->down[last+1];
      if (!NWayTree(SplitFullNode)(n)) node = n;                                // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
      continue;
     }

    for(long i = 1; i < nl; ++i)                                                // Search the keys in this node as greater than least key and less than largest key
     {if (key == (NWayTree(Node_keys)(node, i)))                                               // Found key
       {return NWayTree(NewFindResult)
         (node, key, NWayTree(FindComparison_equal), i);
       }
      if (key < (NWayTree(Node_keys)(node, i)))                                                // Greater than current key
       {if (NWayTree(IsLeaf)(node)) return NWayTree(NewFindResult)              // Leaf
         (node, key, NWayTree(FindComparison_lower), i);
        NWayTree(Node) * const n = node->down[i];
        if (!NWayTree(SplitFullNode)(node)) node = n; else node = n->up;        // Split the node we have stepped to if necessary - if we do we will ahve to restart the descent from one level up because the key might have moved to the other  node.
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
// Normal   : 1,122,468
static NWayTree(FindResult) NWayTree(Find)                                      // Find a key in a tree returning its associated data or undef if the key does not exist.
 (NWayTree(Tree) * const tree, NWayTreeDataType const key)                      // Tree to search, key to search
 {NWayTree(Node) * node = tree->node;                                           // Current node we are searching
  if (!node) return NWayTree(NewFindResult)(node, key,
    NWayTree(FindComparison_notFound), -1);                                     // Empty tree

  for(long j = 0; j < NWayTreeLongMaxIterations; ++j)                           // Same code as above
   {const long nl = NWayTree(Node_length)(node);
    if (key > (node->keys[nl-1]))                                               // Bigger than every  key
     {if (NWayTree(IsLeaf)(node)) return NWayTree(NewFindResult)                // Greater than all the keys in the node
       (node, key, NWayTree(FindComparison_higher), nl-1);
      node = node->down[nl];
     }
    else
     {for(long i = 0; i < nl; ++i)                                              // Search the keys in this node as less than largest key
       {const NWayTreeDataType k = NWayTree(Node_keys)(node, i);                               // Key from tree
        if (key == k)                                                           // Found key
         {return NWayTree(NewFindResult)
           (node, key, NWayTree(FindComparison_equal), i);
         }
        else if (key < k)                                                       // Lower than current key
         {if (NWayTree(IsLeaf)(node)) return NWayTree(NewFindResult)            // Leaf
           (node, key, NWayTree(FindComparison_lower), i);
          node = node->down[i];
          break;
         }
       }
     }
   }
  assert(0);
 }

static long NWayTree(IndexInParent)                                             //P Get the index of a node in its parent.
 (NWayTree(Node) * const node)                                                  // Node to locate in parent
 {NWayTree(Node) * const p = node->up;
  assert(p);
  const long pl = NWayTree(Node_length)(p);
  for(long i = 0; i <= pl; ++i)
   {if (p->down[i] == node) return i;
   }
  assert(0);
 }

static void NWayTree(FillFromLeftOrRight)                                       //P Fill a node from the specified sibling.
 (NWayTree(Node) * const n, long const dir)                                     // Node to fill, direction to fill from
 {NWayTree(Node) * const p = n->up;                                             // Parent of leaf
  assert(p);
  const long i  = NWayTree(IndexInParent)(n);                                   // Index of leaf in parent
  const long pl = NWayTree(Node_length)(p);
  const long nl = NWayTree(Node_length)(n);

  if (dir)                                                                      // Fill from right
   {assert(i < pl);                                                             // Cannot fill from right
    NWayTree(Node) * const r = p->down[i+1];                                    // Right sibling
    const long rl = NWayTree(Node_length)(r);
    n->keys[nl] = p->keys[i];                                                   // Transfer key and data to parent
    n->data[nl] = p->data[i];

    (p->keys[i]) = ArrayLongShift((r->keys), rl);                               // Transfer keys and data from right
    (p->data[i]) = ArrayLongShift((r->data), rl);

    if (!NWayTree(IsLeaf)(n))                                                   // Transfer node if not a leaf
     {ArrayVoidPush((void *)n->down, nl,
       (void *)ArrayVoidShift((void *)r->down, rl));
      n->down[nl+1]->up = n;
     }
    NWayTree(Node_setLength)(r, rl-1);
    NWayTree(Node_setLength)(n, nl+1);
   }
  else                                                                          // Fill from left
   {assert(i);                                                                  // Cannot fill from left
    const long I = i-1;
    NWayTree(Node) * const l = p->down[I];                                      // Left sibling
    const long ll = NWayTree(Node_length)(l);

    ArrayLongUnShift((l->keys), ll, (p->keys[I]));                              // Shift in keys and data from left
    ArrayLongUnShift((l->data), ll, (p->data[I]));

    p->keys[I] = ArrayLongPop((l->keys), ll);                                   // Transfer key and data to parent
    p->data[I] = ArrayLongPop((l->data), ll);

    if (!NWayTree(IsLeaf)(l))                                                   // Transfer node if not a leaf
     {ArrayVoidUnShift(     (void *)l->down, ll,
       (void *)ArrayVoidPop((void *)l->down, ll));
      l->down[0]->up = l;
     }
   }
 }

static void NWayTree(MergeWithLeftOrRight)                                      //P Merge two adjacent nodes.
 (NWayTree(Node) * const n, long const dir)                                     // Node to fill, direction to fill from
 {assert(NWayTree(HalfFull)(n));                                                // Confirm leaf is half full
  NWayTree(Node) * const p = n->up;                                             // Parent of leaf
  assert(p);
  assert(NWayTree(HalfFull)(p) && p->up);                                       // Parent must have more than the minimum number of keys because we need to remove one unless it is the root of the tree

  const long i = NWayTree(IndexInParent)(n);                                    // Index of leaf in parent
  const long pl = NWayTree(Node_length)(p);
  const long nl = NWayTree(Node_length)(n);

  if (dir)                                                                      // Merge with right hand sibling
   {assert(i < pl);                                                             // Cannot fill from right
    const long I = i+1;
    NWayTree(Node) * const r = p->down[I];                                      // Leaf on right
    assert(NWayTree(HalfFull)(r));                                              // Confirm right leaf is half full
    const long rl = NWayTree(Node_length)(r);

    const NWayTreeDataType k = ArrayLongDelete((p->keys), pl, I);               // Transfer keys and data from parent
    const NWayTreeDataType d = ArrayLongDelete((p->data), pl, I);
    ArrayLongPush((n->keys), nl, k);
    ArrayLongPush((n->data), nl, d);

    ArrayLongPushArray((n->keys), nl+1, (r->keys), rl);                         // Transfer keys
    ArrayLongPushArray((n->data), nl+1, (r->data), rl);                         // Transfer data

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
    const long I = i-1;
    NWayTree(Node) * const l = p->down[I];                                      // Node on left
    assert(NWayTree(HalfFull)(l));                                              // Confirm left leaf is half full
    const long ll = NWayTree(Node_length)(l);
    const NWayTreeDataType k = ArrayLongDelete(p->keys, pl, I);                 // Transfer parent key and data
    const NWayTreeDataType d = ArrayLongDelete(p->data, pl, I);
    ArrayLongUnShift     (n->keys, nl,   k);
    ArrayLongUnShift     (n->data, nl,   d);
    ArrayLongUnShiftArray(n->keys, nl+1, (l->keys), ll);                        // Transfer left keys and data
    ArrayLongUnShiftArray(n->data, nl+1, (l->data), ll);

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

static void NWayTree(Merge)                                                     //P Merge the current node with its sibling.
 (NWayTree(Node) * const node)                                                  // Node to merge into
 {const long i = NWayTree(IndexInParent)(node);                                 // Index in parent
  if (i)                                                                        // Merge with left node
   {NWayTree(Node) * const l = node->up->down[i-1];                             // Left node
    NWayTree(Node) * const r = node;
    if (NWayTree(HalfFull)(r))
     {NWayTree(HalfFull)(l) ? NWayTree(MergeWithLeftOrRight)(r, 0):
                              NWayTree(FillFromLeftOrRight) (r, 0);             // Merge as left and right nodes are half full
     }
   }
  else
   {NWayTree(Node) * const r = node->up->down[1];                               // Right node
    NWayTree(Node) * const l = node;
    if (NWayTree(HalfFull)(l))
     {NWayTree(HalfFull)(r) ? NWayTree(MergeWithLeftOrRight)(l, 1):
                              NWayTree(FillFromLeftOrRight) (l, 1);             // Merge as left and right nodes are half full
     }
   }
 }

static void NWayTree(MergeOrFill)                                               //P Make a node larger than a half node.
 (NWayTree(Node) * const node)                                                  // Node to merge or fill
 {if (NWayTree(HalfFull)(node)) return;                                         // No need to merge of if not a half node
  NWayTree(Node) * const p = node->up;                                          // Parent exists

  if (p->up)                                                                    // Merge or fill parent which is not the root
   {NWayTree(MergeOrFill)(p);
    NWayTree(Merge)(node);
   }
  else
   {NWayTree(Node) * const l = p->down[0];
    NWayTree(Node) * const r = p->down[1];
    const long pl = NWayTree(Node_length)(p);
    if (pl == 1 && NWayTree(HalfFull)(l) && NWayTree(HalfFull)(r))              // Parent is the root and it only has one key - merge into the child if possible
     {const long L = NWayTree(Node_length)(l);
      const long R = NWayTree(Node_length)(r);
      const long N = NWayTree(Node_length)(node);
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

      NWayTree(ReUp)(p);                                                        // Reconnect children to parent
     }
    else                                                                        // Parent is the root but it has too may keys to merge into both sibling so merge with a sibling if possible
     {NWayTree(Merge)(node);
     }
   }
 }

static void NWayTree(Insert)                                                    // Insert a key and its associated data into a tree
 (NWayTree(Tree) * const tree,                                                  // Tree to insert into
  NWayTreeDataType const key, NWayTreeDataType const data)                      // Key to insert, data associated with key
 {if (!tree->node)                                                              // Empty tree
   {NWayTree(Node) * const n = NWayTree(NewNode)(tree);
    (n->keys[0]) = key;
    (n->data[0]) = data;
    NWayTree(Node_setLength)(n, 1);
    tree->keys++;
    tree->node   = n;
    return;
   }

  NWayTree(Node) * const n = tree->node;
  const long nl = NWayTree(Node_length)(n);

  if (nl < NWayTree(MaximumNumberOfKeys)(tree) &&                               // Node is root with no children and room for one more key
     !n->up && NWayTree(IsLeaf)(n))
   {for(long i = 0; i < nl; ++i)                                                // Each key
     {if (key == (n->keys[i]))                                                  // Key already present
       {(n->data[i]) = data;
        return;
       }
      if (key < (n->keys[i]))                                                   // We have reached the insertion point
       {ArrayLongInsert(n->keys, nl+1, key,  i);
        ArrayLongInsert(n->data, nl+1, data, i);
        NWayTree(Node_setLength)(n, nl+1);
        tree->keys++;
        return;
       }
     }
    ArrayLongPush(n->keys, nl, key);                                            // Insert the key at the end of the block because it is greater than all the other keys in the block
    ArrayLongPush(n->data, nl, data);
    NWayTree(Node_setLength)(n, nl+1);
    tree->keys++;
   }
  else                                                                          // Insert node
   {NWayTree(FindResult) const r = NWayTree(FindAndSplit)(tree, key);           // Check for existing key
    NWayTree(Node) * const n = r.node;
    if (NWayTree(FindResult_cmp)(r) == NWayTree(FindComparison_equal))                                // Found an equal key whose data we can update
     {(n->data[r.index]) = data;
     }
    else                                                                        // We have room for the insert
     {long index = r.index;
      if (NWayTree(FindResult_cmp)(r) == NWayTree(FindComparison_higher)) ++index;                    // Position at which to insert new key
      const long nl = NWayTree(Node_length)(n);
      ArrayLongInsert(n->keys, nl+1, key,  index);
      ArrayLongInsert(n->data, nl+1, data, index);

      NWayTree(Node_setLength)(n, nl+1);
      NWayTree(SplitFullNode)(n);                                               // Split if the leaf is full to force keys up the tree
     }
   }
 }

static NWayTree(FindResult) NWayTree(IterStart)                                 // Start an iterator
 (NWayTree(Tree) * const tree)                                                  // Tree to iterate
 {NWayTree(FindResult) f = NWayTree(GoAllTheWayLeft)(tree->node);
  return f;
 }

static long NWayTree(IterCheck)                                                 // True if we can continue to iterate
 (NWayTree(FindResult) const find)                                              // Find result of last iteration
 {return NWayTree(FindResult_cmp)(find) != NWayTree(FindComparison_notFound);
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

#if (__INCLUDE_LEVEL__ == 1)
void test_3_1()                                                                 // Tests
 {NWayTree(Tree) * const t = NWayTree(NewTree)(3);
  for(int i = 0; i < 1; ++i) NWayTree(Insert)(t, i, 2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
));
 }

void test_31_1()                                                                // Tests
 {NWayTree(Tree) * const t = NWayTree(NewTree)(31);
  for(int i = 0; i < 1; ++i) NWayTree(Insert)(t, i, 2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
));
 }

void test_3_2()                                                                 // Tests
 {NWayTree(Tree) * const t = NWayTree(NewTree)(3);
  for(int i = 0; i < 2; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
));
 }

void test_31_2()                                                                // Tests
 {NWayTree(Tree) * const t = NWayTree(NewTree)(31);
  for(int i = 0; i < 2; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
));
 }

void test_3_3()                                                                 // Tests
 {NWayTree(Tree) * const t = NWayTree(NewTree)(3);
  for(int i = 0; i < 3; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   0                                   2\n"
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_31_3()                                                                // Tests
 {NWayTree(Tree) * const t = NWayTree(NewTree)(31);
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
  (n->keys[0]) = a; (n->data[0]) = 2*a;
  (n->keys[1]) = b; (n->data[1]) = 2*b;
  (n->keys[2]) = c; (n->data[2]) = 2*c;
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
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;
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
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;

  assert(p->down[1] == n1);

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
  p->down[0] = n0; n0->up = p;
  p->down[1] = n1; n1->up = p;
  p->down[2] = n2; n2->up = p;
  t->node    = p;

  assert(p->down[1] == n1);

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
  //NWayTree(ErrAsC)(t);
 }

void test_3_4()                                                                 // Tests
 {test_3_4a();
  test_3_4b();
  test_3_4c();
  test_3_4d();
 }

void test_3_insert1()                                                           // Insert tests
 {const long N = 1;
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   3\n"
));
 }

void test_3_insert2()
 {const long N = 2;
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
  for(int i = 1; i <= N; ++i) NWayTree(Insert)(t, i, i+2);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   3\n"
"   2                                   4\n"
));
 }

void test_3_insert3()
 {const long N = 3;
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
  for(int i = 0; i < N; ++i) NWayTree(Insert)(t, N-i, N-i+1);
  //NWayTree(ErrAsC)(tree);
  assert(NWayTree(EqText)(t,
"   1                                   2\n"
"   2                                   3\n"
));
 }

void test_3_insert3r()
 {const long N = 3;
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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
  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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

  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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

  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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

  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
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

  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
  for(long i = 0; i < N; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  for(long i = 0; i < N; ++i)
   {NWayTree(FindResult) r = NWayTree(Find)(t, A[i]);
    assert(NWayTree(FindResult_data(r)) == i);
    assert(NWayTree(FindResult_cmp)(r)  == NWayTree(FindComparison_equal));
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

  NWayTree(Tree) * const t = NWayTree(NewTree)(3);                              // Create the tree
  for(long i = 0; i < NN; ++i)
   {NWayTree(Insert)(t, A[i], i);
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

  NWayTree(Tree) * const t = NWayTree(NewTree)(31);
  for(long i = 0; i < NN; ++i)
   {NWayTree(Insert)(t, A[i], i);
   }
  for(long i = 0; i < NN; ++i)
   {NWayTree(FindResult) r = NWayTree(Find)(t, A[i]);
    assert(NWayTree(FindResult_data(r)) == i);
    assert(NWayTree(FindResult_cmp)(r)  == NWayTree(FindComparison_equal));
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

  NWayTree(Tree) * const t = NWayTree(NewTree)(3);
  for(long i = 0; i < N;     ++i) NWayTree(Insert)(t, i*2, i*2);
  //NWayTree(ErrAsC)(tree);
  for(long i =-1; i < 2 * N; ++i)
   {NWayTree(FindResult) r = NWayTree(Find)(t, i);
    //NWayTree(ErrFindResult)(r);
    const long cmp = NWayTree(FindResult_cmp)(r);
    assert(i % 2 == 0 ? cmp == NWayTree(FindComparison_equal) :
                        cmp != NWayTree(FindComparison_equal));

    if (i == -1) assert((r.node->keys[r.index]) ==  0 && cmp == NWayTree(FindComparison_lower)  && r.index == 0);
    if (i ==  0) assert((r.node->keys[r.index]) ==  0 && cmp == NWayTree(FindComparison_equal)  && r.index == 0);
    if (i ==  1) assert((r.node->keys[r.index]) ==  0 && cmp == NWayTree(FindComparison_higher) && r.index == 0);

    if (i == 11) assert((r.node->keys[r.index]) == 12 && cmp == NWayTree(FindComparison_lower)  && r.index == 0);
    if (i == 12) assert((r.node->keys[r.index]) == 12 && cmp == NWayTree(FindComparison_equal)  && r.index == 0);
    if (i == 13) assert((r.node->keys[r.index]) == 12 && cmp == NWayTree(FindComparison_higher) && r.index == 0);
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
