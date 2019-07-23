#pragma once
// dbj_errc.h
// (c) 2019 by dbj@dbj.org
// Licence Appache 2.0
// https://github.com/DBJDBJ/dbj_error_concept

#include <optional>
#include <utility>
#include "dbj_id_and_message.h"

namespace dbj::errc {

	template<typename VALUE_TYPE, typename ERROR_TYPE>
	struct error_concept final
	{
		using type = error_concept;
		using value_type = VALUE_TYPE;
		using error_type = ERROR_TYPE;

		using value_option = std::optional<VALUE_TYPE>;
		using error_option = std::optional<ERROR_TYPE>;

		using return_type = std::pair<
			value_option,
			error_option
		>;

		/*
		make and return an 'empty' return type
		*/
		static return_type make() noexcept {
			return { std::nullopt, std::nullopt };
		}

		/*
		we do not overload 'make' in order to make
		code simpler *and* the calling code simpler 

		we use standard ISO C++ where arguments are passed by value

		make no error, just value
		*/
		static return_type make_val(value_type const & val_) noexcept {
			return { val_, std::nullopt };
		}

		/*
		make no value, just error
		*/
		static return_type make_err(error_type const & err_) noexcept {
			return { std::nullopt , err_ };
		}

		/*
		make both value and error
		*/
		static return_type make_val_err(value_type & val_, error_type & err_) noexcept {
			return { val_, err_ };
		}

	}; // error_concept

} // namespace dbj::errc