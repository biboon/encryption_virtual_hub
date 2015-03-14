/** fichier libnet.c **/

/************************************************************/
/** Ce fichier contient des fonctions reseau.              **/
/************************************************************/

/**** Fichiers d'inclusion ****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "libnet.h"

/**** Constantes ****/

#define MAX_TAMPON	1024
#define TAP_PRINCIPAL	"/dev/net/tun"

/**** Variables globales *****/

/**** Prototype des fonctions locales *****/

#ifdef DEBUG
static void afficheAdresse(FILE *flux,void *ip,int type);
static void afficheAdresseSocket(FILE *flux,struct sockaddr_storage *padresse);
static void afficheHote(FILE *flux,struct hostent *hote,int type);
static int configurationSocket(int ds);
#endif

/**** Fonctions de gestion des sockets ****/

#ifdef DEBUG
/** Impression d'une adresse generale **/

static void afficheAdresse(FILE *flux,void *ip,int type)
{
char adresse[MAX_TAMPON];
inet_ntop(type,ip,adresse,MAX_TAMPON);
fprintf(flux,adresse);
} 

/** Impression d'une adresse de socket **/

static void afficheAdresseSocket(FILE *flux,struct sockaddr_storage *padresse)
{
void *ip;
int port;
if(padresse->ss_family==AF_INET){
    struct sockaddr_in *ipv4=(struct sockaddr_in *)padresse;
    ip=(void *)&ipv4->sin_addr;
    port=ipv4->sin_port;
    }
if(padresse->ss_family==AF_INET6){
    struct sockaddr_in6 *ipv6=(struct sockaddr_in6 *)padresse;
    ip=(void *)&ipv6->sin6_addr;
    port=ipv6->sin6_port;
    }
fprintf(flux,"Adresse IP%s : ",padresse->ss_family==AF_INET?"v4":"v6");
afficheAdresse(flux,ip,padresse->ss_family);
fprintf(flux,"\nPort de la socket : %d.\n",ntohs(port));
}

/** Impression des informations d'un hote **/

static void afficheHote(FILE *flux,struct hostent *hote,int type)
{
char **params;

fprintf(flux,"Nom officiel : '%s'.\n",hote->h_name);
fprintf(flux,"Surnoms: ");
for(params=hote->h_aliases;*params!=NULL;params++){
	fprintf(flux,"%s",*params);
	if(*(params+1)==NULL) fprintf(flux,",");
	}
fprintf(flux,"\n");
fprintf(flux,"Type des adresses   : %d.\n",hote->h_addrtype);
fprintf(flux,"Taille des adresses : %d.\n",hote->h_length);
fprintf(flux,"Adresses: ");
for(params=hote->h_addr_list;params[0]!=NULL;params++){
	afficheAdresse(flux,(struct in_addr *)params,type);
	if((*params+1)!=NULL) fprintf(flux,",");
	}
fprintf(flux,"\n");
}
#endif

/** Cherche l'adresse IP d'un hote (0 sur succes et -1 sinon) **/

int nomVersAdresse(char *hote,struct sockaddr_storage *padresse)
{

/* Informations de deverminage */

#ifdef DEBUG
fprintf(stderr,"Recherche de \"%s\".\n",hote);
#endif

/* On regarde s'il s'agit deja d'une adresse IP */

struct sockaddr_in *ipv4_sock=(struct sockaddr_in *)padresse;
unsigned char *ipv4=(unsigned char *)&ipv4_sock->sin_addr;
int size=inet_pton(AF_INET,hote,ipv4);
if(size>0){
#ifdef DEBUG
  fprintf(stderr,"Adresse IPv4 : ");
  int i; for(i=0;i<size;i++) fprintf(stderr,"%02x ",ipv4[i]); fprintf(stderr,"\n");
#endif
  padresse->ss_family=AF_INET;
  return 0;
  }
struct sockaddr_in6 *ipv6_sock=(struct sockaddr_in6 *)padresse;
unsigned char *ipv6=(unsigned char *)&ipv6_sock->sin6_addr;
size=inet_pton(AF_INET6,hote,ipv6);
if(size>0){
#ifdef DEBUG
  fprintf(stderr,"Adresse IPv6 : ");
  int i; for(i=0;i<size;i++) fprintf(stderr,"%02x ",ipv6[i]); fprintf(stderr,"\n");
#endif
  padresse->ss_family=AF_INET6;
  return 0;
  }

/* Sinon on cherche l'adresse via le DNS */

struct hostent *h=gethostbyname2(hote,AF_INET6);
if(h!=NULL){
#ifdef DEBUG
  afficheHote(stderr,h,AF_INET6);
#endif
  memcpy(ipv6,h->h_addr_list[0],h->h_length);
#ifdef DEBUG
  fprintf(stderr,"Adresse IPv6 : ");
  int i; for(i=0;i<h->h_length;i++) fprintf(stderr,"%02x ",ipv6[i]); fprintf(stderr,"\n");
#endif
  padresse->ss_family=AF_INET6;
  return 0;
  }
h=gethostbyname2(hote,AF_INET);
if(h!=NULL){
#ifdef DEBUG
  afficheHote(stderr,h,AF_INET);
#endif
  memcpy(ipv4,h->h_addr_list[0],h->h_length);
#ifdef DEBUG
  fprintf(stderr,"Adresse IPv4 : ");
  int i; for(i=0;i<h->h_length;i++) fprintf(stderr,"%02x ",ipv4[i]); fprintf(stderr,"\n");
#endif
  padresse->ss_family=AF_INET;
  return 0;
  }
return -1;
}

/** Retourne le nom de la machine connectee sur la     **/
/** socket distante d'un descripteur                   **/

int socketVersNom(int ds,char *nom)
{
struct sockaddr_storage adresse;
struct sockaddr *padresse=(struct sockaddr *)&adresse;
socklen_t taille=sizeof adresse;
int statut;
struct hostent *machine;

/* Recupere l'adresse de la socket distante */
statut=getpeername(ds,padresse,&taille);
if(statut<0){
    perror("socketVersNom.getpeername");
    exit(-1);
    }

/* Recupere le nom de la machine */
void *ip;
int taille_ip;
int taille_nom;
if(padresse->sa_family==AF_INET){
    struct sockaddr_in *ipv4=(struct sockaddr_in *)padresse;
    ip=(void *)&ipv4->sin_addr;
    taille_ip=sizeof(ipv4->sin_addr);
    taille_nom=INET_ADDRSTRLEN;
    }
if(padresse->sa_family==AF_INET6){
    struct sockaddr_in6 *ipv6=(struct sockaddr_in6 *)padresse;
    ip=(void *)&ipv6->sin6_addr;
    taille_ip=sizeof(ipv6->sin6_addr);
    taille_nom=INET6_ADDRSTRLEN;
    }
machine=gethostbyaddr(ip,taille_ip,padresse->sa_family);
if(machine==NULL){
    inet_ntop(padresse->sa_family,ip,nom,taille_nom);
    return -1;
    }
else{
    strcpy(nom,machine->h_name);
    return 0;
    }
}

/** Fonction permettant de configurer une socket avec   **/
/** les options habituelles                             **/

static int configurationSocket(int ds)
{
int vrai=1;
if(setsockopt(ds,SOL_SOCKET,SO_REUSEADDR,&vrai,sizeof(vrai))<0) return -1;
if(setsockopt(ds,IPPROTO_TCP,TCP_NODELAY,&vrai,sizeof(vrai))<0) return -1;
return 0;
}

/** Ouvre une socket sur un hote et un port determine   **/
/** retourne le descripteur de la socket ou -1 (erreur) **/

int connexionServeur(char *hote,int port)
{
int df;
struct sockaddr_in6 adresse;
socklen_t taille=sizeof adresse;

/* Creation d'une socket */
df=socket(PF_INET6,SOCK_STREAM,0);
if(df<0){
        perror("connexionServeur.socket");
        exit(-1);
        }

/* Quelques options sur la socket */
if(configurationSocket(df)<0){ 
  perror("connexionServeur.configurationSocket");
  exit(-1);
  }

/* Connection de la socket a l'hote */
adresse.sin6_family=AF_INET6;
if(nomVersAdresse(hote,(void *)&adresse)<0) return -1;
adresse.sin6_port=htons(port);
adresse.sin6_flowinfo=0;
adresse.sin6_scope_id=0;
#ifdef DEBUG
fprintf(stderr,"Connexion a l'adresse TCP :\n");
afficheAdresseSocket(stderr,(struct sockaddr_storage *)&adresse);
#endif
if(connect(df,(struct sockaddr *)&adresse,taille)<0) return(-1);
else return df;
}

/** Ouvre une socket en lecture et retourne le numero    **/
/** du port utilise. La fonction retourne le descripteur **/
/** de socket ou -1 en cas d'erreur.                     **/

int initialisationServeur(short int *port)
{
int df;
struct sockaddr_in6 adresse;
socklen_t taille=sizeof adresse;
int statut;

/* Creation d'une socket */
df=socket(PF_INET6,SOCK_STREAM,0);
if(df<0){
    perror("initialisationServeur.socket");
    exit(-1);
    }

/* Quelques options sur la socket */
if(configurationSocket(df)<0){ 
  perror("connexionServeur.configurationSocket");
  exit(-1);
  }

/* Specification de l'adresse de la socket */
adresse.sin6_family=AF_INET6;
adresse.sin6_addr=in6addr_any;
adresse.sin6_port=htons(*port);
adresse.sin6_flowinfo=0;
adresse.sin6_scope_id=0;
#ifdef DEBUG
fprintf(stderr,"Adresse TCP demandee :\n");
afficheAdresseSocket(stderr,(struct sockaddr_storage *)&adresse);
#endif
statut=bind(df,(struct sockaddr *)&adresse,taille);
if(statut<0) return -1;

/* Recuperation du numero du port utilise */
statut=getsockname(df,(struct sockaddr *)&adresse,&taille);
if(statut<0){
    perror("initialisationServeur.getsockname");
    exit(-1);
    }
*port=ntohs(adresse.sin6_port);

/* Taille de la queue d'attente */
statut=listen(df,MAX_CONNEXIONS);
if(statut<0) return -1;

return df;
}

/** Lecture d'un tableau d'octets de taille precise **/

int read_fixed(int descripteur,unsigned char *array,int size){
int bytes=0;
while(bytes<size){
  int offset=read(descripteur,array+bytes,size-bytes);
  if(offset<=0) return -1; else bytes += offset;
  }
return bytes;
}

/** Ouverture d'une interface Ethernet virtuelle **/

int creationInterfaceVirtuelle(char *nom)
{
struct ifreq interface;
int fd,erreur;

/* Ouverture du peripherique principal */
if((fd=open(TAP_PRINCIPAL,O_RDWR))<0) return fd;

/* Preparation de la structure */
memset(&interface,0,sizeof(interface));
interface.ifr_flags=IFF_TAP|IFF_NO_PI;
if(nom!=NULL) strncpy(interface.ifr_name,nom,IFNAMSIZ);

/* Creation de l'interface */
if((erreur=ioctl(fd,TUNSETIFF,(void *)&interface))<0){ close(fd); return erreur; }

/* Recuperation du nom de l'interface */
if(nom!=NULL) strcpy(nom,interface.ifr_name);

return fd;
}
