/*

Copyright 2015 Jonathan Watmough
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jwHash.h"

#ifdef HASHTEST
#include <sys/time.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// STATIC HELPER FUNCTIONS

// http://stackoverflow.com/a/12996028
// hash function for int keys
static inline long int hashInt(long int x)
{
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x);
	return x;
}

// http://www.cse.yorku.ca/~oz/hash.html
// hash function for string keys djb2
static inline long int hashString(char * str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}

// helper for copying string keys and values
static inline char * copystring(char * value)
{
	char * copy = (char *)malloc(strlen(value)+1);
	if(!copy) {
		printf("Unable to allocate string value %s\n",value);
		abort();
	}
	strcpy(copy,value);
	return copy;
}


////////////////////////////////////////////////////////////////////////////////
// CREATING A NEW HASH TABLE

// Create hash table
jwHashTable *create_hash( size_t buckets )
{
	// allocate space
	jwHashTable *table= (jwHashTable *)malloc(sizeof(jwHashTable));
	if(!table) {
		// unable to allocate
		return NULL;
	}
	// setup
	table->bucket = (jwHashEntry **)malloc(buckets*sizeof(void*));
	if( !table->bucket ) {
		free(table);
		return NULL;
	}
	memset(table->bucket,0,buckets*sizeof(void*));
	table->buckets = table->bucketsinitial = buckets;
	HASH_DEBUG("table: %x bucket: %x\n",table,table->bucket);
	return table;
}

////////////////////////////////////////////////////////////////////////////////
// ADDING / DELETING / GETTING BY STRING KEY

// Add str to table - keyed by string
HASHRESULT add_str_by_str( jwHashTable *table, char *key, char *value )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	HASH_DEBUG("adding %s -> %s hash: %ld\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key) && 0==strcmp(value,entry->value.strValue))
			return HASHALREADYADDED;
		// check for replacing entry
		if(0==strcmp(entry->key.strValue,key) && 0!=strcmp(value,entry->value.strValue))
		{
			free(entry->value.strValue);
			entry->value.strValue = copystring(value);
			return HASHREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	HASH_DEBUG("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	HASH_DEBUG("new entry: %x\n",entry);
	entry->key.strValue = copystring(key);
	entry->valtag = HASHSTRING;
	entry->value.strValue = copystring(value);
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	HASH_DEBUG("added entry\n");
	return HASHOK;
}

HASHRESULT add_dbl_by_str( jwHashTable *table, char *key, double value )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	HASH_DEBUG("adding %s -> %f hash: %ld\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key) && value==entry->value.dblValue)
			return HASHALREADYADDED;
		// check for replacing entry
		if(0==strcmp(entry->key.strValue,key) && value!=entry->value.dblValue)
		{
			entry->value.dblValue = value;
			return HASHREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	HASH_DEBUG("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	HASH_DEBUG("new entry: %x\n",entry);
	entry->key.strValue = copystring(key);
	entry->valtag = HASHNUMERIC;
	entry->value.dblValue = value;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	HASH_DEBUG("added entry\n");
	return HASHOK;
}

HASHRESULT add_int_by_str( jwHashTable *table, char *key, long int value )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	HASH_DEBUG("adding %s -> %d hash: %ld\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key) && value==entry->value.intValue)
			return HASHALREADYADDED;
		// check for replacing entry
		if(0==strcmp(entry->key.strValue,key) && value!=entry->value.intValue)
		{
			entry->value.intValue = value;
			return HASHREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	HASH_DEBUG("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	HASH_DEBUG("new entry: %x\n",entry);
	entry->key.strValue = copystring(key);
	entry->valtag = HASHNUMERIC;
	entry->value.intValue = value;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	HASH_DEBUG("added entry\n");
	return HASHOK;
}

HASHRESULT add_ptr_by_str( jwHashTable *table, char *key, void *ptr )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	HASH_DEBUG("adding %s -> %x hash: %ld\n",key,ptr,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key) && ptr==entry->value.ptrValue)
			return HASHALREADYADDED;
		// check for replacing entry
		if(0==strcmp(entry->key.strValue,key) && ptr!=entry->value.ptrValue)
		{
			entry->value.ptrValue = ptr;
			return HASHREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	HASH_DEBUG("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	HASH_DEBUG("new entry: %x\n",entry);
	entry->key.strValue = copystring(key);
	entry->valtag = HASHPTR;
	entry->value.ptrValue = ptr;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	HASH_DEBUG("added entry\n");
	return HASHOK;
}

// Delete by string
HASHRESULT del_by_str( jwHashTable *table, char *key )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	HASH_DEBUG("deleting: %s hash: %ld\n",key,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	jwHashEntry *previous = NULL;
	
	// found an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key))
		{
			// skip first record, or one in the chain
			if(!previous)
				table->bucket[hash] = entry->next;
			else
				previous->next = entry->next;
			// delete string value if needed
			if( entry->valtag==HASHSTRING )
				free(entry->value.strValue);
			free(entry->key.strValue);
			free(entry);
			return HASHDELETED;
		}
		// move to next entry
		previous = entry;
		entry = entry->next;
	}
	return HASHNOTFOUND;
}

// Lookup str - keyed by str
HASHRESULT get_str_by_str( jwHashTable *table, char *key, char **value )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	HASH_DEBUG("fetching %s -> ?? hash: %d\n",key,hash);

	// get entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	while(entry)
	{
		// check for key
		HASH_DEBUG("found entry key: %d value: %s\n",entry->key.intValue,entry->value.strValue);
		if(0==strcmp(entry->key.strValue,key)) {
			*value =  entry->value.strValue;
			return HASHOK;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// not found
	return HASHNOTFOUND;
}

// Lookup int - keyed by str
HASHRESULT get_int_by_str( jwHashTable *table, char *key, int *i )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	HASH_DEBUG("fetching %s -> ?? hash: %d\n",key,hash);

	// get entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	while(entry)
	{
		// check for key
		HASH_DEBUG("found entry key: %s value: %ld\n",entry->key.strValue,entry->value.intValue);
		if(0==strcmp(entry->key.strValue,key)) {
			*i = entry->value.intValue;
			return HASHOK;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// not found
	return HASHNOTFOUND;
}

// Lookup dbl - keyed by str
HASHRESULT get_dbl_by_str( jwHashTable *table, char *key, double *val )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	HASH_DEBUG("fetching %s -> ?? hash: %d\n",key,hash);

	// get entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	while(entry)
	{
		// check for key
		HASH_DEBUG("found entry key: %s value: %f\n",entry->key.strValue,entry->value.dblValue);
		if(0==strcmp(entry->key.strValue,key)) {
			*val = entry->value.dblValue;
			return HASHOK;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// not found
	return HASHNOTFOUND;
}

////////////////////////////////////////////////////////////////////////////////
// ADDING / DELETING / GETTING BY LONG INT KEY

// Add to table - keyed by int
HASHRESULT add_str_by_int( jwHashTable *table, long int key, char *value )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	HASH_DEBUG("adding %d -> %s hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for already indexed
		if(entry->key.intValue==key && 0==strcmp(value,entry->value.strValue))
			return HASHALREADYADDED;
		// check for replacing entry
		if(entry->key.intValue==key && 0!=strcmp(value,entry->value.strValue))
		{
			free(entry->value.strValue);
			entry->value.strValue = copystring(value);
			return HASHREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	HASH_DEBUG("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	HASH_DEBUG("new entry: %x\n",entry);
	entry->key.intValue = key;
	entry->valtag = HASHSTRING;
	entry->value.strValue = copystring(value);
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	HASH_DEBUG("added entry\n");
	return HASHOK;
}

// Add dbl to table - keyed by int
HASHRESULT add_dbl_by_int( jwHashTable* table, long int key, double value )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	HASH_DEBUG("adding %d -> %f hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for already indexed
		if(entry->key.intValue==key && value==entry->value.dblValue)
			return HASHALREADYADDED;
		// check for replacing entry
		if(entry->key.intValue==key && value!=entry->value.dblValue)
		{
			entry->value.dblValue = value;
			return HASHREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	HASH_DEBUG("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	HASH_DEBUG("new entry: %x\n",entry);
	entry->key.intValue = key;
	entry->valtag = HASHNUMERIC;
	entry->value.dblValue = value;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	HASH_DEBUG("added entry\n");
	return HASHOK;
}

HASHRESULT add_int_by_int( jwHashTable* table, long int key, long int value )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	HASH_DEBUG("adding %d -> %d hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for already indexed
		if(entry->key.intValue==key && value==entry->value.intValue)
			return HASHALREADYADDED;
		// check for replacing entry
		if(entry->key.intValue==key && value!=entry->value.intValue)
		{
			entry->value.intValue = value;
			return HASHREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	HASH_DEBUG("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	HASH_DEBUG("new entry: %x\n",entry);
	entry->key.intValue = key;
	entry->valtag = HASHNUMERIC;
	entry->value.intValue = value;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	HASH_DEBUG("added entry\n");
	return HASHOK;
}


// Delete by int
HASHRESULT del_by_int( jwHashTable* table, long int key )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	HASH_DEBUG("deleting: %d hash: %d\n",key,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	jwHashEntry *prev = NULL;
	
	// found an entry
	HASH_DEBUG("entry: %x\n",entry);
	while(entry!=0)
	{
		HASH_DEBUG("checking entry: %x\n",entry);
		// check for entry to delete
		if(entry->key.intValue==key)
		{
			// skip first record, or one in the chain
			if(!prev)
				table->bucket[hash] = entry->next;
			else
				prev->next = entry->next;
			// delete string value if needed
			if( entry->valtag==HASHSTRING )
				free(entry->value.strValue);
			free(entry);
			return HASHDELETED;
		}
		// move to next entry
		prev = entry;
		entry = entry->next;
	}
	return HASHNOTFOUND;
}

// Lookup str - keyed by int
HASHRESULT get_str_by_int( jwHashTable *table, long int key, char **value )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	HASH_DEBUG("fetching %d -> ?? hash: %d\n",key,hash);

	// get entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	while(entry)
	{
		// check for key
		HASH_DEBUG("found entry key: %d value: %s\n",entry->key.intValue,entry->value.strValue);
		if(entry->key.intValue==key) {
			*value = entry->value.strValue;
			return HASHOK;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// not found
	return HASHNOTFOUND;
}

#ifdef HASHTEST
#define HASHCOUNT 1000000
int test()
{
	// create
	jwHashTable * table = create_hash(HASHCOUNT);
	
	// add a few values
	char * str1 = "string 1";
	char * str2 = "string 2";
	char * str3 = "string 3";
	char * str4 = "string 4";
	char * str5 = "string 5";
	add_str_by_int(table,0,str1);
	add_str_by_int(table,1,str2);
	add_str_by_int(table,2,str3);
	add_str_by_int(table,3,str4);
	add_str_by_int(table,4,str5);
	
	char * sstr1; get_str_by_int(table,0,&sstr1);
	char * sstr2; get_str_by_int(table,1,&sstr2);
	char * sstr3; get_str_by_int(table,2,&sstr3);
	char * sstr4; get_str_by_int(table,3,&sstr4);
	char * sstr5; get_str_by_int(table,4,&sstr5);
	printf("got strings: \n1->%s\n2->%s\n3->%s\n4->%s\n5->%s\n",sstr1,sstr2,sstr3,sstr4,sstr5);
	
	del_by_int(table, 0);
	del_by_int(table, 1);
	del_by_int(table, 2);
	del_by_int(table, 3);
	del_by_int(table, 4);
	
	// Test hashing by string
	printf("hashString: %s -> %ld\n","halp",hashString("halp"));
		
	char * strv1 = "Jonathan";
	char * strv2 = "Zevi";
	char * strv3 = "Jude";
	char * strv4 = "Voldemort";
	
	add_str_by_str(table,"oldest",strv1);
	add_str_by_str(table,"2ndoldest",strv2);
	add_str_by_str(table,"3rdoldest",strv3);
	add_str_by_str(table,"4tholdest",strv4);
	
	char * sstrv1; get_str_by_str(table,"oldest",&sstrv1);
	char * sstrv2; get_str_by_str(table,"2ndoldest",&sstrv2);
	char * sstrv3; get_str_by_str(table,"3rdoldest",&sstrv3);
	char * sstrv4; get_str_by_str(table,"4tholdest",&sstrv4);
	printf("got strings:\noldest->%s \n2ndoldest->%s \n3rdoldest->%s \n4tholdest->%s\n",
		sstrv1,sstrv2,sstrv3,sstrv4);
	


	// hash a million strings into various sizes of table
	struct timeval tval_before, tval_done1, tval_done2, tval_writehash, tval_readhash;
	gettimeofday(&tval_before, NULL);
	char buffer[512];
	int i;
	for(i=0;i<HASHCOUNT;++i) {
		sprintf(buffer,"%d",i);
		add_int_by_str(table,buffer,i);
	}
	gettimeofday(&tval_done1, NULL);
	int j;
	int error = 0;
	for(i=0;i<HASHCOUNT;++i) {
		sprintf(buffer,"%d",i);
		get_int_by_str(table,buffer,&j);
		if(i!=j) {
			printf("Error: %d != %d\n",i,j);
			error = 1;
		}
	}
	if(!error) {
		printf("No errors.\n"); 
	}
	gettimeofday(&tval_done2, NULL);
	timersub(&tval_done1, &tval_before, &tval_writehash);
	timersub(&tval_done2, &tval_done1, &tval_readhash);
	printf("Store %d ints by string: %ld.%06ld sec, read %d ints: %ld.%06ld sec\n",HASHCOUNT,
		(long int)tval_writehash.tv_sec, (long int)tval_writehash.tv_usec,HASHCOUNT,
		(long int)tval_readhash.tv_sec, (long int)tval_readhash.tv_usec);
	
	return 0;
}


int main(int argc, char *argv[])
{
	test();
	return 0;
}

#endif





