#pragma once
// dbj_errc.h
// (c) 2019 by dbj@dbj.org
// Licence Appache 2.0
// https://github.com/DBJDBJ/dbj_error_concept

/*
The universal return type, contains value AND status
The key is AND not OR as it is in many other designs

Returned value can be in four (4) states

1. Normal
2. Error
3. Info
4. Empty

What is Normal state?

Normal state is non existece of the status returned
Value must exist in a normal state

What is Error state?

Error state is non existence of value returned.
In that case `status` is the `error`

What is Info state?

Info state is existence of both value and state, returned.

What is Empty state?

Empty state is **non** existence of both value and state, returned.

*/

namespace dbj::errc {

	template<typename VALUE_TYPE, typename STATUS_TYPE>
	struct return_type final
	{
		using type = return_type;
		using value_type = VALUE_TYPE;
		using status_type = STATUS_TYPE;

		using value_option = std::optional<VALUE_TYPE>;
		using status_option = std::optional<STATUS_TYPE>;

		// return structure
		// for structured return consuming
		using structure = std::pair<
			value_option,
			status_option
		>;

#pragma region make into one of four states

		/*
		make and return an 'empty' return type
		State -- Empty
		*/
		static constexpr structure empty () noexcept {
			return { std::nullopt, std::nullopt };
		}

		/*
		we do not overload 'make' in order to make
		code simpler *and* the calling code simpler 

		make no error, just value
		State -- Normal
		*/
		static constexpr structure normal (value_type const& val_) noexcept {
			return { val_, std::nullopt };
		}

		/*
		make no value, just error
		State -- Error
		*/
		static constexpr structure error(status_type const & err_) noexcept {
			return { std::nullopt , err_ };
		}

		/*
		make both value and error
		State -- Info
		*/
		static constexpr structure info(value_type & val_, status_type & err_) noexcept {
			return { val_, err_ };
		}
#pragma endregion

	}; // return_type

} // namespace dbj::errc