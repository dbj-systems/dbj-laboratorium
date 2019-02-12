#pragma once
#include "../pch.h"
#pragma region ANY WRAPPER
namespace dbj {

	namespace any {

		using namespace std;
		template <typename T> class wrapper;

		template <typename T>
		class wrapper final
		{
		public:
			using type = wrapper;
			using value_type = std::decay_t<T>;

		private:
			static_assert(!std::is_reference_v< value_type >,
				"\n\n[dbj::wrapper] Can not hold a reference type\n");

			static_assert(!std::is_array_v< value_type >,
				"\n\n[dbj::wrapper] Can not hold an array type\n");

			static_assert(std::is_constructible_v < value_type >,
				"\n\n[dbj::wrapper] T must be constructible\n");

			static_assert(std::is_move_constructible_v < value_type >,
				"\n\n[dbj::wrapper] T must be move constructible\n");

			std::any any_{};

			// an example how to overload dbj::console::out for UDT
			friend void out(const type & anyt_)
			{
				std::error_code ec_;
				auto data_ = anyt_.get(ec_);
				if (!ec_)
					::dbj::console::out(data_);
				else
					::dbj::console::out(ec_);
			}

		public:

			// we take references, pointers and a such ;)
			explicit wrapper(const value_type & ref) noexcept : any_(ref) {}
			// we take no r-references or values
			wrapper(value_type && ref) = delete;

			wrapper() noexcept {	}

			// only if function is stored
			template< class... ArgTypes	>
			auto /* invoke_result_t< value_type &, ArgTypes...> */
				operator() (ArgTypes&&... args) const
			{
				static_assert(is_invocable_v<value_type, ArgTypes ... >
					, "[dbj any wrapper] does not hold invocable type ");
				if (empty())
					throw std::make_error_code(std::errc::not_supported);
				std::error_code ec_;
				value_type fun = get(ec_);
				if (ec_) throw ec_;
				return invoke(fun, forward<ArgTypes>(args)...);
			}

			constexpr value_type get(std::error_code & ec_) const noexcept
			{
				ec_.clear();
				try {
					return any_cast<value_type>(this->any_);
				}
				catch (std::bad_any_cast &) {
					ec_ = std::make_error_code(std::errc::not_supported);
				}
				return {};
			}

			constexpr bool empty() const noexcept {
				return !(this->any_).has_value();
			}

		}; // dbj any wrapper

		// factory methods ----------------------------------------

		template < typename T >
		inline wrapper<T> make(T const & val_)
		{
			// can make only from values
			//  see the ctor deleted
			return wrapper< T >(std::move(val_));
		};

		// template < typename T >	wrapper<T> make(T && val_) = delete ;

		// given T(&)[N]
		// returns std::array of N * wrapper<T>
		// thus array of any's effectively
		// made from native array
		template <
			typename T, std::size_t N, typename ANYW = wrapper<T>
		>
			static std::array< ANYW, N >
			range(const T(&arrf)[N])
		{
			std::array< ANYW, N > rezult;
			std::size_t j = 0;

			for (auto && element : arrf) {
				rezult[j] = any::make(element);
				j += 1;
			}
			return rezult;
		};

		// r-references to constants/literlas <-- no can do
		template <
			typename T,
			std::size_t N,
			typename ANYW = wrapper<T>,
			typename RETT = std::array< ANYW, N >,
			typename std_arr_t = RETT
		>
			static RETT range(const T(&&arrf)[N]) = delete;
	} // any

} // dbj
#pragma endregion

namespace dbj_any_wrapper_testing {

	// TESTING
	struct X;
	extern const X empty_x;
	struct X final
	{
		static inline const X & ref = empty_x;
		operator const char * () const noexcept { return "X test struct, instance"; }
	};
	inline const X empty_x;


	DBJ_TEST_UNIT(dbj_strong_optional)
	{
		using int_ = dbj::optional_handle<int>;
		using float_ = dbj::optional_handle<float>;
		using size_ = dbj::optional_handle<unsigned int>;
		// interesting
		using void_ = dbj::optional_handle<void>;
		int_	ii = 42;
		float_	ff{ 42.0f };
		size_	ss(1024U);

		int_			i2{ 84 };
		float_			f2{ 84.0f };
		size_			 s2{ 2048U };

		DBJ_TEST_ATOM(ii = i2);
		DBJ_TEST_ATOM(ff = f2);
		DBJ_TEST_ATOM(ss = s2);
	}

	DBJ_TEST_UNIT(dbj_any_wrapper)
	{
		using namespace ::dbj::any;
		int int_arr[]{ 1,2,3 };
		auto any_0 = DBJ_TEST_ATOM(range(int_arr));
		auto not_a_temporary = "YES CAN DO"s;
		auto any_2 = DBJ_TEST_ATOM(make(not_a_temporary));
		auto DBJ_MAYBE(any_3) = DBJ_TEST_ATOM(any_2); // copy wrapper to wrapper

		// no can do , T has to be constructible
		// auto any_4 = make([&](auto x) { return typeid(x).name();  });
		// auto DBJ_MAYBE(rez_4) = any_4(true);
	}

} // namespace dbj_any_wrapper_testing 

/*
Another idea 2019-02-10

Keep optional<T> and the function to get T val, together
*/

namespace another_idea_about_optional_helpers
{
	// it appears this is all we need to 
	// get the value from any optional<T>
	// // optional<T> actually never looses it's 'T'
	// with added functionality of user 
	// defined dflt optional val
	template<typename T>
	inline T optival
	(std::optional<T> opt, T dflt_ = T{})
		noexcept // daring
	{
		return opt.value_or(dflt_);
	}

	// same as above but adding "meta default" 
	// and delivering optional<T> instance
	// the two above in an std::pair
	template<typename T>
	inline auto optional_pair(T defval = {})
	{
		// here add static_assert's to narrow the choice of the type allowed
		static_assert(std::is_constructible_v<T>,
			"\n\n" __FUNCSIG__ "\nT must be constructible\n");
		static_assert(std::is_copy_constructible_v<T>,
			"\n\n" __FUNCSIG__ "\nT must be copy constructible\n");

		return std::make_pair(
			// the first
			// pay attention: we do not need "functor" here
			[=]
		(std::optional<T> opt_, std::optional<T>  dflt_ = std::nullopt)
			noexcept -> T
		{
			T meta_def{ defval };
			return opt_.value_or(dflt_.value_or(meta_def));
		},
			// the second
			std::optional<T>{}
		);
	}

	/*
	any and optional and pair working together

	NOTE: use pointer overload of any_cast to avoid exceptions
		  use C++17 'extended' if syntax with it

		  if (auto ptr = std::any_cast<int>(&a1); ptr ) {
			  // ptr is not nullptr here if int value is inside
		  }
	*/
	template<typename T>
	auto make_any(T const & v_ = 0)
	{
		(void)noexcept(v_);
		// using type = std::decay_t< T >;
		using optitype = std::optional< T >;

		std::any any_any_;
		return
			std::make_pair(
				[=](std::any any_) -> optitype
		{
			if (!any_.has_value())	return std::nullopt;
			return optitype{ std::any_cast<T>(any_) };
		},
				std::any{ v_ }
		);
	}

	// send it to some bad function far-far away
	// but in the same program
	template< typename O>
	void bad_function(O some_opt)
	{
		// get to the value 
		if (some_opt.has_value()) {
			auto opt_val = some_opt.value();
			DBJ_TEST_ATOM(opt_val);
		}
	}

	// type for testing
	struct type_for_testing;
	struct type_for_testing final {
		static auto const & type_hash() {
			static auto id_
				= dbj::util::hash(typeid(type_for_testing).name());
			return id_;
		}
		std::array<char, BUFSIZ> payload{ {0} };

		explicit type_for_testing(const char * ss_ = "") noexcept {
			std::memcpy(payload.data(), ss_, ::strlen(ss_));
		}
		type_for_testing & operator = (char const * ss_) noexcept
		{
			payload.fill(0);
			std::memcpy(payload.data(), ss_, ::strlen(ss_));
			return *this;
		}
		bool operator == (type_for_testing const & another_) const noexcept {
			return this->payload == another_.payload;
		}
	private:
		friend void out(type_for_testing const & d_) noexcept
		{
			::dbj::console::prinf("%s { type_hash: %d }", typeid(type_for_testing).name(), d_.type_hash());
		}
	};

	DBJ_TEST_UNIT(another_idea_about_optional_helpers)
	{
		DBJ_ATOM_FLIP; // show false results only

		auto get_it_from_here = [](auto  maybe_opt)
		{
			// optional<T> actually never looses it's 'T'
			return maybe_opt.value();
		};
		{
			using namespace std;
			optional<string> os; os = "Hello!";
			bad_function(os);
		}

		{

			auto[f, a] = make_any(324);

			if (auto ptr = std::any_cast<bool>(&a); ptr) {
				// ptr != nullptr here if bool value is inside
				bool v = *ptr;
			}
			else {
				// signal the error
			}

			auto optnl = f(a);
			assert( 324 == optnl.value());
		}

		{
			auto[f, o] = optional_pair<int>(); // no default
			auto[f2, o2] = optional_pair<int>(); // no default
			DBJ_ATOM_TEST(f(o) == f2(o2)); // just ask for value

			//default is T{} , int{} == 0
		}
		{

			auto[f, o] = optional_pair<int>(13);
			auto[f2, o2] = optional_pair<int>(13); // 13 is meta_default
			DBJ_ATOM_TEST(f(o) == f2(o2)); // use meta_default
			o2 = o = 42;
			DBJ_ATOM_TEST(f(o) == f2(o2)); // 42
			DBJ_ATOM_TEST(f(o, 54) == f2(o2, 54)); // 42 as already assigned

			DBJ_ATOM_TEST(get_it_from_here(o2));

			o.reset();		o2.reset();
			DBJ_ATOM_TEST(f(o, 54) == f2(o2, 54)); // use 54 as current default
			DBJ_ATOM_TEST(optival(o, 54) == f2(o2, 54)); // use 54 as current default
		}
		{
			// def. val is empty string
			auto[f, o] = optional_pair<std::string>();
			auto[f2, o2] = optional_pair<std::string>();
			DBJ_ATOM_TEST(f(o) == f2(o2));
			o2 = o = "Hurah!"; // o2 = "Hurah!";
			DBJ_ATOM_TEST(f(o) == f2(o2));
		}
		{
			namespace a = another_idea_about_optional_helpers;
			// 
			auto[f, o] = optional_pair<a::type_for_testing>(a::type_for_testing("defualt type_for_testing"));
			auto[f2, o2] = optional_pair<a::type_for_testing>(a::type_for_testing("defualt type_for_testing"));
			DBJ_ATOM_TEST(f(o) == f2(o2));
			o2 = o = "Hurah for type_for_testing!";
			DBJ_ATOM_TEST(f(o) == f2(o2));
			DBJ_ATOM_TEST(get_it_from_here(o2));
		}

	}

}