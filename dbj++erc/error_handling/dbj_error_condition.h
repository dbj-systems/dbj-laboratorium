#pragma once

#if DBJ_ERROR_CONDITION
namespace dbj::err{

	enum class conditions
	{
		ok = 100,
		core = 101,
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
		virtual const char* name() const noexcept {
			return "dbj++conditions";
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
}
namespace std {
template <>
struct is_error_condition_enum<::dbj::err::conditions>
	: public true_type {};
}

/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"
#endif // #DBJ_ERROR_CONDITION