////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// guards! guards!
#ifndef jwhash_h
#define jwhash_h

// needed for size_t
#include <stddef.h>

// resuts codes
typedef enum 
{
	JWHTADDED,
	JWHTREPLACEDVALUE,
	JWHTALREADYADDED,
	JWHTDELETED,
	JWHTNOTFOUND,
} JWHTRESULT;

typedef enum
{
    JWHTNUMERIC,
    JWHTSTRING,
} JWHTVALTAG;
    

typedef struct jwHashEntry jwHashEntry;
struct jwHashEntry
{
	union
	{
		char  *strValue;
		double dblValue;
		int    intValue;		
	} key;
    JWHTVALTAG valtag;
    union
	{
		char  *strValue;
		double dblValue;
		int    intValue;		
	} value;
	jwHashEntry *next;
};

typedef struct jwHashTable jwHashTable;
struct jwHashTable
{
	jwHashEntry **bucket;			// pointer to array of buckets
	size_t buckets;
	size_t bucketsinitial;			// if we resize, may need to hash multiple times
    JWHTRESULT lastError;
};

// Create hash table
jwHashTable *jwHashCreate( size_t buckets );

// Add to table - keyed by string
JWHTRESULT jwHashAddStrByStr( jwHashTable*, char *key, char *value );
JWHTRESULT jwHashAddDblByStr( jwHashTable*, char *key, double value );
JWHTRESULT jwHashAddIntByStr( jwHashTable*, char *key, long int value );

// Delete by string
JWHTRESULT jwHashDelByStr( jwHashTable*, char *key );

// Add to table - keyed by int
JWHTRESULT jwHashAddStrByInt( jwHashTable*, long int key, char *value );
JWHTRESULT jwHashAddDblByInt( jwHashTable*, long int key, double value );
JWHTRESULT jwHashAddIntByInt( jwHashTable*, long int key, long int value );

// Delete by int
JWHTRESULT jwHashDelByInt( jwHashTable*, long int key );

#endif



