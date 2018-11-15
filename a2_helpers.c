/* 	AUTHOR: LEILA ERBAY 260672158
	PURPOSE: HELPER METHODS FOR a2_lib.c
*/

#include "a2_lib.h"


//#define MAGIC_HASH_NUMBER 5381




/*----------------------------------------------------------------------------------------------
---------------------------------- METHODS PROVIDED BY TA --------------------------------------
-----------------------------------------------------------------------------------------------*/


/*	Name: hash (provided by TA)
	Purpose: Provide the hashing mechanism to place keys in certain pods
	Input: char * key
	Output: unsigned long hash value
*/
unsigned long hash(unsigned char *str) {
	int c;
    #ifndef SDBM
	unsigned long hash = MAGIC_HASH_NUMBER;
	while (c = *str++)
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	
	#else
        unsigned long hash = 0;
        while (c = *str++)
            hash = c + (hash << 6) + (hash << 16) - hash;
	#endif
	return hash;
}


/*
	Name: generate_string (provided by TA)
	Purpose: Generate a random string of some specified length
	Input: char []buf, int length
	Output: None
	Note: updates the buf input by filling it with characters
*/
void generate_string(char buf[], int length){
    int type;
    for(int i = 0; i < length; i++){
        type = rand() % 3;
        if(type == 2)
            buf[i] = rand() % 26 + 'a';
        else if(type == 1)
            buf[i] = rand() % 10 + '0';
        else
            buf[i] = rand() % 26 + 'A';
    }
    buf[length - 1] = '\0';
}


/*	Name: generate_unique_data (provided by TA)
	Purpose:
	Inputs: char buf[], int length, char **keys_buf, int num_keys
	Output: none
	Note: 
*/
void generate_unique_data(char buf[], int length, char **keys_buf, int num_keys){
    generate_string(buf, __TEST_MAX_DATA_LENGTH__);
    int counter = 0;
    for(int i = 0; i < num_keys; i++){
        if(strcmp(keys_buf[i], buf) == 0){
            counter++;
        }
    }
    if(counter > 1){
        generate_unique_data(buf, length, keys_buf, num_keys);
    }
    return;
}

/*	Name: generate_key (provided by TA)
*/
void generate_key(char buf[], int length, char **keys_buf, int num_keys){
    generate_string(buf, __TEST_MAX_KEY_SIZE__);
    int counter = 0;
    for(int i = 0; i < num_keys; i++){
        if(strcmp(keys_buf[i], buf) == 0){
            counter++;
        }
   }
    if(counter > 1){
        generate_key(buf, length, keys_buf, num_keys);
    }
    return;
}


/*-------------------------------------------------------------------------------------------
---------------------------------- INITIALIZERS -----------------------------------------------
---------------------------------------------------------------------------------------------*/
/*	Name: init_KV
	Purpose: Allocate memory and set the initial values
	Input: none
	Output: KV_object pointer (KV_object *)
*/
void init_KV(KV_pod *pod){
		
	for(int i = 0; i < MAX_NUM_KV_OBJECTS; i++){
		KV_object *kv = &(pod->KV_arr[i]);
		memset(kv->key, '\0', sizeof(char)*MAX_KEY_SIZE);
		kv->amt_val_space_avail = MAX_NUM_VALUES;
		kv->next_empty_val_index = 0;					
		kv->oldest_val_index = 0;						
		kv->last_read_index = -1;
		memset(kv->values,'\0', sizeof(char)*MAX_NUM_VALUES*MAX_VALUE_SIZE);
	}

}


/*	Name: init_KVpod()
	Purpose: initialize memory space and set the values of the new pod as well as its attributes
	Input: none
	Output: KV_pod pointer (KV_pod *)
*/
void init_KVpod(KV_store *storage){
	//Set memory for values or set values;
	for(int i = 0; i < MAX_NUM_PODS; i++){
		KV_pod *pod = &(storage->store[i]);
		pod->amt_KV_space_avail = MAX_NUM_KV_OBJECTS;
		pod->next_empty_KV_index = 0;
		pod->oldest_KV_index = 0;
		init_KV(pod);
	}
	
}


/*	Name: init_store()
	Purpose: Allocate memory and set initial values of the store
	Input: None
	Output: KV_store pointer (KV_store *)
*/
void init_store(KV_store * storage){

	for(int i = 0; i < MAX_NUM_PODS; i++){
		memset(&(storage->store[i]), '\0', sizeof(KV_pod));
	}
	storage->current_num_readers = 0;

	init_KVpod(storage);

}


/*-----------------------------------------------------------------------------------------------
------------------------------------------- RESET KV_OBJECT -------------------------------------
------------------------------------------------------------------------------------------------*/
/*	Name: reset_KV
	Purpose: reset the KV_object when the pod is full and needing to place a new key and value
	Input: KV_object *ptr
	Output:	none 
	Note: updates ptr
*/
void reset_KV(KV_object *key_val){
	memset(key_val->key, '\0', sizeof(char)*MAX_KEY_SIZE);
		key_val->amt_val_space_avail = MAX_NUM_VALUES;
		key_val->next_empty_val_index = 0;
		key_val->oldest_val_index = 0;
		key_val->last_read_index = -1;
	memset(key_val->values,'\0', sizeof(char)*MAX_NUM_VALUES*MAX_VALUE_SIZE);

	return;
}

/*------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------- EXISTENCE CHECKS ---------------------------------------------
------------------------------------------------------------------------------------------------------------------------*/

/*	Name: does_key_exist
	Purpose: determine if the pod already contains the key
	Input: KV_pod* pod, char* userInput
	Output: int
			if key does exist return is the index where it exists
			if key does not exist return -1
*/
int does_key_exist(KV_pod* pod, char* userKey){

	int key_index = FAILURE;
	for (	int i = 0; i < MAX_NUM_KV_OBJECTS; i++){
		if (strcmp(pod->KV_arr[i].key, userKey) ==0){	//if key and user input are the same 
			key_index = i;
			return key_index;
		}
	}
	return FAILURE;			// no key exists
}


/* 	Name: does_value_exist
	Purpose: determine if value exists (if it does we will not change key or pod)
	Input: KV_pod*, char* userKey, char* userValue
	Output: bool -- true if value does exist, false if it doesn't exist
*/
bool does_value_exist(KV_pod* pod, char* userValue, int keyIndex){
	for(int i = 0; i < MAX_NUM_VALUES; i++){								//if value exists return true
		if(strcmp((pod->KV_arr[keyIndex]).values[i], userValue) == 0){
			return true;
		}
	}

	return false;
}

/*-------------------------------------------------------------------------------------------------------
---------------------------------------- VALIDITY CHECKS --------------------------------------------
------------------------------------------------------------------------------------------------------*/

/*	Name: is_key_valid
	Purpose: determine if the key is a valid length
	Inputs: char * userKey
	Outputs: bool
			true if key length (including null) is less than max allotted length
			false if key length (including null) is greater than max allotted length
*/
bool is_key_valid(char * userKey){
	if ((strlen(userKey)+1) > MAX_KEY_SIZE ){
		return false;
	}
	return true;
}


/*	Name: is_value_valid
	Purpose: determine if the value is a valid lenght
	Inputs: char * userValue
	Outputs: bool
			true if value length (including null) is less than max allotted length
			false if value length (including null) is greater than max allotted length
*/
bool is_value_valid(char* userValue){
	if((strlen(userValue)+1) > MAX_VALUE_SIZE){
		return false;
	}
	return true;
}




/*---------------------------------------------------------------------------------------------------
------------------------------------------- CAPACITY CHECKS ------------------------------------------
----------------------------------------------------------------------------------------------------*/
/*	Name: is_pod_full
	Purpose: determine if pod is full
	Input: KV_pod *pod
	Output: bool
			true if pod has reached max capacity of keys
			false if pod has not reached max capacity of keys
*/
bool is_pod_full(KV_pod *pod){
	if(pod->amt_KV_space_avail == 0) return true;
	else return false;
}


/*	Name: is_key_full
	Purpose: determine if key has reached max capacity of values
	Input: KV_pod *pod, int key_pod_index
	Output: bool
			true if key is full
			false if key is not full
	Note: this is after the key is known to exist in pod
*/
bool is_key_full(KV_pod *pod, unsigned long key_pod_index){
	if(pod->KV_arr[key_pod_index].amt_val_space_avail == 0 ){
		return true;
	}
	return false;
}


/*-------------------------------------------------------------------------------------------------
------------------------------------------- MISCELLANEOUS ------------------------------------------
---------------------------------------------------------------------------------------------------*/

/*	Name: get_pod_index
	Purpose: get the index of the pod inside the Store
	Input: unsigned long hash
	Output: unsigned long the pod_index in the store
*/
unsigned long get_pod_index(unsigned long hash){
	return (hash % MAX_NUM_PODS);		// returns the pod index
}



/*	Name: truncate_key
	Purpose: shorten key  if it has a length greater than allotted to it
	Input: char * userKey
	Output: none
	Note: will change the userKey if needed
*/
void truncate_key(char * userKey){
	if(!is_key_valid(userKey)){
		char keyCpy[MAX_KEY_SIZE];
		strncpy(keyCpy, userKey, MAX_KEY_SIZE-1);		//copy 31 characters
		keyCpy[MAX_KEY_SIZE-1] = '\0'; 					//add null character
		strcpy(userKey, keyCpy);
	}
	return ;
}


/*	Name: truncate_value
	Purpose: shorten value if it has a length greater than allotted to it
	Input:  char* userValue
	Output: none
	Note: will change the  userValue if needed
*/
void truncate_value( char * userValue){
	if(!is_value_valid(userValue)){
		char valCpy[MAX_VALUE_SIZE];
		strncpy(valCpy, userValue, MAX_VALUE_SIZE-1);			//copy 255 chars
		valCpy[MAX_VALUE_SIZE-1] = '\0';						//add null character at the end
		strcpy(userValue, valCpy); 
	}

	return ;
}


/*------------------------------------------------------------------------------------------------
----------------------------------- POD-WRITER INTERACTION ----------------------------------------
-------------------------------------------------------------------------------------------------*/

/*	Name: writer_update_pod
	Purpose: if pod is full, replace the earliest KV object with the userKey and userValue
	Input: KV_pod *pod, char * userKey, char* userValue
	Output: none
*/
void writer_update_pod(KV_pod *pod, char *userKey, char* userValue){
	KV_object *temp; 

	if(is_pod_full(pod) == false) { 	//POD IS NOT FULL ---> GO TO THE NEXT_EMPTY SPOT IN POD TO PLACE KV_OBJECT
		
		temp = &(pod->KV_arr[(pod->next_empty_KV_index)]);	//get next empty index
		strcpy(temp->key, userKey);				//place userKey into newKV
		strcpy(temp->values[0], userValue);	
		temp->next_empty_val_index++;
		temp->amt_val_space_avail--;

	}

	else if(is_pod_full(pod) == true) { 		//if pod is full, must replace oldest KV_object in the pod

		temp = &(pod->KV_arr[(pod->oldest_KV_index)]);			//temp pointer to the KV_object inside the pod at the index 
																//of the OLDEST KV_object that exists inside the pod
		reset_KV(temp);				//reset KV_object
		strcpy(temp->key, userKey);				//place userKey into key o
		strcpy(temp->values[0], userValue);		//place the user Key into the first
		temp->next_empty_val_index++;		//update attributes
		temp->amt_val_space_avail--;

	}
	
	writer_update_pod_booking_info(pod);	//update the info(oldest index,  next empty index, and amt space) of the pod

	return;
}

/*	Name: writer_update_pod_booking_info
	Purpose: update the indices of the pod that had a KV written to it and update the amount of space left in pod
	Input: KV_pod *pod
	Output: none
*/
void writer_update_pod_booking_info(KV_pod *pod){
	if(is_pod_full(pod)) {
		pod->oldest_KV_index++;	//oldest will move to the next index (due to fifo)

		if((pod->oldest_KV_index) >= MAX_NUM_KV_OBJECTS){
			pod->oldest_KV_index = 0;
		}
	}

	pod->next_empty_KV_index++;			//update attributes
	pod->amt_KV_space_avail --;

	if ((pod->next_empty_KV_index) >= MAX_NUM_KV_OBJECTS){	//if next empty exceeds number of KV_objects per pod, reset it
		pod->next_empty_KV_index = 0;
	}

	if(pod-> amt_KV_space_avail < 0){	//if amt of space avail in pod is less than zero then pod is full
		pod->amt_KV_space_avail = 0;
	}

	return;

}


/*-----------------------------------------------------------------------------------------------
----------------- KV_OBJECT-WRITER INTERACTION: UPDATE A VALUE IN A KV_OBJECT ------------------
-----------------------------------------------------------------------------------------------*/


/*	Name: writer_update_keyval
	Purpose: Update the value associated to the key 
			If key's maxed out the values, replace the oldest value
			If key's values space is avail, replace next empty
	Input: KV_pod *pod, KV_object key_index, char* userKey, char*userValue
	Output: none
*/
void writer_update_keyval(KV_pod *pod, int key_index, char*userValue){

	KV_object *temp = &(pod->KV_arr[key_index]);	//get access to KV_object addr

	if(is_key_full(pod, key_index)) {	//check if key has max amount of values
		
		strcpy(temp->values[temp->oldest_val_index], userValue);	//if key is full : replace oldest value in key
	}
	else{

		strcpy(temp->values[temp->next_empty_val_index], userValue);	//if key is not full : place value in next empty space
	}

	writer_update_keyval_booking_info(pod, key_index);

	return;
}


/*	Name: writer_update_keyval_booking_info
	Purpose: update the booking info related to the key value
			depends if the array of values is full or not
	Input: KV_pod *pod, KV_object key_index
	Output: none
*/
void writer_update_keyval_booking_info(KV_pod *pod, int key_index){
	KV_object * temp = &(pod->KV_arr[key_index]);

	if(is_key_full(pod, key_index)){			//change index value where the oldest key val object will be
		temp->oldest_val_index++;

		if(temp->oldest_val_index >= MAX_NUM_VALUES){
			temp->oldest_val_index = 0;
		}
	}
		
	temp->next_empty_val_index ++;
	temp->amt_val_space_avail --;

	if(temp->next_empty_val_index >= MAX_NUM_VALUES){	//if next empty exceeds number of values per key, reset it
		temp->next_empty_val_index = 0;
	}

	if(temp->amt_val_space_avail < 0){		//if amt of space avail in key-val object is less than zero then key is full
		temp->amt_val_space_avail = 0;
	}
	
	return;
}



/*----------------------------------------------------------------------------------------------------
---------------------------------------------------- WRITER -----------------------------------------
---------------------------------------------------------------------------------------------------*/

/*	Name: write_to_store
	Purpose: function for writing into store (ie into pod or keyval object)
	Input: char * userKey, char* userValue
	Output: int status 
	Note: udpates Pod or KeyVal object depending on the existence of they key
*/
int write_to_store(char * userKey, char* userValue, KV_store * storage){

	unsigned long pod_index = get_pod_index( hash((unsigned char*)userKey) );	//hash  ****REALLY NEED TO UPDATE THIS?!?!?!?!***
	
 	//index to pod 			NOTE NEED TO CHANGE THIS FOR MEMORY MAPPING
	truncate_key(userKey);
	truncate_value(userValue);

	// KV_pod * = &(storage->store[pod_index]);
	int KV_arr_index = does_key_exist(&(storage->store[pod_index]), userKey); 	//does key exist in pod


	if (KV_arr_index >= 0) {
		
			writer_update_keyval(&(storage->store[pod_index]), KV_arr_index, userValue);	//update the KV object that exists inside the pod
			return SUCCESS;
		
	}	

	else if(KV_arr_index < 0){	//Key Does not Exist 

		writer_update_pod(&(storage->store[pod_index]), userKey, userValue);		//place KV into pod

		return SUCCESS;
	}
	
	else return FAILURE;		//return -1 on error of writing
				
}



/*----------------------------------------------------------------------------------------------------
-------------------------------- READING  SINGLE VALUE----------------------------------------
---------------------------------------------------------------------------------------------------*/

/*	Name: read_from_store
	Purpose: Read from KV object inside the pod
	Input: char * userKey
	Output: char * copy of value string
*/
char * read_single_from_store(char* userKey, KV_store * storage){

	unsigned long pod_index = get_pod_index(hash ((unsigned char*)userKey));	//get pod_index in the store

	int KV_arr_index = does_key_exist(&(storage->store[pod_index]), userKey);	//check if key exist


	if (KV_arr_index >=0 ){			//Key does exist

		KV_pod * pod  = &(storage->store[pod_index]);
		KV_object *temp  = &(pod->KV_arr[KV_arr_index]);

		//nothing has every been read set the last read index to the oldest_value
		if (temp->last_read_index == -1 ){	
			temp->last_read_index = 0;
		}

		
		char * value = (char*)calloc(1, MAX_VALUE_SIZE);

		strcpy(value, temp->values[temp->last_read_index]);		//get value from the last read index of values 

		reader_update_keyval_booking_info(temp);		//increment value of the last_read_index

		return value;
	}

	else if (KV_arr_index < 0){		//Key does not exist

		return NULL;				//BAD :'(

	}
	
	else return NULL;

}


/*	Name: reader_update_keyval_booking_info
	Purpose: update the key val booking info
	Input: KV_object *kv_obj 
	Output: void
*/
void reader_update_keyval_booking_info(KV_object *kv_obj ){
	kv_obj->last_read_index++;	
	
	if(kv_obj->last_read_index  >= (MAX_NUM_VALUES - kv_obj->amt_val_space_avail)){		//if the last_read_index bleeds into the empty spaces then reset
		kv_obj->last_read_index = kv_obj->oldest_val_index;
	}

	return;
}




/*----------------------------------------------------------------------------------------------------------------
--------------------------------------- READ_ALL HELPERS--------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/


/*	NAME: read_all_from_store
	Purpose: read all values from a certain key
	Inputs: char *userKey, KV_store * storage
	Output: char ** (copy of the values at that key)
*/
char ** read_all_from_store(char *userKey, KV_store * storage){

	unsigned long pod_index = get_pod_index(hash ((unsigned char*) userKey));		//get pod index
	
	KV_pod * pod = &(storage->store[pod_index]);			

	int KV_arr_index = does_key_exist(pod, userKey);		//find the key in the specific pod

	
	if (KV_arr_index >= 0){			// Key does exist


		KV_object * temp = &(pod->KV_arr[KV_arr_index]); 	//get KV_object
		
		temp -> last_read_index = 0; 					//read all starting from the oldest

		//get a copy of the values
		return copy_of_values(temp->values, MAX_NUM_VALUES - temp->amt_val_space_avail, temp->last_read_index);


	}

	else if (KV_arr_index < 0){		//Key does not exist

		return NULL;				//BAD :O

	}

	else return NULL;
}



/*	name: copy of values
	Purpose: copy the values at that key and return to calling function
	Input: char ** original_values, int num_values
	Output: char ** copy of the original values
*/
char ** copy_of_values(char original_values[MAX_NUM_VALUES][MAX_VALUE_SIZE], int num_values, int start_index){

	char ** cpy_vals = calloc(num_values+1, sizeof(char*));
	for (int i = 0;i < num_values;i++){
			cpy_vals[i] = calloc(MAX_VALUE_SIZE, sizeof(char));
	}

	for (int i = start_index; i < num_values; i++){			//copying of values
		if(strcmp(original_values[i], "") ==0){ 
			continue;	//skip any null strings
		}
		else {
			strcpy(cpy_vals[i], original_values[i]);
			if(++i >= num_values) i = 0;
			--i;
			if(++i == start_index) break;
			--i;
		}
	}

	cpy_vals[num_values] = NULL;
	return cpy_vals;

}


/*--------------------------------------------------------------------
--------------------------- ERROR -------------------------------------
-----------------------------------------------------------------------*/
/*	Name: nullExit
	Purpose: print error with the function and line number where error occurred
	Input: char* function, int lineNum , char * msg
	Output: char* NULL (always)
*/
char* nullExit(char* function, int lineNum , char * msg) {
	char num_buff[20];
	sprintf(num_buff, "%d", lineNum);

	char str_buff[200];
	strcat(str_buff, function);
	strcat(str_buff, num_buff);
	strcat(str_buff, msg);

	perror(str_buff); 
	return NULL;
}


/*	Name: failExit
	Purpose: print error with the function and line number where error occurred
	Input: char* function, int lineNum , char * msg
	Output: int FAILURE (-1) (always)
*/
int failExit(char* function, int lineNum , char * msg) {
	char num_buff[20] = {'\0',};
	sprintf(num_buff, "%d", lineNum);


	char str_buff[200] = {'\0',};
	strcat(str_buff, function);
	strcat(str_buff, num_buff);
	strcat(str_buff, msg);

	perror(str_buff); 
	return FAILURE;
}

