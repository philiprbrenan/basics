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

my $FindResult = sub                                                            # The structure of a find result
 {my $f = Zero::Emulator::areaStructure("NWayTree_FindResult");
  $f->name(q(node));                                                            # Node found
  $f->name(q(cmp));                                                             # Result of the last comparison
  $f->name(q(key));                                                             # Key searched for
  $f->name(q(index));                                                           # Index in the node of located element
  $f
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

sub NWayTree_Node_new($)                                                        # Create a variable refering to a new node descriptor
 {my ($tree) = @_;                                                              # GTree node is being created in

  my $new = Procedure 'NWayTree_Node_new', sub
   {my ($p) = @_;                                                               # Procedure description
    my ($t, $n, $k, $d, $D) = $p->variables->temporary(5);                      # Tree, node, keys, data, down
    ParamsGet $t, 0;                                                            # Tree as a parameter
    Alloc $$_ for \($n, $k, $d, $D);                                            # Allocate node, keys, data, down areas
    Mov $Node->address(q(keys)), $n, $k, undef;                                 # Keys area
    Mov $Node->address(q(data)), $n, $d, undef;                                 # Data area
    Mov $Node->address(q(down)), $n, $D, undef;                                 # Down area
    Mov $Node->address(q(tree)), $n, $t, undef;                                 # Containing tree
    NWayTree_incNodes($tree);
    Mov $Node->address(q(id)),   $n, $Tree->address(q(nodes)), $t;              # Assign an id to this node within the tree
    ReturnPut 0, $n;                                                            # Return id of area containing node
    Return;
   };

  ParamsPut \0, $tree;                                                          # Tree parameter
  Call $new;                                                                    # Create a new node in the tree
  my $n = $main->variables->temporary;                                          # Create a variable to reference the new node
  ReturnGet $n, \0;                                                             # Return reference to new node
  $n
 }

sub NWayTree_Node_length($)                                                     # Get number of keys in a node
 {my ($node) = @_;                                                              # Node
  my $l = $main->variables->temporary;                                          # Temporary variable
  Mov $l, $Node->address(q(length)), $node;                                     # Get length
  $l                                                                            # Memory location holding length
 }

sub NWayTree_Node_setLength($$)                                                 # Set the length of a node
 {my ($node, $length) = @_;                                                     # Node, length
  Mov $Node->address(q(length)), $node, $length;                                # Set length attribute
 }

sub NWayTree_Node_id($)                                                         # Get id of a node
 {my ($node) = @_;                                                              # Node
  my $i = $main->variables->temporary;                                          # Temporary variable
  Mov $i, $Node->address(q(id)), $node;                                         # Get id
  $i                                                                            # Memory location holding length
 }

sub NWayTree_Node_up($)                                                         # Get parent node from this node
 {my ($node) = @_;                                                              # Node
  my $u = $main->variables->temporary;                                          # Temporary variable
  Mov $u, $Node->address(q(node)), $node;                                       # Get parent
  $u                                                                            # Memory location holding length
 }

sub NWayTree_Node_setUp($$)                                                     # Set the parent of a node
 {my ($node, $parent) = @_;                                                     # Node, parent node, area containing parent node reference
  Mov $Node->address(q(up)), $node, $parent;                                    # Set parent
 }

sub NWayTree_Node_tree($)                                                       # Get tree containing a node
 {my ($node) = @_;                                                              # Node
  my $t = $main->variables->temporary;                                          # Temporary variable
  Mov $t, $Node->address(q(tree)), $node;                                       # Get tree
  $t                                                                            # Memory location holding tree
 }

sub NWayTree_Node_getIndex($$$)                                                 # Get the indexed field from a node
 {my ($node, $index, $field) = @_;                                              # Node, index of field, field name
  my ($F, $f) = $main->variables->temporary(2);                                 # Fields, field
  Mov $F, $Node->address($field), $node;                                        # Fields
  Mov $f, undef, $index, $F;                                                    # Field
  $f                                                                            # Memory location holding field
 }

sub NWayTree_Node_keys($$)                                                      # Get the indexed key from a node
 {my ($node, $index) = @_;                                                      # Node, index of key
  NWayTree_Node_getIndex($node, $index, q(keys));                               # Keys
 }

sub NWayTree_Node_data($$)                                                      # Get the indexed data from a node
 {my ($node, $index) = @_;                                                      # Node, index of data
  NWayTree_Node_getIndex($node, $index, q(data));                               # Data
 }

sub NWayTree_Node_down($$)                                                      # Get the indexed child node from a node
 {my ($node, $index) = @_;                                                      # Node, index of child
  NWayTree_Node_getIndex($node, $index, q(down));                               # Child
 }

sub NWayTree_Node_isLeaf($$)                                                    # Put 1 in a temporary variable if a node is a leaf else 0
 {my ($node) = @_;                                                              # Node
  my $l = $main->variables->temporary;                                          # Leaf
  my $d = NWayTree_Node_down($node, 0);                                         # First child
  IfEq $d, 0,
  Then
   {Mov $l, 1;                                                                  # Leaf
   },
  Else
   {Mov $l, 0;                                                                  # Not a leaf
   };
  $l                                                                            # Memory location holding leaf flag
 }

sub NWayTree_Node_setIndex($$$$)                                                # Set an indexed field to a specified value
 {my ($node, $index, $field, $value) = @_;                                      # Node, index, field name, value
  my $F = $main->variables->temporary;                                          # Indexed fields
  Mov $F, undef, $Node->address($field), $node;                                 # Fields
  Mov $index, $F, $value;                                                       # Set field to value
 }

sub NWayTree_Node_setKeys($$$)                                                  # Set a key by index
 {my ($node, $index, $value) = @_;                                              # Node, index, value
  NWayTree_Node_setIndex($node, $index, q(keys), $value)                        # Set indexed key
 }

sub NWayTree_Node_setData($$$)                                                  # Set a data field by index
 {my ($node, $index, $value) = @_;                                              # Node, index, value
  NWayTree_Node_setIndex($node, $index, q(data), $value)                        # Set indexed key
 }

sub NWayTree_Node_setDown($$$)                                                  # Set a child by index
 {my ($node, $index, $value) = @_;                                              # Node, index, value
  NWayTree_Node_setIndex($node, $index, q(down), $value)                        # Set indexed key
 }

#define NWayTree_FindResult(f, code)           const NWayTree(FindResult) f = code;

sub NWayTree_FindResult_getField($$)                                            # Get a field from a find result
 {my ($findResult, $field) = @_;                                                # Find result, name of field
  my $f = $main->variables->temporary;                                          # Field value
  Mov $f, undef, $FindResult->address($field), $findResult;                     # Fields
  $f                                                                            # Memory location holding field
 }

sub NWayTree_FindResult_key($)                                                  # Get key from find result
 {my ($f) = @_;                                                                 # Find result
  NWayTree_FindResult_getField($f, q(key))                                      # Key
 }

sub NWayTree_FindResult_data($)                                                 # Get data from find result
 {my ($f) = @_;                                                                 # Find result
  NWayTree_FindResult_getField($f, q(data))                                     # Data
 }

sub NWayTree_FindResult_cmp($)                                                  # Get comparison from find result
 {my ($f) = @_;                                                                 # Find result
  NWayTree_FindResult_getField($f, q(cmp))                                      # Comparison
 }

sub NWayTree_FindResult_index($)                                                # Get index from find result
 {my ($f) = @_;                                                                 # Find result
  NWayTree_FindResult_getField($f, q(index))                                    # Index
 }

sub NWayTree_FindComparison($$)                                                 # Convert a symbolic name for a find result comparison to an integer
 {my ($f, $cmp) = @_;                                                           # Find result, comaprison result name
  return 0 if $cmp eq q(lower);
  return 1 if $cmp eq q(equal);
  return 2 if $cmp eq q(higher);
  return 3 if $cmp eq q(notFound);
 }

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
