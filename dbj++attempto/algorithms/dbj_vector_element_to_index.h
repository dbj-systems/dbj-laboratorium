#pragma once
#include "../pch.h"

namespace dbj {

	// return index from vector value
	// this obviously  returns the index of the
	// first value found
	// as a sanity check vector max size is 0xFFFF
	template<typename V, typename A, size_t max_size = 0xFFFF,
		class vec_type = std::vector<V, A>,
		class val_type = typename std::vector<V, A>::value_type
	>
		inline auto v2i(
			std::vector<V, A> vector_,
			typename std::vector<V, A>::value_type value_
		) -> int
	{
		static_assert(::dbj::is_std_vector_v<vec_type>);

		DBJ_VERIFY(vector_.size() < max_size);
		auto	index_ = 0U;
		for (val_type & element_ : vector_) {
			if (element_ == value_)
				return index_;
			index_ += 1;
		}
		return index_;
	};

} // dbj

namespace dbj::samples {

	struct S final
	{
		const int number;
		const char name;
	private:
		// note: name is ignored by this comparison operator
		friend
			const bool operator < (const S& q, const S& s) noexcept
		{
			return ((q.number < s.number) && (q.name < s.name));
		}
		friend
			const bool operator == (const S& q, const S& s) noexcept
		{
			return ((q.number == s.number) && (q.name == s.name));
		}

		friend void out(struct S s_) {
			::dbj::console::PRN.printf(L"{ %d : %C }", s_.number, s_.name);
		}
	};


	using s_vector = std::vector<S>;

	// we can not place a friend inside s_vector iterator
	// so 
	void out(typename s_vector::iterator const & svi_)
	{
		::dbj::console::out(L"\ns_vector::iterator: ");
		out(*svi_);
	}


	DBJ_TEST_UNIT(test_vector_element_to_index)
	{
		using ::dbj::console::print;
		
		// note: not ordered, only partitioned w.r.t. 
		s_vector vec = { {1,'A'}, {2,'B'}, {2,'C'}, {2,'D'}, {4,'G'}, {3,'F'} };
		// this is ok since S operator <, ignores the name member
		s_vector::value_type value = { 2, '?' };
		// this works OK
		auto p1 = std::equal_range(vec.begin(), vec.end(), value);
		// this asserts in debug builds
		// is this not supposed to throw an std::exception 
		// or an derivative of?
		// auto p2 = ( std::upper_bound(last_ptr,first_ptr,4) );

		typename s_vector::iterator i1 = p1.first;
		typename s_vector::iterator i2 = p1.second;

		print("\n", p1, "\n\n");
		std::for_each(i1, i2, [&](const auto & s_) {
			using ::dbj::console::prinf;
			auto pos_ = ::dbj::v2i(vec, s_);
			prinf("\n[%03d] == ", pos_ );
			out(s_);
		});
	}

} // namespace dbj::samples 
