#pragma once


DBJ_TEST_SPACE_OPEN( dbj_util )

using namespace std::string_view_literals;

enum class engine_tag { diesel, petrol };
// We are talking 100% encapsulation, here
auto car_maker(engine_tag engine_tag_ ) {

	// this ABC can be outside of the factory method
	struct engine  {
		virtual char const * const name() = 0 ; 
	};

	struct diesel final : engine {
		char const * const name() override { return "DIESEL"; }
	};

	struct petrol final : engine {
		char const * const name() override { return "PETROL"; }
	};

	// this can inherit from the interface that would be
	// outside od the factory method
	struct front final {
		front( engine * const engine_ ) 
			: impl_( engine_)
		{		}

		auto engine_kind() const noexcept {
			return impl_->name();
		}
	private:
		// PIMPL;
		mutable ::std::unique_ptr<engine> impl_;
	};

	if (engine_tag_ == engine_tag::diesel) {
		return front(new diesel{});
	}
	else { // petrol
		return front(new petrol{});
	}

}

DBJ_TEST_UNIT( test_pimpl_with_improved_reference_wrapper )
{
	auto diesel_ = car_maker(engine_tag::diesel);
	auto petrol_ = car_maker(engine_tag::petrol);

	::dbj::console::print(
		"\n1-st car is: ", diesel_.engine_kind(),
		"\n2-nd car is: ", petrol_.engine_kind()
	);
}

DBJ_TEST_UNIT(dbjutilunitrangecontainertest) 
{
	using RCI10 = dbj::util::rac<int, 10>;
	RCI10 rci10{};

	int j{};
	for (auto && elem : rci10) {
		elem = j++;
	}

	auto & dta1 = rci10.value ;
	DBJ_TEST_ATOM( rci10.data() );
}

	DBJ_TEST_UNIT(dbjutilmathfloat_to_integertest) {

		using namespace dbj::num::float_to_integer;

		auto test = [](float val_) {
			dbj::console::print("\n\nInput:\t");
			DBJ_TEST_ATOM(val_);
			DBJ_TEST_ATOM(nearest(val_));
			DBJ_TEST_ATOM(drop_fractional_part(val_));
			DBJ_TEST_ATOM(next_highest(val_));
			DBJ_TEST_ATOM(integer(val_));
		};

		test(123.00f);
		test(123.500f);
		test(123.4901f);
		test(123.501f);
	}

	DBJ_TEST_UNIT(dbjdbj_util_test) {

		using dbj::util::remove_duplicates;

		int intarr[]{ 1,1,2,2,3,4,5,6,6,6,7,8,9,9,0,0 };
		auto DBJ_MAYBE(ret1)
			= DBJ_TEST_ATOM ( remove_duplicates(intarr) );
		std::string as2[16]{
			"abra", "ka", "dabra", "babra",
			"abra", "ka", "dabra", "babra",
			"abra", "ka", "dabra", "babra",
			"abra", "ka", "dabra", "babra",
		};

		DBJ_TEST_ATOM( remove_duplicates(as2) );

		char carr[] { 'c','a','b','c','c','c','d', 0x0 };
		DBJ_TEST_ATOM( remove_duplicates(carr) );

		int ia[10]{ 0,8,3,4,6,6,7,8,7,1 };

		DBJ_ATOM_TEST(remove_duplicates(ia, ia + 10, true)); // sorted too

		using namespace std::string_view_literals;

		DBJ_TEST_ATOM( dbj::str::starts_with("abra ka dabra"sv, "abra"sv) );
		DBJ_TEST_ATOM( dbj::str::starts_with(L"abra ka dabra"sv, L"abra"sv) );
	};


	DBJ_TEST_SPACE_CLOSE
