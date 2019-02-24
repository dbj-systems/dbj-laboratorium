#pragma once

namespace dbj {
template<typename INVOCABLE >
struct apply_helper final
{

	static_assert(
		!std::is_array_v<INVOCABLE>,
		"invoking on array? How?");

 INVOCABLE invocable_;

// apply the pair of values
template< typename T1, typename T2>
auto operator () ( std::pair<T1,T2> pp_ ) {
	return  apply(invocable_, pp_);
}
// apply the tuple or args
template< typename ... ARGS >
auto operator ()  ( ARGS ... args ) 
{
	auto tuple_ = make_tuple(args...);
	return  apply(invocable_, tuple_);
}

// apply the native array 
// also takes care of init list call
template< typename T, size_t N>
auto operator () (const T(&array_)[N]) {
	array<T, N> std_array = dbj::arr::native_to_std_array(array_);
	return  apply(invocable_, std_array);
}

template< typename T, size_t N>
auto operator () (std::array<T, N> array_ ) {
	return  apply(invocable_, array_);
}
};

	template<typename F>
	auto make_applicator (F invocable_) {
		return apply_helper<F>{invocable_};
	};
} // dbj

/* recursive generic lambda solution */
auto summa = [](auto first, auto ... second) {

	if constexpr (sizeof ... (second) > 0) {
		return first + summa(second ...);
	}
	else {
		return first;
	}
};
#define TU(x) ::dbj::console::print("\nexpression: ", (# x), "\n\tresult: ", (x))

DBJ_TEST_UNIT(dbj_aplikator_testing)
{
		auto sumator = dbj::make_applicator(summa);

		// pair arg
		TU(sumator(std::pair(1, 2)));
		TU(std::apply(summa, std::pair(1, 2)));

		// tuple arg
		TU(sumator(2.0f, 3.0f, 1, 2));
		TU(std::apply(summa, std::make_tuple(2.0f, 3.0f, 1, 2)));

		// init list arg
		TU(sumator({ 1,2,3 }));
		// no can do -- TU(std::apply(summa, {1,2,3}));

		{
			std::array<int, 5> a5 = { 1, 2, 3, 4, 5 };
			TU(sumator(a5));
			TU(std::apply(summa, a5));
		}
		{
			// native array arg
			int a5[]{ 1, 2, 3, 4, 5 };
			TU(sumator(a5));
			// no can do -- TU(std::apply(summa, a5));
		}
		{
			// sumator as arg
			// 2.0f + 3.0f + (11 + 12) <-- summa(2.0, 3.0, summa(11,12))
			TU(sumator(2.0,3.0, sumator(11,12) ));

			TU(std::apply(summa, std::make_tuple(2.0f, 3.0f,
				std::apply(summa, std::make_pair(11, 12))
			)));
		}
}
#undef TU