#include "pch.h"
#define TEST_DBJ_DYNAMIC
#define DBJ_STRING_TEST
#include "../dbj++clib/dbjclib.h"
// #include "../dbj_en_dictionary/dbj_en_dictionary.h"

using namespace ::std::string_view_literals;

DBJ_TEST_UNIT(dbj_string_c_lib)
{
#ifdef TEST_DBJ_DYNAMIC
	char buf_here[BUFSIZ]{0};
	dbj_string_list_test( & buf_here);
	::dbj::console::prinf("%s", buf_here);
#endif
	dbj_string_test();
}
#undef TEST_DBJ_DYNAMIC

#define DBJ_SLL_TESTING
#include "../dbj++clib/dbj_sll/dbj_sll.h"

#ifdef DBJ_SLL_TESTING

extern "C" inline  
void test_dbj_sll_local (const char * what_to_append, size_t how_many_times, bool verbose)
{

	dbj_sll_node * head_ = dbj_sll_make_head();

	while (1 < how_many_times--) dbj_sll_append(head_, what_to_append);

	if (verbose) {
		printf("\nDBJ SLL dump");
		dbj_sll_foreach(head_, dbj_sll_node_dump_visitor);
		printf("\n");
	}

	assert(0 == strcmp(dbj_sll_remove_tail(head_)->data, what_to_append));
	dbj_sll_erase(head_);
	if (verbose) {
		printf("\nHead after SLL erasure");
		dbj_sll_node_dump_visitor(head_);
	}
	assert(true == is_dbj_sll_empty(head_));

	unsigned long k1 = dbj_sll_append(head_, "Abra")->key;
	unsigned long k2 = dbj_sll_append(head_, "Ka")->key;
	unsigned long k3 = dbj_sll_append(head_, "Dabra")->key;

	(void)(sizeof(k1)); (void)(sizeof(k3)); // combat the no warning hassle

	dbj_sll_node * node_ = dbj_sll_find(head_, k2);
	assert(0 == strcmp(node_->data, "Ka"));
	assert(false == is_dbj_sll_empty(head_));
	dbj_sll_erase_with_head(head_);
}
#endif /*DBJ_SLL_TESTING */



DBJ_TEST_UNIT(dbj_c_lib_sll)
{
	auto test = []() {
		for (int k = 0; k < BUFSIZ; k++)
		{
			test_dbj_sll_local("1234567812345678", BUFSIZ, false);
		}
	};

	dbj::fmt::print("\ndbj clib SLL test finished in %s "sv,
		::dbj::kalends::miliseconds_measure(test)
	);
}
#undef DBJ_SLL_TESTING

DBJ_TEST_UNIT(dbj_c_lib_strndup_test)
{
	using auto_char_arr = std::unique_ptr<char>;

	auto dbj_strndup_test = []() {
		for (int k = 0; k < BUFSIZ; k++)
		{
		auto_char_arr to_be_freed_1{  dbj_strdup("Mamma mia!?") };
		_ASSERTE(dbj::dbj_ordinal_string_compareA(to_be_freed_1.get(), "Mamma mia!?", true));

		auto_char_arr to_be_freed_2{  dbj_strndup(to_be_freed_1.get(), 5) };
		_ASSERTE(dbj::dbj_ordinal_string_compareA(to_be_freed_2.get(), "Mamma", true));

		auto_char_arr to_be_freed_3{ dbj_str_shorten("Abra Ka dabra", " ")	};
		_ASSERTE(dbj::dbj_ordinal_string_compareA(to_be_freed_3.get(), "AbraKadabra", true));
		}
	};

	dbj::fmt::print("\ndbj clib dbj strndup test finished in %s"sv,
	::dbj::kalends::miliseconds_measure(dbj_strndup_test)
		) ;
}

// 'hidden' inside dbj++clib
extern "C" void dbj_string_trim_test();
DBJ_TEST_UNIT(dbj_string_trim)
{
	dbj_string_trim_test();
}

#ifdef DBJ_CLIB_ERR_CONCEPT

#define DBJ_ERR(n) dbj_error_service.create(__LINE__, __FILE__, n, nullptr)

DBJ_TEST_UNIT(dbj_err_system)
{
	auto test = [](unsigned int err_num_) {
		using error_descriptor = error_descriptor;
		auto * err_desc_0 = DBJ_ERR(err_num_);

		_ASSERTE(dbj_error_service.is_valid_descriptor(err_desc_0));
		dbj_error_service.release(&err_desc_0);
		_ASSERTE(false == dbj_error_service.is_valid_descriptor(err_desc_0));
	};

	test(dbj_error_code::DBJ_EC_BAD_ERR_CODE);
	test(dbj_error_code::DBJ_EC_BAD_STD_ERR_CODE);
	test(dbj_error_code::DBJ_EC_DBJ_LAST_ERROR);
	test(dbj_error_code::DBJ_EC_INDEX_TOO_LARGE);
	test(dbj_error_code::DBJ_EC_INDEX_TOO_SMALL);
	test(dbj_error_code::DBJ_EC_INVALID_ARGUMENT);

	auto DBJ_MAYBE(dummy) = true;
}
#endif // DBJ_CLIB_ERR_CONCEPT
