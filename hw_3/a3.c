#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>


#define IWRITE "RESP_PIPE_47634"
#define IREAD "REQ_PIPE_47634"
#define SHM_KEY 17718


//b trebuie sa fie null terminated, a stringul din pipe
int equals(char *a, char *b){
	for(int i=0; i<strlen(b); i++){
		if(a[i]!=b[i]){
			return 0;
		}
	}
	return 1;
}

 int wrString(char* string, char len, int wrThing){
 	int result = 0;
 	result += write(wrThing,&len,sizeof(char));
	result += write(wrThing,string,(int)len);

	return result;
}

int main(){
	char conString[] = "CONNECT";
	int respFD=-1, reqFD=-1;
	char conSize = sizeof(conString)-1;
	//2.2

	if (mkfifo(IWRITE,0600)!= 0){
		printf("ERROR\ncannot create the response pipe | cannot open the request pipe\n");
	}else{
		reqFD = open(IREAD,O_RDONLY);
		respFD = open(IWRITE,O_WRONLY);
		if(reqFD == -1 && respFD == -1){
			printf("ERROR\ncannot create the response pipe | cannot open the request pipe\n");
		}else{

			if((write(respFD,&conSize,sizeof(char)) + write(respFD,conString,(int)conSize)) != sizeof(conString)){
				printf("ERROR\ncannot create the response pipe | cannot open the request pipe\n");
				close(reqFD);
				close(respFD);
			}else{
				printf("SUCCESS\n");
			}
		}
	}

	// used 
	int shId=0;
	char *memArea=NULL;
	char *mapFile=NULL;
	shmdt(memArea);
	char *fileName=NULL;
	int fd = 0;
	struct stat paralelLol;
	unsigned int fileSize;

	unsigned int area_size=0;
	char err[] = "ERROR";
	char succ[] = "SUCCESS";
	for(;;){

		char len = 0;
		char *comm = NULL;
		
		
		//unsigned int noParam = 0;
		//unsigned int* params;

		read(reqFD, &len, sizeof(len));//citesc size-ul requestului
		if(len == 0){
			break;
		}else{
			comm = (char*)calloc((int)len,sizeof(char));//aoloc memorie pt buffer
			if(comm == NULL){
				perror("Eroare la alocare");
				exit(1);
			}
			read(reqFD, comm, (int)len);//citesc comanda
			//read(reqFD, &noParam, sizeof(unsigned int));//citesc numarul de parametrii
			if(equals(comm,"PING")){
				char pong[] = "PONG";
				unsigned int nr = 47634;
				//unsigned int respParam = 1;
				write(respFD,&len,sizeof(len));
				write(respFD,comm,(int)len);
				write(respFD,&len,sizeof(len));
				write(respFD,pong,(int)len);
				//write(respFD,&respParam, sizeof(unsigned int));
				write(respFD,&nr,sizeof(unsigned int));
			}else if(equals(comm, "CREATE_SHM")){
				
				read(reqFD, &area_size, sizeof(unsigned int));

				shId = shmget(SHM_KEY,area_size,IPC_CREAT | 0664);
				memArea = (char*)shmat(shId, NULL, 0);

				//result
				wrString(comm,len,respFD);
				if(shId == -1 || memArea == (void*)-1){
					//wr error
					wrString(err,(char)sizeof(err)-1,respFD);
				}else{
					//wr success
					wrString(succ,(char)sizeof(succ)-1,respFD);
				}


			}else if(equals(comm, "WRITE_TO_SHM")){
				unsigned int reqOffset = 0;
				unsigned int reqValue = 0;
				read(reqFD, &reqOffset, sizeof(unsigned int));
				read(reqFD, &reqValue, sizeof(unsigned int));

				//result
				wrString(comm,len,respFD);
				if(sizeof(reqValue) + reqOffset <= area_size && reqOffset >= 0){
					//try to wr
					//*(memArea + reqOffset-10) = reqValue; /// like.. whyyy ?
 					unsigned int *dummy = (unsigned int*)(memArea + reqOffset);
					*dummy = reqValue;
					//memcpy(memArea+reqOffset, &reqValue, sizeof(reqValue));
					wrString(succ,(char)sizeof(succ)-1,respFD);
				}else{
					//fail
					wrString(err,(char)sizeof(err)-1,respFD);
				}

			}else if(equals(comm, "MAP_FILE")){
				char fileNameLen;//pt numele fisierului

				read(reqFD, &fileNameLen, sizeof(char));
				fileName = (char*)calloc((int)fileNameLen + 1,sizeof(char));//aoloc memorie pt numele fisierului... oare trebuie un \0 la final ???
				if(fileName == NULL){
					perror("Eroare la alocare");
					break;
				}

				read(reqFD, fileName, (int)fileNameLen); // pt fisierul care trebuie mapat
				fileName[fileNameLen+1]='\0';///// hm.. de aici..., nu stiam ca open asteapta string null terminated, dar era cam logic
				fd = open(fileName, O_RDONLY);
				fstat(fd, &paralelLol);
				fileSize = paralelLol.st_size;


				wrString(comm,len,respFD);
				//printf("%d\n",fd);
				//printf("%s\n",fileName);
				mapFile = (char*)mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0); //param: alege so zona, cat spatiu, read, priv, file, offfse
				if(fd == -1 || mapFile == (void*)-1){
					//pipe err
					wrString(err,(char)sizeof(err)-1,respFD);
				}else{
					//pipe succ
					wrString(succ,(char)sizeof(succ)-1,respFD);
				}
			}else if(equals(comm, "READ_FROM_FILE_OFFSET")){
				unsigned int mapReadAmm = 0;
				unsigned int mapReadOffset = 0;
				read(reqFD, &mapReadOffset, sizeof(unsigned int));
				read(reqFD, &mapReadAmm, sizeof(unsigned int));

				char* mapData = (char*)calloc(mapReadAmm,sizeof(char));
				if(mapData == NULL){
					perror("Eroare la alocare");
					break;
				}

				wrString(comm,len,respFD);
				if(mapFile == (void*)-1 || mapReadAmm + mapReadOffset > fileSize || memArea == (void*)-1){
					//pipe err
					wrString(err,(char)sizeof(err)-1,respFD);
				}else{
					//add to mem and pipe succ
					printf("%d %d\n",mapReadOffset,mapReadAmm);
					for(int i=mapReadOffset;i<mapReadOffset+mapReadAmm;i++){
						mapData[i-mapReadOffset] = mapFile[i];
					}
					memcpy(memArea, mapData, mapReadAmm);

					wrString(succ,(char)sizeof(succ)-1,respFD);
				}

				free(mapData);

			}else if(equals(comm, "READ_FROM_FILE_SECTION")){
				unsigned int mapSection = 0;
				unsigned int mapReadAmm = 0;
				unsigned int mapReadOffset = 0;
				read(reqFD, &mapSection, sizeof(unsigned int));
				read(reqFD, &mapReadOffset, sizeof(unsigned int));
				read(reqFD, &mapReadAmm, sizeof(unsigned int));

				char* mapData = (char*)calloc(mapReadAmm,sizeof(char));
				if(mapData == NULL){
					perror("Eroare la alocare");
					break;
				}

				wrString(comm,len,respFD);
				if(mapFile == (void*)-1 || mapSection > (unsigned int)mapFile[8] || memArea == (void*)-1){
					//pipe err
					wrString(err,(char)sizeof(err)-1,respFD);
				}else{
					//add to mem and pipe succ
					unsigned int idxStart = 0;
					//printf("%d %d\n",mapReadOffset,mapReadAmm);
					unsigned int sectionOffset = (mapSection-1)*23 + 9 + 15;// locul in header de unde incepe pozitia sectiunii
					
					memcpy(&idxStart, mapFile + sectionOffset, sizeof(unsigned int)); // iau cei 4 octeti din fiser care reprezinta offsetul sectiunii
					idxStart += mapReadOffset; // obtin inceputul zonei dorite din sectiune

					for(int i=idxStart;i<idxStart+mapReadAmm;i++){
						mapData[i-idxStart] = mapFile[i];
					}
					memcpy(memArea, mapData, mapReadAmm);

					wrString(succ,(char)sizeof(succ)-1,respFD);
				}

				free(mapData);

			}else if (equals(comm, "EXIT")){
				free(fileName);
				close(fd);
				free(comm);
				shmdt(memArea);
				memArea = NULL;
				shmctl(SHM_KEY, IPC_RMID, 0);
				close(respFD);
				close(reqFD);
				break;
			}else {
				//2.9... nope nope nope 
				free(fileName);
				close(fd);
				free(comm);
				shmdt(memArea);
				memArea = NULL;
				shmctl(SHM_KEY, IPC_RMID, 0);
				close(respFD);
				close(reqFD);
				break;
			}
			free(fileName);
			close(fd);	
			free(comm);

		}


	}

	//2.3
	//free(comm);
	shmdt(memArea);
	memArea = NULL;
	shmctl(SHM_KEY, IPC_RMID, 0);
	free(fileName);
	close(fd);	
	//free(comm);				
	close(respFD);
	close(reqFD);
	return 0;
}