/*
* http://fr.wikipedia.org/wiki/%C3%89change_de_cl%C3%A9s_Diffie-Hellman
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define P 251
#define G 19
#define RANDWIDTH 1024

#define BUFSIZE 64

uint8_t bobKey, aliceKey;
unsigned int bobSecret, aliceSecret;

//returns n^p mod m, use non recursive if p is too high
unsigned int my_pow_mod (unsigned int n, unsigned int p, unsigned int m) {
    return (p != 0) ? (n * my_pow_mod(n, p - 1, m)) % m : 1;
}

#if 0
//Non recursive version
unsigned int my_pow_mod (unsigned int n, unsigned int p, unsigned int m) {
    int i = 0;
    unsigned int res = 1;
    for (i = 0; i < p; i++) res = (res * n) % m;
    return res;
}
#endif

//Simulation du serveur en local avec plusieures fonctions
uint8_t aliceToBob1() {
    aliceSecret = rand() % RANDWIDTH + 1;
    return my_pow_mod(G, aliceSecret, P);
}

uint8_t bobToAlice1() {
    bobSecret = rand() % RANDWIDTH + 1;
    uint8_t A = aliceToBob1();
    bobKey = my_pow_mod(A, bobSecret, P);
    return my_pow_mod(G, bobSecret, P);
}

void aliceCalcKey() {
    uint8_t B = bobToAlice1();
    aliceKey = my_pow_mod(B, aliceSecret, P);
}

#if 0
//mode = 0 -> encryption, else decrypt, use non recursive if length is too high
void cipherBlock(uint8_t key, unsigned int mode, uint8_t* in, uint8_t* out, unsigned int length) {
    if (length != 0) {
        *out = *in ^ key;
        cipherBlock((mode == 0) ? *in : *out, mode, in + 1, out + 1, length - 1);
    }
    else *out = '\0';
}
#endif

void cipherBlock(unsigned int key, unsigned int mode, unsigned char* in, unsigned char* out, int length) {
    int i = 0;
    unsigned int newKey = key;
    while (i < length) {
        out[i] = in[i] ^ newKey;
        newKey = (mode == 1) ? out[i] : in[i];
        i++;
    }
    out[i] = '\0';
}

int main(int argc, char** argv) {
    srand(time(NULL));
    aliceCalcKey();
    printf("bobKey: 0x%x : %i\n", bobKey, bobKey);
    printf("aliceKey: 0x%x : %i\n", aliceKey, aliceKey);
    printf("bobSecret: %i\n", bobSecret);
    printf("aliceSecret: %i\n", aliceSecret);
    
    printf("\nMessages:\n");
    uint8_t packet[BUFSIZE] = "Hello World !";
    uint8_t crypted[BUFSIZE];
    uint8_t decrypted[BUFSIZE];
    cipherBlock(aliceKey, 0, packet, crypted, 13);
    printf("Encrypt> packet: \t%s\t crypted: \t%s\n", packet, crypted);
    cipherBlock(bobKey, 1, crypted, decrypted, 13);
    printf("Decrypt> crypted: \t%s\t decrypted: \t%s\n", crypted, decrypted);
    
    printf("\nFinished\n");
    return 0;
}
