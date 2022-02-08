#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define BUFLEN 2000
#define MAXCHAR 100

typedef struct{
	int work;
	struct sockaddr_in SOCKET;
	socklen_t slen;
	struct sockaddr_in GROUP;
	socklen_t slenG;
	int mult;
}shm;

int shmid;             
shm * shared_var;

char buffer[BUFLEN];
char auxbuffer[BUFLEN];
pid_t principal;
int send_len;
struct sockaddr_in addr;
socklen_t len=sizeof(addr);
int fd;
char user[20];

void erro(char *msg);
void readFd(int fd);
void ctrlchandler(int sig);


int main(int argc, char *argv[]) {
	principal=getpid();
	signal(SIGINT, ctrlchandler);

  	struct sockaddr_in addr;
  	struct hostent *hostPtr;
  	
  	char command[MAXCHAR];
  	

 	char pass[20];
 	char user_pass[BUFLEN];
 	char message[(int)(0.5*BUFLEN)];	
 
	char endServer[MAXCHAR];

	
  	if (argc != 3) {
    	printf("cliente <host> <port>\n");
    	exit(-1);
  	}
	
	
  	strcpy(endServer, argv[1]);
  	if ((hostPtr = gethostbyname(endServer)) == 0)
    	erro("Não consegui obter endereço");
	
  	bzero((void *) &addr, sizeof(addr));
  	addr.sin_family = AF_INET;
  	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
  	addr.sin_port = htons((short) atoi(argv[2]));
	
	if(strcmp(argv[2],"160")==0) {
		int nread;
	
		if ((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	  		erro("socket");
	
  		if (connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
	  		erro("Connect");
	  	
	  	while(1){
	  		printf("Commands:\n -> LIST\n -> ADD <User-id>  <IP> <Password>  <Cliente-Servidor>  <P2P>  <Grupo>\n -> DEL <User-id>\n -> QUIT\n");
	  		printf("Write the command here: ");
	  		fgets(command, MAXCHAR , stdin);
	  		
	  		
	  		
	  		if(strcmp(command,"QUIT\n")==0){
	  			printf("Terminating session..\n");
	  			write(fd, command, strlen(command));
	  			close(fd);
  				exit(0);
  			}
  			else{
  				write(fd, command, strlen(command));
	  			if((nread = read(fd, buffer, BUFLEN))>0){
  					buffer[nread]='\0';
  					printf("\n%s\n", buffer);	
  				}
  			}
  			
  			
	  		
	  		memset(buffer,0,sizeof(buffer));
	  		fflush(stdout);
	  	}
	}
	else if(strcmp(argv[2],"80")==0){
 		
 		if((shmid=shmget(IPC_PRIVATE, sizeof(shm) ,IPC_CREAT | 0766))<0){
			perror("Error in shmget with IPC_GREAT\n");
			exit(1);
		}
		
		if((shared_var = (shm*) shmat(shmid,NULL,0))==(shm*)-1){
			perror("Shmat error!");
			exit(1);
		}
		
		shared_var->work=0;
	
 		strcpy(endServer, argv[1]);
 		if ((hostPtr = gethostbyname(endServer)) == 0)
 			erro("Não consegui obter endereço");
	
 		
	
 		if((fd = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP)) == -1)
			erro("socket");
		
		
		printf("Welcome to the program, dear Client u.u!\nStart your session please:\n");
		
		do{	
			printf("Username: ");
			fgets(user, 20 , stdin);
			user[strlen(user)-1]='\0';
			printf("Password: ");
			fgets(pass, 20 , stdin);
			pass[strlen(pass)-1]='\0';
			sprintf(user_pass, "AUTENT %s %s", user, pass);
	
			if((send_len=sendto(fd, user_pass, 1+ strlen(user_pass),0,(struct sockaddr *) &addr, (socklen_t)len) == -1)){
				erro("sending the information to the server");
			}
				
			readFd(fd);
			printf("\n%s\n",buffer);
			
		}while(strcmp(buffer,"Server | welcome to our message server")!=0);
	
		pid_t anothermessCS;
		char userid[MAXCHAR];
 		char toSend[BUFLEN];
 		char P2P [3][MAXCHAR];
		char MC [2][MAXCHAR];
		
		
		if((anothermessCS=fork())==0){
			while(1){
				int count=0;
				char *aux;
				memset(buffer,0,sizeof(buffer));
				memset(auxbuffer, 0, sizeof(auxbuffer));
				readFd(fd);
				strcpy(auxbuffer,buffer);
				
				aux=strtok(buffer," ");
     			while(aux!=NULL || count<3){
     				strcpy(MC[count],aux);
        			strcpy(P2P[count],aux);
        			aux=strtok(NULL," ");
        			count++;
    			} 
    			if(strcmp(P2P[0],"P2P")==0){
    				shared_var->work=1;
    				shared_var->SOCKET.sin_family = AF_INET;
    				shared_var->SOCKET.sin_port=htons(atoi(P2P[2]));
    				shared_var->SOCKET.sin_addr.s_addr=inet_addr(P2P[1]);
    				shared_var->slen=sizeof(shared_var->SOCKET);
    			}
    			else if(strcmp(MC[0],"MC")==0){
    				shared_var->work=2;
    				shared_var->GROUP.sin_family = AF_INET;
    				shared_var->GROUP.sin_port=htons(atoi(MC[2]));
    				shared_var->GROUP.sin_addr.s_addr=inet_addr(MC[1]);
    				shared_var->slenG=sizeof(shared_var->GROUP);
    			}
				else{		
					printf("%s\n", auxbuffer);
				}
					
				
			}
		}
		else{
			while(1){
     			fgets(command, MAXCHAR , stdin);
     			command[strlen(command)-1] = '\0';	
     			
     			if(strcmp(command,"1")==0){
    				printf("Used_id: ");
					fgets(userid, MAXCHAR , stdin);
					userid[strlen(userid)-1]='\0';
					printf("Message: ");
					fgets(message, MAXCHAR , stdin);
					message[strlen(message)-1]='\0';
					
					sprintf(toSend,"%s %s %s %s",user, command, userid, message);
					
					if((send_len=sendto(fd, toSend, BUFLEN,0,(struct sockaddr *) &addr, (socklen_t)len) == -1)){
						erro("sending the information to the server");
					}
    			}
    			else if(strcmp(command,"2")==0){
    				printf("User_id: ");
					fgets(userid, MAXCHAR , stdin);
					userid[strlen(userid)-1]='\0';
    				sprintf(message,"%s %s %s", user, command, userid);
    				
    				if((send_len=sendto(fd, message, BUFLEN,0,(struct sockaddr *) &addr, (socklen_t)len) == -1)){
						erro("sending the information to the server");
					}
    				
    				sleep(1);
    				if(shared_var->work==1){	
    					printf("Message: ");
						fgets(message, MAXCHAR , stdin);
						message[strlen(message)-1]='\0';
						sprintf(toSend,"Message from %s: %s  | (Continua a fazer o que estavas a fazer)\n", user, message);
					
						if((send_len=sendto(fd, toSend, BUFLEN,0,(struct sockaddr *) &shared_var->SOCKET, (socklen_t)shared_var->slen) == -1)){
							erro("sending the information to the server");
						}
						
						
						sprintf(toSend,"P2PComplete %s", user);
						if((send_len=sendto(fd, toSend, BUFLEN,0,(struct sockaddr *) &addr, (socklen_t)len) == -1)){
							erro("sending the information to the server");
						}
					}
					shared_var->work=0;
    			}	
    			else if(strcmp(command,"3")==0){
    				sprintf(toSend,"%s %s", user, command);
    				if((send_len=sendto(fd, toSend, BUFLEN,0,(struct sockaddr *) &addr, (socklen_t)len) == -1)){
						erro("sending the information to the server");
					}
					int multicastTTL = 255;
					
					sleep(1);
					if(shared_var->work==2){
						int recv_len;
						char multicast[MAXCHAR];
						
						printf("Connection from %s:%d\n", inet_ntoa(shared_var->GROUP.sin_addr), ntohs(shared_var->GROUP.sin_port));
						if((shared_var->mult = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP)) == -1)
						erro("socket");
					
						if (setsockopt(shared_var->mult, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &multicastTTL,sizeof(multicastTTL)) < 0) {
    						perror("socket opt");
    						return 1;
						}
						
						if(fork()==0){
							int fd = socket(AF_INET, SOCK_DGRAM, 0);
    						if (fd < 0) {
        						perror("socket");
        						return 1;
    						}
						
    						// allow multiple sockets to use the same PORT number
    						//
    						u_int yes = 1;
    						if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0){
       							perror("Reusing ADDR failed");
       							return 1;
    						}
						
        						// set up destination address
    						//
    						struct sockaddr_in addr;
    						memset(&addr, 0, sizeof(addr));
    						addr.sin_family = AF_INET;
    						addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    						addr.sin_port = htons(ntohs(shared_var->GROUP.sin_port));
						
    						// bind to receive address
    						//
    						if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        						perror("bind");
        						return 1;
    						}
						
    						// use setsockopt() to request that the kernel join a multicast group
    						//
    						struct ip_mreq mreq;
    						mreq.imr_multiaddr.s_addr = inet_addr(inet_ntoa(shared_var->GROUP.sin_addr));
    						mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    						if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0){
        						perror("setsockopt");
        						return 1;
        					}
							
							
							while(1){
								socklen_t slenG = sizeof(addr);
								
								memset(buffer,0,sizeof(buffer));
								if((recv_len=recvfrom(fd, multicast, MAXCHAR, 0, (struct sockaddr *) &addr,  (socklen_t*)&slenG)) && (recv_len<0)){
									erro("Erro a receber a mensagem do servidor");
								}
								
								printf("\n%s | (continua o que estavas a fazer)\n", multicast);
						
							}
						}
						else{
							int fd = socket(AF_INET, SOCK_DGRAM, 0);
    						if (fd < 0) {
        						perror("socket");
        						return 1;
    						}
						
    						// set up destination address
    						//
    						struct sockaddr_in addr;
    						memset(&addr, 0, sizeof(addr));
    						addr.sin_family = AF_INET;
    						addr.sin_addr.s_addr = inet_addr(inet_ntoa(shared_var->GROUP.sin_addr));
    						addr.sin_port = htons(ntohs(shared_var->GROUP.sin_port));
						
							while(strcmp(message,"exit")!=0){
								sleep(1);
								printf("Message (write exit if you want to leave the group): ");
								fgets(message, MAXCHAR , stdin);
								message[strlen(message)-1]='\0';
								sprintf(toSend,"%s says: %s", user, message);
								
								if((send_len=sendto(fd, toSend, BUFLEN,0,(struct sockaddr *) &addr, sizeof(addr)) == -1)){
									erro("sending the information to the server");
								}
							}
						}
						
					}
					sprintf(toSend,"MULTIComplete %s", user);
					if((send_len=sendto(fd, toSend, BUFLEN,0,(struct sockaddr *) &addr, (socklen_t)len) == -1)){
						erro("sending the information to the server");
					}
					shared_var->work=0;
    			}
    			else{
    				sprintf(toSend,"%s %s", user, command);
    				if((send_len=sendto(fd, toSend, BUFLEN,0,(struct sockaddr *) &addr, (socklen_t)len) == -1)){
						erro("sending the information to the server");
					}
    			}
  			}
  		}
  	}
  	else{
  		printf("Port %s is not configure for any protocol, programm finishing..\n", argv[2]);
  		exit(1);
  	}
}

void readFd(int fd){
	int recv_len;

	memset(buffer,0,sizeof(buffer));
	if((recv_len=recvfrom(fd,buffer,BUFLEN,0,(struct sockaddr *)&addr, (socklen_t*)&len) == -1) && (recv_len<0)){
		erro("Erro a receber a mensagem do servidor");
	}
	
	//buffer[recv_len-1] = '\0';
	
	fflush(stdout);
}

void ctrlchandler(int sig){
	if(principal==getpid()){
		sprintf(buffer,"EXIT %s",user);
		if((send_len=sendto(fd, buffer, BUFLEN,0,(struct sockaddr *) &addr, (socklen_t)len) == -1)){
			erro("sending the information to the server");
		}
		shmdt(shared_var);
		shmctl(shmid, IPC_RMID, NULL);
		printf("\n");
		close(fd);
		exit(0);
	}
	else{
		wait(NULL);
		exit(0);
	}
}

void erro(char *msg) {
	printf("Error: %s\n", msg);
	exit(-1);
}
