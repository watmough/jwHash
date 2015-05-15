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
	HASHOK,
	HASHADDED,
	HASHREPLACEDVALUE,
	HASHALREADYADDED,
	HASHDELETED,
	HASHNOTFOUND,
} HASHRESULT;

typedef enum
{
	HASHPTR,
	HASHNUMERIC,
	HASHSTRING,
} HASHVALTAG;
	

typedef struct jwHashEntry jwHashEntry;
struct jwHashEntry
{
	union
	{
		char  *strValue;
		double dblValue;
		int	   intValue;
	} key;
	HASHVALTAG valtag;
	union
	{
		char  *strValue;
		double dblValue;
		int	   intValue;
		void  *ptrValue;
	} value;
	jwHashEntry *next;
};

typedef struct jwHashTable jwHashTable;
struct jwHashTable
{
	jwHashEntry **bucket;			// pointer to array of buckets
	size_t buckets;
	size_t bucketsinitial;			// if we resize, may need to hash multiple times
	HASHRESULT lastError;
};

// Create/delete hash table
jwHashTable *create_hash( size_t buckets );
void *delete_hash( jwHashTable *table );		// clean up all memory


// Add to table - keyed by string
HASHRESULT add_str_by_str( jwHashTable*, char *key, char *value );
HASHRESULT add_dbl_by_str( jwHashTable*, char *key, double value );
HASHRESULT add_int_by_str( jwHashTable*, char *key, long int value );
HASHRESULT add_ptr_by_str( jwHashTable*, char *key, void *value );

// Delete by string
HASHRESULT del_by_str( jwHashTable*, char *key );

// Get by string
HASHRESULT get_str_by_str( jwHashTable *table, char *key, char **value );
HASHRESULT get_int_by_str( jwHashTable *table, char *key, int *i );
HASHRESULT get_dbl_by_str( jwHashTable *table, char *key, double *val );


// Add to table - keyed by int
HASHRESULT add_str_by_int( jwHashTable*, long int key, char *value );
HASHRESULT add_dbl_by_int( jwHashTable*, long int key, double value );
HASHRESULT add_int_by_int( jwHashTable*, long int key, long int value );
HASHRESULT add_ptr_by_int( jwHashTable*, long int key, void *value );

// Delete by int
HASHRESULT del_by_int( jwHashTable*, long int key );

// Get by int
HASHRESULT get_str_by_int( jwHashTable *table, long int key, char **value );
HASHRESULT get_int_by_int( jwHashTable *table, long int key, int *i );
HASHRESULT get_dbl_by_int( jwHashTable *table, long int key, double *val );

#endif



