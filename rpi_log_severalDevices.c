#include <unistd.h>             //Needed for I2C port
#include <fcntl.h>              //Needed for I2C port
#include <sys/ioctl.h>          //Needed for I2C port
#include <linux/i2c-dev.h>      //Needed for I2C port
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define NB_MIDDLE_BOARD         1
#define NB_MB_UART_PORT         1
#define MAX_BUFFER_SIZE         100
#define DEBUG                   1

/*!
 *	Structure définissant diverses informations concernants un capteur à savoir :
 *	- l'adresse i2c de la middle Board qui fait l'intermédiaire avec la rpi
 * 	- son devEui
 */
typedef struct 
{
    int i2cAddr;
    unsigned char devEui[8];
} deviceInfo ;


/*!
 *	Descripteur de fichier du bus i2c
 */
int file_i2c;

/*!
 *	Descripteur de fichier du fichier de log du device
 */
FILE *file_csv_current;

/*!
 *	Label du fichier de log du device
 */
char filename_csv_current[50] = { 0 };

/*!
 *  Tableau contenant les informations des différents capteurs
 */
deviceInfo devInfArr[NB_MIDDLE_BOARD][NB_MB_UART_PORT] = { 0 };

/*!
 *	Buffers de reception
 */
char rx_buffer = 0;
char str_buffer[MAX_BUFFER_SIZE];

/*!
 *  Tableau contenant les adresses i2c des boards intermédiaires
 */
int devices_addr[] = {
    0x08
};
/*!
 *  Index pour le parcours des boards intermédiaires
 */
int index_mb;
/*!
 *  Index pour le parcours des capteurs
 */
int index_s;

/*!
 *  @brief  Analyse des différents charactères des données de la trame pour reconnaitre ou non le DevEui
 *  @param  str      chaine de charactères à analyser
 *  @retval boolean
 */
bool isDevEui(char *str);

/*!
 *  @brief  Analyse de l'élément devEui de la structure di afin de savcoir s'il est vide ou non
 *  @param  di      structure de type deviceInfo à analyser
 *  @retval boolean
 */
bool emptyDevEui(deviceInfo di);

/*!
 *  @brief  Permet de récuperer le devEui contenu dans la chaine str et de le formater une structure de type deviceInfo
 *  @param  str      chaine de charactères à copier
 *  @retval deviceInfo
 */
deviceInfo devEuiCpy(char *str);

/*!
 * 	@brief  Permet de créer le nom du fichier de log courant
 * 	@retval void
 */
void createDevEuiFilename();

int main(int argc, char **argv)
{
    time_t t;
    char date[20];

    //----- OPEN THE I2C BUS -----
    char *filename = (char *)"/dev/i2c-1";
    if ((file_i2c = open(filename, O_RDWR)) < 0)
    {
        fprintf(stderr, "Error open : (errno %d) %s\r\n",errno,strerror(errno));
        return EXIT_FAILURE;
    }
    int current_addr = 0x00;       //current i2c addr
    
#if DEBUG == 1
    printf("###### ===== INVISSYS LOG RECUP ===== ######\r\n\r\n");
    printf("NUMBER OF SENSORS : %d\r\n", (NB_MB_UART_PORT*NB_MIDDLE_BOARD) );
    printf("NUMBER OF MIDDLE BOARDS : %d\r\n", NB_MIDDLE_BOARD);
    printf("I2C ADDRESS OF BOARDS : \r\n");
    for(int i=0 ; i<NB_MIDDLE_BOARD ; i++)
        printf("\t0x%02X\r\n",devices_addr[i]);
    printf("###### ============================== ######\r\n\r\n");
#endif

    while (true)
    {     
        //Communication avec les cartes intermédiaires
        for (index_mb = 0; index_mb < NB_MIDDLE_BOARD; index_mb++)
        { 
            current_addr = devices_addr[index_mb];

            //permet d'initier la connexion avec l'esclave courant
            if (ioctl(file_i2c, I2C_SLAVE, current_addr) < 0)
            {
                fprintf(stderr, "Error ioctl : (errno %d) %s\r\n",errno, strerror(errno));
                continue;
            }
            //Communication avec les cartes 
            for (index_s = 0; index_s < NB_MB_UART_PORT; index_s++)
            { 
                //read a line
                while(true)
                {
                    if (read(file_i2c, &rx_buffer, 1) > 0)
                    {
                        if(rx_buffer == '\r' || rx_buffer == '\n')
                        {
                            //si la chaine est vide (e.g : si elle contient uniquement un \r ou \n)
                            if(strlen(str_buffer) <= 0)
                                break;

                            if(emptyDevEui(devInfArr[index_mb][index_s]))
                            {
                                if( isDevEui(str_buffer) )
                                {
                                    //on remplit la structure du device
                                    devInfArr[index_mb][index_s] = devEuiCpy(str_buffer);
                                    devInfArr[index_mb][index_s].i2cAddr = current_addr;
                                }
                            }
                            else
                            {
                                //on crée une nom de fichier en fonction du devEui
                                createDevEuiFilename();

                                if ((file_csv_current = fopen(filename_csv_current, "a")) < 0)
                                {
                                    fprintf(stderr, "Error open : (errno %d) %s\r\n",errno, strerror(errno));
                                    return EXIT_FAILURE;
                                }
                                else
                                {
                                    //on vérifie si le fichier est vide
                                    if(ftell(file_csv_current) == 0)
                                        fprintf(file_csv_current,"Timestamp; MiddleBoard Address; Log\r\n");
                                    
                                    //on génère le timestamp
                                    time(&t);
                                    strftime(date,sizeof(date),"%d/%m/%Y %H:%M:%S",localtime(&t));

                                    //on écrit dans le fichier de log
#if DEBUG == 1
                                    fprintf(stdout,"%s; 0x%02X; %s\r\n",date,devInfArr[index_mb][index_s].i2cAddr,str_buffer);
#endif
                                    fprintf(file_csv_current,"%s; 0x%02X; %s\r\n",date,devInfArr[index_mb][index_s].i2cAddr,str_buffer);

                                    //fermeture du fichier de log
                                    fclose(file_csv_current);
                                }
                            }
                            //remise de la chaine à zero
                            memset( str_buffer, '\0', sizeof(char) * MAX_BUFFER_SIZE );	
                            break;
                        }
                        else
                        {
                            //concaténation de la chaine str_buffer avec le caractère recu en i2c
                            strncat(str_buffer, &rx_buffer, 1);
                        }
                    }
                    else
                    {
                        break;
                    }                   	                                           
                }
            }
        }
    }
    close(file_i2c);     
    return EXIT_SUCCESS;
}

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
 *  @param  di      structure de type deviceInfo à analyser
 *  @retval boolean
 */
bool emptyDevEui(deviceInfo di)
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
 *  @brief  Permet de récuperer le devEui contenu dans la chaine str et de le formater une structure de type deviceInfo
 *  @param  str      chaine de charactères à copier
 *  @retval deviceInfo
 */
deviceInfo devEuiCpy(char *str)
{
	deviceInfo d;
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
			d.devEui[k] = ibuff;
			k++;
		}
		if (i == j+23)  //si on a parcouru tout les éléments du devEui on sort de la boucle
			break;
	}
	return d;
}

/*!
 * 	@brief  Permet de créer le nom du fichier de log courant
 *      Format du chemin :
 *          "./logs/devices/dev_eui_du_capteur_logs.csv"
 * 	@retval void
 */
void createDevEuiFilename()
{
    sprintf(filename_csv_current,"./logs/devices/%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X_logs.csv",devInfArr[index_mb][index_s].devEui[0],devInfArr[index_mb][index_s].devEui[1],devInfArr[index_mb][index_s].devEui[2],devInfArr[index_mb][index_s].devEui[3],devInfArr[index_mb][index_s].devEui[4],devInfArr[index_mb][index_s].devEui[5],devInfArr[index_mb][index_s].devEui[6],devInfArr[index_mb][index_s].devEui[7]);
}