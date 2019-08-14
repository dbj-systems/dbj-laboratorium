#pragma once
#include "dbj_status_code.h"


/*
		dbjdbj: "The future is bright", one just needs to wait for it"

		P1095
		http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2289.pdf

		int safe_divide(int i, int j) fails(arithmetic_errc) {
		if (j == 0)
		return failure(arithmetic_errc::divide_by_zero);
		if (i == INT_MIN && j == -1)
		return failure(arithmetic_errc::integer_divide_overflows);
		if (i % j != 0)
		return failure(arithmetic_errc::not_integer_division);
		else return i / j;
		}

		double caller(double i, double j, double k) throws {
		return i + safe_divide(j, k);
		}

		we use "dbj_" prefix so when C++23 arrives we can easily search/replace
		*OR* run without changes since we will not have a clash with
		new C++23 keywords
	*/

// control the existence of things decribed in P1095
#define DBJ_ERR_P1095 

#ifdef DBJ_ERR_P1095

// handling the future 'throws' function marker. now
// just nothing for the time being
#define dbj_throws

// P1095 fails used "now"
// declares the return value pair type
// to which failure/succes making inside 
// the same function has to conform
#define dbj_fails(vt,et) -> std::pair<vt,et>

#define dbj_erc_retval auto

namespace dbj::err {

	template<typename T>
	auto failure(T v, std::errc e_) {
		return std::pair{ v, std::make_error_code(e_) };
	}

	template<typename T>
	auto failure(T v, std::error_code e_) {
		return std::pair{ v, e_ };
	}

	template<typename T>
	auto failure(T v, std::error_condition en_) {
		return std::pair{ v, std::make_error_code(en_) };
	}

	template<typename T>
	auto succes(T v) {
		return std::pair{ v, dbj_universal_ok };
	};

} // namespace dbj::err

#endif // DBJ_ERR_P1095
/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"