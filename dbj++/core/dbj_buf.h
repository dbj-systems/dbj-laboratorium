#pragma once

#ifndef _HAS_CXX17
#error C++17 please ...
#endif

// #define DBJ_BUFFER_TESTING

// makes stremaing related operators 
// on dbj buffer types
// #ifdef DBJ_BUFFERS_IOSTREAMS

// 2019-01-30	dbj@dbj.org	created
//
// char runtime dynamic buffer type
// faster and lighter vs the usual suspects: string and vector
// note: this is part of the dbj++core , thus we will try and absolutely minimise
// the dependancies  thus we will sometimes
// reimplement things here which perhaps exist in other projects
//
// note: this is not C++17 way of doing things
// so that good legacy code can enjoy this too

namespace dbj::chr_buf {

#pragma region yanb (yet another buffer)

		/*
		Note: static_assert() will not kick-in before instantiation
		of the template definition. Thus we use SFINAE to stop making
		illegal types, from this template.

		BUT!

		That yanb_tpl<T> template declaration bellow will leave the type defined as:
		yanb<char> --- creates --> yanb_tpl<char,1>
		yanb_tpl<char> -- as a type will  not exist! ever.
		just: yanb_tpl<char,1>

		SO!

		the correct way of using it "upstream" for SFINAE is:
		yanb_tpl<char>::type
		*/

	template<
		typename T,
		std::enable_if_t< std::is_same_v<char, T> || std::is_same_v<wchar_t, T>, bool > = true
	>
		struct yanb_tpl final
		// note: template (tpl) is not a type  ... wink,wink
	{
		using type = yanb_tpl;
		using data_type = T;
		using value_type = std::shared_ptr<data_type>;
		using value_type_ref = std::reference_wrapper<value_type>;

		type& reset(data_type const* payload_)
		{
			core::assign(this->data_, payload_);
			return *this;
		}

		yanb_tpl( value_type payload_) {
			core::assign(this->data_, payload_.get());
		}

		yanb_tpl(data_type const* payload_) {
			core::assign(this->data_, payload_);
		}

		size_t size() noexcept { return core::length(this->data_); }
		size_t size() const noexcept { return core::length(this->data_); }

		data_type* data() noexcept { return this->data_.get(); }
		data_type const* data() const noexcept { return this->data_.get(); }

		operator data_type* () noexcept { return this->data_.get(); }
		operator data_type const* () const noexcept { return this->data_.get(); }

		operator bool() noexcept { return data_.operator bool(); }
		operator bool() const noexcept { return data_.operator bool(); }

		yanb_tpl() = default;
		yanb_tpl(yanb_tpl const&) = default;
		yanb_tpl& operator = (yanb_tpl const&) = default;
		yanb_tpl(yanb_tpl&&) = default;
		yanb_tpl& operator = (yanb_tpl&&) = default;

	private:
		value_type data_{};

#ifdef DBJ_BUFFERS_IOSTREAMS

		/*
		Careful, a big and recurring gotcha!
		One can use both std::cout and std::wcout in one program.
		Do not do that. The issue:

		std::cout << yanb_tpl<wchar_t>(L"Hello?")
		prints: 0x000FFCD27, aka "the pointer address"
		why? because:
		std::cout does not use std::wostream !
		std::wcout does not use std::ostream !

		Bellow is a version for programs targeting std::cout.
		The wide version is trivial if you understand this one.
		*/
		friend  std::ostream&
			operator << (std::ostream& os, yanb_tpl<data_type> const& bufy_)
		{
			if constexpr (std::is_same_v<wchar_t, data_type>) {
				/*
				This is just casting internaly and thus works only
				for the ASCI subset.
				This is not locale aware.
				*/
				std::wstring_view sv_(bufy_.data());
				std::string str_(sv_.begin(), sv_.end());
				return os << str_.c_str();
			}
			else {
				return os << bufy_.data();
			}
		}
#endif // DBJ_BUFFERS_IOSTREAMS

	}; // yanb_tpl<T>

	/////////////////////////////////////////////////////////////////////////////////////////////////
	using yanb =  yanb_tpl<char>;
	using yanwb = yanb_tpl<wchar_t>;

#ifdef DBJ_YANB_TESTING

	void test_yanb()
	{
		{
			auto mover = [](yanb bufy) { return bufy; };
			yanb b1("abra");
			yanb b2 = b1;
			DBJ_TEST_ATOM(mover(b2));
			DBJ_TEST_ATOM(mover("narrow dabra"));
		}
		{
			auto mover = [](yanwb bufy) { return bufy; };
			yanwb b1(L"abra");
			DBJ_TEST_ATOM(b1);
			DBJ_TEST_ATOM(b1.data());
			yanwb b2 = b1;
			DBJ_TEST_ATOM(mover(b2));
			DBJ_TEST_ATOM(mover(L"wide dabra"));
		}
	}
#endif // DBJ_YANB_TESTING

#pragma endregion yanb (yet another buffer)



	// this is basically just set of helpers
	// to use the buffer from above
	template<
		typename CHAR,
		std::enable_if_t<
		std::is_same_v<char, CHAR> || std::is_same_v<wchar_t, CHAR>
		, bool > = true
	>
		struct helper final
	{
		using type = helper;
		using value_type = CHAR;

		using storage_t = yanb_tpl<CHAR>;

		using pointer = typename storage_t::value_type;
		using ref_type = type &;

		// always use this function to make  
		// buff of particular size
		// array is sized & zeroed
		static storage_t make(inside_1_and_max size_) noexcept {
			return
			{
			(std::make_unique<value_type[]>(size_ + 1)).get()
			};
		}

		static constexpr bool empty(	pointer buff_ ) noexcept
		{
			return (buff_ && (buff_[0] != '\0'));
		}

		/*
		NOTE!
		stringlen on char array filled with 0's returns 0 (zero)
		not the allocated size of the array.
		As I am using the inside_1_and_max as a return type
		it will 'kick the bucket' on the length 0!
		that is the point of using it
		*/
		static auto length(pointer buf_) noexcept -> inside_1_and_max
		{
			if constexpr (std::is_same_v<wchar_t, value_type>)
			{
				return std::wcslen(buf_.get());
			}
			else {
				return std::strlen(buf_.get());
			}
		}

		// the meta maker ;)
		static storage_t make(
			value_type const* first_, value_type const* last_
		) noexcept
		{
			_ASSERTE(first_ && last_);
			std::basic_string_view<value_type> sv_(
				first_, std::distance(first_, last_)
			);
			_ASSERTE(sv_.size() > 0);
			return { sv_.data() };
		}
		// here we depend on a zero terminated string
		static storage_t make(value_type const* first_) noexcept
		{
			_ASSERTE(first_);
			return { first_ };
		}
		/*	from array of char's	*/
		template<size_t N>
		static storage_t make(const value_type(&charr)[N]) noexcept
		{
			return { charr };
		}
		/* from string_view */
		static  storage_t	make(std::basic_string_view< value_type > sv_) noexcept
		{
			return { sv_.data() };
		}
		/* from string  */
		static  storage_t	make(std::basic_string< value_type > sv_)  noexcept
		{
			return { sv_.c_str() };
		}
		/* from vector  */
		static  storage_t	make(std::vector< value_type > sv_)  noexcept
		{
			return { sv_.data() };
		}
		/* from array  */
		template<size_t N>
		static  storage_t	make(std::array< value_type, N > sv_)  noexcept
		{
			return { sv_.data() };
		}

		/* from An Other */
		static storage_t make(storage_t another_) noexcept
		{
			_ASSERTE(true == another_);
			return { another_.data() };
		}

		/*
		NOTE!
		stringlen on char array filled with 0's returns 0!
		not the allocated size of the array.
		if you do not send the size and if it is found to be 0
		and no fill will be done.
		*/
		static  storage_t&
			fill(
				storage_t& buff_,
				value_type val_,
				size_t N = 0
			) noexcept
		{
			_ASSERTE(buff_);
			if (buff_)
			{
				const auto bufsiz_ = buff_.size();
				N = (N >= 0 ? N : bufsiz_);
				_ASSERTE(N <= bufsiz_);
				::std::fill(buff_.data(), buff_.data() + N, val_);
			}
			return buff_;
		}

		/*
		assign array to instance of yanb_tpl
		*/
		template<size_t N>
		static auto	assign(yanb_tpl<CHAR>& sp_, const CHAR(&arr)[N])
			noexcept -> yanb_tpl<CHAR> &
		{
			_ASSERTE(sp_);
			sp_.reset(arr);
			return sp_;
		}

	}; // helper<CHAR>

	//using buff_type = typename helper<char>::type;
	//using buff_pointer = typename helper<char>::pointer;
	//using buff_ref_type = typename helper<char>::ref_type;

	//using wbuff_type = typename helper<wchar_t>::type;
	//using wbuff_pointer = typename helper<wchar_t>::pointer;
	//using wbuff_ref_type = typename helper<wchar_t>::ref_type;



	// do not try this at home. ever.
	//extern "C"	inline void	secure_reset(void* s, size_t n) noexcept
	//{
	//	volatile char* p = (char*)s;
	//	while (n--)* p++ = 0;
	//}


	/*----------------------------------------------------------------------------
	binary comparisons
	*/

	template<typename CHAR, typename BT = helper<CHAR>  >
	inline bool operator == (
		const typename BT::ref_type left_,
		const typename BT::ref_type right_
		)
		noexcept
	{
		const size_t left_size = BT::length(left_);
		const size_t right_size = BT::length(right_);

		if (left_size != right_size)
			return false;

		return std::equal(
			left_.get(), left_.get() + left_size,
			right_.get(), right_.get() + right_size
		);
	}
	/*----------------------------------------------------------------------------
	streaming

	using buff_type = typename helper<char>::type;
	using buff_pointer = typename helper<char>::pointer;
	using buff_ref_type = typename helper<char>::ref_type;

	using wbuff_type = typename helper<wchar_t>::type;
	using wbuff_pointer = typename helper<wchar_t>::pointer;
	using wbuff_ref_type = typename helper<wchar_t>::ref_type;

	*/

#ifdef	DBJ_USES_STREAMS

	inline std::ostream& operator << (std::ostream& os,
		std::unique_ptr<char[]>& the_buffer_)
	{
		return os << the_buffer_.get();
	}

	inline std::wostream& operator << (std::wostream& os,
		std::unique_ptr<wchar_t[]>& the_buffer_)
	{
		return os << the_buffer_.get();
	}

	inline std::ostream& operator << (std::ostream& os,
		yanb_tpl<char>& the_buffer_)
	{
		return os << the_buffer_.data();
	}

	inline std::wostream& operator << (std::wostream& os,
		yanb_tpl<wchar_t>& the_buffer_)
	{
		return os << the_buffer_.data();
	}
#endif

} // dbj::chr_buf






