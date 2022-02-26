# RC_Project-Messaging_System

- [x] Finished

## Index
- [Description](#description)
- [Technologies used](#technologies-used)
- [To run this project](#to-run-this-project)
- [Notes important to read](#notes-important-to-read)
- [Authors](#authors)

## Description
This project was developed for Communication Networks subject @University of Coimbra, Informatics Engineering <br>
Consists in develop a program that implements a Messaging System using UNIX specification and network elements like sockets and the protocols TCP and UDP.
#### Main Languages:
![](https://img.shields.io/badge/-C-333333?style=flat&logo=C%2B%2B&logoColor=5459E2) 

## Technologies used:
1. C ([documentation](https://devdocs.io/c/))
2. [GNS3](https://www.gns3.com/software/download)

## To run this project:
You have one way to run this project:
1. Using GNS3 and containers inside:
    * We will use some materials on GNS3, 5 containers (with Linux in this case), 3 routers and 3 switches
    * Replicate the image above
      ![image](https://i.imgur.com/f0r67j7.png)
    * Download the folder "src" and file Registos.txt that is on "resources" folder
    * On container called "Server" compile and run the file server.c
      ```shellscript
      [your-disk]:[name-path]> gcc server.c -o server
      ```
      ```shellscript 
      [your-disk]:[name-path]> ./server 80 160 Registos.txt
      ```
    * On others containers compile and run the file client.c
      ```shellscript
      [your-disk]:[name-path]> gcc server.c -o server
      ```
      ```shellscript 
      [your-disk]:[name-path]> ./server 80 160 Registos.txt
      ```
    * Finally use the program as you want!

## Notes important to read:
- The containers used were provided with the image of Ubuntu VM that I used to do this project. Because this was provided by my teacher/course I can't make it available, so try to create a container on gns3 that use Linux (don't look at me I don't know how to do it xd)
- The routers were Cisco 2600 and again the image for these routers were provided by my teacher/course, so again I can't make it available, try to find one :'))
- File "Registos.txt" must be on same folder as "server.c" on "Server" container
- For more information about how to use this project consult the Statement (to learn more about commands, signals available and structure of "Config.txt")

## Authors:
- [João Silva](https://github.com/ikikara)
- [Pedro Martins](https://github.com/PedroMartinsUC)
