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
     $t->name(q(NumberOfKeysPerNode));                                          # The maximum number of keys in a node of this tree
     $t->name(q(root));                                                         # Root node
     $t->name(q(keys));                                                         # Number of keys in tree
     $t->name(q(nodes));                                                        # Number of nodes in tree
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
    my ($t, $n) = $p->variables->names(qw(t n));                                # Tree, maximum number of keys per node in this tree
    Alloc $t;                                                                   # Allocate tree descriptor
    ParamsGet $n, \0;                                                           # Maximum number of keys per node
    Put $t, $Tree->address(q(NumberOfKeysPerNode)), $n;                         # Save maximum number of keys per node
    Put $t, $Tree->address(q(root)), 0;                                         # Clear root
    Put $t, $Tree->address(q(keys)),  0;                                        # Clear keys
    Put $t, $Tree->address(q(nodes)), 0;                                        # Clear nodes
    ReturnPut 0, $t;                                                            # Return id of area containing tree descriptor
    Return;
   };

  ParamsPut \0, $n;
  Call $new;                                                                    # Create a new tree descriptor
  my ($t) = $main->variables->names($n);                                        # Create a variable to hold the results of this call
  ReturnGet $t, \0;                                                             # Id of area containing tree descriptor
  $t
 }

sub NWayTree_numberOfKeysPerNode($$)                                            # Get the maximum number of keys per node for a tree
 {my ($name, $tree) = @_;                                                       # Name of variable to hold the result, tree to examine
  my ($n) = $main->variables->names($name);                                     # Create a variable to hold the results of this call
  Get $n, $tree, $Tree->address(q(NumberOfKeysPerNode));                        # Get attribute from tree descriptor
  $n
 };


sub NWayTree_maximumNumberOfKeys($$)                                            # Get the maximum number of keys per node for a tree
 {my ($name, $tree) = @_;                                                       # Name of variable to hold the result, tree to examine
  my ($n) = $main->variables->names($name);                                     # Create a variable to hold the results of this call
  Get $n, $tree, $Tree->address(q(NumberOfKeysPerNode));                        # Get attribute from tree descriptor
  $n
 };

sub NWayTree_node($$)                                                           # Get the root node of a tree
 {my ($name, $tree) = @_;                                                       # Name of variable to hold the result, tree to examine
  my ($n) = $main->variables->names($name);                                     # Create a variable to hold the results of this call
  Get $n, $tree, $Tree->address(q(NumberOfKeysPerNode));                        # Get attribute from tree descriptor
  $n
 };

#define NWayTree_node(n, tree)                 NWayTree(Node) *       n = tree->node;

ok $Tree->offset(q(nodes)) == 3;
ok $Node->offset(q(tree))  == 6;

if (1)                                                                          #T
 {$main = Start 1;                                                              # Start assembly

  my $t = NWayTree_new(q(t), 3);
  AssertEq $t, 1000006;

  my $n = NWayTree_maximumNumberOfKeys(q(n), $t);
  AssertEq $n, 3;

  my $r = Execute;
  say STDERR "AAAA", dump($r->memory->{1000006});
  is_deeply $r->memory->{1000006} => [3, 0, 0];
 }

done_testing;
