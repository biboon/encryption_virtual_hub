/* This file contains descriptions of network functions */

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <time.h>

/** Constants **/
#define MAX_CONNEXIONS 32
#define BUFSIZE 2048
#define TAP_PRINCIPAL "/dev/net/tun"

/* Encryption key calculus */
#define P 251
#define G 19
#define RANDWIDTH 1024

/* Types */
typedef unsigned char uint8_t;

/** Functions **/
void socketToClient(int s, char **hote, char **service);
int serverConnection(char *hote, char *service);
int serverInitialization(char *service, int connexions);
int read_fixed(int descripteur, unsigned char *array, int size);
int virtualInterfaceCreation(char *nom);
int serverLoop(int ecoute);
int clientLoop(int sock, int iface);
uint16_t pow_mod (uint16_t n, uint16_t p, uint16_t m);
void cipherBlock(uint16_t key, uint16_t mode, unsigned char* in, unsigned char* out, int length);
