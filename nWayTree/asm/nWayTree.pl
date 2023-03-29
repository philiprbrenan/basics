#!/usr/bin/perl -I/home/phil/perl/zero/lib/
#-------------------------------------------------------------------------------
# Use Zero assembler language to implement a generic N-Way tree.
# Philip R Brenan at appaapps dot com, Appa Apps Ltd Inc., 2023
#-------------------------------------------------------------------------------
use v5.30;
package NWayTree::Assembler;
use warnings FATAL => qw(all);
use strict;
use Data::Dump qw(dump);
use Data::Table::Text qw(:all);
use Zero::Emulator qw(:all);
use utf8;
use Test::More qw(no_plan);

my $Tree = sub                                                                  # The structure of an n-way tree
 {my $t = Zero::Emulator::areaStructure("NWayTree_Structure");
     $t->name(q(keys));                                                         # Number of keys in tree
     $t->name(q(nodes));                                                        # Number of nodes in tree
     $t->name(q(NumberOfKeysPerNode));                                          # The maximum number of keys in a node of this tree
     $t->name(q(root));                                                         # Root node
     $t
 }->();

my $Node = sub                                                                  # The structure of an n-way tree node
 {my $n = Zero::Emulator::areaStructure("NWayTree_Node_Structure");
     $n->name(q(length));                                                       # The current number of keys in the node
     $n->name(q(id));                                                           # A number identifying this node within this tree
     $n->name(q(up));                                                           # Parent node unless at the root node
     $n->name(q(keys));                                                         # Keys associated with this node
     $n->name(q(data));                                                         # Data associated with each key associated with this node
     $n->name(q(down));                                                         # Next layer of nodes down from this node
     $n->name(q(tree));                                                         # The definition of the containing tree
     $n
 }->();

our $main;                                                                      # Assembly we are creating code in

sub NWayTree_new($)                                                             # Create a variable refering to a new tree descriptor
 {my ($n) = @_;                                                                 # Maximum number of keys per node in this tree

  my $new = Procedure 'NWayTree_new', sub
   {my ($p) = @_;                                                               # Procedure description
    my ($t, $n) = $p->variables->temporary(2);                                  # Tree, maximum number of keys per node in this tree
    Alloc $t;                                                                   # Allocate tree descriptor
    ParamsGet $n, \0;                                                           # Maximum number of keys per node
    Mov $Tree->address(q(NumberOfKeysPerNode)), $t, $n, undef;                  # Save maximum number of keys per node
    Mov $Tree->address(q(root)),                $t,  0, undef;                  # Clear root
    Mov $Tree->address(q(keys)),                $t,  0, undef;                  # Clear keys
    Mov $Tree->address(q(nodes)),               $t,  0, undef;                  # Clear nodes
    ReturnPut 0, $t;                                                            # Return id of area containing tree descriptor
    Return;
   };

  ParamsPut \0, $n;
  Call $new;                                                                    # Create a new tree descriptor
  my $t = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  ReturnGet $t, \0;                                                             # Id of area containing tree descriptor
  $t
 }

sub NWayTree_numberOfKeysPerNode($)                                             # Get the maximum number of keys per node for a tree
 {my ($tree) = @_;                                                              # Tree to examine
  my $n = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  Mov $n, $tree, $Tree->address(q(NumberOfKeysPerNode));                        # Get attribute from tree descriptor
  $n
 };


sub NWayTree_maximumNumberOfKeys($)                                             # Get the maximum number of keys per node for a tree
 {my ($tree) = @_;                                                              # Tree to examine
  my $n = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  Mov $n, $Tree->address(q(NumberOfKeysPerNode)), $tree;                        # Get attribute from tree descriptor
  $n
 };

sub NWayTree_root($)                                                            # Get the root node of a tree
 {my ($tree) = @_;                                                              # Tree to examine
  my $r = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  Mov $r, $Tree->address(q(root)), $tree;                                       # Get attribute from tree descriptor
  $r                                                                            # Memory location holding root
 };

sub NWayTree_setRoot($$)                                                        # Set the root node of a tree
 {my ($tree, $name) = @_;                                                       # Tree, name of variable referencing root
  Mov $Tree->address(q(root)), $tree, $name, undef;                             # Set root attribute
 };

sub NWayTree_incKeys($)                                                         # Increment the number of keys in a tree
 {my ($tree) = @_;                                                              # Tree
  Inc $Tree->address(q(keys)), $tree;                                           # Number of keys
 };

sub NWayTree_incNodes($)                                                        # Increment the number of nodes n a tree
 {my ($tree) = @_;                                                              # Tree
  Inc $Tree->address(q(nodes)), $tree;                                          # Number of nodes
 };

sub NWayTree_new($)                                                             # Create a variable refering to a new tree descriptor
 {my ($n) = @_;                                                                 # Maximum number of keys per node in this tree

  my $new = Procedure 'NWayTree_new', sub
   {my ($p) = @_;                                                               # Procedure description
    my ($t, $n) = $p->variables->temporary(2);                                  # Tree, maximum number of keys per node in this tree
    Alloc $t;                                                                   # Allocate tree descriptor
    ParamsGet $n, \0;                                                           # Maximum number of keys per node
    Mov $Tree->address(q(NumberOfKeysPerNode)), $t, $n, undef;                  # Save maximum number of keys per node
    Mov $Tree->address(q(root)),                $t,  0, undef;                  # Clear root
    Mov $Tree->address(q(keys)),                $t,  0, undef;                  # Clear keys
    Mov $Tree->address(q(nodes)),               $t,  0, undef;                  # Clear nodes
    ReturnPut 0, $t;                                                            # Return id of area containing tree descriptor
    Return;
   };

  ParamsPut \0, $n;
  Call $new;                                                                    # Create a new tree descriptor
  my $t = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  ReturnGet $t, \0;                                                             # Id of area containing tree descriptor
  $t
 }

sub NWayTree_Node_length($)                                                     # Get number of keys in a node
 {my ($node) = @_;                                                              # Node
  my $l = $main->variables->temporary;                                          # Temporary variable
  Mov $l, $Node->address(q(length), $node;                                      # Get length
  $l                                                                            # Memory location holding length
 };

//#define NWayTree_Node_length(l, node)          const long l = node->length;
//#define NWayTree_Node_setLength(node, n)       node->length = n;
//#define NWayTree_Node_id(i, node)              const long i = node->id;
//#define NWayTree_Node_up(u, node)              NWayTree(Node) * const u = node->up;
//#define NWayTree_Node_setUp(node, n)           node->up = n;
//#define NWayTree_Node_tree(t, node)            NWayTree(Tree) * const t = node->tree;
//
//#define NWayTree_Node_keys(k, node, index)     const NWayTreeDataType k = node->keys[index];
//#define NWayTree_Node_data(d, node, index)     const NWayTreeDataType d = node->data[index];
//#define NWayTree_Node_down(n, node, index)     NWayTree(Node) * const n = node->down[index];
//#define NWayTree_Node_isLeaf(l, node)          const long l = node->down[0] == 0;
//
//#define NWayTree_Node_setKeys(node, index, k)  node->keys[index] = k;
//#define NWayTree_Node_setData(node, index, d)  node->data[index] = d;
//#define NWayTree_Node_setDown(node, index, n)  node->down[index] = n;
//
//#define NWayTree_FindResult(f, code)           const NWayTree(FindResult) f = code;
//#define NWayTree_FindResult_Key(k, f)          const NWayTreeDataType k = f.key;
//#define NWayTree_FindResult_Data(d, f)         const NWayTreeDataType d = NWayTree(FindResult_data)(f);
//#define NWayTree_FindResult_cmp(c, f)          const long c = f.cmp;
//#define NWayTree_FindResult_Index(i, f)        const long i = f.index;
//#define NWayTree_FindResult_node(n, f)         NWayTree(Node) * const n = f.node;
//#define NWayTree_FindComparison(f, value)      const NWayTree(FindComparison) f = NWayTree(FindComparison_##value)

ok $Tree->offset(q(nodes)) == 1;
ok $Node->offset(q(tree))  == 6;

if (1)
 {$main = Start 1;

  my $t = NWayTree_new(3);

  my $r = NWayTree_root($t);

  NWayTree_setRoot($t, 1);
  my $R = NWayTree_root($t);

  my $n = NWayTree_maximumNumberOfKeys($t);

  NWayTree_incKeys($t) for 1..3;
  Out $Tree->address(q(keys)), $t;

  NWayTree_incNodes($t) for 1..5;
  Out $Tree->address(q(nodes)), $t;

  my $e = Execute;
  is_deeply $e->out         => [3, 5];
  is_deeply $e->memory->{6} => [3, 5, 3, 1];
 }

done_testing;
