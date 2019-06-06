#include <unistd.h>             //Needed for I2C port
#include <fcntl.h>              //Needed for I2C port
#include <sys/ioctl.h>          //Needed for I2C port
#include <linux/i2c-dev.h>      //Needed for I2C port
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 100
#define DEBUG			1

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


// variables globales

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
 *	Buffers de reception
 */
char rxBuffer;
char rxStrbuffer[MAX_BUFFER_SIZE] = { 0 };

/*!
 *	Structure d'information du device
 */
deviceInfo devInfo = {0};


/*!
 *  @brief  Analyse des différents charactères des données de la trame pour reconnaitre ou non le DevEui
 *  @param  str      chaine de charactères à analyser
 *  @retval boolean
 */
bool is_dev_eui(char *str);

/*!
 *  @brief  Analyse de l'élément devEui de la structure di afin de savcoir s'il est vide ou non
 *  @param  di      structure de type deviceInfo à analyser
 *  @retval boolean
 */
bool empty_dev_eui(deviceInfo di);

/*!
 *  @brief  Permet de récuperer le devEui contenu dans la chaine str et de le formater une structure de type deviceInfo
 *  @param  str      chaine de charactères à copier
 *  @retval deviceInfo
 */
deviceInfo dev_eui_cpy(char *str);

/*!
 * 	@brief       Permet de créer le nom du fichier de log courant
 * 	@retval      void
 */
void create_deveui_filename();

int main(int argc, char **argv)
{

	time_t t;
  char date[20];

	//----- OPEN THE I2C BUS -----
	char *filename = (char *)"/dev/i2c-1";
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		fprintf(stderr, "Error open : (errno %d) %s\r\n",errno, strerror(errno));
		return EXIT_FAILURE;
	}

	int current_addr = 0x08;
	if (ioctl(file_i2c, I2C_SLAVE, current_addr) < 0)
	{
		fprintf(stderr, "Error ioctl : (errno %d) %s\r\n",errno, strerror(errno));
		return EXIT_FAILURE;
	}
	while(true)
	{
		if (read(file_i2c, &rxBuffer, 1) > 0)
		{		
			if(rxBuffer == '\r' || rxBuffer == '\n')
			{
				//si la chaine est vide (e.g : si elle contient uniquement un \r ou \n)
				if(strlen(rxStrbuffer) <= 0)
					continue;

				if(empty_dev_eui(devInfo))
				{
					if( is_dev_eui(rxStrbuffer) )
					{
						//on remplit la structure du device
						devInfo = dev_eui_cpy(rxStrbuffer);
						devInfo.i2cAddr = current_addr;
					}
				}
				else
				{
					//on crée une nom de fichier en fonction du devEui
					create_deveui_filename();

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
						fprintf(stdout,"%s; 0x%02X; %s\r\n",date,current_addr,rxStrbuffer);
#endif
						fprintf(file_csv_current,"%s; 0x%02X; %s\r\n",date,current_addr,rxStrbuffer);

						//fermeture du fichier de log
						fclose(file_csv_current);
					}
				}
				memset( rxStrbuffer, '\0', sizeof(char) * MAX_BUFFER_SIZE );	
			}
			else
			{
				strncat(rxStrbuffer, &rxBuffer, 1);
			}	
		}		
	}
	close(file_i2c);
	return EXIT_SUCCESS;
}

bool is_dev_eui(char *str)
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

bool empty_dev_eui(deviceInfo di)
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

deviceInfo dev_eui_cpy(char *str)
{
	deviceInfo d;
	int j = 0;
	char buff[3];
	int ibuff = 0;
	while( !isxdigit(str[j]) )
	{
		j++;
	}
	for (int i = j, k = 0; i < j+23; i++)
	{
		if (str[i] != '-' && str[i + 1] != '-' && i != j+23)
		{
			buff[0] = str[i];
			buff[1] = str[i + 1];
			buff[2] = '\0';
			//conversion en base 16 du buffer dans un entier
			ibuff = (int)strtol(buff, NULL, 16);
			d.devEui[k] = ibuff;
			k++;
		}
		if (i == j+23)
			break;
	}
	return d;
}

void create_deveui_filename()
{
        sprintf(filename_csv_current,"./logs/devices/%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X_logs.csv",devInfo.devEui[0],devInfo.devEui[1],devInfo.devEui[2],devInfo.devEui[3],devInfo.devEui[4],devInfo.devEui[5],devInfo.devEui[6],devInfo.devEui[7]);
}

