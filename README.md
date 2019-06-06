# rpi_log

## TO-DO List

### Programme multi-capteurs
- [ ] Valider l'envoi par les différents port UART sur la board du milieu
- [ ] Rajouter des FIFO
- [ ] Envoyer une ligne par capteur
- [ ] *Optionnel* Envoyer le port UART en plus
- [ ] Rajouter des structures pour les différents capteurs
- [ ] Rajouter un tableau pour les adresses i2c des Middle Boards
- [ ] *Optionnel* Mettre en place la récupération de l'association des DevEui aux Middle Boards et aux ports UART 

## Programme de récupération de log pour un capteur

Cette version est la première version du programme de récupération de log. Elle sera la base afin de développer la version multi-capteurs

### Algorithme du programme

![alt text](https://github.com/yocvito/rpi_log/blob/master/Algo_oneDevice.png)

### Dictionnaire des données

__Nom :__ file_i2c  
__Type :__ entier (int)  
__Description :__ Descripteur de fichier du bus i2c  
  
__Nom :__ filename_i2c  
__Type :__ chaine de caractères (char *)  
__Description :__ Label du chemin vers le bus i2c linux  
  
__Nom :__ file_csv  
__Type :__ descripteur de fichier (FILE *)  
__Description :__ Descripteur de fichier du fichier de log du capteur  
  
__Nom :__ filename_csv  
__Type :__ chaine de caractères (char *)  
__Description :__ Label du fichier de log du capteur  
  
__Nom :__ rxBuffer  
__Type :__ caractère (char)  
__Description :__ Buffer de reception  
  
__Nom :__ rxStrbuffer  
__Type :__ chaine de caractères (char *)  
__Description :__ chaine constituant le message de log  
  
__Nom :__ devInfo  
__Type :__ information de capteur (deviceInfo)  
__Description :__ Structure d'information du capteur  
  
__Nom :__ t  
__Type :__ temps (time_t)  
__Description :__ Variable de récupération de date  
  
__Nom :__ date  
__Type :__ chaine de caractères (char *)  
__Description :__ Chaine contenant l'horodatage formaté  
  
__Nom :__ slave_addr  
__Type :__  entier (int)  
__Description :__ Adresse i2c de l'esclave (Middle Board)  


