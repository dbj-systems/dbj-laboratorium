#pragma once

// #include "dbj_kalends.h"
DBJ_TEST_SPACE_OPEN(dbj_kalends_testing)

using namespace std;

/*
  first we declare return value type we will use
*/
template< typename V_ >
using retval_type = dbj::errc::error_concept< V_, dbj::errc::error_type >;

/* imaginary errors repository */
namespace e_repo
{
	using e = dbj::errc::error_type;
	// _buff UDL lives here
	using namespace dbj::errc::varnish;

	constexpr inline auto BASE_ERR_ID = 1000;

	inline const array errors{
		e{ { BASE_ERR_ID + 0, "Unknown error"_buff }},
		e{ { BASE_ERR_ID + 1, "Well, this is weird?"_buff }},
		e{ { BASE_ERR_ID + 2, "Has anybody seen this?"_buff }},
		e{ { BASE_ERR_ID + 3, "Divisor 0 has no meaing"_buff }}
	};

	// search by 1,2,3 ... 0 is "unknown error"
	inline e query_by_id(e::id_type find_this_id_)
	{
		e rt_ = errors[0]; // the unknown

		for (auto& el_ : errors) {
			if (el_.error.id == (BASE_ERR_ID + find_this_id_))
				return el_;
		}
		return rt_;
	}
} // e_repo

/*
now we use the above

for int's we decalre reqiored return value type
*/
using int_retval = retval_type<int>;

inline int_retval::return_type
	divider(int number, int divisor)
	{
	if (divisor == 0) {
		// first we need to create/find the error object
		auto e_ = e_repo::query_by_id(3);
		// second we need to add the location to it
		e_.location = dbj::errc::helpers::idmessage_make ( __LINE__, __FILE__ );
		// third we create and return the retval 
		// with error info only
		return int_retval::make_err(e_);
	}

	int result_ = number / divisor;
	//  create and return the retval 
	// with value only, no error info only
	return int_retval::make_val(result_);
	}

DBJ_TEST_UNIT(dbj_timers_) 
{
	auto & print = ::dbj::console::print;
	/* here we consume the return value */
	auto [value_, error_] = divider(1, 0);

	if (error_) 
	{
		auto err = *error_;
		print("\nError returned",
			"\nid: ",  err.error.id,
			"\nmessage: ", err.error.message,
			"\nlocation",
			"\nline: ", err.location.id,
			"\nfile: ", err.location.message
		);
	}

	if (value_) {
		print("\nResult: ", *value_);
	}

}

DBJ_TEST_SPACE_CLOSE