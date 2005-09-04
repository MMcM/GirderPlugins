/* This function parsers the code given in str and returns the bytecode. The length of the
   bytecode is given in len. */

#ifdef __cplusplus
} //extern "C" {
#endif

void * lua_bytecode ( char * str, int *len);
/* Don't forget to free the bytecode when you are done with it. */
void lua_freebytecode(void * bytecode);

#ifdef __cplusplus
} //extern "C" {
#endif
