/**** Fichier principal pour le commutateur virtuel ****/

/** Fichiers d'inclusion **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "libnet.h"

/** Quelques constantes **/

#define MAX_CLIENTS	1024
#define MAX_PAQUET	2048

/** Variables globales **/

struct pollfd clients[MAX_CLIENTS];
int nb_clients=0;

/** Fonctions **/

/* Traitemenet des clients TCP */
int traitement(int dialogue){

return 0;
}

/* Fonction principale */
int main(int argc,char *argv[]){
// Analyse des arguments
if(argc!=2){
  fprintf(stderr,"Syntaxe : switch <port>\n");
  exit(-1);
  }
short int port=atoi(argv[1]);

// Initialisation du serveur
int ecoute=initialisationServeur(&port);
clients[0].fd=ecoute;
clients[0].events=POLLIN;

// Traitement des connexions et des messages
while(1){
  int nb=poll(clients,nb_clients+1,-1);
  if(nb<0){ perror("poll"); exit(-1); }
  // Connexion de client
  if((clients[0].revents&POLLIN)!=0){
    int dialogue=accept(ecoute,NULL,NULL);
    if(nb_clients<=MAX_CLIENTS-2){
      clients[nb_clients+1].fd=dialogue;
      clients[nb_clients+1].events=POLLIN;
      nb_clients++;
#ifdef DEBUG
      fprintf(stderr,"Nouveau client socket=%d nb=%d\n",dialogue,nb_clients);
#endif
      }
    }
  // Envoi de paquet par un client
  int i;
  for(i=0;i<nb_clients;i++)
    if((clients[i+1].revents&POLLIN)!=0){
      uint16_t longueur;
      int j;
      int taille=read_fixed(clients[i+1].fd,(unsigned char *)&longueur,sizeof(longueur));
      if(taille<=0){
        close(clients[i+1].fd);
        for(j=i;j<nb_clients-1;j++) clients[j+1]=clients[j+2];
        nb_clients--; i--;
#ifdef DEBUG
        fprintf(stderr,"Deconnexion du client #%d nb=%d\n",i+1,nb_clients);
#endif
        }
      else{
        unsigned char paquet[MAX_PAQUET];
        int hlongueur=ntohs(longueur);
#ifdef DEBUG
        fprintf(stderr,"Paquet du client %d, longueur=%d\n",i,hlongueur);
#endif
        read_fixed(clients[i+1].fd,paquet,hlongueur);
        for(j=0;j<nb_clients;j++)
          if(j!=i){
#ifdef DEBUG
            fprintf(stderr,"Envoi du paquet sur le client #%d (descripteur=%d)\n",j,clients[j+1].fd);
#endif
            if(write(clients[j+1].fd,&longueur,sizeof(longueur))==sizeof(longueur));
              write(clients[j+1].fd,paquet,hlongueur);
            }
        }
      }
  }
return 0;
}
