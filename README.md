# Simple hash table implementation for C.

I just wanted a simple and straightforward hash table implementation that I could drop into my
own C-based projects on whatever platform.

I haven't implemented one of these before, so it may be super naive, but it does appear to work and
my implementation choices appear reasonable.

## Features

You can create a hash table, and add strings, long ints, doubles and pointers to it, keyed by strings
or long ints.

You can retrieve strings, long ints, doubles and pointers via the get functions.

All strings saved in a hash table are copied, and copies of strings are returned on retrieval.

## TODO

1. Support multi-threading,
2. Implement clean-up,
3. Implement re-hashing to a larger hash table,
4. Implement a callback to allow iterating through keys, values

## API



## Example


