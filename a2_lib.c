/*
	Purpose: Source File for A2
	Author: Leila Erbay

*/

#include "a2_lib.h"	
//#include "a2_helpers.c"


/*---------------------------------------------------------------------------------------------
-------------------------------- ASSIGNMENT REQUIRED FUNCTIONS --------------------------------
------------------------------------------------------------------------------------------------ */

KV_store*  storage_addr;	// initialize storage -- global var
//KV_store storage;
sem_t *writer_sem, *reader_sem;
//char* user_store_name;
//int current_num_readers;



/*---------------------------------------------------------------------------------------------
----------------------------- CREATE STORE -----------------------------------------------------
---------------------------------------------------------------------------------------------*/
/*
	Name: kv_store_create
	Purpose: open shared memory for the purpose of reading and writing. Here occurs initial creation
	Input: char * store_name
	Output:success (0) fail (-1)
	NOTE: store_name is useless and not used anywhere
*/
int kv_store_create(char *store_name){
	
	int fd = shm_open(KV_STORE_NAME,O_CREAT | O_EXCL|O_RDWR, S_IRWXU);	//open the shared memory

	if(errno == EEXIST) return SUCCESS;

	//MAY BE GIVING ERRORS for TEST2
	if(fd < 0) 
		return failExit("kv_store_create:", __LINE__, "\t --- Error occurred in obtaining shared memory address.\t");
	



	storage_addr = (KV_store*) mmap(NULL, sizeof(KV_store),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //map shared memory to virtual address space
	
	if (storage_addr == MAP_FAILED)
		return failExit("kv_store_create:", __LINE__, "\t --- Error occurred in obtaining shared memory address.\t");

	int sizing_ret = ftruncate(fd, sizeof(KV_store));	//set size of shared memory 
	if(sizing_ret < 0) 
		return failExit("kv_store_create:", __LINE__, "\t --- Error occurred in ftruncate.\t");
	

	

	
	init_store(storage_addr);	// initialize storage
	


	int munmap_ret = munmap(storage_addr, sizeof(KV_store));		//memory unmap
	if(munmap_ret < 0)
		return failExit("kv_store_create:", __LINE__, "\t --- Error in closing mapped memory:\t ");	

	// create semaphores
	sem_unlink(WRITER_SEMAPHORE);
	writer_sem = sem_open(WRITER_SEMAPHORE, O_CREAT, S_IRWXU, 1);

	sem_unlink(READER_SEMAPHORE);
	reader_sem = sem_open(READER_SEMAPHORE, O_CREAT, S_IRWXU, 1);

	if(writer_sem == SEM_FAILED) 
		return failExit("kv_store_create:", __LINE__, "\t --- Error in initial created writer semaphore.\t");

	if(reader_sem == SEM_FAILED)
		return failExit("kv_store_create:", __LINE__, "\t --- Error in intial created reader semaphore.\t");

	sem_close(writer_sem);
	sem_close(reader_sem);


	return SUCCESS;

}

/*-----------------------------------------------------------------------------------------------
-------------------------------------- WRITE -----------------------------------------------------
-----------------------------------------------------------------------------------------------*/

/*	name: kv_store_write
	Purpose: write to the store in shared memory
	Input: char * key, char * value
	Output: int - success or failure
*/
int kv_store_write(char * key, char * value){
	int fd;

	writer_sem = sem_open(WRITER_SEMAPHORE, O_CREAT, S_IRWXU, 1);		//open writer semaphore
	if (writer_sem == SEM_FAILED)
		return failExit("kv_store_write:", __LINE__, "\t --- Error in reopening writer semaphore.\t");


	if ((fd = shm_open(KV_STORE_NAME, O_RDWR, S_IRWXU)) < 0) 		//open shared mem
		return failExit("kv_store_write:", __LINE__, "\t ---Error occurred in opening shared memory.\t");
		
	storage_addr = (KV_store *)mmap(NULL, sizeof(KV_store),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);		//map shared mem
	if(storage_addr == MAP_FAILED) 
		return failExit("kv_store_write:", __LINE__, "\t --- Error occurred in obtaining shared memory address.\t ");
	

	int wrt = sem_wait(writer_sem);		//lock writer semaphore
	if(wrt < 0)
		return failExit("kv_store_write:", __LINE__, "\t --- Error in locking writing semaphore.\t");
	

/*-------------------------------------------------- CRITICAL SECTION : START ------------------------------------------------*/
	

	if (write_to_store(key, value, storage_addr) < 0) 			//MAIN WRITING OCCURS HERE
		return failExit("kv_store_write:", __LINE__, "\t --- Error in writing.\t");

	
	

/*--------------------------------------------------- CRITICAL SECTION  : END ----------------------------------------------------*/

	if(sem_post(writer_sem)< 0)			//unlock writer semaphore
		return failExit("kv_store_write:", __LINE__, "\t --- Error in unlocking writer semaphore: \t");
	
	sem_close(writer_sem);		//close writer semaphore

	close(fd);
	
	if(munmap(storage_addr, sizeof(KV_store)) < 0)		//unmap shared mem
		return failExit("kv_store_write:", __LINE__, "\t --- Error in closing mapped memory:\t ");	

	//sem_close(writer_sem);
	return SUCCESS;

}




/*----------------------------------------------------------------------------------------------
---------------------------------------- SINGLE READ ------------------------------------------
------------------------------------------------------------------------------------------------*/

/*	name: kv_store_read
	Purpose: read from the store in shared memory
	Input: char * key
	Output: char * copy of the value
*/
char * kv_store_read(char* key){
	int fd;
	char * val_cpy;//= value;

	//reopen reader lock
	reader_sem = sem_open(READER_SEMAPHORE, O_CREAT, S_IRWXU, 1);			//reopen reader store
	if (reader_sem == SEM_FAILED) 
		return nullExit("kv_store_read:", __LINE__, "\t --- Error in reopening reader semaphore.\t");
	
	//reopen writer lock
	writer_sem = sem_open(WRITER_SEMAPHORE, O_CREAT, S_IRWXU, 1);	//reopen writer store
	if (writer_sem == SEM_FAILED) 
		return nullExit("kv_store_read:", __LINE__, "\t --- Error in reopening writer semaphore.\t");


	fd = shm_open(KV_STORE_NAME, O_RDWR, S_IRWXU);	//open shared mem
	if (fd < 0) 
		return nullExit("kv_store_read:", __LINE__, "\t --- Error occurred in opening shared memory.\t");		

	storage_addr  =(KV_store*) mmap(NULL, sizeof(KV_store),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);		//map shared mem
	if(storage_addr == MAP_FAILED) 
		return nullExit("kv_store_read:", __LINE__, "\t --- Error occurred in obtaining shared memory address.\t");

	/*----------------------------------------------- CRITICAL SECTION : START----------------------------------------------------------*/

	//lock reader lock
	if(sem_wait(reader_sem) < 0) 
		return nullExit("kv_store_read:", __LINE__, "\t --- Error in locking reader semaphore.\t");

		//increment number of readers
		storage_addr->current_num_readers++;


	if(storage_addr->current_num_readers == 1) {			//if readers exist, make writers wait
		if(sem_wait(writer_sem) < 0) 
			return	nullExit("kv_store_read:", __LINE__, "\t --- Error in locking writing semaphore.\t");
	}

	if(sem_post(reader_sem)< 0){		//unlock reader
		sem_post(writer_sem);	
		return nullExit("kv_store_read:", __LINE__, "\t --- Error in unlocking reader semaphore.\t"); 
	}

/*--------------------------------------------------- CRITICAL SECTION: END -----------------------------------------------------------------*/



		val_cpy = read_single_from_store(key, storage_addr);		//MAIN READING ACTION OCCURS HERE



/*----------------------------------------------- CRITICAL SECTION : START----------------------------------------------------------*/

	if(sem_wait(reader_sem) < 0) {		//lock reader sem
		sem_post(writer_sem); 
		return nullExit("kv_store_read:", __LINE__, "\t --- Error in locking reader semaphore.\t"); 
	}
		
		//decrement readers
		storage_addr->current_num_readers--;


	if(storage_addr->current_num_readers== 0){		
		if(sem_post(writer_sem)< 0)  				//unlock writers if necessary
			return nullExit("kv_store_read:", __LINE__, "\t --- Error in unlocking writer semaphore: \t");
	}


	if(sem_post(reader_sem)< 0)
		return nullExit("kv_store_read:", __LINE__, "\t --- Error in unlocking reader semaphore.\t"); 

/*--------------------------------------------------- CRITICAL SECTION: END -----------------------------------------------------------------*/

	close(fd);
	if(munmap(storage_addr, sizeof(KV_store)) < 0) 	//unmap shared memory
		return nullExit("kv_store_read:", __LINE__, "\t --- Error in closing mapped memory:\t ");		

	sem_close(writer_sem);		//close semaphores
	sem_close(reader_sem);


	
	return val_cpy;

}


/*-----------------------------------------------------------------------------------------------------------------------
------------------------------------------------------ READ ALL ---------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------*/


/*	name: kv_store_read_all
	Purpose: return all the values associated with a key
	Input: char * key
	Output: char ** copy of values
*/
char** kv_store_read_all(char *key){

	int fd;
	char ** values_cpy;

	
	reader_sem = sem_open(READER_SEMAPHORE, O_CREAT, S_IRWXU, 1);		//reopen reader store
	if (reader_sem == SEM_FAILED)  
		return (char**)nullExit("kv_store_read_all:", __LINE__, "\t --- Error in reopening reader semaphore.\t");

	
	writer_sem = sem_open(WRITER_SEMAPHORE, O_CREAT, S_IRWXU, 1);	//reopen writer sem
	if (writer_sem == SEM_FAILED) 
		return (char**)nullExit("kv_store_read_all:", __LINE__, "\t ---  Error in reopening writer semaphore.\t");
	

	fd = shm_open(KV_STORE_NAME, O_RDWR, S_IRWXU);		
	if ( fd< 0) 			//OPEN SHARED MEM
		return (char**)nullExit("kv_store_read_all:",__LINE__, "\t --- Error occurred in opening shared memory.\t" );
	
	 storage_addr = (KV_store*) mmap(storage_addr, sizeof(KV_store),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	//map shared memory
	if(storage_addr == MAP_FAILED) 
		return (char**)nullExit("kv_store_read_all:", __LINE__, "\t --- Error occurred in obtaining shared memory address.\t");
	
	/*----------------------------------------------------- CRITICAL SECTION : START -----------------------------------------------*/

	if(sem_wait(reader_sem) < 0) 	//reader waits
		return (char**)nullExit("kv_store_read_all:" , __LINE__,"\t ---  Error in locking reader semaphore.\t");
	

		//increment number of readers
		storage_addr->current_num_readers++;

	if(storage_addr->current_num_readers == 1){ 
		if(sem_wait(writer_sem) < 0) 			//if readers exist then writers to wait
			return (char**)nullExit("kv_store_read_all:", __LINE__, "\t --- Error in locking writer semaphore.\t");
	}	

	if(sem_post(reader_sem)< 0){ 		//unlock reader
		sem_wait(writer_sem);
		return (char**)nullExit("kv_store_read_all:",__LINE__, "\t --- Error in unlocking reader semaphore.\t");
	}

/*---------------------------------------------------- CRIICAL SECION: END --------------------------------------------------------*/

	

	values_cpy = read_all_from_store(key, storage_addr); 	//MAIN READING ACTION OCCURS HERE


/*----------------------------------------------------- CRITICAL SECTION : START -----------------------------------------------*/
	if(sem_wait(reader_sem) < 0) 		//lock reader
		return (char**)nullExit("kv_store_read_all:",__LINE__, "\t --- Error in locking reader semaphore.\t");


		storage_addr->current_num_readers--;	//decrement readers


	if(storage_addr->current_num_readers== 0){
		if(sem_post(writer_sem)< 0) 				//unlock writer
			return (char**)nullExit("kv_store_read_all:",__LINE__, "\t --- Error in unlocking writer semaphore.\t");
	}


	if(sem_post(reader_sem)< 0) 		//unlock reader
		return (char**)nullExit("kv_store_read_all:", __LINE__, "\t --- Error in unlocking reader semaphore.\t");
	
/*---------------------------------------------------- CRIICAL SECION: END --------------------------------------------------------*/


	close(fd);
	if(munmap(storage_addr, sizeof(KV_store))< 0) 		//unmap shared mem
		return (char**)nullExit("kv_store_read_all:" , __LINE__, "\t --- Error in closing shared memory.\t");

	
	sem_close(writer_sem);		//close writer sem
	sem_close(reader_sem);		//close reader sem
	
	
	return values_cpy;

	
}