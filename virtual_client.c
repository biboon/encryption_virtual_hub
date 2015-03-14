/**** Fichier principal pour le commutateur virtuel ****/

/** Fichiers d'inclusion **/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "libnet.h"

/** Quelques constantes **/

#define MAX_PAQUET	2048

/** Variables globales */

/** Fonctions **/

/* Fonction principale */
int main(int argc,char *argv[]){
// Analyse des arguments
if(argc!=4){
  fprintf(stderr,"Syntaxe : client <serveur> <port> <nom interface>\n");
  exit(-1);
  }
char *serveur=argv[1];
int port=atoi(argv[2]);
char *nom=argv[3];

// Connexion au serveur
int dialogue=connexionServeur(serveur,port);
if(dialogue<0){
  fprintf(stderr,"Serveur inaccessible !\n");
  exit(-1);
  }

// Ouverture de l'interface reseau
int interface=creationInterfaceVirtuelle(nom);
if(interface<0){
  fprintf(stderr,"Ouverture impossible de l'interface !\n");
  exit(-1);
  }

// Communication avec le serveur
struct pollfd descripteurs[2];
descripteurs[0].fd=dialogue;
descripteurs[0].events=POLLIN;
descripteurs[1].fd=interface;
descripteurs[1].events=POLLIN;
while(1){
  int nb=poll(descripteurs,2,-1);
  if(nb<0){ perror("poll"); exit(-1); }
  if((descripteurs[0].revents&POLLIN)!=0){
    unsigned char paquet[MAX_PAQUET];
    uint16_t longueur;
    int taille=read_fixed(dialogue,(unsigned char *)&longueur,sizeof(longueur));
    if(taille<=0){
      fprintf(stderr,"Serveur HS !\n");
      exit(-1);
      }
    int hlongueur=ntohs(longueur);
    taille=read_fixed(dialogue,paquet,hlongueur);
#ifdef DEBUG
    fprintf(stderr,"Paquet de taille=%d recu par le HUB\n",hlongueur);
#endif
    if(taille!=hlongueur) fprintf(stderr,"Un paquet de taille erronee recu !\n");
    else if(write(interface,paquet,hlongueur)!=hlongueur) {
           fprintf(stderr,"Echec d'ecriture sur l'interface !\n");
           exit(-1);
           }
    }
  if((descripteurs[1].revents&POLLIN)!=0){
    unsigned char paquet[MAX_PAQUET];
    int hlongueur=read(interface,paquet,MAX_PAQUET);
    if(hlongueur<=0){
      fprintf(stderr,"Interface HS !\n");
      exit(-1);
      }
#ifdef DEBUG
    fprintf(stderr,"Paquet de taille=%d recu sur l'interface\n",hlongueur);
#endif
    uint16_t longueur=htons(hlongueur);
    if(write(dialogue,&longueur,sizeof(longueur))!=sizeof(longueur)){
      fprintf(stderr,"Echec d'ecriture sur le serveur !\n");
      exit(-1);
      }
    if(write(dialogue,paquet,hlongueur)!=hlongueur){
      fprintf(stderr,"Echec d'ecriture sur le serveur !\n");
      exit(-1);
      }
    }
  }
return 0;
}
