#pragma once

/*
include dbj_crt.h before this

#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <map>
*/

/// <summary>
/// The Command Line encapsulation aka TCL-ENC
/// by dbj.org
/// (c) statement is on the bottom of this file
/// <remarks>
/// Perhaps interesting in this design is the decision 
/// not to succumb to C like cli 
/// which is based on char ** (or wchar_t ** in case of Windows). 
/// Internaly to represent the command line we will  use 
/// <code>
///  std::vector< std::wstring >; ;
/// </code>
/// when dealing with the actual cli we will transform asap to  this
/// thus internaly we do not have to deal with raw pointers.
/// </remarks>
/// </summary>
namespace dbj::app_env {

	using string_type = std::wstring;

	/// <summary>
	/// we develop only unicode windows app's
	/// this dats type we use everyhwere
	/// to provide CLI interface implementation
	/// </summary>
	using data_type = std::vector< std::wstring >;

	using map_type =
		std::map<std::wstring, std::wstring>;

	namespace inner {

		inline auto app_env_initor () {

			/// <summary>
			/// transform argw to data_type
			/// base your cli proc code on data_type
			/// instead of raw pointers 
			/// data_type is standard c++ range
			/// </summary>
			/// <param name="args">__argw</param>
			/// <param name="ARGC">__argc</param>
			/// <returns>
			/// instance of data_type
			/// </returns>
			auto command_line_data = [](size_t ARGC, wchar_t **  args)
				-> data_type
			{
				_ASSERTE(*args != nullptr);
				return data_type{ args, args + ARGC };
			};

			/// <summary>
			/// pointers to the runtime environment
			/// from UCRT
			/// </summary>
#define _CRT_DECLARE_GLOBAL_VARIABLES_DIRECTLY
			wchar_t **  warg = (__wargv);
			const unsigned __int64	argc = (__argc);
			wchar_t **  wenv = (_wenviron);
#undef _CRT_DECLARE_GLOBAL_VARIABLES_DIRECTLY

			/// <summary>
			///  we are here *before* main so 
			/// __argv or __argw might be still empty
			/// thus we will wait 1 sec for each of them 
			/// if need to be
			///  TODO: this is perhaps naive implementation?
			/// </summary>
			if (*warg == nullptr) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			_ASSERTE(*warg != nullptr);

			if (*wenv == nullptr) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			_ASSERTE(*wenv != nullptr);

			// extract the unicode  command line
			data_type
				warg_data{ command_line_data( static_cast<std::size_t>(argc), warg) };

			// calculate the count of env vars 
			auto count_to_null = [](auto ** list_) constexpr->size_t {
				size_t rez{ 0 };
				_ASSERTE(*list_ != nullptr);
				for (; list_[rez] != NULL; ++rez) {};
				return rez;
			};

			size_t evc = count_to_null(wenv);

			// extract the environment pointer list
			data_type
				wenvp_data{ command_line_data(evc, wenv) };
			app_env::map_type
				wenvp_map{};

			//transform env vars to k/v map
			// each envar entry is 
			// L"key=value" format
			for (auto kv : wenvp_data) {
				auto delimpos = kv.find(L"=");
				auto key = kv.substr(0, delimpos);
				auto val = kv.substr(delimpos + 1);
				wenvp_map[key] = val;
			}

			return make_tuple(argc, warg_data, evc, wenvp_map);
		}; // app_env_initor()
	}

	// client code gets the instance of this
	class structure final {
	public:
		
		using string_type = typename data_type::value_type;
		using cli_type = data_type;
		using env_var_type = map_type;

		mutable unsigned __int64	cli_args_count{};
		mutable data_type		cli_data{};
		mutable unsigned __int64	env_vars_count{};
		mutable map_type		env_vars{};

		// get's cli data -- no error check
		dbj::app_env::string_type 
			operator [] 
			(size_t pos_) const noexcept
				{ return cli_data[pos_]; }
		// get's env var data -- no error check
		typename env_var_type::mapped_type 
			operator []
				(typename env_var_type::key_type key_) const noexcept 
					{ return env_vars[key_]; }

	private:

		void* operator new(std::size_t sz);
		void operator delete(void* ptr);

		structure(
			unsigned __int64 argc,
			data_type   argv,
			unsigned __int64 env_count,
			map_type	env_vars_
		)
			: cli_args_count(argc),
			cli_data(argv),
			env_vars_count(env_count),
			env_vars(env_vars_)
		{}

		friend void out(const structure & cli_struct) 
		{
			::dbj::console::PRN.printf
			(
				L"\ndbj::app_env::structure\n%d CLI arguments and %d env vars collected \n", 
				cli_struct.cli_args_count, 
				cli_struct.env_vars_count)	;
		}
	// public:
		friend structure instance();
	}; // structure

	inline structure instance() {
		auto once = []() {
			auto[argc, warg, envc, env_map] = inner::app_env_initor();
			return structure{ argc, warg, envc, env_map };
		};
		static structure app_env_single_instance_ = once();
		return app_env_single_instance_;
	}

} // dbj::app_env

namespace dbj {
	inline auto application_environment_data 
		= app_env::instance();
}; // dbj


/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
