#include "pch.h"

namespace dbj::pertempto {

	DBJ_TEST_UNIT(_GetGeoInfoEx_)
	{
		using namespace std::literals;

		constexpr auto us_ = L"US"sv;
		constexpr auto rs_ = L"RS"sv;

		auto DBJ_UNUSED(us_data) = dbj::win32::geo_info((PWSTR)us_.data());
		auto DBJ_UNUSED(rs_data) = dbj::win32::geo_info((PWSTR)rs_.data());

		dbj::console::print(us_data);
		dbj::console::print(rs_data);
	}

} // namespace dbj::pertempto 
