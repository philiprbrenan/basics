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

sub NWayTree_new($$)                                                            # Create a variable refering to a new tree descriptor
 {my ($tree, $n) = @_;                                                          # Name of variable to refer to tree, maximum number of keys per node in this tree

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
  my ($t) = $main->variables->names($n);                                        # Create a variable to hold the results of this call
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
  my $n = $main->variables->temporary;                                          # Create a variable to hold the results of this call
  Mov $n, $Tree->address(q(root)), $tree;                                       # Get attribute from tree descriptor
  $n
 };

sub NWayTree_setRoot($$)                                                        # Set the root node of a tree
 {my ($tree, $name) = @_;                                                       # Tree, name of variable referencing root
  Mov $Tree->address(q(root)), $tree, $name, undef;                             # Set root attribute
 };

sub NWayTree_incKeys($)                                                         # Increment the number of keys
 {my ($tree) = @_;                                                              # Tree
  Inc $Tree->address(q(keys)), $tree;                                           # Number of keys
 };

#define NWayTree_incKeys(tree)               ++tree->keys;
#define NWayTree_incNodes(tree)              ++tree->nodes;



ok $Tree->offset(q(nodes)) == 1;
ok $Node->offset(q(tree))  == 6;

if (1)                                                                          #T
 {$main = Start 1;                                                              # Start assembly

  my $t = NWayTree_new(q(t), 3);
  AssertEq $t, 6;

  my $r = NWayTree_root($t);
  AssertEq $r, 0;

  NWayTree_setRoot($t, 1);
  my $R = NWayTree_root($t);
  AssertEq $R, 1;

  my $n = NWayTree_maximumNumberOfKeys($t);
  AssertEq $n, 3;

  my $e = Execute;
  is_deeply $e->memory->{6} => [0, 0, 3, 1];
 }

done_testing;
