# rpi_log

## Programme de récupération de log pour un capteur

Cette version est la première version du programme de récupération de log. Elle sera la base afin de développer la version multi-capteurs

### Algorithme du programme

![alt text](https://github.com/yocvito/rpi_log/blob/master/Algo_oneDevice.png)

### Dictionnaire des données

<b>Nom : </b>file_i2c
<b>Type : </b>entier (int)
<b>Description : </b>Descripteur de fichier du bus i2c

<b>Nom : </b>filename_i2c
<b>Type : </b>chaine de caractères (char *)
<b>Description : </b>Label du chemin vers le bus i2c linux

<b>Nom : </b>file_csv
<b>Type : </b>descripteur de fichier (FILE *)
<b>Description : </b>Descripteur de fichier du fichier de log du capteur

<b>Nom : </b>filename_csv
<b>Type : </b>chaine de caractères (char *)
<b>Description : </b>Label du fichier de log du capteur

<b>Nom : </b>rxBuffer
<b>Type : </b>caractère (char)
<b>Description : </b>Buffer de reception

<b>Nom : </b>rxStrbuffer
<b>Type : </b>chaine de caractères (char *)
<b>Description : </b>chaine constituant le message de log

<b>Nom : </b>devInfo
<b>Type : </b>information de capteur (deviceInfo)
<b>Description : </b>Structure d'information du capteur

<b>Nom : </b>t
<b>Type : </b>temps (time_t)
<b>Description : </b>Variable de récupération de date

<b>Nom : </b>date
<b>Type : </b>chaine de caractères (char *)
<b>Description : </b>Chaine contenant l'horodatage formaté

<b>Nom : </b>slave_addr
<b>Type : </b> entier (int)
<b>Description : </b>Adresse i2c de l'esclave (Middle Board)


