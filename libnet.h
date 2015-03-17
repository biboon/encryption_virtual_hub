/* This file contains descriptions of network functions */
#ifndef __LIBNET_H
#define __LIBNET_H

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
unsigned int pow_mod (unsigned int n, unsigned int p, unsigned int m);
void cipherBlock(unsigned int key, unsigned int mode, unsigned char* in, unsigned char* out, int length);

#endif
