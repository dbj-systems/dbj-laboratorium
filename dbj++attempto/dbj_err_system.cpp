#include "pch.h"
/*
http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-4.html
*/
namespace dbj::err {
#pragma region error codes
	// error codes
	enum class codes {
		bad_argument = 100,
		bad_index,
		bad_length,
		bad_type
	}; // codes

	class common_category
		: public std::error_category
	{
	public:
		virtual const char* name()  const noexcept {
			return "dbj++common";
		}
		virtual std::string message(int ev) const noexcept {

			switch ( (codes)ev) {
			case codes::bad_argument:
				return "Bad Argument";
			case codes::bad_index:
				return "Bad index";
			case codes::bad_length:
				return "Bad length";
			case codes::bad_type:
				return "Bad type";
			default:
				return "Unknown error";
			}
		}
	};

	const std::error_category& category()
	{
		static common_category category_;
		return category_ ;
	}

	std::error_code make_error_code(::dbj::err::codes e)
	{
		return std::error_code(
			static_cast<int>(e),
			category());
	}

	std::error_condition make_error_condition(::dbj::err::codes e)
	{
		return std::error_condition(
			static_cast<int>(e),
			category());
	}
#pragma endregion

#pragma region error conditions
	enum class conditions
	{
		core = 100,
		strings,
		numerics,
		types,
		utils,
		console,
		testing_fw,
		testing
	};

	class conditions_category_impl
		: public std::error_category
	{
	public:
		virtual const char* name() const noexcept { return "dbj++conditions";  
		}
		virtual std::string message(int ev) const noexcept {
			switch ((conditions)ev) {
			case conditions::core:
				return "Core";
			case conditions::numerics:
				return "Numerics";
			case conditions::types:
				return "Types";
			case conditions::utils:
				return "Utilities";
			case conditions::console:
				return "Console";
			case conditions::testing_fw:
				return "Testing Framework";
			case conditions::testing:
				return "Testing";
			default:
				return "Unknown condition";
			}
		} // message

/*
defines equivalence between error_conditions in the current category 
with error_codes from any category.
*/
#if 0
		virtual bool equivalent(
			const std::error_code& code,
			int condition) const noexcept 
		{
			DBJ_NOUSE(code);
			switch ((conditions)condition) {
			case conditions::core:
			case conditions::numerics:
			case conditions::types:
			case conditions::utils:
			case conditions::console:
			case conditions::testing_fw:
			case conditions::testing:
				return true;
			default: return false;
			}
		} // equivalent
#endif
	};

	const std::error_category& conditions_category()
	{
		static conditions_category_impl instance;
		return instance;
	}

	std::error_condition make_error_condition(conditions e)
	{
		return std::error_condition(
			static_cast<int>(e),
			conditions_category());
	}

	std::error_code make_error_code(conditions e)
	{
		return std::error_code(
			static_cast<int>(e),
			conditions_category());
	}

#pragma endregion

} // dbj::err

namespace std
{
	template <>
	struct is_error_code_enum<::dbj::err::codes>
		: public true_type {};

	template <>
	struct is_error_condition_enum<::dbj::err::conditions>
		: public true_type {};
}

DBJ_TEST_UNIT(dbj_error_system_by_the_book)
{
	auto server_side_handler = [] (
		std::error_code& ec)
	{
		ec = ::dbj::err::codes::bad_argument ;
	};

	std::error_code ec;
	server_side_handler(ec);
	if (ec == ::dbj::err::codes::bad_argument)
	{
		DBJ_TEST_ATOM(ec.message());
		DBJ_TEST_ATOM(ec.value() );

		auto cond = ec.default_error_condition();
		DBJ_TEST_ATOM(cond.message());
		DBJ_TEST_ATOM(cond.value());
	}

	// condition testing
	// conditions are apparently 
	// platform agnostic error codes
	auto client_side_handler = [](
		std::error_condition & ec)
	{
		ec = ::dbj::err::conditions::core;
	};

	std::error_condition ecn;
	client_side_handler(ecn);

	if (ecn == ::dbj::err::conditions::core)
	{
		DBJ_TEST_ATOM(ecn.message());
		DBJ_TEST_ATOM(ecn.value());
	}

	DBJ_TEST_ATOM(ec == ::dbj::err::conditions::core);
}