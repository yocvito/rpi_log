# rpi_log

## Programme de récupération de log pour un capteur

Cette version est la première version du programme de récupération de log. Elle sera la base afin de développer la version multi-capteurs

### Algorithme du programme

![alt text](https://github.com/yocvito/rpi_log/blob/master/Algo_oneDevice.png)

### Dictionnaire des données

**Nom : **file_i2c
**Type : **entier (int)
**Description : **Descripteur de fichier du bus i2c

**Nom : **filename_i2c
**Type : **chaine de caractères (char *)
**Description : **Label du chemin vers le bus i2c linux

**Nom : **file_csv
**Type : **descripteur de fichier (FILE *)
**Description : **Descripteur de fichier du fichier de log du capteur

**Nom : **filename_csv
**Type : **chaine de caractères (char *)
**Description : **Label du fichier de log du capteur

**Nom : **rxBuffer
**Type : **caractère (char)
**Description : **Buffer de reception

**Nom : **rxStrbuffer
**Type : **chaine de caractères (char *)
**Description : **chaine constituant le message de log

**Nom : **devInfo
**Type : **information de capteur (deviceInfo)
**Description : **Structure d'information du capteur

**Nom : **t
**Type : **temps (time_t)
**Description : **Variable de récupération de date

**Nom : **date
**Type : **chaine de caractères (char *)
**Description : **Chaine contenant l'horodatage formaté

**Nom : **slave_addr
**Type : ** entier (int)
**Description : **Adresse i2c de l'esclave (Middle Board)


