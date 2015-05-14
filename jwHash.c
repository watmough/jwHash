////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jwHash.h"

// Create hash table
jwHashTable *jwHashCreate( size_t buckets )
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
	printf("table: %x bucket: %x\n",table,table->bucket);
	return table;
}

// http://stackoverflow.com/a/12996028
// hash function for int keys
static long int hashInt(long int x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

// http://www.cse.yorku.ca/~oz/hash.html
// hash function for string keys djb2
static long int hashString(char * str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

// helper for copying string keys and values
static char * copystring(char * value)
{
	char * copy = (char *)malloc(strlen(value)+1);
	if(!copy) {
		printf("Unable to allocate string value %s\n",value);
		abort();
	}
	strcpy(copy,value);
	return copy;
}

// Add to table - keyed by string
JWHTRESULT jwHashAddStrByStr( jwHashTable *table, char *key, char *value )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	printf("adding %s -> %s hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	printf("entry: %x\n",entry);
	while(entry!=0)
	{
		printf("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key) && 0==strcmp(value,entry->value.strValue))
			return JWHTALREADYADDED;
		// check for replacing entry
		if(0==strcmp(entry->key.strValue,key) && 0!=strcmp(value,entry->value.strValue))
		{
			free(entry->value.strValue);
			entry->value.strValue = copystring(value);
			return JWHTREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	printf("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	printf("new entry: %x\n",entry);
	entry->key.strValue = copystring(key);
    entry->valtag = JWHTSTRING;
	entry->value.strValue = copystring(value);
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	printf("added entry\n");
	return JWHTADDED;
}

JWHTRESULT jwHashAddDblByStr( jwHashTable *table, char *key, double value )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	printf("adding %s -> %f hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	printf("entry: %x\n",entry);
	while(entry!=0)
	{
		printf("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key) && value==entry->value.dblValue)
			return JWHTALREADYADDED;
		// check for replacing entry
		if(0==strcmp(entry->key.strValue,key) && value!=entry->value.dblValue)
		{
			entry->value.dblValue = value;
			return JWHTREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	printf("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	printf("new entry: %x\n",entry);
	entry->key.strValue = copystring(key);
    entry->valtag = JWHTNUMERIC;
	entry->value.dblValue = value;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	printf("added entry\n");
	return JWHTADDED;
}

JWHTRESULT jwHashAddIntByStr( jwHashTable *table, char *key, long int value )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	printf("adding %s -> %d hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	printf("entry: %x\n",entry);
	while(entry!=0)
	{
		printf("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key) && value==entry->value.intValue)
			return JWHTALREADYADDED;
		// check for replacing entry
		if(0==strcmp(entry->key.strValue,key) && value!=entry->value.intValue)
		{
			entry->value.intValue = value;
			return JWHTREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	printf("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	printf("new entry: %x\n",entry);
	entry->key.strValue = copystring(key);
    entry->valtag = JWHTNUMERIC;
	entry->value.intValue = value;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	printf("added entry\n");
	return JWHTADDED;
}

// Delete by string
JWHTRESULT jwHashDelByStr( jwHashTable *table, char *key )
{
	// compute hash on key
	size_t hash = hashString(key) % table->buckets;
	printf("deleting: %s hash: %d\n",key,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	jwHashEntry *previous = NULL;
	
	// found an entry
	printf("entry: %x\n",entry);
	while(entry!=0)
	{
		printf("checking entry: %x\n",entry);
		// check for already indexed
		if(0==strcmp(entry->key.strValue,key))
        {
            // skip first record, or one in the chain
            if(!previous)
                table->bucket[hash] = entry->next;
            else
                previous->next = entry->next;
            // delete string value if needed
            if( entry->valtag==JWHTSTRING )
                free(entry->value.strValue);
            free(entry->key.strValue);
            free(entry);
			return JWHTDELETED;
        }
		// move to next entry
        previous = entry;
		entry = entry->next;
	}
    return JWHTNOTFOUND;
}

// Add to table - keyed by int
JWHTRESULT jwHashAddStrByInt( jwHashTable *table, long int key, char *value )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	printf("adding %d -> %s hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	printf("entry: %x\n",entry);
	while(entry!=0)
	{
		printf("checking entry: %x\n",entry);
		// check for already indexed
		if(entry->key.intValue==key && 0==strcmp(value,entry->value.strValue))
			return JWHTALREADYADDED;
		// check for replacing entry
		if(entry->key.intValue==key && 0!=strcmp(value,entry->value.strValue))
		{
			free(entry->value.strValue);
			entry->value.strValue = copystring(value);
			return JWHTREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	printf("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	printf("new entry: %x\n",entry);
	entry->key.intValue = key;
    entry->valtag = JWHTSTRING;
	entry->value.strValue = copystring(value);
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	printf("added entry\n");
	return JWHTADDED;
}

// Add to table - keyed by int
JWHTRESULT jwHashAddDblByInt( jwHashTable* table, long int key, double value )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	printf("adding %d -> %f hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	printf("entry: %x\n",entry);
	while(entry!=0)
	{
		printf("checking entry: %x\n",entry);
		// check for already indexed
		if(entry->key.intValue==key && value==entry->value.dblValue)
			return JWHTALREADYADDED;
		// check for replacing entry
		if(entry->key.intValue==key && value!=entry->value.dblValue)
		{
			entry->value.dblValue = value;
			return JWHTREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	printf("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	printf("new entry: %x\n",entry);
	entry->key.intValue = key;
    entry->valtag = JWHTNUMERIC;
	entry->value.dblValue = value;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	printf("added entry\n");
	return JWHTADDED;
}

JWHTRESULT jwHashAddIntByInt( jwHashTable* table, long int key, long int value )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	printf("adding %d -> %d hash: %d\n",key,value,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	printf("entry: %x\n",entry);
	while(entry!=0)
	{
		printf("checking entry: %x\n",entry);
		// check for already indexed
		if(entry->key.intValue==key && value==entry->value.intValue)
			return JWHTALREADYADDED;
		// check for replacing entry
		if(entry->key.intValue==key && value!=entry->value.intValue)
		{
			entry->value.intValue = value;
			return JWHTREPLACEDVALUE;
		}
		// move to next entry
		entry = entry->next;
	}
	
	// create a new entry and add at head of bucket
	printf("creating new entry\n");
	entry = (jwHashEntry *)malloc(sizeof(jwHashEntry));
	printf("new entry: %x\n",entry);
	entry->key.intValue = key;
    entry->valtag = JWHTNUMERIC;
	entry->value.intValue = value;
	entry->next = table->bucket[hash];
	table->bucket[hash] = entry;
	printf("added entry\n");
	return JWHTADDED;
}


// Delete by int
JWHTRESULT jwHashDelByInt( jwHashTable* table, long int key )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	printf("deleting: %d hash: %d\n",key,hash);

	// add entry
	jwHashEntry *entry = table->bucket[hash];
	jwHashEntry *prev = NULL;
	
	// found an entry
	printf("entry: %x\n",entry);
	while(entry!=0)
	{
		printf("checking entry: %x\n",entry);
		// check for entry to delete
		if(entry->key.intValue==key)
        {
            // skip first record, or one in the chain
            if(!prev)
                table->bucket[hash] = entry->next;
            else
                prev->next = entry->next;
            // delete string value if needed
            if( entry->valtag==JWHTSTRING )
                free(entry->value.strValue);
            free(entry);
			return JWHTDELETED;
        }
		// move to next entry
        prev = entry;
		entry = entry->next;
	}
    return JWHTNOTFOUND;
}

// Add to table - keyed by int
char * jwHashGetStrByInt( jwHashTable *table, long int key )
{
	// compute hash on key
	size_t hash = hashInt(key) % table->buckets;
	printf("fetching %d -> ?? hash: %d\n",key,hash);

	// get entry
	jwHashEntry *entry = table->bucket[hash];
	
	// already an entry
	while(entry)
	{
		// check for key
		printf("found entry key: %d value: %s\n",entry->key.intValue,entry->value.strValue);
		if(entry->key.intValue==key)
			return entry->value.strValue;
		// move to next entry
		entry = entry->next;
	}
	
	// not found
	return NULL;
}

int main(int argc, char *argv[])
{
	// create
	jwHashTable * table = jwHashCreate(1);
	
	// add a few values
	char * str1 = "string 1";
	char * str2 = "string 2";
	char * str3 = "string 3";
	char * str4 = "string 4";
	char * str5 = "string 5";
	jwHashAddStrByInt(table,0,str1);
	jwHashAddStrByInt(table,1,str2);
	jwHashAddStrByInt(table,2,str3);
	jwHashAddStrByInt(table,3,str4);
	jwHashAddStrByInt(table,4,str5);
	
	char * sstr1 = jwHashGetStrByInt(table,0);
	char * sstr2 = jwHashGetStrByInt(table,1);
	char * sstr3 = jwHashGetStrByInt(table,2);
	char * sstr4 = jwHashGetStrByInt(table,3);
	char * sstr5 = jwHashGetStrByInt(table,4);
	printf("got strings: 1->%s\n2->%s\n3->%s\n4->%s\n5->%s\n",sstr1,sstr2,sstr3,sstr4,sstr5);
	return 0;
}


