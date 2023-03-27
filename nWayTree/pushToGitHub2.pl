#!/usr/bin/perl -I/home/phil/perl/cpan/DataTableText/lib/ -I/home/phil/perl/cpan/GitHubCrud/lib/
#-------------------------------------------------------------------------------
# Compile generic from an actual
# Philip R Brenan at gmail dot com, Appa Apps Ltd Inc., 2022
#-------------------------------------------------------------------------------
say STDERR qx(gcc -finput-charset=UTF-8 -fmax-errors=7 -rdynamic -Wall -Wextra -Wno-unused-function -I/home/phil/c/ -I. -I. -mavx512f -O0 -g3 -rdynamic  -o "/home/phil/c/nWayTree/long" "/home/phil/c/nWayTree/long.c" -lm && timeout 100s sde -mix -- /home/phil/c/nWayTree/long);
