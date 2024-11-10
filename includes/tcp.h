#ifndef __TCP_H
#define __TCP_H

/* Errors */

#define ECREATESOCK  0
#define ESETOPTSOCK  1
#define EBINDSOCK    2
#define ELISTENSOCK  3
#define ECONNECTSOCK 4
#define ETCPADDRINFO 5

/* Creates a TCP listener at given address.*/
int TCP_createlistener(const char *host, const char *port);

/* Create a TCP client and connect to given address */
int TCP_createclient(const char *host, const char *port);

/* Returns the TCP error message */
char *TCP_geterr(void);

#endif