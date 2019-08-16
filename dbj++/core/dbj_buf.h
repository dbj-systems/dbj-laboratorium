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
		of the template definition. Thus we **might* use SFINAE to stop making
		illegal types, from this template.

		BUT, if we do this:

			template<
		typename T,
		std::enable_if_t< std::is_same_v<char, T> || std::is_same_v<wchar_t, T>, bool > = true
                 	>
		struct yanb_tpl final
	    {    } ;

		The type of the  yanb_tpl template defined will be "poluted". Namely it will be:

		yanb_tpl<char,true> and yanb_tpl<wchar_t, true>

		SO ... we will revert to static_assert as bellow
		*/

	template< typename T >
		struct yanb_tpl final
	{
		static_assert( std::is_same_v<char, T> || std::is_same_v<wchar_t, T> , "\n\n" __FILE__ "\n\nyanb_tpl requires char or wchar_t only\n\n" );

		using type = yanb_tpl;
		using char_type = T;
		using value_type = std::shared_ptr<char_type[]>;
		using value_type_ref = std::reference_wrapper<value_type>;

		type& reset(char_type const* payload_)
		{
			core::assign(this->data_, payload_);
			return *this;
		}

		yanb_tpl& assign(value_type const& another_smart_ptr) {
			this->data_ = another_smart_ptr;
			return *this;
		}

		yanb_tpl& operator = (value_type const& another_smart_ptr) {
			this->data_ = another_smart_ptr;
			return *this;
		}


		yanb_tpl( value_type const & another_smart_ptr) {
			this->data_ = another_smart_ptr;
		}

		yanb_tpl (std::size_t sze_)
			: data_(std::shared_ptr<char_type[]>(new char_type[sze_ + 1]) )
		{
			DBJ_VERIFY( sze_ > 0 );
			DBJ_VERIFY( chr_buf::DBJ_MAX_BUFF_LENGTH >= sze_ );

			// added +1 for String Terminator, now set it
			constexpr char_type ST = char_type(0);
			data_[sze_] = ST ;
		}

		yanb_tpl(char_type const* payload_) {
			core::assign(this->data_, payload_);
		}

		size_t size() noexcept { return core::length(this->data_); }
		size_t size() const noexcept { return core::length(this->data_); }

		char_type* data() noexcept { return this->data_.get(); }
		char_type const* data() const noexcept { return this->data_.get(); }

		operator char_type* () noexcept { return this->data_.get(); }
		operator char_type const* () const noexcept { return this->data_.get(); }

		operator bool() noexcept { return data_.operator bool(); }
		operator bool() const noexcept { return data_.operator bool(); }

		yanb_tpl() = default;
		yanb_tpl(yanb_tpl const&) = default;
		yanb_tpl& operator = (yanb_tpl const&) = default;
		yanb_tpl(yanb_tpl&&) = default;
		yanb_tpl& operator = (yanb_tpl&&) = default;

	private:
		value_type data_{};

#ifdef DBJ_USES_STREAMS

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
			operator << (std::ostream& os, yanb_tpl<char_type> const& bufy_)
		{
			if constexpr (std::is_same_v<wchar_t, char_type>) {
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
    // narrow and wide buffer are defined here
	using yanb =  yanb_tpl<char>;
	using yanwb = yanb_tpl<wchar_t>;

#pragma endregion yanb (yet another buffer)

	// this is basically just set of helpers
	// to use the buffer from above
	template< typename CHAR	>
		struct yanb_helper final
	{
		static_assert(std::is_same_v<char, CHAR> || std::is_same_v<wchar_t, CHAR>, "\n\n" __FILE__ "\n\nhelper requires char or wchar_t only\n\n");

		using type = yanb_helper;
		using char_type = CHAR;

		using storage_t = yanb_tpl<CHAR>;

		using pointer = typename storage_t::char_type;
		using ref_type = type &;

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
		And that is the point of using it
		*/
		static auto length(pointer buf_) noexcept -> between_0_and_max
		{
			if constexpr (std::is_same_v<wchar_t, char_type>)
			{
				return std::wcslen(buf_.get());
			}
			else {
				return std::strlen(buf_.get());
			}
		}

		// the meta maker ;)
		// this will cause serious SEGV is first and last are not 
		// pointing to the same memory block
		static storage_t make(
			char_type const* first_, char_type const* last_
		) noexcept
		{
			_ASSERTE(first_ && last_);
			std::basic_string_view<char_type> sv_(
				first_, std::distance(first_, last_)
			);
			_ASSERTE(sv_.size() > 0);
			return { sv_.data() };
		}
		// here we depend on a zero terminated string
		static storage_t make(char_type const* first_) noexcept
		{
			_ASSERTE(first_);
			return { first_ };
		}
		/*	from array of char's	*/
		template<size_t N>
		static storage_t make(const char_type(&charr)[N]) noexcept
		{
			return { charr };
		}
		/* from string_view */
		static  storage_t	make(std::basic_string_view< char_type > sv_) noexcept
		{
			return { sv_.data() };
		}
		/* from string  */
		static  storage_t	make(std::basic_string< char_type > sv_)  noexcept
		{
			return { sv_.c_str() };
		}
		/* from vector  */
		static  storage_t	make(std::vector< char_type > sv_)  noexcept
		{
			return { sv_.data() };
		}
		/* from array  */
		template<size_t N>
		static  storage_t	make(std::array< char_type, N > sv_)  noexcept
		{
			return { sv_.data() };
		}

		/* from An Other */
		static storage_t make(storage_t another_) noexcept
		{
			// using bool operator on the shared_ptr
			DBJ_VERIFY(true == another_);
			return { another_ };
		}

		static  storage_t&	fill( storage_t& buff_,	char_type val_	) noexcept
		{
			DBJ_VERIFY( true == buff_);
			const auto bufsiz_ = buff_.size();
			DBJ_VERIFY( bufsiz_ > 0 );
				::std::fill(buff_.data(), buff_.data() + bufsiz_, val_);
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

	}; // yanb_helper<CHAR>


	/*----------------------------------------------------------------------------
	binary comparisons
	*/

	template<typename CHAR >
	inline bool operator == (
		const yanb_tpl<CHAR> & left_,	const yanb_tpl<CHAR> & right_
		)
		noexcept
	{
		const auto left_size = left_.size();
		const auto right_size = left_.size();

		if (left_size != right_size)
			return false;

		return std::equal(
			left_.data() , left_.data() + left_size,
			right_.data(), right_.data() + right_size
		);
	}	
	
	template<typename CHAR >
	inline bool operator < (
		const yanb_tpl<CHAR> & left_,	const yanb_tpl<CHAR> & right_
		)
		noexcept
	{
		const auto left_size = left_.size();
		const auto right_size = left_.size();

		if (left_size >= right_size)
			return false;

		return std::less(
			left_.data(), left_.data() + left_size,
			right_.data(), right_.data() + right_size
		);
	}

	/*----------------------------------------------------------------------------*/

	using narrow_helper_type	= typename yanb_helper<char>;
	using wide_helper_type		= typename yanb_helper<wchar_t>;

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






