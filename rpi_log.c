/*******************************************************
 *      file :                  rpi-i2c.c
 *      description :   programme maitre pour la
 *                                      communicaion i2c
 *
 *      format des trames i2c :
 *
 *              type : string
 *              données :
 *                      -> de l'index 0 à 7 :
 *                              Device Unique Id
 *                      -> index 9 :
 *                              i2c addr
 *                      -> index 11 :
 *                              uart port
 *
 *
 *
 *******************************************************/

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

#define NB_MIDDLE_BOARD         1
#define NB_UART_PORT_BY_MB      1


//typedefs
typedef struct 
{
        int i2cAddr;
        int uartPort;
        unsigned char devEui[8];
} deviceInfo ;


//variables
static const nb_sensors = NB_MIDDLE_BOARD * NB_UART_PORT_BY_MB;

int file_i2c;
FILE *file_csv;

deviceInfo devInfArr[nb_sensors] = { 0 };

char buffer = 0;
char filename_csv_current[32] = { 0 };

int current_addr = 0;       //current i2c addr
int current_up = 0;     //current uart port
int devices_addr[] = {
        0x08
        //Devices addr here
};

//permet de reconnaitre un devEui
bool is_dev_eui(char *str);

bool emptyDevEui(deviceInfo di);

//retransmettre les informations de la trame i2c dans chaque varibles correspondantes
void i2c_parse(unsigned char *buff);

//compare l'adresse i2c courante et l'adresse contenu dans la trame
bool cmp_i2c_addr(int addr, unsigned char *buff);

//permet de créer une chaine de charactere contenant le chemin d'accès au fichier csv courant
void create_deveui_filename();

int main(void)
{
        time_t t;
        char date[20];

        //----- OPEN THE I2C BUS -----
        char *filename = (char *)"/dev/i2c-1";
        if ((file_i2c = open(filename, O_RDWR)) < 0)
        {
                //ERROR HANDLING: you can check errno to see what went wrong
                fprintf(stderr, "Error open : Failed to open the i2c bus\r\n");
                return EXIT_FAILURE;
        }
        else
        {
                current_addr = 0x00; //<<<<<The I2C address of the slave
                while (true)
                {
                        for (int i = 0; i < NB_MIDDLE_BOARD; i++)
                        {
                                current_addr = devices_addr[i];

                                if (ioctl(file_i2c, I2C_SLAVE, current_addr) < 0)
                                {
                                        fprintf(stderr, "Error ioctl : Failed to acquire bus access and/or talk to slave : 0x%02X \r\n", current_addr);
                                        //ERROR HANDLING; you can check errno to see what went wrong
                                        continue;
                                }

                                //----- READ DATA -----
                                if (read(file_i2c, buffer, 2) > 0) //read() returns the number of bytes actually read, if it doesn't match then an error occurred (e.g. no response fr$
                                {
                                        //parse the i2c frame
                                        uart_port = buffer[0];

                                        if(emptyDevEui(devInfArr[i]))
                                                

                                
                                        //print into csv
                                        create_deveui_filename();

                                        if ((file_csv_current = fopen(filename_csv_current, "a+")) < 0)
                                        {
                                                        //ERROR HANDLING: you can check errno to see what went wrong
                                                fprintf(stderr, "Error open : Failed to open the csv current file\r\n");
                                                return EXIT_FAILURE;
                                        }
                                        else
                                        {
                                                fseek(file_csv_current,0,SEEK_END);
                                                if(ftell(file_csv_current) == 0)
                                                        fprintf(file_csv_current,"Timestamp, Data\n");
                                                //generate timestamp
                                                time(&t);
                                                strftime(date,sizeof(date),"%d/%m/%Y %H:%M:%S",localtime(&t));

                                                fprintf(file_csv_current,"%s,%s",date,& buffer[10]);

                                                fclose(file_csv_current);
                                        }

                                }
                                else
                                {
                                        //ERROR HANDLING: i2c transaction failed
                                        fprintf(stderr, "Error read : Failed to read data from the i2c bus.\r\n");
                                        continue;
                                }
                        }
                }
                close(file_i2c);
        }      
        return EXIT_SUCCESS;
}

/*!
 * @brief       Recognize if the string passed is a devEui
 * @param       str     string to analyze
 * @retval      bool    true - str is a devEui
 *                      false - str is not a devEui
 */
bool isDevEui(char *str)
{

}

bool emptyDevEui(deviceInfo di)
{
        bool ret = false;
        for(int i=0 ; i<7 ; i++)
        {
                if(di.devEui[i] == 0)
                        ret = true;
                else
                        ret = false;          
        }
        return ret;
}

/*!
 * @brief       Parse the i2c frame
 * @param       buff            the string to parse
 * @retval      none
 */
void i2c_parse(char *buff)
{
        uart_port = buffer[0];

        if(devInfArr[])
        //DevEui
	int k = 0;
        for(int i=2 ; i<=10 ; i++)
	{
                dev_eui[k] = (char)buffer[i];
		k++;
	}

        //uartPort
        
}

/*!
 * @brief       Create the log gilename
 * @retval      void
 */
void create_deveui_filename()
{
        char str[12] = { 0 };
        memset(filename_csv_current, 0, sizeof(filename_csv_current));
        strcat(filename_csv_current,"./logs/devices/");
	for(int i=0 ; i<8 ; i++)
        {
                sprintf(str,(i != 7)?"%02X-":"%02X",dev_eui[i]);
                strcat(filename_csv_current,str);
		printf("%02X - %d\n",dev_eui[i],(int)dev_eui[i]);
        }
        strcat(filename_csv_current,"_logs.csv");
}



