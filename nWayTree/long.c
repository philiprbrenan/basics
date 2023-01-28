//------------------------------------------------------------------------------
// N way tree with 8 long keys
//Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// // 1,057,170 instructions executed
// We only copy the upper partition into the work area and then work down because the upper partition can be smaller in size than the lower one.
// Using binary search seems to slow things down

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

#define NWayTreeLongNumberOfKeysPerNode 8

typedef struct NWayTreeLongNode                                                 // A node in a tree
 {long length;
  long keys[NWayTreeLongNumberOfKeysPerNode];
  long data[NWayTreeLongNumberOfKeysPerNode];
  struct NWayTreeLongNode *down[NWayTreeLongNumberOfKeysPerNode+1];
  struct NWayTreeLongNode *up;
 } NWayTreeLongNode;

typedef struct NWayTreeLongTree                                                 // The root of a tree
 {NWayTreeLongNode *node;
 } NWayTreeLongTree;

typedef enum NWayTreeLongFindComparison                                         // The results of a comparison
 {lower, equal, higher, notFound
 } NWayTreeLongFindComparison;

typedef struct NWayTreeLongFindResult                                           // The results of a find operation
 {NWayTreeLongTree *tree;                                                       // Tree searched
  NWayTreeLongNode *node;                                                       // Node found
  NWayTreeLongFindComparison cmp;                                               // Result of the last comparison
  long index;                                                                   // Index in the node of equal element
 } NWayTreeLongFindResult;


static NWayTreeLongTree *NWayTreeLongNewTree()                                  // Create a new tree
 {NWayTreeLongTree *tree = calloc(sizeof(NWayTreeLongTree), 1);
  return tree;
 }

static NWayTreeLongNode *NWayTreeLongNewNode()                                  // Create a new node
 {NWayTreeLongNode *node = calloc(sizeof(NWayTreeLongNewNode), 1);
  return node;
 }

static NWayTreeLongFindResult NWayTreeLongNewFindResult                         // New find result on stack
 (NWayTreeLongTree *tree, NWayTreeLongNode *node,
  NWayTreeLongFindComparison cmp, long index)
 {NWayTreeLongFindResult r;
  r.tree  = tree;
  r.node  = node;
  r.cmp   = cmp;
  r.index = index;
  return r;
 }

static void NWayTreeLongInsert                                                  // Insert a key and its associated data into a tree
 (NWayTreeLongTree *tree, long key, long data)
 {tree = tree; key = key; data = data;
 }

static long NWayTreeLongMinimumNumberOfKeys()                                   //P Minimum number of keys per node.
 {return (NWayTreeLongNumberOfKeysPerNode - 1) / 2;
 }

static long NWayTreeLongMaximumNumberOfKeys()                                                       //P Maximum number of keys per node.
 {return NWayTreeLongNumberOfKeysPerNode;
 }

static long NWayTreeLong NWayTreeLongMaximumNumberOfNodes()                                                      //P Maximum number of children per parent.
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
//static void NWayTreeLong root(NWayTreeLongTree *tree)                                                                     //  Return the root node of a tree.
// {my ($tree) = @_;                                                              //  Tree
//  confess unless $tree;
//  for(; $tree->up; $tree = $tree->up) {}
//  $tree
// }

static long NWayTreeLongIsLeaf(NWayTreeLongNode *node)                          //  Confirm that the tree is a leaf.
 {return node->down[0] == 0;                                                    //  No children so it must be a leaf
 }

static void NWayTreeLongReUp(NWayTreeLongNode *node)                            //P Reconnect the children to their new parent.
 {for(int i = 0; i < node->length; ++i)
   {node->down[i]->up = node;
   }
 }

static void NWayTreeLongSplitFullNode                                           //P Split a node if it is full.
 (NWayTreeLongTree *tree, NWayTreeLongNode *node)
 {if (node->length < NWayTreeLongMaximumNumberOfKeys()) return;                 // Must be a full node

  NWayTreeLongNode * p = 0;                                                     //Parent node
  if (node->up)
   {p = node->up;
   }
  else                                                                          // Create new parent as we are splitting the top most node
   {p = NWayTreeLongNewNode();                                                  // New child nodes
    tree->node = p;
   }

  NWayTreeLongNode
    * const l = NWayTreeLongNewNode(),                                          // New child nodes
    * const r = NWayTreeLongNewNode();

  long N = NWayTreeLongMaximumNumberOfNodes() / 2;                              // Split points
  long n = NWayTreeLongMaximumNumberOfKeys() % 2 == 0 ? N - 1 : N - 2;

  l->length = n;
  r->length = NWayTreeLongMaximumNumberOfKeys() - n - 2;
  memcpy(l->keys, node->keys,        l->length  * sizeof(long));                // Split left keys and data
  memcpy(l->data, node->data,        l->length  * sizeof(long));
  memcpy(l->down, node->down,     (1+l->length) * sizeof(long));

  memcpy(r->keys, node->keys+n+2,    r->length  * sizeof(long));                // Split right keys and data
  memcpy(r->data, node->data+n+2,    r->length  * sizeof(long));
  memcpy(r->down, node->down+n+2, (1+r->length) * sizeof(long));

  if (!NWayTreeLongIsLeaf(node))                                                // Not a leaf node
   {NWayTreeLongReUp(l);
    NWayTreeLongReUp(r);
   }

  if (node->up)                                                                 // Not a root node
   {const long L = sizeof(long);
    NWayTreeLongNode *p = node->up;                                             // Existing parent node
    l->up = r->up = p;                                                          // Connect children to parent
    if (p->down[0] == node)                                                     // Splitting the first child - move everything up
     {memmove(p->keys+L, p->keys,  p->length    * L);
      memmove(p->data+L, p->data,  p->length    * L);
      memmove(p->down+L, p->down, (p->length++) * L);
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
     {for(long i = 1; i < p->length - 1; ++i)
       {if (p->down[i] == node)                                             //  Find the node that points from the parent to the current node
         {memmove(p->keys+L*(i+1), p->keys+L*i, (p->length-1) * L);
          memmove(p->data+L*(i+1), p->data+L*i, (p->length-1) * L);
          memmove(p->down+L*(i+1), p->down+L*i, (p->length-1) * L);
          p->keys[i]   = node->keys[n];
          p->data[i]   = node->data[n];
          p->down[i-1] = l;
          p->down[i]    = r;
          ++p->length;
          break;
         }
       }
      assert(0);
     }
   }
  else                                                                          // Root node with single key after split
   {NWayTreeLongNode *p = NWayTreeLongNewNode();                                // Existing parent node
    l->up = r->up = p;                                                          // Connect children to parent

    p->keys[0] = node->keys[n+1];                                               // Single key
    p->data[0] = node->data[n+1];                                               //  Data associated with single key
    p->down[0] = l;
    p->down[1] = r;
    p->length  = 1;
   }
 }

static NWayTreeLongFindResult NWayTreeLongFindAndSplit                          //P Find a key in a tree splitting full nodes along the path to the key.
 (NWayTreeLongTree *tree, long key)
 {NWayTreeLongNode *node = tree->node;
  if (!node) return NWayTreeLongNewFindResult(tree, node, notFound, -1);

  NWayTreeLongSplitFullNode(tree, node);                                        // Split the root node if necessary

  for(long j = 0; j < 999; ++j)                                                 // Step down through the tree
   {if (key < node->keys[0])                                                    // Less than smallest key in node
     {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult
       (tree, node, lower, 0);                                                  // Smallest key in tree
      node = node->down[0];
      continue;
     }

    const long last = node->length-1;                                           // Greater than largest key in node
    if (key > node->keys[last])                                                 // Greater than largest key in node
     {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult
       (tree, node, higher, last);
      node = node->down[last+1];
      continue;
     }

    for(long i = 1; i < last; ++i)                                              // Search the keys in this node as greater than least key and less than largest key
     {if (key == node->keys[i])                                                // Found key
       {return NWayTreeLongNewFindResult(tree, node, equal, i);
       }
      else if (key > node->keys[i])                                             //  Greater than current key
       {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult          //  Leaf
         (tree, node, higher, i);
        node = node->down[i+1];
       }
     }
    NWayTreeLongSplitFullNode(tree, node);                                      // Split the node we have stepped to
   }
  assert(0);
 }

static NWayTreeLongFindResult NWayTreeLongFind                                  // Find a key in a tree returning its associated data or undef if the key does not exist.
 (NWayTreeLongTree *tree, long key)
 {NWayTreeLongNode *node = tree->node;
  if (!node) return NWayTreeLongNewFindResult(tree, node, notFound, -1);        // Empty tree

  for(long i = 0; i < 999; ++i)                                                 // Same code as above
   {if (key < node->keys[0])                                                    // Less than smallest key in node
     {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult
       (tree, node, lower, 0);
      node = node->down[0];
     }

    const long last = node->length-1;                                           // Index of last key
    if (key > node->keys[last])                                                 // Greater than largest key in node
     {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult
       (tree, node, higher, last);
      node = node->down[last+1];
      continue;
     }

    for(long i = 1; i < last; ++i)                                              // Search the keys in this node as greater than least key and less than largest key
     {if (key == node->keys[i])                                                 // Found key
       {return NWayTreeLongNewFindResult(tree, node, equal, i);
       }
      else if (key > node->keys[i])                                             // Greater than current key
       {if (NWayTreeLongIsLeaf(node)) return NWayTreeLongNewFindResult          // Leaf
         (tree, node, higher, i);
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
 {NWayTreeLongNode *p = n->up;                                                  //  Parent of leaf
  assert(p);
  const long i = NWayTreeLongIndexInParent(n);                                  //  Index of leaf in parent

  if (dir)                                                                      //  Fill from right
   {assert(i < p->length);                                                      //  Cannot fill from right
    NWayTreeLongNode *r = p->down[i+1];                                         //  Right sibling
    n->keys[n->length] = p->keys[i]; p->keys[i] = ArrayLongShift(r->keys, r->length);  //  Transfer key
    n->data[n->length] = p->data[i]; p->data[i] = ArrayLongShift(r->data, r->length);  //  Transfer data
    if (!NWayTreeLongIsLeaf(n))                                                   //  Transfer node if not a leaf
     {ArrayVoidPush((void *)n->down, n->length, (void *)ArrayVoidShift((void *)r->down, r->length));
      n->down[n->length+1]->up = n;
     }
    r->length--; n->length++;
   }
  else                                                                          //  Fill from left
   {assert(i);                                                                  //  Cannot fill from left
    long I = i-1;
    NWayTreeLongNode *n = p->down[I];                                           //  Left sibling
    ArrayLongUnShift(n->keys, n->length, p->keys[I]); p->keys[I] = ArrayLongPop(n->keys, n->length); //  Transfer key
    ArrayLongUnShift(n->data, n->length, p->data[I]); p->data[I] = ArrayLongPop(n->data, n->length); //  Transfer data
    if (!NWayTreeLongIsLeaf(n))                                                 //  Transfer node if not a leaf
     {ArrayVoidUnShift((void *)n->down, n->length, (void *)ArrayVoidPop((void *)n->down, n->length));
      n->down[0]->up = n;
     }
   }
 }

static void NWayTreeLong mergeWithLeftOrRight                                   //P Merge two adjacent nodes.
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

    ArrayLongPush(n->keys, ->@*, splice($p->keys->@*, $i, 1);                             // Transfer keys
    push $n->keys->@*, splice($p->keys->@*, $i, 1);                             // Transfer keys
    push $n->keys->@*, $r->keys->@*;

    push $n->data->@*, splice($p->data->@*, $i, 1), $r->data->@*;               // Transfer data
    if (!leaf $n)                                                               // Children of merged node
     {push $n->node->@*, $r->node->@*;                                          // Children of merged node
      reUp $n, $r->node;                                                        // Update parent of children of right node
     }
    splice $p->node->@*, $I, 1;                                                 // Remove link from parent to right child
   }
  else                                                                          // Merge with left hand sibling
   {$i > 0 or confess;                                                          // Cannot fill from left
    long I = $i-1;
    long l = $p->node->[$I];                                                    // Node on left
    confess unless halfFull($l);                                                // Confirm right leaf is half full
    unshift $n->keys->@*, $l->keys->@*, splice $p->keys->@*, $I, 1;             // Transfer keys
    unshift $n->data->@*, $l->data->@*, splice $p->data->@*, $I, 1;             // Transfer data
    if (!leaf $n)                                                               // Children of merged node
     {unshift $n->node->@*, $l->node->@*;                                       // Children of merged node
      reUp $n, $l->node;                                                        // Update parent of children of left node
     }
    splice $p->node->@*, $I, 1;                                                 // Remove link from parent to left child
   }
 }

static void NWayTreeLong merge(NWayTreeLongTree *tree)                          //P Merge the current node with its sibling.
 {my ($tree) = @_;                                                              // Tree
  if (long i = indexInParent $tree)                                             // Merge with left node
   {long l = $tree->up->node->[$i-1];                                           // Left node
    if (halfFull(long r = $tree))
     {$l->halfFull ? mergeWithLeftOrRight $r, 0 : fillFromLeftOrRight $r, 0;    // Merge as left and right nodes are half full
     }
   }
  else
   {long r = $tree->up->node->[1];                                               //  Right node
    if (halfFull(long l = $tree))
     {halfFull($r) ? mergeWithLeftOrRight $l, 1 : fillFromLeftOrRight $l, 1;    //  Merge as left and right nodes are half full
     }
   }
 }

static void NWayTreeLong mergeOrFill(NWayTreeLongTree *tree)                                                              //P Make a node larger than a half node.
 {my ($tree) = @_;                                                              //  Tree
  @_ == 1 or confess;

  return  unless halfFull($tree);                                               //  No need to merge of if not a half node
  confess unless long p = $tree->up;                                             //  Parent exists

  if ($p->up)                                                                   //  Merge or fill parent which is not the root
   {__SUB__->($p);
    merge($tree);
   }
  elsif ($p->keys->@* == 1 and halfFull(long l = $p->node->[0])                  //  Parent is the root and it only has one key - merge into the child if possible
                           and halfFull(long r = $p->node->[1]))
   {$p->keys = $tree->keys = [$l->keys->@*, $p->keys->@*, $r->keys->@*];        //  Merge in place to retain addressability
    $p->data = $tree->data = [$l->data->@*, $p->data->@*, $r->data->@*];
    $p->node = $tree->node = [$l->node->@*,               $r->node->@*];

    reUp $p, $p->node;                                                          //  Reconnect children to parent
   }
  else                                                                          //  Parent is the root but it has too may keys to merge into both sibling so merge with a sibling if possible
   {merge($tree);
   }
 }

static void NWayTreeLong leftMost(NWayTreeLongTree *tree)                                                                 //  Return the left most node below the specified one.
 {my ($tree) = @_;                                                              //  Tree
  for(0..999)                                                                   //  Step down through tree
   {return $tree if leaf $tree;                                                 //  We are on a leaf so we have arrived at the left most node
    $tree = $tree->node->[0];                                                   //  Go left
   }
  confess "Should not happen";
 }

static void NWayTreeLong rightMost($)                                                                //  Return the right most node below the specified one.
 {my ($tree) = @_;                                                              //  Tree
  for(0..999)                                                                   //  Step down through tree
   {return $tree if leaf $tree;                                                 //  We are on a leaf so we have arrived at the left most node
    $tree = $tree->node->[-1];                                                  //  Go right
   }
  confess "Should not happen";
 }

static void NWayTreeLong height($)                                                                   //  Return the height of the tree.
 {my ($tree) = @_;                                                              //  Tree
  for long n(0..999)                                                             //  Step down through tree
   {if (leaf $tree)                                                             //  We are on a leaf
     {return $n + 1 if $tree->keys->@*;                                         //  We are in a partially full leaf
      return $n;                                                                //  We are on the root and it is empty
     }
    $tree = $tree->node->[0];
   }
  confess "Should not happen";
 }

static void NWayTreeLong depth($)                                                                    //  Return the depth of a node within a tree.
 {my ($tree) = @_;                                                              //  Tree
  return 0 if !$tree->up and !$tree->keys->@*;                                  //  We are at the root and it is empty
  for long n(1..999)                                                             //  Step down through tree
   {return $n  unless $tree->up;                                                //  We are at the root
    $tree = $tree->up;
   }
  confess "Should not happen";
 }

static void NWayTreeLong deleteLeafKey($$)                                                           //P Delete a key in a leaf.
 {my ($tree, $i) = @_;                                                          //  Tree, index to delete at
  @_ == 2 or confess;
  confess "Not a leaf" unless leaf $tree;
  long key = $tree->keys->[$i];
  mergeOrFill $tree if $tree->up;                                               //  Merge and fill unless we are on the root and the root is a leaf
  long k = $tree->keys;
  for long j(keys @$k)                                                            # Search for key to delete
   {if ($$k[$j] == $key)
     {splice $tree->keys->@*, $j, 1;                                            //  Remove keys
      splice $tree->data->@*, $j, 1;                                            //  Remove data
      return;
     }
   }
 }

static void NWayTreeLong deleteKey($$)                                                               //P Delete a key.
 {my ($tree, $i) = @_;                                                          //  Tree, index to delete at
  @_ == 2 or confess;
  if (leaf $tree)                                                               //  Delete from a leaf
   {deleteLeafKey($tree, $i);
   }
  elsif ($i > 0)                                                                //  Delete from a node
   {long l = rightMost $tree->node->[$i];                                        //  Find previous node
    splice  $tree->keys->@*, $i, 1, $l->keys->[-1];
    splice  $tree->data->@*, $i, 1, $l->data->[-1];
    deleteLeafKey $l, -1 + scalar $l->keys->@*;                                 //  Remove leaf key
   }
  else                                                                          //  Delete from a node
   {long r = leftMost $tree->node->[1];                                          //  Find previous node
    splice  $tree->keys->@*,  0, 1, $r->keys->[0];
    splice  $tree->data->@*,  0, 1, $r->data->[0];
    deleteLeafKey $r, 0;                                                        //  Remove leaf key
   }
 }

static void NWayTreeLong delete($$)                                                                  //  Find a key in a tree, delete it and return any associated data.
 {my ($root, $key) = @_;                                                        //  Tree root, key
  @_ == 2 or confess;

  long tree = $root;
  for (0..999)
   {long k = $tree->keys;

    if ($key < $$k[0])                                                          //  Less than smallest key in node
     {return undef unless $tree = $tree->node->[0];
     }
    elsif ($key > $$k[-1])                                                      //  Greater than largest key in node
     {return undef unless $tree = $tree->node->[-1];
     }
    else
     {for long i(keys @$k)                                                       //  Search the keys in this node
       {if ((my  $s = $key <=> $$k[$i]) == 0)                                   //  Delete found key
         {long d = $tree->data->[$i];                                            //  Save data
          deleteKey $tree, $i;                                                  //  Delete the key
          return $d;                                                            //  Return data associated with key
         }
        elsif ($s < 0)                                                          //  Less than current key
         {return undef unless $tree = $tree->node->[$i];
          last;
         }
       }
     }
   }
  confess "Should not happen";
 }

static void NWayTreeLong insert($$$)                                                                 //  Insert the specified key and data into a tree.
 {my ($tree, $key, $data) = @_;                                                 //  Tree, key, data
  @_ == 3 or confess;

  if (!(long n = $tree->keys->@*))                                               //  Empty tree
   {push $tree->keys->@*, $key;
    push $tree->data->@*, $data;
    return $tree;
   }
  elsif ($n < NWayTreeLongMaximumNumberOfKeys and $tree->node->@* == 0)                     //  Node is root with no children and room for one more key
   {long k = $tree->keys;
    for long i(reverse keys @$k)                                                 //  Each key - in reverse due to the preponderance of already sorted data
     {if ((long s = $key <=> $$k[$i]) == 0)                                      //  Key already present
       {$tree->data->[$i]= $data;
        return;
       }
      elsif ($s > 0)                                                            //  Insert before greatest smaller key
       {long I = $i + 1;
        splice $tree->keys->@*, $I, 0, $key;
        splice $tree->data->@*, $I, 0, $data;
        return;
       }
     }
    unshift $tree->keys->@*, $key;                                              //  Insert the key at the start of the block because it is less than all the other keys in the block
    unshift $tree->data->@*, $data;
   }
  else                                                                          //  Insert node
   {my ($compare, $node, $index) = findAndSplit $tree, $key;                    //  Check for existing key

    if ($compare == 0)                                                          //  Found an equal key whose data we can update
     {$node->data->[$index] = $data;
     }
    else                                                                        //  We have room for the insert
     {++$index if $compare > 0;                                                 //  Position at which to insert new key
      splice $node->keys->@*, $index, 0, $key;
      splice $node->data->@*, $index, 0, $data;
      splitFullNode 0, 1, $node                                                 //  Split if the leaf is full to force keys up the tree
     }
   }
 }

static void NWayTreeLong iterator($)                                                                 //  Make an iterator for a tree.
 {my ($tree) = @_;                                                              //  Tree
  @_ == 1 or confess;
  long i = genHash(__PACKAGE__.'::Iterator',                                     //  Iterator
    tree  => $tree,                                                             //  Tree we are iterating over
    node  => $tree,                                                             //  Current node within tree
    pos   => undef,                                                             //  Current position within node
    key   => undef,                                                             //  Key at this position
    data  => undef,                                                             //  Data at this position
    count => 0,                                                                 //  Counter
    more  => 1,                                                                 //  Iteration not yet finished
   );
  $i->next;                                                                     //  First element if any
  $i                                                                            //  Iterator
 }

static void NWayTreeLong Tree::Multi::Iterator::next($)                                              //  Find the next key.
 {my ($iter) = @_;                                                              //  Iterator
  @_ == 1 or confess;
  confess unless long C = $iter->node;                                           //  Current node required

  ++$iter->count;                                                               //  Count the calls to the iterator

  long new  = static void NWayTreeLong                                                                //  Load iterator with latest position
   {my ($node, $pos) = @_;                                                      //  Parameters
    $iter->node = $node;
    $iter->pos  = $pos //= 0;
    $iter->key  = $node->keys->[$pos];
    $iter->data = $node->data->[$pos]
   };

  long done = static void NWayTreeLong {$iter->more = undef};                                         //  The tree has been completely traversed

  if (!defined($iter->pos))                                                     //  Initial descent
   {long l = $C->node->[0];
    return $l ? &$new($l->leftMost) : $C->keys->@* ? &$new($C) : &$done;        //  Start node or done if empty tree
   }

  long up = static void NWayTreeLong                                                                  //  Iterate up to next node that has not been visited
   {for(long n = $C; long p = $n->up; $n = $p)
     {long i = indexInParent $n;
      return &$new($p, $i) if $i < $p->keys->@*;
     }
    &$done                                                                      //  No nodes not visited
   };

  long i = ++$iter->pos;
  if (leaf $C)                                                                  //  Leaf
   {$i < $C->keys->@* ? &$new($C, $i) : &$up;
   }
  else                                                                          //  Node
   {&$new($C->node->[$i]->leftMost)
   }
 }

static void NWayTreeLong reverseIterator($)                                                          //  Create a reverse iterator for a tree.
 {my ($tree) = @_;                                                              //  Tree
  @_ == 1 or confess;
  long i = genHash(__PACKAGE__.'::ReverseIterator',                              //  Iterator
    tree  => root($tree),                                                       //  Tree we are iterating over
    node  => $tree,                                                             //  Current node within tree
    pos   => undef,                                                             //  Current position within node
    key   => undef,                                                             //  Key at this position
    data  => undef,                                                             //  Data at this position
    count => 0,                                                                 //  Counter
    less  => 1,                                                                 //  Iteration not yet finished
   );
  $i->prev;                                                                     //  Last element if any
  $i                                                                            //  Iterator
 }

static void NWayTreeLong Tree::Multi::ReverseIterator::prev($)                                       //  Find the previous key.
 {my ($iter) = @_;                                                              //  Iterator
  @_ == 1 or confess;
  confess unless long C = $iter->node;                                           //  Current node required

  ++$iter->count;                                                               //  Count the calls to the iterator

  long new  = static void NWayTreeLong                                                                //  Load iterator with latest position
   {my ($node, $pos) = @_;                                                      //  Parameters
    $iter->node = $node;
    $iter->pos  = $pos //= ($node->keys->@* - 1);
    $iter->key  = $node->keys->[$pos];
    $iter->data = $node->data->[$pos]
   };

  long done = static void NWayTreeLong {$iter->less = undef};                                         //  The tree has been completely traversed

  if (!defined($iter->pos))                                                     //  Initial descent
   {long l = $C->node->[-1];
    return $l ? &$new($l->rightMost) : $C->keys->@* ? &$new($C) : &$done;       //  Start node or done if empty tree
    return;
   }

  long up = static void NWayTreeLong                                                                  //  Iterate up to next node that has not been visited
   {for(long n = $C; long p = $n->up; $n = $p)
     {long i = indexInParent $n;
      return &$new($p, $i-1) if $i > 0;
     }
    &$done                                                                      //  No nodes not visited
   };

  long i = $iter->pos;
  if (leaf $C)                                                                  //  Leaf
   {$i > 0 ?  &$new($C, $i-1) : &$up;
   }
  else                                                                          //  Node
   {$i >= 0 ? &$new($C->node->[$i]->rightMost) : &$up
   }
 }

static void NWayTreeLong flat($@)                                                                    //  Print the keys in a tree from left right to make it easier to visualize the structure of the tree.
 {my ($tree, @title) = @_;                                                      //  Tree, title
  confess unless $tree;
  my @s;                                                                        //  Print
  long D;                                                                        //  Deepest
  for(long i = iterator root $tree; $i->more; $i->next)                          //  Traverse tree
   {long d = depth $i->node;
    $D = $d unless $D and $D > $d;
    $s[$d] //= '';
    $s[$d]  .= "   ".$i->key;                                                   //  Add key at appropriate depth
    long l = length $s[$d];
    for long j(0..$D)                                                            //  Pad all strings to the current position
     {long s = $s[$j] //= '';
      $s[$j] = substr($s.(' 'x999), 0, $l) if length($s) < $l;
     }
   }
  for long i(keys @s)                                                            //  Clean up trailing blanks so that tests are not affected by spurious white space mismatches
   {$s[$i] =~ s/\s+\n/\n/gs;
    $s[$i] =~ s/\s+\Z//gs;
   }
  unshift @s, join(' ', @title) if @title;                                      //  Add title
  join "\n", @s, '';
 }

static void NWayTreeLong print($;$)                                                                  //  Print the keys in a tree optionally marking the active key.
 {my ($tree, $i) = @_;                                                          //  Tree, optional index of active key
  confess unless $tree;
  my @s;                                                                        //  Print

  long print = static void NWayTreeLong                                                               //  Print a node
   {my ($t, $in) = @_;
    return unless $t and $t->keys and $t->keys->@*;

    my @t = ('  'x$in);                                                         //  Print keys staring the active key if known
    for long j(keys $t->keys->@*)
     {push @t, $t->keys->[$j];
      push @t, '<=' if defined($i) and $i == $j and $tree == $t;
     }
    push @s, join ' ', @t;                                                      //  Details of one node

    if (long nodes = $t->node)                                                   //  Each key
     {__SUB__->($_, $in+1) for $nodes->@*;
     }
   };

  &$print(root($tree), 0);                                                      //  Print tree

  join "\n", @s, ''
 }

static void NWayTreeLong size($)                                                                     //  Count the number of keys in a tree.
 {my ($tree) = @_;                                                              //  Tree
  @_ == 1 or confess;
  long n = 0;                                                                    //  Print

  long count = static void NWayTreeLong                                                               //  Print a node
   {my ($t) = @_;
    return unless $t and $t->keys and my @k = $t->keys->@*;
    $n += @k;
    if (long nodes = $t->node)                                                   //  Each key
     {__SUB__->($_) for $nodes->@*;
     }
   };

  &$count(root $tree);                                                          //  Count nodes in tree

  $n;
 }
#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {long N = 1;
  NWayTreeLongTree *tree = NWayTreeLongNewTree();

  for(int i = 0; i < N; ++i)
   {NWayTreeLongInsert(tree, i, i);
   }
 }

void tests()                                                                    // Tests
 {test1();
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
#endif
