// the actual dbj++clib low level stuff

#include "dbjclib.h"


/*
strdup and strndup are defined in POSIX compliant systems as :

char *strdup(const char *str);
char *strndup(const char *str, size_t len);
*/
char * dbj_strdup(const char *s) {
	char *d = malloc(strlen(s) + 1);   // Space for length plus nul
	if (d == NULL) {
		errno = ENOMEM;
		return NULL;
	}         // No memory
	strcpy(d, s);                        // Copy the characters
	return d;                            // Return the new string
}

/*
The strndup() function copies at most len characters from the string str 
always null terminating the copied string.
*/
char * dbj_strndup(const char *s, size_t n)
{
	char *result = 0 ;
	size_t len = strlen(s);

	if (n < len)
		len = n;

	result = (char *)malloc(len + 1);
	if (result == NULL) {
		errno = ENOMEM;
		return NULL;
	}  // No memory

	result[len] = '\0';
	return (char *)memcpy(result, s, len);
}

// remove chars given in the string arg
// return the new string
// user has to free() eventually
// both arguments must be zero limited
char * dbj_str_shorten ( const char * str_, const char * chars_to_remove_ )
{
	assert(strchr(str_, 0));
	assert(strchr(chars_to_remove_, 0));

	size_t str_size = strlen(str_);
	// VLA 
	char vla_buff_[str_size];
	char * vla_buff_first = & vla_buff_[0] ;
	char * vla_buff_last  = & vla_buff_[str_size - 1] ;
	char * buff_p = vla_buff_first;
	for  (char * j = (char *)str_; *j != (char)0; j++) {
		if (0 == strchr(chars_to_remove_, *j))
		{
			*buff_p = *j; buff_p++;
		}
		// safety measure
		if (buff_p == (1 + vla_buff_last)) break; 
	}
	*buff_p = 0; 
	return dbj_strndup(vla_buff_, buff_p - vla_buff_);
}