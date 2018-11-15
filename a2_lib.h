/*

Purpose: Header File for Source Code of A2
Author: Leila Erbay 

*/

#ifndef A2_HEADER
#define A2_HEADER


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "comp310_a2_test.h"

#define MAGIC_HASH_NUMBER 5381


#define KV_STORE_NAME "/lerbay1996_store"
#define WRITER_SEMAPHORE "/lerbay_writer_sem"
#define READER_SEMAPHORE "/lerbay_reader_sem"

//SIZEs
#define MAX_NUM_VALUES 256
#define MAX_NUM_KV_OBJECTS 20
#define MAX_NUM_PODS 128

#define MAX_KEY_SIZE 32
#define MAX_VALUE_SIZE 256

//OTHER things
#define SUCCESS 0
#define FAILURE -1
/*#define	SEM_VALUE 1
#define SEM_FLAGS_INIT O_CREAT
#define SEM_FLAG O_CREAT
#define SEM_MODE S_IRWXU
*/


/*----------------- Structs for both Helper file and Lib file ---------------*/

/*
	Name: KV_object
	Purpose: Key and its distinct value(s) if it contains many
*/
typedef struct KV_Objects {
	char key[MAX_KEY_SIZE];
	char values[MAX_NUM_VALUES][MAX_VALUE_SIZE];
	int amt_val_space_avail;		//tells how much space for values is left
	int next_empty_val_index ;		//current next empty value index
	int oldest_val_index ;	//index of earliest value added (for FIFO)
	int last_read_index ;
}KV_object;

/*
	Name: KV_pod
	Purpose: Pods to Hold Keys if they are mapped to a certain pod by the hash function
*/
typedef struct KV_Pods {
	KV_object KV_arr[MAX_NUM_KV_OBJECTS];
	int amt_KV_space_avail;		//how much space for KV Objects is left
	int next_empty_KV_index ;					//index of next empty space
	int oldest_KV_index ;						//index of earliest added KV object (for FIFO)
}KV_pod;


typedef struct KV_Store{
	KV_pod store[MAX_NUM_PODS]; 
	int current_num_readers;
}KV_store;



/*--------------- Methods in a2_lib.c or RELATED TO MEMORY ACCESS------------*/

int kv_store_create(char *name);
int kv_store_write(char *key, char * value);
char* kv_store_read(char *key);
char** kv_store_read_all(char *key);






/*-----------------------------------------------------------------------------
--------------- Helper Methods in a2_helpers.c-----------------------------------
------------------------------------------------------------------------------*/

/*------------------------------- provided by TA ---------------------------*/
//Hash function
unsigned long hash(unsigned char * key);
void generate_string(char buf[], int length);
void generate_unique_data(char buf[], int length, char **keys_buf, int num_keys);
void generate_key(char buf[], int length, char **keys_buf, int num_keys);


/*------------------------- INITIALIZERS ----------------------------------*/
void init_KV(KV_pod *pod);
void init_KVpod(KV_store *storage);
void init_store(KV_store* storage);

/*------------------------------RESET -----------------------------------------*/
void reset_KV(KV_object *key_val);



/*----------------------------- EXISTENCE CHECKS ---------------------------*/
//bool does_pod_exist(int pod_index, KV_store * storage);
int does_key_exist(KV_pod* pod, char* userKey);
bool does_value_exist(KV_pod* pod, char* userKey, int keyIndex);		//ignore this one -- not used


/*-------------------------------- VALIDITY CHECKS ---------------------------*/
bool is_key_valid(char * userKey);
bool is_value_valid(char* userValue);



/*------------------------------ CAPACITY CHECKS --------------------------------*/
bool is_pod_full(KV_pod *pod);
bool is_key_full(KV_pod *pod, unsigned long key_pod_index);


/*-------------------------------- MISCELLANEOUS -------------------------------*/
unsigned long get_pod_index(unsigned long hash);
//int get_pod_mem_loc(int pod_index);

void truncate_key(char * userKey);
void truncate_value( char * userValue);


/*---------------------------- POD-WRITER INTERACTION ------------------------------*/

void writer_update_pod(KV_pod *pod, char *userKey, char* userValue);
void writer_update_pod_booking_info(KV_pod *pod);



/*-------------------------------- KEY-WRITER INTERACTION ----------------------------*/
void writer_update_keyval(KV_pod *pod, int key_index, char*userValue);
void writer_update_keyval_booking_info(KV_pod *pod, int key_index);


/*----------------------------------------- WRITER -------------------------------------*/
int write_to_store(char * userKey, char* userValue, KV_store * storage);


/*------------------------------------ READER METHODS ---------------------------------*/
char * read_single_from_store(char* userKey, KV_store* storage);
void reader_update_keyval_booking_info(KV_object *kv_obj);
char ** read_all_from_store(char *userKey, KV_store * storage);
char ** copy_of_values(char original_values[][MAX_VALUE_SIZE], int num_values, int start_index);



/*---------------------- open and close mem --------------*/

int kv_store_reopen(char * store_name);				//ignore this one -- not used
char* get_store_addr(int fd, KV_store *storage);	//ignore this one -- not used
int kv_store_close(char *store_addr);				//ignore this one -- not used


/*-------------------- semaphores -----------------------*/

sem_t * init_writer_sem();		//ignore this one -- not used
sem_t * init_reader_sem();		//ignore this one -- not used

sem_t * reopen_writer_sem();	//ignore this one -- not used
sem_t * reopen_reader_sem();	//ignore this one -- not used

/*-------------------- errors----------------------------*/
char* nullExit(char* function, int lineNum , char * msg);
int failExit(char* function, int lineNum , char * msg);

#endif

