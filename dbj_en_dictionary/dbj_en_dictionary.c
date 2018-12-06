
#include "dbj_en_dictionary.h"

// null_cb_argument[0] == 0 
static char * null_cb_argument[] = { { 0 } };
static char ZERO_CHAR = '\0';
static dbj_en_dictionary_retval ok_retval(const char * msg) {
	static dbj_en_dictionary_retval ok_retval_ = { dbj_en_dictionary_OK, "OK" };
	if (msg) ok_retval_.message = msg;
	return ok_retval_;
}
static dbj_en_dictionary_retval err_retval(const char * msg) {
	static dbj_en_dictionary_retval err_retval_ = { dbj_en_dictionary_ERR, "ERROR" };
	if (msg) err_retval_.message = msg;
	return err_retval_;
}
static dbj_en_dictionary_retval eof_retval(const char * msg) {
	static dbj_en_dictionary_retval eof_retval_ = { dbj_en_dictionary_EOF, "EOF disctionary file" };
	if (msg) eof_retval_.message = msg;
	return eof_retval_;
}

static FILE * dictionary_file = 0;

// arg is the word to be found
// querying by prefix is allowed
// arg is populated with all the words found
// 
//  char * args[] = {"al"};
//  dbj_en_dictionary_service.find( args ) ;
//  
// minimum length of the prefix is 2!
// maximum length of the prefix is 64!
//
// afterward this function leave the dictionary in a closed state!
//
static dbj_en_dictionary_retval find_(const char * query_ , dbj_en_dictionary_callback cb_ )
{
	static        char * retval[] = { {0} };
	static     size_t words_found = 0;

	for (size_t n = 0; n < words_found; ++n) {
		FREE( retval[n] );
	}
	words_found = 0;
	*retval = (char *)realloc(retval, words_found + 1);

	dbj_en_dictionary_retval fun_retval = dbj_en_dictionary_service.reset(cb_);

	if (fun_retval.status != dbj_en_dictionary_OK)
	{
		return fun_retval;
	}
	// NOTE! this requires dictionary text file to be sorted
	char word[dbj_en_dictionary_BUFISZ] = { 0 };
	while (true) {
		memset(word, ZERO_CHAR, dbj_en_dictionary_BUFISZ);
		fscanf(dictionary_file, "%s", word);

		if (strstr(word, query_)) 
		{
			*retval = (char *)realloc(retval, words_found+1);
			retval[words_found] = _strdup(word) ;
			words_found++;
		}
		else {
			break;
		}
	}

		// TODO: not dealing with
		// the possible !OK event on close	
		dbj_en_dictionary_service.close(cb_);
		cb_(retval);
	return ok_retval(0);
}

// reopen the file
// callback is called with error message or empty string
static dbj_en_dictionary_retval reset_(dbj_en_dictionary_callback cb_)
{
	dbj_en_dictionary_service.close(cb_);
	dictionary_file = fopen( ".\\" dbj_en_dictionary_file, "r");
	if (dictionary_file == NULL)
	{
		return err_retval("could not open " dbj_en_dictionary_file " for reading");
	}

	cb_(null_cb_argument);
	return ok_retval(0);
}
// callback 
// arg is populated with next word read from file
// on file end, dbj_en_dictionary_status.EOF is returned
// callback is called with NULL
dbj_en_dictionary_retval next_(dbj_en_dictionary_callback cb_)
{
	static char * retval[] = { {0} };
	if (dictionary_file == NULL)
	{
		return err_retval(dbj_en_dictionary_file " not open for reading");
	}

	char word[dbj_en_dictionary_BUFISZ] = { 0 };
		memset(word, ZERO_CHAR, dbj_en_dictionary_BUFISZ);
			fscanf(dictionary_file, "%s", word);
		retval[0] = word;
		cb_(null_cb_argument);
	return ok_retval(0);
}
// callback is called with error message or empty string
dbj_en_dictionary_retval close_ (dbj_en_dictionary_callback cb_)
{
	// TODO: fclose can fail
	if ( dictionary_file != NULL)
			fclose(dictionary_file);
		dictionary_file = NULL;
	return ok_retval( dbj_en_dictionary_file " closed");
}

// the one and only
dbj_en_dictionary_type dbj_en_dictionary_service 
= { find_, reset_, next_, close_ };