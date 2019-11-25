#include "dbj_string_list.h"

typedef struct TEST_DBJ_DYNAMIC_METADATA_ {
	const unsigned number_of_iterations;
	const unsigned words_to_append;
	const char * word;
} TEST_DBJ_DYNAMIC_METADATA;

static void worker(TEST_DBJ_DYNAMIC_METADATA * test_descriptor)
{
	dbj_string_list_type head_ = dbj_string_list_new();
	int k;
	for (k = 0; k < test_descriptor->words_to_append; k++) {
		head_ = dbj_string_list_append(head_, test_descriptor->word);
	}
	head_ = dbj_string_list_reset(head_);
	dbj_string_list_free(head_);
}

void dbj_string_list_test( char (*output)[BUFSIZ] )
{
	TEST_DBJ_DYNAMIC_METADATA test_descriptor =
	{ .number_of_iterations = BUFSIZ * 10
	,.words_to_append = BUFSIZ
	,.word = "12345678" };

	time_t   start, finish;
	int k = 0;

	start = clock() / CLOCKS_PER_SEC;
	for (k = 0; k < test_descriptor.number_of_iterations; k++)
	{
		worker(&test_descriptor);
	}
	finish = clock() / CLOCKS_PER_SEC;

	double elapsed_time_sec = (finish - start);
	// double elapsed_time_msec = 1000.0 * (elapsed_time_sec);

	sprintf( *output, "\ndbj list test\n%d iterations, has taken\n\t%6.2f sec"
		"\nEach itrtation added the word '%s', %d times, and then destroyed the list\n\n",
		BUFSIZ, elapsed_time_sec,
		test_descriptor.word, test_descriptor.words_to_append);
}

