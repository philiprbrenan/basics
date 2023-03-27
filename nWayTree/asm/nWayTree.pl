#!/usr/bin/perl -I/home/phil/perl/zero/lib/
#-------------------------------------------------------------------------------
# Use Zero assembler language to implement a generic N-Way tree.
# Philip R Brenan at appaapps dot com, Appa Apps Ltd Inc., 2023
#-------------------------------------------------------------------------------
use v5.30;
use warnings FATAL => qw(all);
use strict;
use Data::Dump qw(dump);
use Data::Table::Text qw(:all);
use Zero::Emulator qw(:all);
use utf8;
use Test::More qw(no_plan);

my $𝗧𝗿𝗲𝗲 = sub                                                                  # The structure of an n-way tree
 {my $t = Zero::Emulator::areaStructure("NWayTree_Structure");
     $t->name(q(NumberOfKeysPerNode));                                          # The maximum number of keys in a node of this tree
     $t->name(q(keys));                                                         # Number of keys in tree
     $t->name(q(nodes));                                                        # Number of nodes in tree
     $t
 }->();

my $𝗡𝗼𝗱𝗲 = sub                                                                  # The structure of an n-way tree node
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

ok $𝗧𝗿𝗲𝗲->offset(q(nodes)) == 2;
ok $𝗡𝗼𝗱𝗲->offset(q(tree)) == 6;

if (1)
 {Start 1;
  Out "hello World";
  ok Execute(out=>["hello World"]);
 }

my $main = Start 1;                                                             # Start assembly

my $tree_new = Procedure 'NWayTree_new', sub
 {my ($p) = @_;                                                                 # Procedure description
  my ($t, $n) = $p->variables->names(qw(t n));                                  # Tree, maximum number of keys per node in this tree
  Alloc $t;                                                                     # Allocate tree descriptor
  ParamsGet $n, \0;                                                             # Maximum number of keys per node
  Put $t, $𝗧𝗿𝗲𝗲->address(q(keys)),  0;                                          # Clear keys
  Put $t, $𝗧𝗿𝗲𝗲->address(q(nodes)), 0;                                          # Clear nodes
  Put $t, $𝗧𝗿𝗲𝗲->address(q(NumberOfKeysPerNode)), $n;                           # Save maximum number of keys per node
  ReturnPut 0, $t;                                                              # Return id of area containing tree descriptor
  Return;
 };

ParamsPut \0, 3;
Call $tree_new;
my ($t) = $main->variables->names(qw(t));
ReturnGet $t, 0;                                                                # Id of area containing tree descriptor
AssertEq $t, 1000009;

my $r = Execute;
is_deeply $r->memory->{1000006} => [3, 0, 0];

done_testing;
