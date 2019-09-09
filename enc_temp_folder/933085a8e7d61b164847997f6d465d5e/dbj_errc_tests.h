#pragma once

DBJ_TEST_SPACE_OPEN(dbj_error_concept)

using namespace std;

using buffer = typename ::dbj::vector_buffer<char>;
using buffer_type = buffer::narrow ;

/*
  first we declare return value type we will use
*/
template< typename V_ >
using retval_type = dbj::errc::return_type< V_, dbj::errc::state_type >;

/* imaginary errors repository */
namespace e_repo
{
	using e = dbj::errc::state_type ;
	// _buff UDL lives here
	using namespace ::dbj::literals;

	constexpr inline e::id_type BASE_ERR_ID = 1000U ;

	inline const array errors{
		e{ { BASE_ERR_ID + 0, "Unknown error ID"_v_buffer } },
		e{{ BASE_ERR_ID + 1, "Well, this is weird?"_v_buffer }},
		e{{ BASE_ERR_ID + 2, "Has anybody seen this?"_v_buffer }},
		e{{ BASE_ERR_ID + 3, "Divisor 0 has no meaing"_v_buffer }}
	};

	// search by 1,2,3 ... 0 is "unknown error"
	inline e query_by_id(e::id_type find_this_id_)
	{
		e rt_ = errors[0]; // the unknown

		for (auto& el_ : errors) {
			if (el_.state.id == (BASE_ERR_ID + find_this_id_))
				return el_;
		}
		return rt_;
	}
} // e_repo

/*
depends on e_repo
why don't we create error reporsitory required interface?
with SQL DB in the back ?! Juu, huu!
*/
template < typename T_, typename RH_ = retval_type<int> >
inline auto failure(
	typename dbj::errc::state_type::id_type id_,
	typename dbj::errc::state_type::id_type loc_id_,
	const char* filename_)
{
	// first we need to create/find the error object
	auto e_ = e_repo::query_by_id(id_);
	// second we need to add the location to it
	e_.location = dbj::errc::idmessage::make(loc_id_, filename_);
	// third we create and return the retval 
	// with error info only
	return RH_::error(e_);
}

template < typename T_, typename RH_ = retval_type<int> >
inline auto success(
	T_ && return_value_
)
{
	return RH_::normal( forward<T_>(return_value_) );
}


/*
now we use the above

for int's we decalre requiored return value type
*/
using int_retval = retval_type<int>;

inline auto	divider(int number, int divisor)
{
	if (divisor == 0)
		return failure<int>(3, __LINE__, __FILE__);

	//  create and return the retval 
	// with value only, no error info only
	return success<int>(number / divisor);
}

DBJ_TEST_UNIT(dbj_err_concept_)
{
	auto& print = ::dbj::console::print;
	/* here we consume the return value */
	auto [value_, error_] = divider(1, 0);

	if (error_)
	{
		auto err = *error_;
		print(
			"\nError returned",
			dbj::console::painter_command::cyan,
			"\nid: ", err.state.id,
			"\nmessage: ", err.state.message,
			dbj::console::painter_command::text_color_reset,
			dbj::console::painter_command::bright_blue,
			"\nlocation",
			"\nline: ", err.location.id,
			"\nfile: ", err.location.message,
			dbj::console::painter_command::text_color_reset
			);
	}

	if (value_) {
		print("\nResult: ", *value_);
	}

}

DBJ_TEST_SPACE_CLOSE