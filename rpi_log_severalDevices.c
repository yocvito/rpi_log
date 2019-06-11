#include <fcntl.h>              //Needed for I2C port
#include <sys/ioctl.h>          //Needed for I2C port
#include <linux/i2c-dev.h>      //Needed for I2C port
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "utils.h"

#define NB_MIDDLE_BOARD         1
#define NB_MB_UART_PORT         1
#define MAX_BUFFER_SIZE         100
#define DEBUG                   1

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
int devices_addr[NB_MIDDLE_BOARD] = {
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

            //permet d'initier la connexion avec l'esclave i2c courant (carte intermédiaire)
            if (ioctl(file_i2c, I2C_SLAVE, current_addr) < 0)
            {
                fprintf(stderr, "Error ioctl : (errno %d) %s\r\n",errno, strerror(errno));
                continue;
            }
            //Parcours de chaque capteur 
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
                                    devEuiCpy(&devInfArr[index_mb][index_s], str_buffer);
                                    devInfArr[index_mb][index_s].i2cAddr = current_addr;
                                }
                            }
                            else
                            {
                                //on crée une nom de fichier en fonction du devEui
                                createDevEuiFilename(filename_csv_current, devInfArr[index_mb][index_s]);

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