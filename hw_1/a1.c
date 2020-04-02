#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h> //files
#include <unistd.h> // files

#define MAX_SIZE 4096



char *strrev(char *str){ // https://stackoverflow.com/questions/26618903/implicit-declaration-of-function-strrev
    char c, *front, *back; //Goddamit linux.....

    if(!str || !*str)
        return str;
    for(front=str,back=str+strlen(str)-1;front < back;front++,back--){
        c=*front;*front=*back;*back=c;
    }
    return str;
}



int getPermValue(char* c){

	int perm = 0;
	for(int i=0;i<9;i++){
		if(c[i]=='r' || c[i]=='w' || c[i]=='x'){
			perm = perm | 1;
		}
		if(i!= 8){
			perm <<= 1;
		}
	}
	return perm;

}


void listRecursive(char* path, char* options, int aFlag, int comm){
	DIR* dir = NULL;
	struct dirent* entry = NULL;
	struct stat statVar;

	char actualPath[MAX_SIZE];

	dir = opendir(path);
	if(dir == NULL){
		printf("ERROR\ninvalid directory path");
		exit(1);
	}else if(aFlag == 0){
		printf("SUCCESS\n");
		aFlag = 1;
	}

	
	while((entry = readdir(dir)) && entry != NULL){
		if( strcmp(entry->d_name,".")  && strcmp(entry->d_name,"..") ){

			snprintf(actualPath, MAX_SIZE, "%s/%s", path, entry->d_name);
			if(lstat(actualPath, &statVar) == 0){

				if(comm & (short int)2){//verific daca are sau nu conditii 
						if(atoi(options) != 0){//size_small
							if(S_ISDIR(statVar.st_mode) == 0 && statVar.st_size < atoi(options)){
								printf("%s\n", actualPath);
							}
						}else if ((statVar.st_mode & 0777) == getPermValue(options)){
							printf("%s\n", actualPath);
						}	
					}else{
						printf("%s\n", actualPath);
					}
				if(S_ISDIR(statVar.st_mode)){
					listRecursive(actualPath, options, aFlag, comm);
				}
			}
		}
	}

	closedir(dir);
}

void listFunction(char* path, char* options, short int comm){
	if(comm < (short int)4){
		printf("ERROR\nInvalid arguments for list function");
        exit(1);
	}
	if(comm & (short int)1){
		listRecursive(path, options, 0, comm);
	}else{
		DIR* dir = NULL;
		struct dirent* entry = NULL;
		struct stat statVar;

		char actualPath[MAX_SIZE];

		//printf("%o\n",getPermValue(options));

		dir = opendir(path);
		if(dir == NULL){
			printf("ERROR\ninvalid directory path\n");
			exit(1);
		}else {
			printf("SUCCESS\n");
		}
		while((entry = readdir(dir)) && entry != NULL){
			if( strcmp(entry->d_name,".") && strcmp(entry->d_name,"..") ){

				snprintf(actualPath, MAX_SIZE, "%s/%s", path, entry->d_name);
				if(lstat(actualPath, &statVar) == 0){
					if(comm & (short int)2){//verific daca are sau nu conditii 
						if(atoi(options) != 0){//size_small
							if((S_ISDIR(statVar.st_mode)==0) && statVar.st_size < atoi(options)){
								printf("%s\n", actualPath);
							}
						}else if ((statVar.st_mode & 0777) == getPermValue(options)){
							printf("%s\n", actualPath);
						}	
					}else{
						printf("%s\n", actualPath);
					}
				}
			}
		}
		closedir(dir);
	}
}

int invalidSectType(int fd, int noSections, int* forAll){
	int dummyType = 0, initialPoz = 0, invalid = 0;
	initialPoz = lseek(fd, 0, SEEK_CUR); // salvez pozitia cursorului dinaintea apelului

	lseek(fd,20,SEEK_SET); // prima iteratie a verificarii pt simplitate
	read(fd, &dummyType, 4);

	if(forAll != NULL){
		(*forAll) = 0;
	}

	if(dummyType != 62 && dummyType != 45 && dummyType != 65 && dummyType != 73 && dummyType != 32 && dummyType != 93){ // 62 45 65 73 32 93
		lseek(fd,initialPoz,SEEK_SET);
		return 1;
	}else{
		if(forAll != NULL && dummyType == 62){
			(*forAll)++;
		}
		for(int i=0;i<noSections-1;i++){
			lseek(fd,19,SEEK_CUR); // pozitionez cursorul pentru urmatoarea citire
			read(fd, &dummyType, 4);

			if(dummyType != 62 && dummyType != 45 && dummyType != 65 && dummyType != 73 && dummyType != 32 && dummyType != 93){ // 62 45 65 73 32 93
				invalid = 1;
				break;
			}else{
				if(forAll != NULL && dummyType == 62){
					(*forAll)++;
				}
			}
		}
	}

	lseek(fd,initialPoz,SEEK_SET);
	return invalid;
}

void parseThis(char *path){

	char magic[3]={0};
	int headerSize = 0, version = 0, noSections = 0;
	char sectName[12];
	int  sectType, sectOffest, sectSize;

	int fd = open(path,O_RDONLY);
	if(fd < 0 ){
		printf("ERROR\n");
		return;
	}else{
		//Partea de validare
		read(fd, magic, 2); magic[2] = '\0'; // magic
		read(fd, &headerSize, 2); //headerSize
		read(fd, &version, 4);// version
		read(fd, &noSections, 1);// nr of sections
		if(strcmp(magic,"88") != 0){
			printf("ERROR\nwrong magic\n");
			close(fd);
			return;
		}else if(version<99 || version>177){
			printf("ERROR\nwrong version\n");
			close(fd);
			return;
		}else if(noSections<6 || noSections>19){
			printf("ERROR\nwrong sect_nr\n");
			close(fd);
			return;
		}else if(invalidSectType(fd,noSections,NULL)){
			printf("ERROR\nwrong sect_types\n");
			close(fd);
			return;
		}else{

			printf("SUCCESS\nversion=%d\nnr_sections=%d\n",version,noSections);
			for(int i=1;i<=noSections;i++){

				read(fd, sectName, 11); sectName[11] = '\0'; // citire SECTION HEADERS
				read(fd, &sectType, 4);
				read(fd, &sectOffest, 4);
				read(fd, &sectSize, 4);
				printf("section%d: %s %d %d\n",i,sectName,sectType,sectSize);

			}

		}

	}
	
	close(fd);
}

int validataFile(int fd, int* forAll){

	char magic[3]={0};
	int headerSize = 0, version = 0, noSections = 0;
	int initialPoz = 0;
	if(fd < 0 ){
		return 0;
	}else{
		initialPoz = lseek(fd,0,SEEK_CUR);// salvare pozitie initiala
		lseek(fd,0,SEEK_SET);// pozitionare pt validare
		//Partea de validare
		read(fd, magic, 2); magic[2] = '\0'; // magic
		read(fd, &headerSize, 2); //headerSize
		read(fd, &version, 4);// version
		read(fd, &noSections, 1);// nr of sections
		if(strcmp(magic,"88") != 0){
			close(fd);
			return 0;
		}else if(version<99 || version>177){
			close(fd);
			return 0;
		}else if(noSections<6 || noSections>19){
			close(fd);
			return 0;
		}else if(invalidSectType(fd,noSections,forAll)){
			close(fd);
			return 0;
		}
	}

	lseek(fd,initialPoz,SEEK_SET);
	return 1;
}

void extractFunction(char* path, int section, int line){
	int noSections = 0;
	int sectOffest = 0, sectSize = 0;

	int fd = open(path,O_RDONLY);
	if(fd < 0 ){
		printf("ERROR\n");
		return;
	}else if(validataFile(fd,NULL) == 0){
		printf("ERROR\ninvalid file");
		close(fd);
		return;
	}else{
		lseek(fd, 8, SEEK_SET);// sar peste magic/header size si ce mai era pe acolo
		read(fd, &noSections, 1);// nr of sections

		if(noSections < section){
			printf("ERROR\ninvalid section");
			close(fd);
			return;
		}else{
			lseek(fd, 15 + (section - 1)*23, SEEK_CUR); // voi sari peste sectiunile care nu conteaza si peste numele si tipul celei dorite
			read(fd, &sectOffest, 4);// iau offsetul si size-ul sectiunii dorite
			read(fd, &sectSize, 4);

			char sectionBuffer[sectSize + 1]; 
			lseek(fd, sectOffest, SEEK_SET);
			read(fd, sectionBuffer, sectSize); sectionBuffer[sectSize] = '\0'; //Citirea intregii sectiuni

			char lineContent[sectSize+1]; //variablia cu linia
			lineContent[sectSize] = '\0';

			char token[3]={0}; 
			token[0]=0x0D; //Token pt despartirea liniilor
			token[1]=0x0A; 
			token[2]='\0';
			int countLines = 0;

			strcpy(lineContent, strtok(sectionBuffer,token));
			countLines++;
			while(countLines < line && lineContent!=NULL){ //selectarea liniei
				strcpy(lineContent, strtok(NULL,token));
				//printf("%s\n", lineContent);
				countLines++;
			}

			if(lineContent != NULL && countLines < line){
				printf("ERROR\ninvalid line");
				close(fd);
				return;
			}else{
				strcpy(lineContent, strrev(lineContent));// inversarea caracterelor
				printf("SUCCESS\n%s\n", lineContent);
				//printf("%d\n", countLines);
			}
		}

	}

	close(fd);

}

void findThemAll(char* path){
	static int aFlag = 0; // pentru a printa doar o singura data succes
	DIR* dir = NULL;
	struct dirent* entry = NULL;
	struct stat statVar;

	char actualPath[MAX_SIZE];

	dir = opendir(path);
	if(dir == NULL){
		printf("ERROR\ninvalid directory path");
		exit(1);
	}else if(aFlag == 0){
		printf("SUCCESS\n");
		aFlag = 1;
	}

	
	while((entry = readdir(dir)) && entry != NULL){
		if( strcmp(entry->d_name,".")  && strcmp(entry->d_name,"..") ){

			snprintf(actualPath, MAX_SIZE, "%s/%s", path, entry->d_name);
			if(lstat(actualPath, &statVar) == 0){
				if(S_ISREG(statVar.st_mode)){

					int fd = open(actualPath,O_RDONLY);
					int forAll = 0;
					if(fd < 0 ){
						closedir(dir);
						return;
					}
					else{
						//printf("%d, %d\n",validataFile(fd,&forAll), forAll);
						if(validataFile(fd,&forAll) && forAll>= 2){
							printf("%s\n",actualPath);
						}
					}
				}
				
				if(S_ISDIR(statVar.st_mode)){
					findThemAll(actualPath);
				}
			}
		}
	}

	closedir(dir);
}

int main(int argc, char **argv){


    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){////////////**********************VARIANT********************///////////////////
            printf("47634\n");
        }else{
        	if((strcmp(argv[1], "list") == 0) ) {////////////**********************LIST********************///////////////////
            	short int comm = 0; // folosit pt determinarea obtiunilor alese
            	char path[MAX_SIZE]; path[0]='\0';
            	char options[100]; options[0]='\0';
            	for(int i=2; i<argc; i++){

            		if(strcmp(argv[i],"recursive") == 0){
            			comm = comm | (short int)1;
            		}else if (strstr(argv[i],"size_smaller=") != NULL){
            					
            			snprintf(options,100,"%s",argv[i]+ strlen("size_smaller="));
            			comm = comm | (short int)2;//sa stiu daca s-a trimis sau nu o optiune

            		}else if (strstr(argv[i],"permissions=") != NULL){

            			snprintf(options,100,"%s",argv[i]+ strlen("permissions="));
            			comm = comm | (short int)2;//sa stiu daca s-a trimis sau nu o optiune

            		}else  if (strstr(argv[i],"path=") != NULL){
         	
            			snprintf(path,MAX_SIZE,"%s",argv[i]+ strlen("path="));
            			comm = comm | (short int)4;

            		}else{
            			printf("ERROR\nInvalid arguments for list function");
            			exit(1);
            		}
            	}

            	listFunction(path, options, comm);
       	 }else if((strcmp(argv[1], "parse") == 0)){////////////**********************PARSE********************///////////////////
       	 			char path[MAX_SIZE]; path[0]='\0';;
       	 			if (strstr(argv[2],"path=") != NULL){
            			snprintf(path,MAX_SIZE,"%s",argv[2]+ strlen("path="));
            			parseThis(path);
            		}
       	 }else if ((strcmp(argv[1], "extract") == 0)){////////////**********************EXTRACT********************///////////////////

       	 		char path[MAX_SIZE]; path[0]='\0';
       	 		char dummy[21] = {0};
           		int line = 0;
           		int section = 0;
            	for(int i=2; i<argc; i++){

            		if (strstr(argv[i],"section=") != NULL){

            			snprintf(dummy,21,"%s",argv[i]+ strlen("section="));
            			section = atoi(dummy);

            		}else if(strstr(argv[i],"line=") != NULL) {

            			snprintf(dummy,21,"%s",argv[i]+ strlen("line="));
            			line = atoi(dummy);

            		}else  if (strstr(argv[i],"path=") != NULL){
         	
            			snprintf(path,MAX_SIZE,"%s",argv[i]+ strlen("path="));

            		}else{
            			printf("ERROR\nInvalid arguments for extract function");
            			exit(1);
            		}
            	}

            	extractFunction(path,section,line);

       	 }else if((strcmp(argv[1], "findall") == 0)){
       	 	char path[MAX_SIZE]; path[0]='\0';
            snprintf(path,MAX_SIZE,"%s",argv[2]+ strlen("path="));
            findThemAll(path);
       	 }
       	 
   	 }
    return 0;
	}
}