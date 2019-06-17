#include <fcntl.h>         //Needed for I2C port
#include <sys/ioctl.h>     //Needed for I2C port
#include <linux/i2c-dev.h> //Needed for I2C port
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "utils.h"

#define MAX_BUFFER_SIZE 100
#define DEBUG 1

int main(int argc, char **argv)
{
    /***********************************************************/
    /**************** DECLARATION DES VARIABLES ****************/
    /***********************************************************/


    //Descripteur de fichier du bus i2c
    int file_i2c;
 
    //Descripteur du fichier de log du device
    FILE *file_csv_current;

    //Label du fichier de log du device
    char filename_csv_current[50] = {0};

    //Tableau contenant les informations des différents capteurs
    deviceInfo_t devInfArr[MAX_I2C_ADDR][MAX_UART_PORT] = {0};

    //Buffers de reception
    char ch_buffer = 0;
    char str_buffer[MAX_BUFFER_SIZE];

    //Tableau contenant les adresses i2c des boards intermédiaires
    //plages d'adresses i2c : 0x08 - 0x77
    int devices_addr[MAX_I2C_ADDR] = {0};

    //Variables de temps
    time_t t;

    //Chaine de caractères contenant le timestamp
    char date[20];

    //Buffer de reception i2c
    char rx_buff[2];

    /****************************************************/
    /**************** CORPS DU PROGRAMME ****************/
    /****************************************************/

    //----- AFFICHAGE DU MENU DE PARAMETRAGE -----
    configMenu(devices_addr);

    //----- OUVERTURE DU BUS I2C -----
    char *filename = (char *)"/dev/i2c-1";
    if ((file_i2c = open(filename, O_RDWR)) < 0)
    {
        fprintf(stderr, "Error open : (errno %d) %s\r\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    int current_addr = 0x00; //adresse i2c courante

    while (true)
    {
        //Communication avec les cartes intermédiaires (index_mb = index middle boards | index_s = index sensors)
        for (int index_mb = 0, index_s = 0; index_mb < getNbElements(devices_addr); index_mb++)
        {
            current_addr = devices_addr[index_mb];

            //permet d'initier la connexion avec l'esclave i2c courant (carte intermédiaire)
            if (ioctl(file_i2c, I2C_SLAVE, current_addr) < 0)
            {
                fprintf(stderr, "Error ioctl : (errno %d) %s\r\n", errno, strerror(errno));
                continue;
            }
            //read a line
            while (true)
            {
                if (read(file_i2c, rx_buff, 2) > 0)
                {
                    //on vérifie si l'on a bien recu le buffer avec le port uart en deuxième caractère
                    if (!isdigit(rx_buff[1]))
                    {
                        continue;
                    }
                    index_s = atoi(&rx_buff[1]) - 1; //convertion du caractère en entier -1
                    i2cParse(&devInfArr[index_mb][index_s], &ch_buffer, rx_buff);

                    if (ch_buffer == '\r' || ch_buffer == '\n')
                    {
                        //si la chaine est vide (e.g : si elle contient uniquement un \r ou \n)
                        if (strlen(str_buffer) <= 0)
                            break; //A VOIR

                        if (emptyDevEui(devInfArr[index_mb][index_s]))
                        {
                            if (isDevEui(str_buffer))
                            {
                                //on remplit la structure d'information du capteur
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
                                fprintf(stderr, "Error open : (errno %d) %s\r\n", errno, strerror(errno));
                                return EXIT_FAILURE;
                            }
                            else
                            {
                                //on vérifie si le fichier est vide
                                if (ftell(file_csv_current) == 0)
                                    fprintf(file_csv_current, "Timestamp; MiddleBoard Address; Uart Port on MB; Log\r\n");

                                //on génère le timestamp
                                time(&t);
                                strftime(date, sizeof(date), "%d/%m/%Y %H:%M:%S", localtime(&t));

                                //on écrit dans le fichier de log
#if DEBUG == 1
                                fprintf(stdout, "%s; 0x%02X; %d; %s\r\n", date, devInfArr[index_mb][index_s].i2cAddr, devInfArr[index_mb][index_s].uartPort, str_buffer);
#endif
                                fprintf(file_csv_current, "%s; 0x%02X; %d; %s\r\n", date, devInfArr[index_mb][index_s].i2cAddr, devInfArr[index_mb][index_s].uartPort, str_buffer);

                                fflush(stdout);

                                //fermeture du fichier de log
                                fclose(file_csv_current);
                            }
                        }
                        //remise de la chaine à zero
                        memset(str_buffer, '\0', sizeof(char) * MAX_BUFFER_SIZE);
                        break;
                    }
                    else
                    {
                        //concaténation de la chaine str_buffer avec le caractère recu en i2c
                        strncat(str_buffer, &ch_buffer, 1);
                    }
                }
                else if (strlen(str_buffer) == 0)
                {
                    break;
                }
            }
        }
    }
    close(file_i2c);
    return EXIT_SUCCESS;
}