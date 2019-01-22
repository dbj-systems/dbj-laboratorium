#pragma once
/*
Just utilities
*/
#include <string>
#include <string_view>
#include <filesystem>

namespace dbj::util {
	
	using namespace std;
	using namespace std::string_view_literals;
	namespace fs = std::filesystem;

	[[nodiscard]] inline auto dbj_get_envvar(std::string_view varname_) noexcept
	{
		_ASSERTE(!varname_.empty());
		std::array<char, 256>	bar{ {0} };
		std::error_code			ec_; // contains 0 as the OK val
		::SetLastError(0);
		if (1 > ::GetEnvironmentVariableA(varname_.data(), bar.data(), (DWORD)bar.size()))
		{
			ec_ = error_code(::GetLastError(), std::system_category());
		}
		return pair(string(bar.data()), ec_);
	}

	[[nodiscard]] inline auto program_data_path() noexcept {
		fs::path prog_data_path_;
		auto[v, e] = dbj_get_envvar("ProgramData");
		// if error return
		// thus prog_data_path_ is empty
		if (e) 	return pair(prog_data_path_, e);
		// ok return
		// prog_data_path_ get the path string
		prog_data_path_ = v;
		return pair(prog_data_path_, e);
	}
} // dbj::util
