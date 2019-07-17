#pragma once
#include "pch.h"

namespace dbj::samples {

	using namespace dbj::errc;

	DBJ_TEST_UNIT(dbj_errc_check_the_error_type_maleability)
	{
		error_type err = error_type::make(
			static_cast<error_type::id_type>(std::errc::argument_list_too_long),
			"Argument list too long"
		);
		error_type::locate( err, __LINE__, __FILE__ );
		dbj::fmt::print("\nerror in json format: %s", error_type::json(err).get());
	}

} // dbj::samples
