// the actual dbj++clib low level stuff

#include "dbjclib.h"
#include <stdarg.h>

/*
last arg *must* be NULL
*/
void free_free_set_them_free(void * vp, ...)
{
	const size_t max_args = 255; size_t arg_count = 0;
	va_list marker;
	va_start(marker, vp); /* Initialize variable arguments. */
	while (vp != NULL)
	{
		free(vp);
		vp = NULL;
		vp = va_arg(marker, void*);
		/* feeble attempt to make it safer  */
		if (++arg_count == max_args) break;
	}
	va_end(marker);   /* Reset variable argument list. */
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
	char * vla_buff_ = calloc(str_size, 1 );
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
	char * retval = dbj_strndup(vla_buff_, buff_p - vla_buff_);
	free(vla_buff_);
	return retval;
}