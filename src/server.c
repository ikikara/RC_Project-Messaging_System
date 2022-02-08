#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>

#define BUFLEN 512	// Tamanho do buffer
#define MAXCHAR 100

typedef struct registo{
	char user_id[MAXCHAR];
	char IP[16];
	char password[MAXCHAR];
	char CS[4];
	char P2P[4];
	char Group[4];
	char status[4];
	struct sockaddr_in SOCKET;
}registo;

void erro(char *msg);
void ctrlcHandler(int sig);
void esvazieitingsArray(int* array);
int checkEmptyArray(int* array);
int verify_atoi(char *num);

int isValidIpAddress(char *ipAddress);
int count_lines(char* filename);
void readfile(char* filename, registo* array );
void deleteline(char* filename, int pos, int nrRegs);
int busca_posReg(char* user, registo* array, int nrRegs);
int verify_reg(char* user, char* pass, char* IP, registo* array, int nrRegs);
void sendMenu(int pos, registo* array, struct sockaddr_in si_outra, socklen_t slen);

int s;
char permissions[3][18] = {". Client-Server\n", ". Peer-2-Peer\n", ". Group\n"};

int fd;
int principal;

void ctrlchandler(int sig){
	if(principal==getpid()){
		printf("\nTerminating server..\n");
		close(s);
		exit(0);
	}
	else{
		close(fd);
		wait(NULL);
		exit(0);
	}
}

int main(int argc, char *argv[]) {
	principal=getpid();
	signal(SIGINT, ctrlchandler);

	int nrRegs;
	registo *Registos;

	nrRegs=count_lines(argv[3]);
	Registos = malloc(sizeof(registo)*nrRegs);
	readfile(argv[3], Registos);
	
	/*for(int i=0; i<nrRegs; i++){
			printf("%s | %s | %s | %s | %s | %s\n", Registos[i].user_id, Registos[i].password, Registos[i].IP, Registos[i].CS, Registos[i].P2P, Registos[i].Group);
		}*/

	if (argc != 4) {
 		printf("udp_server <clients port> <config port> <register file>\n");
 		exit(-1);
 	}
	
	if(fork()==0){
			
		char buf[BUFLEN];
		FILE * file;
		
		int option=1;
		int nread;
		char * aux;
		char commands[7][50];
		char auxbuf[BUFLEN];
		int pos;
	
		struct sockaddr_in addr, client_addr;
  		int client_addr_size;
		
  		bzero((void *) &addr, sizeof(addr));
  		addr.sin_family = AF_INET;
  	
  		addr.sin_addr.s_addr = htonl(INADDR_ANY);  
  		addr.sin_port = htons(atoi(argv[2]));
  		
		
  		if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			erro("na funcao socket");
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));   //Libertar a socket
  		if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
			erro("na funcao bind");
  		if( listen(fd, 5) < 0)
			erro("na funcao listen");
  		client_addr_size = sizeof(client_addr);
  		
  		
  		char toAdmin[BUFLEN]="List of clients:\n";
  		char linha[MAXCHAR*4];
  		int count;
  		
  		while(1){
  			while(waitpid(-1,NULL,WNOHANG)>0);
  			int client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
  			if(client>0){
  				if(fork()==0){
  					while(1){
  						count=0;
  						if((nread = read(client, buf, BUFLEN))>0){
  							buf[strlen(buf)-1]='\0';
  							strcpy(auxbuf, buf);
  							aux=strtok(buf," ");
     						while(aux!=NULL ){
        						strcpy(commands[count++],aux);
        						aux=strtok(NULL," ");
    						} 
  							
  							
  							if(strcmp(auxbuf,"LIST")==0){
  								char toAdmin[BUFLEN]="List of clients:\n";
  								for(int i=0; i<nrRegs; i++){
  									sprintf(linha," - %s %s %s %s %s %s\n", Registos[i].user_id, Registos[i].IP, Registos[i].password, Registos[i].CS, Registos[i].P2P, Registos[i].Group);
  									strcat(toAdmin,linha);
  								}
  								write(client, toAdmin, BUFLEN);
  							}
  							else if(strcmp(commands[0],"ADD")==0 && count==7){
  								if(isValidIpAddress(commands[2])==1 && (strcmp(commands[4],"yes")==0 || strcmp(commands[4],"no")==0) && (strcmp(commands[5],"yes")==0 || strcmp(commands[5],"no")==0) && (strcmp(commands[6],"yes")==0 || strcmp(commands[6],"no")==0)){
  									file=fopen(argv[3],"a");
  									sprintf(linha,"%s %s %s %s %s %s\n", commands[1], commands[2], commands[3], commands[4], commands[5], commands[6]);
  									fputs(linha, file);
  									fclose(file);
  									write(client, "Register added!\n", BUFLEN);
  									memset(linha, 0, sizeof(linha));
  								}
  								else{
  									write(client, "Invalid register\n", BUFLEN);
  								}
  							}
  							else if(strcmp(commands[0],"DEL")==0 && count==2){
  								if((pos=busca_posReg(commands[1], Registos, nrRegs))!=-1){
  									deleteline(argv[3],pos,nrRegs);
  									write(client, "User deleted!\n", BUFLEN);
  								}
  								else{
  									write(client, "Error trying deleting the user!\n", BUFLEN);
  								}
  								
  							}
  							else if(strcmp(auxbuf,"QUIT")==0){
  								printf("Admin exit\n");
  								close(client);
  							}
  							else{
  								write(client, "Command inexistent\n", 19);
  							}
  								
  						}
  						memset(auxbuf, 0 , sizeof(auxbuf));
  						memset(buf, 0, sizeof(buf));
  						memset(commands, 0, sizeof(commands));
  						memset(toAdmin,0,sizeof(toAdmin));
  						fflush(stdout);
  					}
  				}
  			}
  		}
  		
	}
	else{
			
		char buf[BUFLEN];
		int recv_len, send_len;
		struct sockaddr_in si_minha, si_outra, si_fora;
		socklen_t slen = sizeof(si_outra);
		socklen_t slen2 = sizeof(si_fora);
		
		int autent=0;
		int count=0;
	
		bzero((void *) &si_minha, sizeof(si_minha));
		
		// Preenchimento da socket address structure
		si_minha.sin_family = AF_INET;
		si_minha.sin_port = htons(atoi(argv[1]));
		si_minha.sin_addr.s_addr = htonl(INADDR_ANY);
		
		// Cria um socket para recepção de pacotes UDP
		if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
			erro("Erro na criação do socket");
		}
	
		// Associa o socket à informação de endereço
		if(bind(s,(struct sockaddr*)&si_minha, sizeof(si_minha)) == -1) {
			erro("Erro no bind");
		}

	
		printf("Server starting...\n");
	
		char command[6][MAXCHAR];
	
		char* aux;
		int pos;
		char toSend[BUFLEN];
		char auxbuf[BUFLEN];
	
	
		while(1){
			count=0;
			if((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_outra, (socklen_t *)&slen)) == -1) {
	  			erro("Erro no recvfrom");
			}
			
			buf[recv_len-1] = '\0';
			//printf("%s\n", buf);
			//printf("%s|\n", buf);
			
			strcpy(auxbuf,buf);
				//printf("Connection from %s:%d\n", inet_ntoa(si_outra.sin_addr), ntohs(si_outra.sin_port));
				aux=strtok(buf," ");
     			while(aux!=NULL ){
        			strcpy(command[count++],aux);
        			aux=strtok(NULL," ");
    			} 
				
				if(strcmp(command[0],"AUTENT")==0){
    				autent=verify_reg(command[1], command[2], inet_ntoa(si_outra.sin_addr), Registos, nrRegs);
    				if(autent==2){
    					pos=busca_posReg(command[1], Registos, nrRegs);			
    				
						if((send_len=sendto(s, "Server | welcome to our message server", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
							erro("Erro a enviar a mensagem para o servidor");
						}	
						
						printf("%s joined the server\n", command[1]);	
						strcpy(Registos[pos].status,"on");
						Registos[pos].SOCKET.sin_port=si_outra.sin_port;
						
						sendMenu(pos,Registos, si_outra,slen);
					}
					else{
						if(autent==1){
							if((send_len=sendto(s, "Server | User correct but wrong password", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
								erro("Erro a enviar a mensagem para o servidor");
							}
						}
						else if(autent==0){
							if((send_len=sendto(s, "Server | User not found", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
								erro("Erro a enviar a mensagem para o servidor");
							}
						}
					
					}	
    			}
    			else if(strcmp(command[0],"P2PComplete")==0){
    				sendMenu(busca_posReg(command[1], Registos, nrRegs),Registos, si_outra,slen);
    			}
    			else if(strcmp(command[0],"MULTIComplete")==0){
    				sendMenu(busca_posReg(command[1], Registos, nrRegs),Registos, si_outra,slen);
    			}
    			else if(strcmp(command[0],"EXIT")==0){
    				pos=busca_posReg(command[1], Registos, nrRegs);	
    				strcpy(Registos[pos].status,"off");
    				printf("%s left the server\n", Registos[pos].user_id);
    			}
    			
    			else{
    				pos=busca_posReg(command[0], Registos, nrRegs);	
    				
    				int pos2;
    				if(strcmp(command[1],"1")==0){
    					if((pos2=busca_posReg(command[2], Registos, nrRegs))!=-1){
    						if(strcmp(Registos[pos2].status,"on")==0){
    							if(strcmp(command[0],command[2])!=0){
    								si_fora.sin_family = AF_INET;
    								si_fora.sin_port=Registos[pos2].SOCKET.sin_port;
    								si_fora.sin_addr.s_addr=inet_addr(Registos[pos2].IP);
    								sprintf(toSend,"Message from %s: %s | (Continua a fazer o que estavas a fazer)\n", command[0], command[3]);
    								
    								if((send_len=sendto(s, toSend, BUFLEN,0,(struct sockaddr *) &si_fora, (socklen_t)slen2) == -1)){
										erro("Erro a enviar a mensagem para o servidor");
									}
									
									if((send_len=sendto(s, "Server | Message has been sent!", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
										erro("Erro a enviar a mensagem para o servidor");
									}	
								}
								else{
									if((send_len=sendto(s, "Server | You can't send a message to you xD", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
										erro("Erro a enviar a mensagem para o servidor");
									}	
								}
							}
							else{
								if((send_len=sendto(s, "Server | This user is not online at the moment", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
									erro("Erro a enviar a mensagem para o servidor");
								}	
							}
						}
						else if(strcmp(Registos[pos].CS,"no")==0){
							if((send_len=sendto(s, "Server | You don't have permission to use this communication", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
								erro("Erro a enviar a mensagem para o servidor");
							}
						}
						else{
							if((send_len=sendto(s, "Server | User not found", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
								erro("Erro a enviar a mensagem para o servidor");
							}
							
						}
						sendMenu(pos,Registos, si_outra,slen);
    				}
    				else if(strcmp(command[1],"2")==0){
    					if((pos2=busca_posReg(command[2], Registos, nrRegs))!=-1){
    						if(strcmp(Registos[pos2].status,"on")==0){
    							if(strcmp(command[0],command[2])!=0){	
    								sprintf(toSend,"P2P %s %d", Registos[pos2].IP, ntohs(Registos[pos2].SOCKET.sin_port));	
									
									if((send_len=sendto(s, toSend, BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
										erro("Erro a enviar a mensagem para o servidor");
									}	
								}
								else{
									if((send_len=sendto(s, "Server | You can't send a message to you xD", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
										erro("Erro a enviar a mensagem para o servidor");
									}	
									sendMenu(pos,Registos, si_outra,slen);
								}
							}
							else{
								if((send_len=sendto(s, "Server | This user is not online at the moment", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
									erro("Erro a enviar a mensagem para o servidor");
								}	
								sendMenu(pos,Registos, si_outra,slen);
							}
						}
						else if(strcmp(Registos[pos].CS,"no")==0){
							if((send_len=sendto(s, "Server | You don't have permission to use this communication", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
								erro("Erro a enviar a mensagem para o servidor");
							}
							sendMenu(pos,Registos, si_outra,slen);
						}
		
						else{
							if((send_len=sendto(s, "Server | User not found", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
								erro("Erro a enviar a mensagem para o servidor");
							}
							sendMenu(pos,Registos, si_outra,slen);
						}
						
    				}
    				else if(strcmp(command[1],"3")==0){
    					if(strcmp(Registos[pos].Group,"yes")==0){
    						if((send_len=sendto(s, "MC 224.0.0.2 4003", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
								erro("Erro a enviar a mensagem para o servidor");
							}
    						
    					}
    					else{
    						if((send_len=sendto(s, "Server | You don't have permission to use this communication", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
								erro("Erro a enviar a mensagem para o servidor");
							}
    					}
    				}
    				else{
    					if((send_len=sendto(s, "Type of communication inexistent", BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1)){
							erro("Erro a enviar a mensagem para o servidor");
						}
						sendMenu(pos,Registos, si_outra,slen);
    				}
	
    				
    			}
    			
    			memset(buf, 0, sizeof(buf));
				memset(toSend, 0, sizeof(toSend));;
				memset(command, 0, sizeof(command));
				
				fflush(stdout);
			
		}	
    }
    
	// Fecha socket e termina programa
	close(s);
	return 0;
}



int count_lines(char* filename){
	FILE * f;
	int count=0;
	char c;
	f = fopen(filename, "r");
	
	if(f==NULL){
		printf("Could not open file %s", filename);
        return 0;
    }
    
    for(c=getc(f); c!=EOF;c=getc(f)){
    	if(c=='\n'){
    		count+=1;
    	}
    }
    
    fclose(f);
	return count;
}

void readfile(char* filename, registo* array ){
	FILE* file;
    char str[MAXCHAR];
    char* a;
    int linha = 0;

	file=fopen(filename,"r");
    if(file==NULL){
		printf(" COULDN'T OPEN THE FILE: %s\n", filename);
        exit(1);
    }

	while (fgets(str, MAXCHAR, file) != NULL) {
		char* pos;
		if ((pos=strchr(str, '\n')) != NULL)
            *pos = '\0';
	
        a = strtok( str, " " );
        strcpy(array[linha].user_id,a);      
        a = strtok( NULL, " ");
        if(isValidIpAddress(a)==1){;
			strcpy(array[linha].IP,a);
		}
		else{
			printf("Erros nos IPs dos registos\n");
			raise(SIGINT);
		}
		a = strtok( NULL, " ");
		strcpy(array[linha].password,a);
		a = strtok( NULL, " ");
		if(strcmp(a,"yes")==0 || strcmp(a,"no")==0){
			strcpy(array[linha].CS,a);
		}
		else{
			printf("Erros nas permissões dos registos\n");
			raise(SIGINT);
		}
		a = strtok( NULL, " ");
		if(strcmp(a,"yes")==0 || strcmp(a,"no")==0){
			strcpy(array[linha].P2P,a);
		}
		else{
			printf("Erros nas permissões dos registos\n");
			raise(SIGINT);
		}
		a = strtok( NULL, " ");
		if(strcmp(a,"yes")==0 || strcmp(a,"no")==0){
			strcpy(array[linha].Group,a);
		}
		else{
			printf("Erros nas permissões dos registos\n");
			raise(SIGINT);
		}
		
		linha++;
   	}
        
    fclose(file);	
}

int isValidIpAddress(char *ipAddress){
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    if( result == 1){
    	return 1;
    }
    return 0;
}

void deleteline(char* filename, int pos, int nrRegs){
	FILE* file;
    char str[MAXCHAR];
    
    int linha=0;
    file=fopen(filename,"r");
    if(file==NULL){
		printf(" COULDN'T OPEN THE FILE: %s\n", filename);
        exit(1);
    }
    char linhas [nrRegs][MAXCHAR];
    
    int count=0;
    while (fgets(str, MAXCHAR, file) != NULL) {
    	if(linha!=pos){
    		strcpy(linhas[count++],str);
    	}
    	linha++;
    }
    fclose(file);
    
    file=fopen(filename,"w");
    if(file==NULL){
		printf(" COULDN'T OPEN THE FILE: %s\n", filename);
        exit(1);
    }
    
    for(int i=0; i<count; i++){
    	fputs(linhas[i], file);
    }
    fclose(file);
    
}

int busca_posReg(char* user, registo* array, int nrRegs){
	for(int i=0; i<nrRegs; i++){
		if(strcmp(array[i].user_id, user)==0){
			return i;
		}
	}
	return -1;
}

int verify_reg(char* user, char* pass, char* IP, registo* array, int nrRegs){
	int pos;
	int rt=0;   // 0 -> Não encontra User  / 1 -> Encontra User / 2 -> Encontra User e Pass

	for(int i=0; i<nrRegs; i++){
		//printf("%s|%s\n", array[i].IP, IP);
		if(strcmp(array[i].user_id, user)==0 && strcmp(array[i].IP, IP)==0){
			rt=1;
			pos=i;
		}
	}
	
	if(rt==1){
		if(strcmp(array[pos].password, pass)==0){
			return 2;
		}
		else{
			return 1;
		}
	}
	
	return 0;
}

void sendMenu(int pos, registo* array, struct sockaddr_in si_outra, socklen_t slen){
	int index=1;
	char num[5], toSend[BUFLEN];
	memset(toSend, 0, sizeof(toSend));
	
	strcat(toSend,"\nChoose the type of communication:\n");
	
	if(strcmp(array[pos].CS,"yes")==0){
		sprintf(num," %d", index++);
		strcat(toSend,num);
		strcat(toSend,permissions[0]);
	}
	if(strcmp(array[pos].P2P,"yes")==0){
		sprintf(num," %d", index++);
		strcat(toSend,num);
		strcat(toSend,permissions[1]);
	}
	if(strcmp(array[pos].Group,"yes")==0){
		sprintf(num," %d", index);
		strcat(toSend,num);
		strcat(toSend,permissions[2]);
	}
					
	if(sendto(s, toSend, BUFLEN,0,(struct sockaddr *) &si_outra, (socklen_t)slen) == -1){
		erro("Erro a enviar a mensagem para o servidor");
	}
	//printf("%s\n", toSend);
	
	memset(toSend, 0, sizeof(toSend));
}

void erro(char *msg){
	printf("Error: %s\n", msg);
	exit(-1);
}
