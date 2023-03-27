#!/usr/bin/perl -I/home/phil/perl/cpan/DataTableText/lib/ -I/home/phil/perl/cpan/GitHubCrud/lib/
#-------------------------------------------------------------------------------
# Push Basic code to GitHub
# Philip R Brenan at gmail dot com, Appa Apps Ltd Inc., 2022
#-------------------------------------------------------------------------------
use warnings FATAL => qw(all);
use strict;
use Carp;
use Data::Dump qw(dump);
use Data::Table::Text qw(:all);
use GitHub::Crud qw(:all);
use feature qw(say current_sub);

my $home = q(/home/phil/c/);                                                    # Local files
my $user = q(philiprbrenan);                                                    # User
my $repo = q(basics);                                                           # Store code here so it can be referenced from a browser
my $wf   = q(.github/workflows/main.yml);                                       # Work flow on Ubuntu

expandWellKnownWordsInMarkDownFile                                              # Documentation
  fpe($home, qw(README md2)), fpe $home, qw(README md);

push my @files, searchDirectoryTreesForMatchingFiles($home, qw(.c .md .pl));    # Files

for my $s(@files)                                                               # Upload each selected file
 {#next unless $s =~ m(nWay|array)i;
  my $p = readFile($s);                                                         # Load file
  my $t = swapFilePrefix $s, $home;
  next if $s =~ m(/backups/);                                                   # Ignore this folder
  my $w = writeFileUsingSavedToken($user, $repo, $t, $p);
  lll "$w $s $t";
 }

my $d = dateTimeStamp;

my $y = <<'END';
# Test $d

name: Test

on:
  push

jobs:
  test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2
      with:
        ref: 'main'

    - name: Sde
      run: |
        wget -q -O sde.tar.xz https://downloadmirror.intel.com/751535/sde-external-9.14.0-2022-10-25-lin.tar.xz
        tar -xf sde.tar.xz
        mv sde-external*lin/ sde

    - name: 2d
      run: |
        gcc -I. 2d/2d.c -lm;      ./a.out

    - name: Array
      run: |
        gcc -I. array/long.c;     ./a.out
        gcc -I. array/void.c;     ./a.out

    - name: Basics
      run: |
        gcc -I. basics/basics.c;  ./a.out

    - name: BinarySearch
      run: |
        gcc -I. binarySearch/long.c; ./a.out

    - name: Bits
      run: |
        gcc -I. bits/bits.c;      ./a.out

    - name: Hash
      run: |
        gcc -I. hash/int.c;       ./a.out

    - name: HeapSortLong
      run: |
        gcc -I. heapSort/long.c      -mavx512f; sde/sde64 -mix -- ./a.out

    - name: InsertionSort
      run: |
        gcc -I. insertionSort/long.c -mavx512f; sde/sde64 -mix -- ./a.out

    - name: MergeSortLong
      run: |
        gcc -I. mergeSort/long.c     -mavx512f; sde/sde64 -mix -- ./a.out

    - name: Modulo
      run: |
        gcc -I. modulo/modulo.c;  ./a.out

    - name: NWayTreeLong
      run: |
        gcc -I. nWayTree/long.c;  ./a.out

    - name: QuickSortLong
      run: |
        gcc -I. quickSort/long.c     -mavx512f; sde/sde64 -mix -- ./a.out

    - name: Simd
      run: |
        #gcc -I. simd/test.c         -mavx512f; sde/sde64 -mix -- ./a.out
        #cat sde-mix-out.txt

    - name: StackLong
      run: |
        gcc -I. stack/long.c;     ./a.out

    - name: Tree
      run: |
        gcc -I. tree/tree.c;      ./a.out
END

lll "Ubuntu work flow for $repo ", writeFileUsingSavedToken($user, $repo, $wf, $y);
