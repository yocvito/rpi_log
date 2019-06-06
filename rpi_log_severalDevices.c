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

//defines
#define NB_MIDDLE_BOARD         1
#define NB_MB_UART_PORT         1
#define MAX_STR_SIZE            100

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
char str_buffer[MAX_STR_SIZE];

/*!
 *  Tableau contenant les adresses i2c des boards intermédiaires
 */
int devices_addr[] = {
    0x08
    //Devices addr here
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
    else
    {
        int current_addr = 0x00;       //current i2c addr
        while (true)
        {       
            //communicate with middle boards
            for (index_mb = 0; index_mb < NB_MIDDLE_BOARD; index_mb++)
            {
                current_addr = devices_addr[index_mb];

                if (ioctl(file_i2c, I2C_SLAVE, current_addr) < 0)
                {
                    fprintf(stderr, "Error ioctl : (errno %d) %s\r\n",errno, strerror(errno));
                    continue;
                }
                //communicate with sensors
                for (index_s = 0; index_s < NB_MB_UART_PORT; index_s++)
                {
                    //read a line
                    while(true)
                    {
                        //----- READ DATA -----
                        if (read(file_i2c, &rx_buffer, 1) > 0)
                        {
                            if(rx_buffer == '\r' || rx_buffer == '\n')
                            {

                            }
                            else
                            {

                            }                                            
                        }
                        else
                        {
                            //ERROR HANDLING: i2c transaction failed
                            fprintf(stderr, "Error read : (errno %d) %s\r\n",errno, strerror(errno));
                            break;
                        }
                    }
                }
            }
        }
        close(file_i2c);
    }      
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

deviceInfo devEuiCpy(char *str)
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
			//conversion en base 16 du buff dans un entier
			ibuff = (int)strtol(buff, NULL, 16);
			d.devEui[k] = ibuff;
			k++;
		}
		if (i == j+23)
			break;
	}
	return d;
}

/*!
 * @brief       Create the log gilename
 * @retval      void
 */
void createDevEuiFilename()
{
    sprintf(filename_csv_current,"./logs/devices/%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X_logs.csv",devInfo.devEui[0],devInfo.devEui[1],devInfo.devEui[2],devInfo.devEui[3],devInfo.devEui[4],devInfo.devEui[5],devInfo.devEui[6],devInfo.devEui[7]);
}