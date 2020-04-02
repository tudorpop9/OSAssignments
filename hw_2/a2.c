#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "a2_helper.h"

typedef struct aName{
	int pn;
	int tn;
	sem_t *mySem;
	sem_t *semt2;
	//pthread_mutex_t *mutex;
	//pthread_cond_t *condVar;
}th_struct;

sem_t* sem3;// pt 2.5
sem_t* sem4;
sem_t* barSem;
sem_t* put12towait;

int nrthBarr = 0;
int isTh12 = 0;

void initusMaximus(){
	static int initOnce = 0;
	if(initOnce == 0){
		sem3 = sem_open("moon_moon_sem3", O_CREAT, 0644, 0);
		sem4 = sem_open("moon_moon_sem4", O_CREAT, 0644, 0);
		barSem = sem_open("pls_work", O_CREAT, 0644, 0); // bariera
		put12towait = sem_open("12_barrier", O_CREAT, 0644, 0);
		initOnce = 1;
	}
}

int p3t1Flag = 0;
int p3t2Flag = 0;
void* p3ProcTh(void* arg){
	th_struct *data = (th_struct*)arg;

	if(data->tn == 1){
		sem_wait(data->mySem); // 0 perm initial, daca ajunge 1 primul aici se blocheaza inainte de BEGIN
		info(BEGIN, data->pn, data->tn);
	}else if (data->tn ==2 ){
		info(BEGIN, data->pn, data->tn);
		sem_post(data->mySem); // data th2 ajunge primul ii da voie la 1 sa inceapa
	}else if (data->tn == 4){ //waits for signal from t4.2
		sem_wait(sem3);
		info(BEGIN, data->pn, data->tn);
	}else{
		info(BEGIN, data->pn, data->tn);
	}
	
	//info(BEGIN, data->pn, data->tn);

	
	if(data->tn == 2){
		sem_wait(data->semt2); // 0 perm initial, daca ajunge 2 primul aici se blocheaza
		info(END, data->pn, data->tn);
	}else if (data->tn == 1){
		info(END, data->pn, data->tn);
		sem_post(data->semt2); // dupa ce 1 ajunge la final ii da voie la 2 sa se finalizeze
	}else if(data->tn == 4){
		info(END, data->pn, data->tn);
		sem_post(sem4);
	}else{
		info(END, data->pn, data->tn);
	}

	//info(END, data->pn, data->tn);
	return NULL;
}
void* p4ProcTh(void* arg){
	th_struct *data = (th_struct*)arg;
	
	if(data->tn == 1){
		sem_wait(sem4); // waits for t3.4 to end
		info(BEGIN, data->pn, data->tn);
	}else{
		info(BEGIN, data->pn, data->tn);
	}

	//info(BEGIN, data->pn, data->tn);

	//info(END, data->pn, data->tn);

	if(data->tn == 2){
		info(END, data->pn, data->tn);
		sem_post(sem3);// signal t3.4 to start
	}else{
		info(END, data->pn, data->tn);
	}
	return NULL;
}

void* barProcTh(void* arg){
	th_struct *data = (th_struct*)arg;

	sem_wait(data->mySem);
	info(BEGIN, data->pn, data->tn);
	/// RIP the dream
	if(data->tn == 12){ ///
		isTh12 = 1;
	}
	nrthBarr++;
	/*if(nrthBarr == 5){// lasa-l pe 12 sa treaca
		sem_post(put12towait);
	}


	if(data->tn != 12 && isTh12 == 1){
		sem_wait(barSem);
	}else{
		if(data->tn == 12){
			if(nrthBarr < 5){
				sem_wait(put12towait);// pune-l pe 12 sa astepte pt 5 threaduri
			}
			nrthBarr--;
			isTh12 = 0;
			info(END, data->pn, data->tn);
			for(int i=0;i<4;i++){
				sem_post(barSem); // da voie altor threaduri sa iasa
			}
		}else{
			nrthBarr--;
			info(END, data->pn, data->tn);
			sem_post(data->mySem);
		}
	}*/
	info(END, data->pn, data->tn);
	sem_post(data->mySem);

	return NULL;
}


int main(){
    init();
    info(BEGIN, 1, 0);

  	//initusMaximus(); // initializez semafoarele //Hopefully, o singura data
  	sem_unlink("moon_moon_sem3");
  	sem_unlink("moon_moon_sem4");
  	sem_unlink("pls_work");
  	sem_unlink("12_barrier");
  	sem3 = sem_open("moon_moon_sem3", O_CREAT, 0644, 0);
	sem4 = sem_open("moon_moon_sem4", O_CREAT, 0644, 0);
	barSem = sem_open("pls_work", O_CREAT, 0644, 0); // bariera
	put12towait = sem_open("12_barrier", O_CREAT, 0644, 0);

    pid_t  pid2, pid3, pid4, pid5, pid6, pid7, pid8;

    pid2 = fork();
    if(pid2 == 0){
    	info(BEGIN, 2, 0);

    	pid3 = fork();
    	if(pid3 == 0){
			info(BEGIN, 3, 0);
			pthread_t th[4];
			th_struct args[4];
			sem_t* sem1;
			sem_t* sem2;
			sem_unlink("monn_sem1");
			sem_unlink("monn_sem2");
			sem1 =sem_open("monn_sem1", O_CREAT, 0644, 0);
			sem2 =sem_open("moon_sem2", O_CREAT, 0644, 0);
			
			//pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;
			//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

			/*if(sem_init(&p3Sem, 0, 1) != 0){
				exit(-1);
			}*/
			for(int i=0 ;i<4 ;i++){ // init args
				args[i].pn = 3;
				args[i].tn = i+1;
				args[i].mySem = sem1;
				args[i].semt2 = sem2;


				pthread_create(&th[i], NULL, p3ProcTh, &args[i]); // start threads
			}
			
			for(int i=0 ;i<4 ; i++){
				pthread_join(th[i], NULL); // wait for threads
			}

			sem_close(sem1);
			sem_close(sem2);
			//sem_destroy(&sem1);
			//sem_destroy(&sem2);
			//pthread_cond_destroy(&condVar);
			info(END, 3, 0);    		
    	}else{
    		pid7 = fork();
    		if(pid7 == 0){
			info(BEGIN, 7, 0);

			info(END, 7, 0);    		
    		}else{
    			pid8 = fork();
    			if(pid8 == 0){
					info(BEGIN, 8, 0);

					info(END, 8, 0);    		
    			}
    			else{

    				th_struct args[47];
    				pthread_t th[47];
    				sem_t max5Sem;
    				sem_init(&max5Sem, 0, 5);
    				for(int i=0 ;i<47 ;i++){ // init args
						args[i].pn = 2;
						args[i].tn = i+1;
						args[i].mySem = &max5Sem;
						//args[i].condVar = &condVar;

						pthread_create(&th[i], NULL, barProcTh, &args[i]); // start threads
					}

					for(int i=0 ;i<47 ; i++){
						pthread_join(th[i], NULL); // wait for threads
					}


					sem_destroy(&max5Sem);
    				waitpid(pid3, NULL, 0);
    				waitpid(pid7, NULL, 0);
    				waitpid(pid8, NULL, 0);
    				info(END, 2, 0);
    			}
    		}
   		}

    	

    }else{
    	pid4 = fork();
 		if(pid4 == 0){
			info(BEGIN, 4, 0);
			pid6 = fork();

			if(pid6 == 0){
				info(BEGIN, 6, 0);
		
				info(END, 6, 0);    		
    		}else{


    			th_struct args[4];
    			pthread_t th[4];
    			//sem_t max5Sem;
    			//sem_init(&max5Sem, 0, 5);
    			for(int i=0 ;i<4 ;i++){ // init args
					args[i].pn = 4;
					args[i].tn = i+1;
					//args[i].mySem = &max5Sem;
					//args[i].condVar = &condVar;

					pthread_create(&th[i], NULL, p4ProcTh, &args[i]); // start threads
				}
				for(int i=0 ;i<4 ; i++){
					pthread_join(th[i], NULL); // wait for threads
				}


    			waitpid(pid6, NULL, 0);
				info(END, 4, 0);
    		}    		
  	  }else{
  	  	pid5 = fork();
    	if(pid5 == 0){
			info(BEGIN, 5, 0);

			info(END, 5, 0);    		
    	}
    	else{
   	 		waitpid(pid2, NULL, 0);
   			waitpid(pid4, NULL, 0);
		    waitpid(pid5, NULL, 0);

    		info(END, 1, 0);
    	}
  	  }
    }
    sem_close(sem3);
	sem_close(sem4);
    return 0;
}