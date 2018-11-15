/*------- READ_ME -----------------*/<br/>
Author: Leila Erbay<br/>
Purpose: Describe my approach on assignment 3 <br/>
Note: Please have mercy :D <br/>

If you look in the a2_lib.h you can see the structure of my store.<br/>
I took a different approach: each pod doesn't store a list of key value pairs but rather key value objects which I will explain below.


/*------------------------------------- DESCRIPTION OF STORE STRUCTURING ------------------------------------------------*/<br/>

A KV_store: <br/>
&nbsp;----- ATTRIBUTE -----------&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;------ PURPOSE OF ATTRIBUTE ---------------<br/>
&nbsp;1) KV_pod store[MAX_NUM_PODS] ---- an array of pods <br/>				
&nbsp;2) current_num_readers ---- an attribute for the purpose of the reader-writer problem <br/> 

A KV_pod:<br/> 
&nbsp;----- ATTRIBUTE -----------&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;------ PURPOSE OF ATTRIBUTE ---------------<br/>
&nbsp;1) KV_object KV_arr[MAX_NUM_KV_OBJECTS] ---- an array of key-value objects	<br/>
&nbsp;2) amt_KV_space_avail ---- an int to track the amount of empty spaces left for inserting key value objects<br/>	
&nbsp;3) next_empty_KV_index ---- an int to indicate the next empty spot to insert a new key value object<br/>
&nbsp;4) oldest_KV_index ---- an int to indicate which key value object was the first one to be added to that specific pod<br/>

A KV_object: <br/>
	----- ATTRIBUTE -----------&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;------ PURPOSE OF ATTRIBUTE ---------------<br/>
&nbsp;1) key[MAX_KEY_SIZE] ---- a char array that will hold the key that has been entered by the "user"<br/>
&nbsp;2) values[MAX_NUM_VALUES][MAX_VALUE_SIZE] ---- a char double array that will hold an amount of MAX_NUM_VALUES of values each of size MAX_VALUE_SIZE <br/>
&nbsp;3) amt_val_space_avail ---- an int to track the number of spaces left of MAX_NUM_VALUES to insert a new value<br/>
&nbsp;4) next_empty_val_index ---- an int to indicate which index in the values array is empty or to be written over<br/>
&nbsp;5) oldest_val_index ---- an int to indicate which index contains the first value that was added to that KV_object<br/>
&nbsp;6) last_read_index ---- an int to indicate which value was the last one to be read<br/>



/*------------------------------------------------ SELECTING SIZES -----------------------------------------------------*/
	
1) MAX_NUM_VALUES = 128	----- number of values per key ----- 256 was selected since that is the amount the tester uses<br/>
 *** NOTE: if MAX_NUM_VALUES is not a multiple of __TEST_MAX_POD_ENTRY__ (from comp310_a2_test.h) the FIFO test returns errors *** <br/>
<br/>

2) MAX_NUM_KV_OBJECTS = 20 ----- number of key-value objects per pod ----- 20 was selected at random<br/>
 *** NOTE: if MAX_NUM_KV_OBJECTS is less than 7 with __TEST_MAX_KEY__ (from comp310_a2_test.h) with being 256 FIFO will return errors *** <br/>
<br/>

3) MAX_NUM_PODS = 128 ----- number of pods in the store	----- it was said to keep number of pods half the size of the __TEST_MAX_KEY__<br/> 
<br/>

4) MAX_KEY_SIZE	= 32 ----- length / number of chars in a key ----- 32 was suggested <br/>
<br/>

5) MAX_VALUE_SIZE = 256	----- length / number of chars in a key ----- 256 was suggested<br/>
<br/>

6) TOTAL SIZE OF A KV_object = SIZEOF(key) + (4 * SIZEOF (int)) + (MAX_NUM_VALUES*SIZEOF(values)) = 32 + 16 + 128*256 = 32816 bytes<br/>
<br/>

7) TOTAL SIZE of KV_pod = (SIZEOF(KV_object) * MAX_NUM_KV_OBJECTS) + (3 * SIZEOF(int)) = 32812 * 20 + 12 = 656332 bytes<br/>
<br/>


8) TOTAL SIZE OF STORE =  MAX_NUM_PODS * SIZEOF(KV_pod) + SIZEOF(INT) = 128 * 656332 + 4 = 84010500 bytes = 80.1187 MB<br/>
  <br/>

/*------------------------------------------------- HANDLING COLLISIONS: THE IDEA ----------------------------------------*/
POD-LEVEL:<br/>
	- If pod is full, replace the oldest KV_object that has been placed in that pod with a new one.<br/>
	- This new KV_object placed in the pod will represent a fresh new KV_object such that the attributes are reset.<br/>
	- This new KV_object will hold the key and in the first place in its values array will hold the value that was entered with the key.<br/>
		*** NOTE: this means that I lose 5 values associated with the key that is being replaced ***	<br/>
	- I increment the oldest_KV_index so that the next one is now the oldest KV_object that will be replaced next.<br/>
<br/>
<br/>

KEY-VALUE-LEVEL:<br/>
	- If key is full, replace the oldest value that was placed in this key's values aray with the new one entered.<br/>
	- I increment the index oldest_val_index so that the next value is now the oldest so that it will be the next to be replaced.<br/>
<br/>

/*--------------------------------------------------------- SYNCHRONIZATION ---------------------------------------------*/ 
READ_COUNTER:<br/>
	- Is an attribute of the store and when I mmap I get a pointer of a store in VM and use this pointer to initialize and change the store<br/>
<br/>
LOCKING:<br/>
	- I lock the entire store<br/>


/*------------------------------------------------------- THE PROCEDURE: STEP BY STEP -----------------------------------*/

	a2_lib.c:<br/>
	KV_store*  storage_addr; ------ this will be used as a pointer to the shared memory <br/>
<br/>

	kv_store_create:<br/>
		1) open shared memory for the first time<br/>
		2) check if the shared memory exists and check failures with the file descriptor <br/>
		3) mmap and cast it to a KV_store pointer<br/>
		4) resize the shared memeory space<br/>
		5) init_store<br/>
			a2_helpers.c:<br/>
		
				init_store():<br/>
					1) memset the pods in the store<br/>
					2) set number of readers<br/>
					3) fill each pod and kv_object with necessary information<br/>
<br/>
	
		6) after setting store, unmap shared memory<br/>
		7) initiate semaphores 	(i now realize this part is not very necessary)<br/>
<br/>

	kv_store_write:<br/>
		1) open writer semaphore<br/>
		2) open shared memory<br/>
		3) mmap shared memory and cast<br/>
		5) lock writer sempahore 	<--- CRITICAL SECTION START<br/>
		6) write_to_store<br/>		
			a2_helpers.c:<br/>
			
				write_to_store(char * userKey, char* userValue, KV_store* storage):<br/>
				1) determine the pod to place the key (applies get_pod_index on hash function)<br/>
				2) truncate key and value if they don't match the specified sizes<br/>
				3) determine the index in the pod where the key value object should be placed <br/>
					
					(calls does_key_exist which loops through the key-value objects in the pod and <br/>						checks if a key in the pod matches the one entered by the user,<br/>
					if it exists return the index where it exists in the pod else it returns -1 )<br/>
				
				4) if key exists in the pod:<br/>
					
					1) writer_update_keyval(pod, key-val index, user's value)<br/>
						(checks if key is full:	<br/>
						if full: replace oldest value with user's value<br/>
						if not full: place user's value in the next empty index of the values array)<br/>
					2)writer_update_keval_booking_info(pod, key-val index)<br/>
						(updates the index where the oldest value exists, the next empty space for a <br/>						value and the amount of space available )<br/>
					RETURN SUCCESS
				
				5) if key DOES NOT exist in the pod:
					1) writer_update_pod(pod, userKey, userValue):
						
						[ checks if the pod is full:
						|	if pod is NOT full: 
						|		place key value object into the next empty space for key value objects	| 
						|		and update attributes related to that key value object			|
						|	if pod is full:									|
						|		replace the oldest key value object with the new key value object	|
						|		and update attributes related to key value object 			]

					2) writer_update_pod_booking_info(pod)
						
						[update the oldest_key value object if needed and the next empty index and amt of space	|
						| left in the pod									]

					RETURN SUCCESS
				
				6) if nothing worked return FAILURE (-1)

		7) unlock writer semaphore	<--- CRITICAL SECTION END
		8) close writer semaphore
		9) unmap shared memory
		10) return SUCCESS


	kv_store_read:
		1) open reader semaphore
		2) open writer semaphore
		3) open shared memory
		4) mmap shared memory
		5) lock reader semaphore	<--- CRITICAL SECTION START 
		6) increment the number of readers in shared memory
		7) lock writer semaphore if needed
		8) unlock reader semaphore 	<--- CRITICAL SECTION END

		9) read_single_from_store
		
			a2_helpers.c:
				read_single_from_store(key, KV_store *ptr):
					1) determine the pod to place the key (applies get_pod_index on hash function)
					2) determine if key-value object exists in the pod (calls does_key_exist)
					3) if key value object exists: 
						[ read the oldest value in key and place in a copy ]
					
						1) reader_update_keyval_booking_info(KV_object *temp):
							[ increment the last_read_index related to the key-value object ]
					
						2) return value copy
					4) if key value object doesn't exist return NULL


		10) lock reader semaphore	<--- CRITICAL SECTION START 
		11) decrement the number of readers
		12) unlock writer semaphore if needed
		13) unlock reader semaphore	<--- CRITICAL SECTION END
		14) close the file descriptor
		15) unmape the shared memory
		16) close the semaphores 
		17) return copy of the value



	kv_store_read_all:
		1) open reader semaphore
		2) open writer semaphore
		3) open shared memory
		4) mmap shared memory
		5) lock reader semaphore	<--- CRITICAL SECTION START 
		6) increment the number of readers in shared memory
		7) lock writer semaphore if needed
		8) unlock reader semaphore 	<--- CRITICAL SECTION END

		9) read_all_from_store
		
			a2_helpers.c:
				read_all_from_store(key, KV_store *ptr):
					1) determine the pod to place the key (applies get_pod_index on hash function)
					2) determine if key-value object exists in the pod (calls does_key_exist)
					3) if key_value object does exist:
						[get the key value object that the specified index in the pod]	
						reading from 0th position in values array
						copy values of the specified key-value object (calls copy_of_values)	
					return copy of values
				
					4) if key-value object does not exist return NULL
					
		10) lock reader semaphore	<--- CRITICAL SECTION START 
		11) decrement the number of readers
		12) unlock writer semaphore if needed
		13) unlock reader semaphore	<--- CRITICAL SECTION END
		14) close the file descriptor
		15) unmape the shared memory
		16) close the semaphores 
		17) return copy of the values
	
<br/>

Functions not referenced:<br/>
a2_helpers.c:<br/>
&nbsp;&nbsp;&nbsp;	nullExit: prints out the function name, line number of where error occurred and perror and returns NULL<br/>
&nbsp;&nbsp;&nbsp;	failExit: prints out the function name, line number of where error occurred and perror and returns FAILURE (-1)<br/>
			
	


















