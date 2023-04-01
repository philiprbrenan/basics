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
eval "use Test::More qw(no_plan)" unless caller;

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
    Mov [$t, $Tree->address(q(NumberOfKeysPerNode))], $n;                       # Save maximum number of keys per node
    Mov [$t, $Tree->address(q(root))               ],  0;                       # Clear root
    Mov [$t, $Tree->address(q(keys))               ],  0;                       # Clear keys
    Mov [$t, $Tree->address(q(nodes))              ],  0;                       # Clear nodes
    ReturnPut 0, $t;                                                            # Return id of area containing tree descriptor
    Return;
   };

  ParamsPut \0, $n;
  Call $new;                                                                    # Create a new tree descriptor
  my $t = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  ReturnGet $t, \0;                                                             # Id of area containing tree descriptor
  $t
 }

sub NWayTree_Tree_getField($$)                                                  # Get a field from a tree descriptor
 {my ($tree, $field) = @_;                                                      # Tree, field name
  my $f = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  Mov $f, [$tree, $Tree->address($field)];                                      # Get attribute from tree descriptor
  $f
 }

sub NWayTree_maximumNumberOfKeys($)                                             # Get the maximum number of keys per node for a tree
 {my ($tree) = @_;                                                              # Tree to examine
  NWayTree_Tree_getField($tree, q(NumberOfKeysPerNode));                        # Get attribute from tree descriptor
 };

sub NWayTree_root($)                                                            # Get the root node of a tree
 {my ($tree) = @_;                                                              # Tree to examine
  NWayTree_Tree_getField($tree, q(root));                                       # Get attribute from tree descriptor
 };

sub NWayTree_setRoot($$)                                                        # Set the root node of a tree
 {my ($tree, $root) = @_;                                                       # Tree, root
  Mov [$tree, $Tree->address(q(root))], $root;                                  # Set root attribute
 };

sub NWayTree_keys($)                                                            # Get the number of keys in the tree
 {my ($tree) = @_;                                                              # Tree to examine
  NWayTree_Tree_getField($tree, q(keys));                                       # Keys
 };

sub NWayTree_incKeys($)                                                         # Increment the number of keys in a tree
 {my ($tree) = @_;                                                              # Tree
  Inc [$tree, $Tree->address(q(keys))];                                         # Number of keys
 };

sub NWayTree_nodes($)                                                           # Get the number of nodes in the tree
 {my ($tree) = @_;                                                              # Tree to examine
  NWayTree_Tree_getField($tree, q(nodes));                                      # Nodes
 };

sub NWayTree_incNodes($)                                                        # Increment the number of nodes n a tree
 {my ($tree) = @_;                                                              # Tree
  Inc [$tree, $Tree->address(q(nodes))];                                        # Number of nodes
 };

sub NWayTree_Node_new($)                                                        # Create a variable refering to a new node descriptor
 {my ($tree) = @_;                                                              # GTree node is being created in

  my $new = Procedure 'NWayTree_Node_new', sub
   {my ($p) = @_;                                                               # Procedure description
    my ($t, $n, $k, $d, $D) = $p->variables->temporary(5);                      # Tree, node, keys, data, down
    ParamsGet $t, \0;                                                           # Tree as a parameter
    Alloc $$_ for \($n, $k, $d, $D);                                            # Allocate node, keys, data, down areas
    Mov [$n, $Node->address(q(length))],  0;                                    # Length
    Mov [$n, $Node->address(q(up))],      0;                                    # Parent
    Mov [$n, $Node->address(q(keys))],   $k;                                    # Keys area
    Mov [$n, $Node->address(q(data))],   $d;                                    # Data area
    Mov [$n, $Node->address(q(down))],   $D;                                    # Down area
    Mov [$n, $Node->address(q(tree))],   $t;                                    # Containing tree
    NWayTree_incNodes($t);
    Mov [$n, $Node->address(q(id))], [$t, $Tree->address(q(nodes))];            # Assign an id to this node within the tree

    ReturnPut 0, $n;                                                            # Return id of area containing node
    Return;
   };

  ParamsPut \0, $tree;                                                          # Tree parameter
  Call $new;                                                                    # Create a new node in the tree
  my $n = $main->variables->temporary;                                          # Create a variable to reference the new node
  ReturnGet $n, \0;                                                             # Return reference to new node
  $n
 }

sub NWayTree_Node_getField($$)                                                  # Get a field from a node descriptor
 {my ($node, $field) = @_;                                                      # Node, field name
  my $f = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  Mov $f, [$node, $Node->address($field)];                                      # Get attribute from node descriptor
  $f
 }

sub NWayTree_Node_length($)                                                     # Get number of keys in a node
 {my ($node) = @_;                                                              # Node
  NWayTree_Node_getField($node, q(length));                                     # Get length
 }

sub NWayTree_Node_setLength($$)                                                 # Set the length of a node
 {my ($node, $length) = @_;                                                     # Node, length
  Mov [$node, $Node->address(q(length))], $length;                              # Set length attribute
 }

sub NWayTree_Node_id($)                                                         # Get id of a node
 {my ($node) = @_;                                                              # Node
  NWayTree_Node_getField($node, q(id));                                         # Get id
 }

sub NWayTree_Node_up($)                                                         # Get parent node from this node
 {my ($node) = @_;                                                              # Node
  NWayTree_Node_getField($node, q(up));                                         # Get up
 }

sub NWayTree_Node_setUp($$)                                                     # Set the parent of a node
 {my ($node, $parent) = @_;                                                     # Node, parent node, area containing parent node reference
  Mov $Node->address(q(up)), $node, $parent;                                    # Set parent
 }

sub NWayTree_Node_tree($)                                                       # Get tree containing a node
 {my ($node) = @_;                                                              # Node
  NWayTree_Node_getField($node, q(tree));                                       # Get tree
 }

sub NWayTree_Node_getIndex($$$)                                                 # Get the indexed field from a node
 {my ($node, $index, $field) = @_;                                              # Node, index of field, field name
  my ($F, $f) = $main->variables->temporary(2);                                 # Fields, field
  Mov $F, [$node, $Node->address($field)];                                      # Fields
  Mov $f, [$F, \$index];                                                        # Field
  $f                                                                            # Memory location holding field
 }

sub NWayTree_Node_setIndex($$$$)                                                # Set an indexed field to a specified value
 {my ($node, $index, $field, $value) = @_;                                      # Node, index, field name, value
  my $F = $main->variables->temporary;                                          # Indexed fields
  Mov $F, [$node, $Node->address($field)];                                      # Fields
  Mov [$F, \$index], $value;                                                     # Set field to value
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
  Mov $f, [$findResult, $FindResult->address($field)];                          # Fields
  $f                                                                            # Memory location holding field
 }

sub NWayTree_FindResult_key($)                                                  # Get key from find result
 {my ($f) = @_;                                                                 # Find result
  NWayTree_FindResult_getField($f, q(key))                                      # Key
 }

sub NWayTree_FindResult_cmp($)                                                  # Get comparison from find result
 {my ($f) = @_;                                                                 # Find result
  NWayTree_FindResult_getField($f, q(cmp))                                      # Comparison
 }

sub NWayTree_FindResult_index($)                                                # Get index from find result
 {my ($f) = @_;                                                                 # Find result
  NWayTree_FindResult_getField($f, q(index))                                    # Index
 }

sub NWayTree_FindResult_node($)                                                 # Get node from find result
 {my ($f) = @_;                                                                 # Find result
  NWayTree_FindResult_getField($f, q(node))                                     # Node
 }

sub NWayTree_FindResult_data($)                                                 # Get data field from find results
 {my ($f) = @_;                                                                 # Find result

  my $n = NWayTree_FindResult_node ($f);
  my $i = NWayTree_FindResult_index($f);
  my $d = NWayTree_Node_data($n, $i);
  $f
 }

sub NWayTree_FindComparison($$)                                                 # Convert a symbolic name for a find result comparison to an integer
 {my ($f, $cmp) = @_;                                                           # Find result, comaprison result name
  return 0 if $cmp eq q(lower);
  return 1 if $cmp eq q(equal);
  return 2 if $cmp eq q(higher);
  return 3 if $cmp eq q(notFound);
 }

sub NWayTree_Node_open($$$)                                                     # Open a gap in a node
 {my ($node, $offset, $length) = @_;                                            # Node
  my ($l, $L, $i, $p, $q) = $main->variables->temporary(5);                     # Variables
  Add $l, $offset, $length;
  Add $L, $l, 1;

  my $n = NWayTree_Node_down($node, $l);
  NWayTree_Node_setDown($node, $L, $n);

  For start => sub{Mov $i, $length},
      check => sub{Jle $_[0], $i, 0},
      next  => sub{Dec $i},
      block => sub
       {Add $p, $offset, $i;
        Add $q, $p, -1;
        my $k = NWayTree_Node_keys   ($node, $q);
        my $d = NWayTree_Node_data   ($node, $q);
        my $n = NWayTree_Node_down   ($node, $q);
                NWayTree_Node_setKeys($node, $p, $k);
                NWayTree_Node_setData($node, $p, $d);
                NWayTree_Node_setDown($node, $p, $n);
       };
 }

sub NWayTree_Node_copy($$$$$)                                                   # Copy part of one node into another going down.
 {my ($t, $s, $to, $so, $length) = @_;                                          # Target node
  my ($i, $S, $T) = $main->variables->temporary(3);                             # Variables

  For start => sub{Mov $i, 0},
      check => sub{Jge $_[0], $i, $length},
      next  => sub{Inc $i},
      block => sub
       {Add $S, $so, $i;
        Add $T, $to, $i;
        my $k = NWayTree_Node_keys   ($s, $S);
        my $d = NWayTree_Node_data   ($s, $S);
        my $n = NWayTree_Node_down   ($s, $S);
                NWayTree_Node_setKeys($t, $T, $k);
                NWayTree_Node_setData($t, $T, $d);
                NWayTree_Node_setDown($t, $T, $n);
       };

  Add $S, $so, $length;
  Add $T, $to, $length;

  my $n = NWayTree_Node_down($s, $S);
  NWayTree_Node_setDown($t, $T, $n);
 }

sub NWayTree_FreeNode($)                                                        # Free a node
 {my ($node) = @_;                                                              # Node to free
  free($node);
 }

sub NWayTree_FindResult_new($$$$)                                               # New find result on stack
 {my ($node, $key, $cmp, $index) = @_;                                          # Node,search key, comparison result, index
  my ($f) = $main->variables->temporary(1);                                     # Find result
  Alloc $f;                                                                     # Allocate tree descriptor

  Mov [$f, $FindResult->address(q(node)) ], $node;
  Mov [$f, $FindResult->address(q(key))  ], $key;
  Mov [$f, $FindResult->address(q(cmp))  ], $cmp;
  Mov [$f, $FindResult->address(q(index))], $index;
  $f
 }

return 1 if caller;

eval {goto latest};

sub is_deeply;
sub ok($;$);
sub done_testing;

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
  Out [$t, $Tree->address(q(keys))];

  NWayTree_incNodes($t) for 1..5;
  my $N = NWayTree_nodes($t);
  Out $N;

  my $e = Execute;
  is_deeply $e->out         => [3, 5];
  is_deeply $e->memory->{6} => [3, 5, 3, 1];
 }

if (1)                                                                          #TNWayTree_Node_open
 {$main = Start 1;
  my $t = NWayTree_new(7);                                                      # Create tree
  my $n = NWayTree_Node_new($t);                                                # Create node
  for my $i(0..6)
   {NWayTree_Node_setKeys($n, $i,  1+$i);
    NWayTree_Node_setData($n, $i, 11+$i);
    NWayTree_Node_setDown($n, $i, 21+$i);
   }
  NWayTree_Node_open($n, 2, 4);

  my $e = Execute(trace=>0);
  is_deeply $e->memory, {
  6  => [0, 1, 7, 0],
  10 => [0, 1, 0, 11, 12, 13, 6],
  11 => [1, 2, 3, 3 .. 6],
  12 => [11, 12, 13, 13 .. 16],
  13 => [21, 22, 23, 23 .. 27],
};
 }

#latest:;
if (1)                                                                          #TNWayTree_Node_copy
 {$main = Start 1;
  my $t = NWayTree_new(7);                                                      # Create tree
  my $p = NWayTree_Node_new($t);                                                # Create a node
  my $q = NWayTree_Node_new($t);                                                # Create a node
  for my $i(0..6)
   {NWayTree_Node_setKeys($p, $i, 11+$i);
    NWayTree_Node_setData($p, $i, 21+$i);
    NWayTree_Node_setDown($p, $i, 31+$i);
    NWayTree_Node_setKeys($q, $i, 41+$i);
    NWayTree_Node_setData($q, $i, 51+$i);
    NWayTree_Node_setDown($q, $i, 61+$i);
   }
  NWayTree_Node_copy($q, $p, 1, 3, 2);

  my $e = Execute(trace=>0);
  is_deeply $e->memory, {
   6  => [0, 2, 7, 0],
  10 => [0, 1, 0, 11, 12, 13, 6],
  11 => [11 .. 17],
  12 => [21 .. 27],
  13 => [31 .. 37],
  17 => [0, 2, 0, 18, 19, 20, 6],
  18 => [41, 14, 15, 44 .. 47],
  19 => [51, 24, 25, 54 .. 57],
  20 => [61, 34, 35, 36, 65, 66, 67],
};
 }

#latest:;
if (1)                                                                          #TNWayTree_FindResult_new
 {$main = Start 1;
  my $f = NWayTree_FindResult_new(1, 2, 3, 4);
  my $n = NWayTree_FindResult_node($f);
  my $k = NWayTree_FindResult_key($f);
  my $c = NWayTree_FindResult_cmp($f);
  my $i = NWayTree_FindResult_index($f);
  Out $_ for $n, $c, $k, $i;
  my $e = Execute(trace=>0);
  is_deeply $e->out,           [1, 3, 2, 4];
  is_deeply $e->memory, { 3 => [1, 3, 2, 4] };
 }

done_testing;
