/*
 * http://fr.wikipedia.org/wiki/%C3%89change_de_cl%C3%A9s_Diffie-Hellman
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define P 251
#define G 19
#define RANDWIDTH 1024

#define BUFSIZE 32

unsigned int bobKey, aliceKey;
unsigned int bobSecret, aliceSecret;

//renvoie n^p mod m
int my_pow_mod (int n, int p, int m) {
    if (p != 0) {
        return (n * my_pow_mod(n, p - 1, m)) % m;
    } else
        return 1;
}

//Simulation du serveur en local avec plusieures fonctions
int aliceToBob1() {
    aliceSecret = rand() % RANDWIDTH + 1;
    return my_pow_mod(G, aliceSecret, P);
}

int bobToAlice1() {
    bobSecret = rand() % RANDWIDTH + 1;
    int A = aliceToBob1();
    bobKey = my_pow_mod(A, bobSecret, P);
    return my_pow_mod(G, bobSecret, P);
}

void aliceCalcKey() {
    int B = bobToAlice1();
    aliceKey = my_pow_mod(B, aliceSecret, P);
}

//mode = 0 -> encryption, sinon decrypt
#if 0
void transform(unsigned int key, unsigned int mode, unsigned char* in, unsigned char* out, int length) {
    int i = 0;
    unsigned int newKey = key;
    while (i < length) {
        out[i] = in[i] ^ newKey;
        newKey = (mode == 1) ? out[i] : in[i];
        i++;
    }
    out[i] = '\0';
}
#endif

void transform(unsigned int key, unsigned int mode, unsigned char* in, unsigned char* out, int length) {
    if (length != 0) {
        *out = *in ^ key;
        transform((mode == 0) ? *in : *out, mode, in + 1, out + 1, length - 1);
    }
    else *out = '\0';
}

int main(int argc, char** argv) {
    srand(time(NULL));
    aliceCalcKey();
    printf("bobKey: %i\n", bobKey);
    printf("aliceKey: %i\n", aliceKey);
    printf("bobSecret: %i\n", bobSecret);
    printf("aliceSecret: %i\n", aliceSecret);
    
    printf("\nMessages:\n");
    unsigned char packet[BUFSIZE] = "Hello World !";
    unsigned char crypted[BUFSIZE];
    transform(aliceKey, 0, packet, crypted, 13);
    printf("aToB> packet: \t%s\t crypted: \t%s\n", packet, crypted);
    transform(bobKey, 1, crypted, packet, 13);
    printf("bDec> crypted: \t%s\t packet: \t%s\n", crypted, packet);
    
    printf("\nFinished\n");
    return 0;
}
