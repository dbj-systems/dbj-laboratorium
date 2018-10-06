#include "pch.h"
#include "../dbj++clib/dbjclib.h"
// 'hidden' inside dbj++clib
extern "C" void dbj_string_trim_test();

DBJ_TEST_UNIT(dbj_sll)
{
	// uses printf and thus UCRT "sometimes" fails
	/*
	C:\Program Files (x86)\Windows Kits\10\Source\10.0.17134.0\ucrt\inc\corecrt_internal_stdio.h

	ucrtbased.dll!__acrt_stdio_char_traits<char>::validate_stream_is_ansi_if_required(_iobuf * const stream) Line 440	C++
	*/
	// dbj::clib::test_dbj_sll();
}

DBJ_TEST_UNIT(dbj_string_c_lib)
{
	dbj::clib::dbj_string_test();
}

DBJ_TEST_UNIT(dbj_c_lib)
{
	using auto_char_arr = std::unique_ptr<char>;
	auto_char_arr to_be_freed_1{ dbj::clib::dbj_strdup("Mamma mia!?") };
	auto_char_arr to_be_freed_2{ dbj::clib::dbj_strndup(to_be_freed_1.get(), 5) };
	auto_char_arr to_be_freed_3{
		dbj::clib::dbj_str_shorten
		  ("Abra Ka dabra", " ")
	};
}

DBJ_TEST_UNIT(dbj_string_trim)
{
	// SEGV currently -- dbj_string_trim_test();
}

#define DBJ_ERR(n) DBJ::clib::dbj_error_service.create(__LINE__, __FILE__, n, nullptr)

DBJ_TEST_UNIT(dbj_err_system)
{
	// reaching to C code
	using namespace DBJ::clib;

	auto test = [](unsigned int err_num_) {
		using error_descriptor = DBJ::clib::error_descriptor;
		auto * err_desc_0 = DBJ_ERR(err_num_);

		_ASSERTE(DBJ::clib::dbj_error_service.is_valid_descriptor(err_desc_0));
		DBJ::clib::dbj_error_service.release(&err_desc_0);
		_ASSERTE(false == DBJ::clib::dbj_error_service.is_valid_descriptor(err_desc_0));
	};

	test(::dbj::clib::dbj_error_code::DBJ_EC_BAD_ERR_CODE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_BAD_STD_ERR_CODE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_DBJ_LAST_ERROR);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INDEX_TOO_LARGE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INDEX_TOO_SMALL);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INVALID_ARGUMENT);

	auto DBJ_MAYBE(dummy) = true;
}

/*
tuple append
*/
template <
	typename NEW_TYPE,
	typename... TUPLE_TYPES>
std::tuple<TUPLE_TYPES..., NEW_TYPE >
tuple_append
(const std::tuple<TUPLE_TYPES...> &tup, NEW_TYPE el)
{
	// pass el by value
	return std::tuple_cat(tup, std::make_tuple(el));
}

/*
above does receive values and native array references 
but no pointers please, bellow stops using string literals
*/
template < typename NEW_TYPE , typename... TUPLE_TYPES>
std::tuple<TUPLE_TYPES..., NEW_TYPE *>
tuple_append
(const std::tuple<TUPLE_TYPES...> &, const NEW_TYPE *) = delete;

DBJ_TEST_UNIT(test_tuple_append)
{
	using namespace std::string_view_literals;

	using dbj::console::print;
	int iar[]{0,1,2,3};
	auto t0 = std::make_tuple(iar);
	std::tuple<int, float> t1{ 1, 13.0f };
	auto nt1 = tuple_append(t1, iar);
	auto nt2 = tuple_append(nt1, "now this is surely tricky?"sv);
	print("\nThe resulting tuple:", nt2);
}

/* store function pointer and argument value
   this is how P.J.Plauger does it
template<class ARGTYPE>
struct fp_and_argument
{
	explicit
		fp_and_argument(
			void(__cdecl * fp)(std::string &, ARGTYPE), ARGTYPE _Val
		)
		: function_pointer(fp),
		argument_value(_Val)
	{	// construct from function pointer and argument type+value
		// for that function signature
	}

	void(__cdecl * function_pointer)(std::string&, ARGTYPE);	// the function pointer
	ARGTYPE argument_value;	// the argument value
};

Probably he has used this to cut the dependancy on the std::function
and this is DBJ variant C++17
*/

/*
holder of a single argument function and its argument *value*
*/
template<class RETURNTYPE, class ARGTYPE>
struct fp_one_arg final
{
	using argument_type = ARGTYPE;
	using return_type	= RETURNTYPE;
	using function_type = RETURNTYPE(__cdecl *)(argument_type);

	explicit
		fp_one_arg(
			function_type fp,
			argument_type _Val
		)
	: function_pointer(fp),	argument_value(_Val)
	{	// construct from function pointer and argument type+value
		// for the function signature required
	}

	// copy/move the retval
	return_type use() const noexcept {
		return function_pointer(argument_value);
	}

private :
	// return_type(__cdecl * function_pointer)(argument_type);	// the function pointer
	function_type function_pointer;	// the function pointer
	argument_type argument_value;	// the argument value
};

DBJ_TEST_UNIT(test_dbj_function_holder)
{
	using dbj::console::print;

	auto void_bool = [](const char * arg) -> const char * { return arg;  };
	/*
	first we instantiate a template for a concrete
	return type and argument type
	*/
	using function_pointer_holder = fp_one_arg<const char *, const char *>;
	/* now we declare a function that has to conform to the
	   signature of the function_pointer_holder
	   value_type and argument_type */
	[[maybe_unused]] void function_declaration(bool);
	/*
	   then we keep it
	*/
	function_pointer_holder fph(void_bool, "HOLA!");
	/*
	then we use it
	*/
	[[maybe_unused]] 
	function_pointer_holder::return_type rv = fph.use();

}
