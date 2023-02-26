//------------------------------------------------------------------------------
// N way tree with long keys
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
//sde -mix -- ./long
#ifndef NWayTreeLong
#define NWayTreeLong

#define NWayTreeDataType long
#define NWayTreeLongMaxIterations 99                                            /* The maximum number of levels in a tree */
#define NWayTree(name) NWayTreeLong##name                                       /* Function names */
#include "array/long.c"
#include "nWayTree/generic.c"
#endif
//\A(.{80})\s*// \1//
