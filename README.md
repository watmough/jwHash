# Simple hash table implementation for C.

I just wanted a simple and straightforward hash table implementation that I could drop into my
own C-based projects on whatever platform.

I haven't implemented one of these before, so it may be super naive, but it does appear to work pretty well.

NOTE: After exposure on HN, and seeing other hash implementations, I'm planning to restructure the code to a macro-based style, which should cut down the duplication.

## Features

You can create a hash table, and add strings, long ints, doubles and pointers to it, keyed by strings
or long ints.

You can retrieve strings, long ints, doubles and pointers via the get functions.

All strings saved in a hash table are copied, and copies of strings are returned on retrieval.

I added locking on hash buckets which only minorly affects performance, and allows safe retrieval and storing
of key value pairs.

Performance seems decent. Saving 1,000,000 int values by string key is done in about 0.28 secs, and multi-threaded performance scales pretty closely with number of processors.

![Performance Chart](http://jonathanwatmough.com/misc/jwHashPerformance.png)

## Building and Using

Type make in the folder to build the code. Type ./test to run the demo.

## References

The following were key to getting various aspects working:

Hash function for integer keys.
[http://stackoverflow.com/a/12996028](http://stackoverflow.com/a/12996028)

Hash function for string keys.
[http://www.cse.yorku.ca/~oz/hash.html](http://www.cse.yorku.ca/~oz/hash.html)

Efficient lock when low-contention is expected.
[http://stackoverflow.com/questions/1383363/is-my-spin-lock-implementation-correct-and-optimal](http://stackoverflow.com/questions/1383363/is-my-spin-lock-implementation-correct-and-optimal)

## Data Structures

### Hash Table

	typedef struct jwHashTable jwHashTable;
	struct jwHashTable
	{
		jwHashEntry **bucket;			// pointer to array of buckets
		size_t buckets;
		size_t bucketsinitial;			// if we resize, may need to hash multiple times
		HASHRESULT lastError;
	#ifdef HASHTHREADED
		volatile int *locks;			// array of locks
		volatile int lock;				// lock for entire table
	#endif
	};

### Hash Entry

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

## API

### Creating a Hash Table

	jwHashTable *create_hash( size_t buckets );
	void *delete_hash( jwHashTable *table );		// clean up all memory

### Storing by String Key

	HASHRESULT add_str_by_str( jwHashTable*, char *key, char *value );
	HASHRESULT add_int_by_str( jwHashTable*, char *key, long int value );
	HASHRESULT add_dbl_by_str( jwHashTable*, char *key, double value );
	HASHRESULT add_ptr_by_str( jwHashTable*, char *key, void *value );

### Deleting by String

	HASHRESULT del_by_str( jwHashTable*, char *key );

### Retrieving by String

	HASHRESULT get_str_by_str( jwHashTable *table, char *key, char **value );
	HASHRESULT get_int_by_str( jwHashTable *table, char *key, int *i );
	HASHRESULT get_dbl_by_str( jwHashTable *table, char *key, double *val );
	HASHRESULT get_ptr_by_str( jwHashTable *table, char *key, void **val );

[Similar for long int keys]

## TODO

1. Support multi-threading, -- this started, and implemented for the test
2. Implement clean-up,
3. Implement re-hashing to a larger hash table,
4. Implement a callback to allow iterating through keys, values


## Examples

### Example 1 - Save and Retrieve Some Values

	// Test hashing by string
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


### Example 2 - Write and Read Key Value Pairs on Multiple Threads

	#define NUMTHREADS 8
	#define HASHCOUNT 1000000

	typedef struct threadinfo {jwHashTable *table; int start;} threadinfo;
	void * thread_func(void *arg)
	{
		threadinfo *info = arg;
		char buffer[512];
		int i = info->start;
		jwHashTable *table = info->table;
		free(info);
		for(;i<HASHCOUNT;i+=NUMTHREADS) {
			sprintf(buffer,"%d",i);
			add_int_by_str(table,buffer,i);
		}
	}


	int thread_test()
	{
		// create
		jwHashTable * table = create_hash(HASHCOUNT>>2);

		// hash a million strings into various sizes of table
		struct timeval tval_before, tval_done1, tval_done2, tval_writehash, tval_readhash;
		gettimeofday(&tval_before, NULL);
		int t;
		pthread_t * threads[NUMTHREADS];
		for(t=0;t<NUMTHREADS;++t) {
			pthread_t * pth = malloc(sizeof(pthread_t));
			threads[t] = pth;
			threadinfo *info = (threadinfo*)malloc(sizeof(threadinfo));
			info->table = table; info->start = t;
			pthread_create(pth,NULL,thread_func,info);
		}
		for(t=0;t<NUMTHREADS;++t) {
			pthread_join(*threads[t], NULL);
		}
		gettimeofday(&tval_done1, NULL);
		int i,j;
		int error = 0;
		char buffer[512];
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
		printf("\n%d threads.\n",NUMTHREADS);
		printf("Store %d ints by string: %ld.%06ld sec, read %d ints: %ld.%06ld sec\n",HASHCOUNT,
			(long int)tval_writehash.tv_sec, (long int)tval_writehash.tv_usec,HASHCOUNT,
			(long int)tval_readhash.tv_sec, (long int)tval_readhash.tv_usec);
	
		return 0;
	}










