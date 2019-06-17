#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <termios.h>

#define MAX_I2C_ADDR	(0x77-0x08)
#define MAX_UART_PORT	6

/*!
 *	Structure définissant diverses informations concernants un capteur à savoir :
 *	- l'adresse i2c de la middle Board qui fait l'intermédiaire avec la rpi
 * 	- son devEui
 *  [Dans le futur on peut imaginer la transmission par i2c du port uart auquel est connecté 
 *  le capteur sur la board intermédiaire. Dans ce cas là, il faudra implémenter ici aussi 
 *  le port uart]
 */
typedef struct deviceInfo_t
{
	int i2cAddr;
	unsigned char devEui[8];
	int uartPort;
} deviceInfo_t ;

/*!
 *  @brief  Analyse des différents charactères des données de la trame pour reconnaitre ou non le DevEui
 *  @param  str      chaine de charactères à analyser
 *  @retval boolean
 */
bool isDevEui(char *str)
{
  bool ret = false;
  int j = 0;
	//on attend de rencontrer le premier caractère hexadecimal de str
	while( !isxdigit(str[j]) )
	{
		j++;
		if( j > (strlen(str)-1) )			//si j est supérieur à l'index du dernier élément de str
			return false;					//et qu'on n'a toujours pas rencontré de caractère hexadecimal on retourne faux
	}

    //on ne rentre pas dans la boucle si str à partir de l'indice j fais au moins la taille du devEui
    if(strlen(str) >= j+22)
    {
        for (int i = j, k = i+2; i < j+23; i++)
        {
            if (k != i)
            {
                if ( isxdigit(str[i]) )
                {
                    ret = true;
                }
                else
                {
                    ret = false;
                    break;
                }        
            }
            else
            {
                k += 3;
            }
        }
    }	
  return ret;
}

/*!
 *  @brief  Analyse de l'élément devEui de la structure di afin de savcoir s'il est vide ou non
 *  @param  di      structure de type deviceInfo_t à analyser
 *  @retval boolean
 */
bool emptyDevEui(deviceInfo_t di)
{
	bool ret = false;
	for(int i=0 ; i<8 ; i++)
	{
		if(di.devEui[i] == 0)
			ret = true;
		else
			ret = false;          
	}
	return ret;
}

/*!
 *  @brief  Permet de récuperer le devEui contenu dans la chaine str et de le formater une structure de type deviceInfo_t
 *  @param  di      structure d'information du capteur
 *  @param  str     chaine de charactères à copier
 *  @retval none
 */
void devEuiCpy(deviceInfo_t *di, char *str)
{
	int j = 0;
	char buff[3];
	int ibuff = 0;

	while( !isxdigit(str[j]) )
	{
		j++;        //pas besoin de condition de sortie de la boucle puisque cette fonction
	}               //est appelée uniquement si str contient le devEui
	
    //on parcours la chaine sur portion ou est le devEui
    for (int i = j, k = 0; i < j+23; i++)
	{
        
		if (str[i] != '-' && str[i + 1] != '-' && i != j+23)
		{
			buff[0] = str[i];       //permet de copier le nombre
			buff[1] = str[i + 1];   //hexadecimal dans une chaine de caractère
			buff[2] = '\0';         //afin de la convertir en entier

			//conversion en base 16 du buff dans un entier
			ibuff = (int)strtol(buff, NULL, 16);
			di->devEui[k] = ibuff;
			k++;
		}
		if (i == j+23)  //si on a parcouru tout les éléments du devEui on sort de la boucle
			break;
	}
}

/*!
 * 	@brief  Permet de créer le nom du fichier de log courant
 *      Format du chemin :
 *          "./logs/devEuiDuCapteur_logs.csv"
 *  @param  filename    chaine de caractère pour acceuillir le chemin du fichier de log
 *  @param  di          strcture d'information du capteur          
 * 	@retval none
 */
void createDevEuiFilename(char *filename, deviceInfo_t di)
{
    sprintf(filename,"./logs/%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X_logs.csv",di.devEui[0],di.devEui[1],di.devEui[2],di.devEui[3],di.devEui[4],di.devEui[5],di.devEui[6],di.devEui[7]);
}

/*!
 *	@brief	Permet de décomposer les 2 caractères reçus 
 *	@param	di		structure d'information du capteur dont l'élément uartPort va recevoir le premier caractère
 * 	@param	ch		adresse du caractère recevant le deuxième caractère
 * 	@param	buff	buffer contenant les 2 caractères
 * 	@schem		buff	|	car1	|	 car2	|
 * 							 |			  |
 * 							 V			  |
 * 		di->uartPort	|  car1-'0' |	  |				 
 * 		ch				|	 car2	|   <--	
 * 	@retval	none
 */
void i2cParse(deviceInfo_t *di, char *ch, char *buff)
{
	di->uartPort = atoi(&buff[1]);		//on convertie le premier caractère en entier
	*ch = buff[0];		
}

/*!
 *	@brief	Permet de calculer la tailler d'un tableau d'entier 
 * 	ATTENTION : Fonctionne uniquement si pas de valeurs vide en pleins milieu du tableau
 *	@param	p	pointeur vers le premier element du tableau
 */
size_t getNbElements(int *p)
{
	size_t count = 0;
	while(*p != '\0')
	{
		count++;
		p++;
	}
	return count;
}

/*!
 *	@brief	Permet libérer complètement le flux d'entrée standard
 *	@retval	none
 */
void inflush ( void )
{
  int ch;

  do
    ch = fgetc ( stdin ); 
  while ( ch != EOF && ch != '\n' ); 

  clearerr ( stdin );
}

/*!
 *	@brief	Affiche le menu de configuration	
 * 	@param	tab	Tableau d'adresses i2c à configurer
 * 	@retval	none
 */
void configMenu(int *tab)
{
	int nb_mb = 0;						//nombre middle board
	int nb_up = 0;						//nombre uart port
	char *end;
	char buf[3];

	system("clear");

	printf("###### ===== Configuration système de log - INVISSYS ===== ######\r\n\r\n");

	printf("Nombres de boards intermédiaires :\t");
	scanf("%d",&nb_mb);
	printf("\r\n");

	printf("Adresses en hexadecimal - format = XX (faire suivre les adresses i2c): \r\n");
	
	for(int i=0, *p = tab ; i<nb_mb ; i++, p++)
	{
		printf("\t-> ");
		scanf("%s",buf);
		*p = (int)strtol(buf,&end,16);
		if( (strlen(buf) > 2) || (*p > 119) || (*p < 8))
		{
			printf("Suivre le format (XX) et ne pas dépasser 0x77\r\n");
			i--;
			continue;
		}
	}

	printf("Nombres de ports uart par board :\t");
	scanf("%d",&nb_up);
	printf("\r\n");

	printf("Appuyer sur [ENTRER] pour commencer . . .");

	inflush();

	while(getchar() != 10) {}

	system("clear");

    printf("###### ===== INVISSYS LOG RECUP ===== ######\r\n\r\n");
    printf("NUMBER OF SENSORS : %d\r\n", (nb_mb * nb_up));
    printf("NUMBER OF MIDDLE BOARDS : %d\r\n", nb_mb);
    printf("I2C ADDRESS OF BOARDS : \r\n");
    for (int i = 0, *p=tab ; i < getNbElements(tab); i++, p++)
        printf("\t0x%02X\r\n", *p);
    printf("###### ============================== ######\r\n\r\n");
}