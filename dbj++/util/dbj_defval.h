#pragma once

#include "../core/dbj_synchro.h"

// #define VERSION_TWO
#ifdef VERSION_TWO 
#include <optional>
#endif

namespace dbj {

	/*
	Task: implement this for any type
	inline int default_int_width(int new_ = 1) {
		static int dflt_ = new_;
		if (new_ != dflt_) dflt_ = new_;
		return new_;
	}

	"Version THREE"

	No stunts. A plain old function object.
	Simple usable and working. 
	Example:
	 
	 Global declaration/defintion:

	  static inline dbj::holder<int> my_def_signal{42};

     Read elsewhere:

	   auto signal = my_def_signal() ; 

    Or change for everybody else to to see:

	  auto new_signal = my_def_signal(101) ; 
	*/
#pragma warning( push ) 
#pragma warning( disable : 4522 )
	template<typename T>
	class holder final {
		
		// mutable members of const class instances are modifiable
		mutable T default_value_{};
		
		// ask compiler for default ctor
		// but hide it
		holder() noexcept = default;
		
		// dbj::holder is neither copyable nor movable.
		holder(const holder&) = delete;
		holder& operator=(const holder&) = delete;
		holder& operator=(const holder&) volatile = delete;

	public:

		holder(const T & defval_) : default_value_(defval_) {	};
		/* 
		first operator () overload just returns the value 
		*/
		const T & operator () () const noexcept {
			return default_value_;
		}
		/*
		second operator () overload sets and returns the value
		please read
		https://msdn.microsoft.com/en-us/library/031k84se.aspx
		to understand the operator declaration bellow
		it allows for const isntances to be used
		through the operator bellow. Example 
		const holder<REAL> width{ 10 }; // set to 10
		width(1024) ; // OK to change to 1024
		*/
		const volatile T & operator ()
			(const T & new_) const volatile noexcept
		{
			// safe in MT situations
			DBJ_AUTO_LOCK;
			if (new_ != default_value_)
				default_value_ = new_;
			return default_value_;
		}
	};
#pragma warning( pop ) 

#ifdef VERSION_TWO 

	namespace v_two {

		/*
		Function template returning a lambda
		Closer to real stunt, but works.
		And compiler has less problems disambiguating the instances	vs the solution one
		type T has to be move assignable
		*/
		template<typename T,
			typename std::enable_if< std::is_move_assignable_v<T> > * = 0 >
			inline auto holder_maker(T defval_) {

			/*
			Requirement: auto df = defval() should return def val on first and all the other calls.
			So what do we do to make this behave as  we want  it to?
			On the first call we execute the lambda and return it.
			Thus on second and all the other calls it will be used with a proper def val from the first call
			*/
			auto defval_handler = [&](
				/* No if we do bellow T new_ = defval_ , that will not compile, try...
				   the elegant (or is it a stunt?) way arround, is to use std::optional
				   BIG NOTE 2: lambda bellow requires T to be a moveable type
				*/
				const std::optional<T> & new_ = std::nullopt
				) -> const T &&
			{
				static T default_value = defval_;
				if (new_ != std::nullopt) {
					T new_val_ = new_.value_or(default_value);
					if (new_val_ != default_value)
						default_value = new_val_;
				}
				return std::forward<T>(default_value);
			};

			/* only on the first call execution will reach here and
			will return the lambda to be used on the rest of the calls */
			/* as per C++17 memory model this must be thread safe */
			static auto first_call_ = defval_handler();

			return defval_handler;
		}

	} // v_two
#endif // "Version TWO" 
#undef VERSION_TWO

#if 0  /* "Version ONE" */

	/* ilustration of a wrong stunut like approach */


	template< typename T, T defval_ = T() >
	inline T & static_default(const T & new_ = defval_)
	{
		static T default_value = defval_;
		if (new_ != default_value)
			default_value = new_;
		return default_value;
	}

	namespace {
		using namespace Gdiplus;

		const auto sm0 = static_default<SmoothingMode, SmoothingMode::SmoothingModeAntiAlias>();
		const auto sm1 = static_default<SmoothingMode>();

		auto sm2 = static_default<SmoothingMode, SmoothingMode::SmoothingModeAntiAlias>;
		auto lc0 = static_default<LineCap, LineCap::LineCapRound>;
		auto sm3 = static_default<SmoothingMode>;
		auto dlcF = static_default<LineCap>;

		auto dlc = dlcF(LineCap::LineCapRound);

		/*
		but there is a problem here ...
		auto default_problem = static_default < REAL, REAL{10} > ;

		error C3535: cannot deduce type for 'auto' from 'overloaded-function'
		error C2440: 'initializing': cannot convert from 'T &(__cdecl *)(const T *)' to 'int'
		note: Context does not allow for disambiguation of overloaded function

		above template instance is not "different enough" from two previous ones
		*/
	}


#endif // "Version ONE"

} // namespace dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"