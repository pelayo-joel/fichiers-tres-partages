# Fichiers-tres-partages

## Compiler et exécuter le programme

#### Lignes de commandes pour compiler le programme
##### Lignes de commandes via g++
```shell
g++ -o client app/main_client.cpp classes/src/LPTF_Packet.cpp classes/src/LPTF_Socket.cpp classes/src/client.cpp
g++ -o server app/main_server.cpp classes/src/LPTF_Packet.cpp classes/src/LPTF_Socket.cpp classes/src/server.cpp
```
##### Lignes de commandes via le makefile
```shell
make c
make s
```
La commande `make` permet de faire `make c` (client) et `make s` (serveur) en simultané.
*NB : Après avoir compilé le programme via les lignes ci-dessus, deux exécutables seront générés.*

#### Lignes de commandes pour exécuter le programme
> Les deux exécutables se génèrent à la racine du projet.
```shell
./lpf_server
./lpf <username@ftp_serverIP:port> <-download|-upload|-delete|-list|-create|-rm|-rename> <arg1> <arg2>
```

## Structure du projet
> Cette structure est basée sur un accord commun de notre groupe. Elle respecte une certaine convention de nommage avec le dossier `include` et le dossier `src`.

- Dossier `application` : contient les fichiers de lancement pour le `client` et le `serveur`.
- Dossier `classes` : contient les dossiers `include` et `src`.
- Dossier `include` de `classes` : contient tous les fichiers d'en-tête nommés `headers`. Ces fichiers ont généralement l'extension `.h` ou `.hpp` et définissent les interfaces, les structures de données, et les prototypes de fonctions utilisés à travers le projet.
- Dossier `src` de `classes` : contient tous les fichiers source de l'application. Ces fichiers ont l'extension `.cpp` et contiennent les implémentations des fonctions et des classes définies dans les fichiers d'en-tête du dossier `include`.
- Dossier `clientFiles` : Simule l'arborescence d'un client, l'utilisateur peut techniquement choisir un fichier en dehors du projet.
- Dossier `data` : Correspond à l'arborescence du serveur FTP, les clients enregistrés dans le fichier `very_safe_trust_me_bro.txt` possèdent tous leurs dossier racine.
- Dossier `logs` : Dossier destination des logs du serveur lors de son arrêt.

```
Fichiers-tres-partages
├── application/
|   ├── launch_client.cpp
|   └── launch_server.cpp
|
├── classes/
|   ├── include/
|   |   ├── client.hpp
|   |   ├── server.hpp
|   |   ├── logger.hpp
|   |   ├── FTP_Packet.hpp
|   |   └── FTP_Socket.hpp
|   |
|   └── src/
|       ├── client.cpp
|       ├── server.cpp
|       ├── logger.cpp
|       ├── FTP_Packet.cpp
|       └── FTP_Socket.cpp
├── clientFiles/ (Arborescence du client)
├── data/ (Dossier racine du serveur)
└── logs/
    └── logs_server.txt
```