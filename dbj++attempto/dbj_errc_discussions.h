#pragma once
#include "pch.h"

namespace dbj::samples {

	using namespace dbj::errc;
	using namespace dbj::fmt;

	DBJ_TEST_UNIT(dbj_errc_check_the_error_type_maleability)
	{
		error_type err = error_type::make(
			static_cast<error_type::id_type>(std::errc::argument_list_too_long),
			"Argument list too long"
		);
		print("\nno location\n\nerror json format: %s", error_type::json(err).get());
		error_type::locate(err, __LINE__, __FILE__);
		print("\n\nwith location\n\nerror json format: %s", error_type::json(err).get());
	}

	/*
	create return type for testing in here
	*/
	using my_errc_type = 
		dbj::errc::error_concept< std::int64_t, dbj::errc::error_type >;
	/*
	create error instances for testing in here
	in reality users will have them in namespaces

	notice how these predefined instances have no location in them

	notice we start id's from 1001
	this is to try and not clash with POSIX std::errc
	*/
	inline error_type divide_by_zero =
		error_type::make( 1001,"Divide by Zero" );

	inline error_type integer_divide_overflows =
		error_type::make( 1002,"Integer Divide Overflows" );

	inline error_type not_integer_division =
		error_type::make( 1002,"Non Integer Division" );
	/*
	canonical example from 
	http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0709r0.pdf
	page 21 onwards
	*/
	inline my_errc_type::return_type safe_divide(
		my_errc_type::value_type i, 
		my_errc_type::value_type j) 
	{
		if (j == 0) {
			auto & locerr = error_type::locate(divide_by_zero, __LINE__, __FILE__);
			return my_errc_type::make_err( locerr );
		}

		if (i == INT_MIN && j == -1)
			return my_errc_type::make_err(error_type::locate(integer_divide_overflows, __LINE__, __FILE__));

		if (i % j != 0)
			return my_errc_type::make_err(error_type::locate(not_integer_division, __LINE__, __FILE__));

		return my_errc_type::make_val( i / j );
	}
	DBJ_TEST_UNIT(dbj_errc_check_the_errc_maleability)
	{
		auto [v, e] = safe_divide(8, 2);
	}

} // dbj::samples
