/** fichier libnet.h **/

/******************************************************************/
/** Ce fichier decrit les structures et les constantes utilisees **/
/** par les fonctions reseau.                                    **/
/******************************************************************/

/**** Constantes ****/

/** Nombre maximum de connexions tamponnees pour le serveur **/

#define MAX_CONNEXIONS	32

/**** Fonctions ****/

int nomVersAdresse(char *hote,struct sockaddr_storage *padresse);
int socketVersNom(int ds,char *nom);
int connexionServeur(char *hote,int port);
int initialisationServeur(short int *port);
int read_fixed(int descripteur,unsigned char *array,int size);
int creationInterfaceVirtuelle(char *nom);
