#pragma once

#include "../util/dbj_optional_any.h"

DBJ_TEST_UNIT(dbj_any_pair)
{
	DBJ_ATOM_FLIP; // show false results only

	using ::dbj::util::optival;
	using ::dbj::util::any_opt;
	{
		// f is function, a is any
		auto[f, a] = any_opt(324);
		// returns whatever is inside any 
		auto i324 = *f();
		assert(324 == i324);
	}
	{
		auto[f, a] = any_opt(std::string(__TIMESTAMP__));
		auto value_ = DBJ_TEST_ATOM( *f());
	}
}
