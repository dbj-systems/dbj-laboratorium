#pragma once

#include "../util/dbj_optional_any.h"

DBJ_TEST_UNIT(dbj_any_pair)
{
	using ::dbj::util::optival;
	using ::dbj::util::make_any_pair;
	{
		auto[f, a] = make_any_pair(324);
		auto optnl = f(a);
		assert(324 == optnl.value());
	}
	{
		auto[f, a] = make_any_pair(std::string(__TIMESTAMP__));
		auto optnl = DBJ_TEST_ATOM(f(a));

		DBJ_ATOM_TEST(optival(optnl));
	}
}

DBJ_TEST_UNIT(another_idea_about_optional_helpers)
{
	using ::dbj::util::optional_pair;
	using ::dbj::util::optival;

	DBJ_ATOM_FLIP; // show false results only

	{
		auto[f, o] = optional_pair<int>(); // no default
		auto[f2, o2] = optional_pair<int>(); // no default
		DBJ_ATOM_TEST(f(o) == f2(o2)); // just ask for value
		DBJ_ATOM_TEST(optival(o, 54) == f2(o2, 54)); // use 54 as current default
	}
	{
		auto[f, o] = optional_pair<int>(13);
		auto[f2, o2] = optional_pair<int>(13); // 13 is meta_default
		DBJ_ATOM_TEST(f(o) == f2(o2)); // use meta_default
		o2 = o = 42;
		DBJ_ATOM_TEST(f(o) == f2(o2)); // 42
		DBJ_ATOM_TEST(f(o, 54) == f2(o2, 54)); // 42 as already assigned
		o.reset();		o2.reset();
		DBJ_ATOM_TEST(f(o, 54) == f2(o2, 54)); // use 54 as current default
		DBJ_ATOM_TEST(optival(o, 54) == f2(o2, 54)); // use 54 as current default
	}
	{
		auto[f, o] = optional_pair<std::string>();
		auto[f2, o2] = optional_pair<std::string>();
		DBJ_ATOM_TEST(f(o) == f2(o2));
		o2 = o = "Hurah!"; // o2 = "Hurah!";
		DBJ_ATOM_TEST(f(o) == f2(o2));
	}
}
