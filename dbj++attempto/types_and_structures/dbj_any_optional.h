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
			using value_type = std::decay_t<T> ;

		private:
			static_assert(!std::is_reference_v< value_type >,
				"\n\n[dbj::wrapper] Can not hold a reference type\n");

			static_assert(!std::is_array_v< value_type >,
				"\n\n[dbj::wrapper] Can not hold an array type\n");

			static_assert( std::is_constructible_v < value_type >,
				"\n\n[dbj::wrapper] T must be constructible\n");

			static_assert( std::is_move_constructible_v < value_type >,
				"\n\n[dbj::wrapper] T must be move constructible\n");

			std::any any_{};

			// an example how to overload dbj::console::out for UDT
			friend void out(const type & anyt_)
			{
				std::error_code ec_;
				auto data_ = anyt_.get(ec_);
				if ( ! ec_ )
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
				operator() ( ArgTypes&&... args) const
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

			constexpr value_type get(std::error_code & ec_ ) const noexcept 
			{
				ec_.clear();
				try {
					return any_cast<value_type>(this->any_);
				}
				catch (std::bad_any_cast & ) {
					ec_ = std::make_error_code( std::errc::not_supported );
				}
				return {};
			}

			constexpr bool empty() const noexcept {
				return !(this->any_).has_value();
			}

		}; // dbj any wrapper

		// factory methods ----------------------------------------

		template < typename T >
		inline wrapper<T> make( T const & val_)
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
	template<typename T>
	struct optional_get final 
	{
		// here add static_assert's to narrow the choice of the type allowed
		static_assert(std::is_constructible_v<T>,
			"\n\n\nT must be constructible\n");

		using type			= optional_get;
		using value_type	= T;
		using optional		= std::optional<T>;

		mutable T default_value{};

		explicit optional_get(T defval = T{})
			: default_value(defval)
		{	}

		T  operator () (
			std::optional<T> opt, 
			std::optional<T> def_val = std::nullopt
		)
		{
			return opt.value_or(def_val.value_or(default_value) );
		}
	};

	template<typename T>
	inline auto optional_pair(T defval = {} )
		-> std::pair<optional_get<T>, std::optional<T> >
	{
		// here add static_assert's to narrow the choice of the type allowed
		static_assert( std::is_constructible_v<T>, 
			"\n\n" __FUNCSIG__ "\nT must be constructible\n");

		return std::make_pair(
			optional_get<T>(defval),
			std::optional<T>{}
			);
	}

	struct D;
	struct D final {
		static auto const & ID() {
			static auto id_ = dbj::util::hash(typeid(D).name());
			return id_;
		}
		std::array<char, BUFSIZ> payload{ {0} };

		explicit D(const char * ss_ = "") noexcept {
			std::memcpy(payload.data(), ss_, ::strlen(ss_));
		}
		D & operator = (char const * ss_) noexcept
		{
			payload.fill(0);
			std::memcpy(payload.data(), ss_, ::strlen(ss_));
			return *this;
		}
	private:
		friend void out(D const & d_) noexcept
		{
			::dbj::console::prinf("%s { id: %d }",typeid(D).name(), d_.ID());
		}
	};

	DBJ_TEST_UNIT(another_idea_about_optional_helpers) {

		{
			auto[f, o] = optional_pair<int>(); // no default
			DBJ_ATOM_TEST(f(o)); // just ask for value
			//default is T{}
		}
		{
			auto[f, o] = optional_pair<int>(13);
			DBJ_ATOM_TEST(f(o)); // 13
			o = 42;
			DBJ_ATOM_TEST(f(o)); // 42
			DBJ_ATOM_TEST(f(o, 54)); // 42 as already assigned
			o.reset();
			DBJ_ATOM_TEST(f(o, 54)); // 54 
		}
		{
			// def. val is empty string
			auto[f, o] = optional_pair<std::string>();
			o = "Hurah!";
			DBJ_ATOM_TEST(f(o));
		}
		{
			namespace a = another_idea_about_optional_helpers;
			// 
			auto[f, o] = optional_pair<a::D>(a::D("defualt D"));
			o = "Hurah!";
			DBJ_ATOM_TEST(f(o));
		}

	}

}