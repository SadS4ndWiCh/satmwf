#ifndef __SOCK_H
#define __SOCK_H

#include "types.h"

/* Errors */
#define ECREATESOCK  0
#define ESETOPTSOCK  1
#define EBINDSOCK    2
#define ELISTENSOCK  3
#define ECONNECTSOCK 4

int setup_sock(u32 host, u16 port);

#endif