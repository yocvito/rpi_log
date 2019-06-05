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

//typedefs
typedef struct 
{
        int i2cAddr;
        unsigned char devEui[8];
} deviceInfo ;


//variables

int file_i2c;
FILE *file_csv_current;

deviceInfo devInfArr[NB_MIDDLE_BOARD][NB_MB_UART_PORT] = { 0 };

char buffer = 0;
char str_buff[MAX_STR_SIZE];
char filename_csv_current[32] = { 0 };

int current_addr = 0;       //current i2c addr
int devices_addr[] = {
        0x08
        //Devices addr here
};
int index_mb;
int index_s;

//permet de reconnaitre un devEui
bool is_dev_eui(char *str);

//vérifie si le devEui courant est non initialisé
bool empty_dev_eui(deviceInfo di);

//Copie le dev Eui contenant dans la chaine str dans l'élement devEui de la structure deviceInfo
deviceInfo dev_eui_cpy(char *str);

//permet de créer une chaine de charactere contenant le chemin d'accès au fichier csv courant
void create_deveui_filename();

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
                current_addr = 0x00; //<<<<<The I2C address of the slave
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
                                                if (read(file_i2c, &buffer, 1) > 0)
                                                {
                                                        if(buffer != '\r')
                                                                strncat(str_buff, &buffer, 1);
                                                        else
                                                                break;       
                                                                
                                                }
                                                else
                                                {
                                                        //ERROR HANDLING: i2c transaction failed
                                                        fprintf(stderr, "Error read : (errno %d) %s\r\n",errno, strerror(errno));
                                                        break;
                                                }
                                        }
                                
   
                                        if(empty_dev_eui(devInfArr[index_mb][index_s]))
                                        {
                                                if( is_dev_eui(str_buff) )
                                                {
                                                devInfArr[index_mb][index_s] = dev_eui_cpy(str_buff); 
                                                devInfArr[index_mb][index_s].i2cAddr = current_addr;
                                                }
                                        }

                                
                                        //print into csv
                                        create_deveui_filename();

                                        if ((file_csv_current = fopen(filename_csv_current, "a+")) < 0)
                                        {
                                                fprintf(stderr, "Error open : (errno %d) %s\r\n",errno, strerror(errno));
                                                return EXIT_FAILURE;
                                        }
                                        else
                                        {
                                                //vérifie si le fichier est vide
                                                fseek(file_csv_current,0,SEEK_END);
                                                if(ftell(file_csv_current) == 0)
                                                        fprintf(file_csv_current,"Timestamp; Middle Board Address; Data\n");
                                                //generate timestamp
                                                time(&t);
                                                strftime(date,sizeof(date),"%d/%m/%Y %H:%M:%S",localtime(&t));

                                                fprintf(file_csv_current,"%s; 0x%02X; %s",date,current_addr,str_buff);

                                                memset( str_buff, '\0', sizeof(char) * MAX_STR_SIZE );
                                                fclose(file_csv_current);
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
bool is_dev_eui(char *str)
{
  bool ret = false;

  if ((strlen(str) - 1) >= 37)
  {
    for (int i = 15, k = i + 2; i < 37; i++)
    {
      if (k != i)
      {
        if (isxdigit(str[i]))
          ret = true;
        else
          ret = false;
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
        for(int i=0 ; i<7 ; i++)
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
        char buff[3];
        int ibuff = 0;
        for (int i = 0, k = 0; i < strlen((char *)str); i++)
        {
        if (i <= 15)
                continue;
        if (str[i] != '-' && str[i + 1] != '-' && i != 37)
        {
                //on utilise un for au lieu d'une simple copie de valeur de str à buff car problème lors de la copie des valeurs de txBuff
                buff[0] = str[i];
                buff[1] = str[i + 1];
                buff[2] = '\0';
                //conversion en base 16 du buffer dans un entier
                ibuff = (int)strtol(buff, NULL, 16);
                d.devEui[k] = ibuff;
                k++;
        }
        if (i == 37)
                break;
        }
        return d;
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
                sprintf(str,(i != 7)?"%02X-":"%02X",devInfArr[index_mb][index_s].devEui[i]);
                strcat(filename_csv_current,str);
        }
        strcat(filename_csv_current,"_logs.csv");
}



