#include <unistd.h>             //Needed for I2C port
#include <fcntl.h>              //Needed for I2C port
#include <sys/ioctl.h>          //Needed for I2C port
#include <linux/i2c-dev.h>      //Needed for I2C port
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include "utils.h"

#define MAX_BUFFER_SIZE 100
#define DEBUG			1

/*!
 *	Descripteur de fichier du bus i2c
 */
int file_i2c;

/*!
 *	Descripteur de fichier du fichier de log du device
 */
FILE *file_csv;

/*!
 *	Label du fichier de log du device
 */
char filename_csv[50] = { 0 };

/*!
 *	Buffers de reception
 */
char rx_buffer;
char str_buffer[MAX_BUFFER_SIZE] = { 0 };

/*!
 *	Structure d'information du device
 */
deviceInfo_t devInfo = {0};

int main(int argc, char **argv)
{

	time_t t;
  	char date[20];

	//----- OPEN THE I2C BUS -----
	char *filename_i2c = (char *)"/dev/i2c-1";
	if ((file_i2c = open(filename_i2c, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		fprintf(stderr, "Error open : (errno %d) %s\r\n",errno, strerror(errno));
		return EXIT_FAILURE;
	}

	int slave_addr = 0x08;
	if (ioctl(file_i2c, I2C_SLAVE, slave_addr) < 0)
	{
		fprintf(stderr, "Error ioctl : (errno %d) %s\r\n",errno, strerror(errno));
		return EXIT_FAILURE;
	}
	while(true)
	{
		if (read(file_i2c, &rx_buffer, 1) > 0)
		{		
			if(rx_buffer == '\r' || rx_buffer == '\n')
			{
				//si la chaine est vide (e.g : si elle contient uniquement un \r ou \n)
				if(strlen(str_buffer) <= 0)
					continue;

				//on vérifie si le devEui est vide
				if(emptyDevEui(devInfo))
				{
					if( isDevEui(str_buffer) )
					{
						//on remplit la structure du device
						devEuiCpy(str_buffer, &devInfo);
						devInfo.i2cAddr = slave_addr;
					}
				}
				else
				{
					//on crée une nom de fichier en fonction du devEui
					createDevEuiFilename(filename_csv, devInfo);

					if ((file_csv = fopen(filename_csv, "a")) < 0)
					{
						fprintf(stderr, "Error open : (errno %d) %s\r\n",errno, strerror(errno));
						return EXIT_FAILURE;
					}
					else
					{
						//on vérifie si le fichier est vide
						if(ftell(file_csv) == 0)
							fprintf(file_csv,"Timestamp; MiddleBoard Address; Log\r\n");
						
						//on génère le timestamp
						time(&t);
						strftime(date,sizeof(date),"%d/%m/%Y %H:%M:%S",localtime(&t));

						//on écrit dans le fichier de log
#if DEBUG == 1
						fprintf(stdout,"%s; 0x%02X; %s\r\n",date,slave_addr,str_buffer);
#endif
						fprintf(file_csv,"%s; 0x%02X; %s\r\n",date,slave_addr,str_buffer);

						//fermeture du fichier de log
						fclose(file_csv);
					}
				}
				//remise de la chaine à zero
				memset( str_buffer, '\0', sizeof(char) * MAX_BUFFER_SIZE );	
			}
			else
			{
				//concaténation de la chaine str_buffer avec le caractère recu en i2c
				strncat(str_buffer, &rx_buffer, 1);
			}	
		}		
	}
	close(file_i2c);
	return EXIT_SUCCESS;
}