#pragma once

namespace dbj {
	template<typename INVOCABLE >
	struct apply_helper final
	{
		const INVOCABLE & invocable_;

		// apply the pair of values
		template< typename T1, typename T2>
		auto operator () (T1 v1, T2 v2) {
			return  apply(invocable_, make_pair(v1, v2));
		}
		// apply the tuple or args
		template< typename ... ARGS >
		auto operator ()  (std::tuple<ARGS ...> tuple_) {
			return  apply(invocable_, tuple_);
		}

		// apply the native array 
		// also takes care of init list call
		// which will otherwise clash with var args 
		// overload above
		template< typename T, size_t N>
		auto operator () (const T(&array_)[N]) {
			array<T, N> std_array = dbj::arr::native_to_std_array(array_);
			return  apply(invocable_, std_array);
		}
	};

	auto make_apply_helper = [](auto invocable_) {
		return apply_helper<decltype(invocable_)>{invocable_};
	};
} // dbj