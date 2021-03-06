#ifndef _DBJ_BUFFERS_INC
#define _DBJ_BUFFERS_INC

#include "dbj_utils_core.h"
#include "dbj_insider.h"

/*
Basically do not use std::string as buffer of char_types. It is crazy slow.
std::string is not a char_type buffer
std::string was invented an astonishing number of years back.
Officialy approx in 1998. Although it appeared in "STL" in appox 1983.

ditto ...
*/
namespace dbj {
	// think of std::string as text manipulation tool
	using text = std::string;
	using wtext = std::wstring;
} // dbj
/*
DBJ BUFFERING

we do cater only for char and wchar_t, character types

 generaly 'ct_' is a prefix of "compile  time" type name
 generaly 'rt_' is a prefix of "run-time time" type name

 string view

	once made string view is fast,  almost as vector<char>
	it is also having most of std::string methods for comfort

	to make runtime buffer, quickly and efficiently and at compile time
	string view literal works the best *IF* there is a string literal available

	constexpr auto ct_string_literal = "String"sv ;

	But. This is a constant view. This is not a buffer one can write into or change its chars.

*/

namespace dbj {
	// it is not very usefull to have buffers of 
	// unlimited size in programs
	// thus we will define the upper limit
	// it is
	// in posix terms BUFSIZ * 2 * 64 aka 64KB
	constexpr inline std::size_t DBJ_64KB = UINT16_MAX;
	constexpr inline std::size_t DBJ_MAX_BUFF_LENGTH = ::dbj::DBJ_64KB;

	// very doubtfull this can be decided
	// this should be equal to BUFSIZ
	// but that is invetned for setvbuf function, actually
	constexpr inline std::size_t BUFFER_OPTIMAL_SIZE{ 2 * UINT8_MAX };

	using rt_0_to_max =
		::dbj::core::util::insider<size_t, 0, DBJ_MAX_BUFF_LENGTH, ::dbj::core::util::insider_error_code_throw_policy>;

	template<unsigned K>
	using ct_0_to_max = ::dbj::inside_inclusive_t< unsigned, K, 0, DBJ_MAX_BUFF_LENGTH >;

	/*
	usage of the above:

	compile time, failed to specialize alias template -->
	inline inclusive_0_and_max<DBJ_MAX_BUFF_LENGTH + 42 > index_test_;

	runtime -- this will throw the exception as soon as program "passes" through here
	inline rt_0_to_max b0max = DBJ_MAX_BUFF_LENGTH + 2;

	For compile timer char buffer-ing
	my preffered buffer type is std::array<char_type,N>
	2019-02-11 dbj.org
*/
	namespace compile_time_buffers {

		using optimal_buffer_type = ::std::array<char, BUFFER_OPTIMAL_SIZE >;
		using optimal_wbuffer_type = ::std::array<wchar_t, BUFFER_OPTIMAL_SIZE >;

		template<size_t SZ_>
		inline constexpr auto narrow(void) noexcept
		{
			/*
			will not compile if SZ_ is out of boundaries
			*/
			return ::std::array<char, ct_0_to_max<SZ_>::value >{ {char(0)} };
		}

		inline constexpr auto narrow(void) noexcept
		{
			return optimal_buffer_type{ {char(0)} };
		}

		template<size_t SZ_>
		inline constexpr auto wide(void) noexcept
		{
			return ::std::array<wchar_t, ct_0_to_max<SZ_>::value >{ {wchar_t(0)} };
		}

		inline constexpr auto wide(void) noexcept
		{
			return optimal_wbuffer_type{ {wchar_t(0)} };
		}
	} // compile_time_buffers



#ifdef DBJ_DEPRECATED_BUFFERS

#pragma region naked unique_ptr as buffer

	namespace unique_ptr_buffers {

		template <typename CHAR>
		using up_type = typename std::unique_ptr<CHAR[]>;

		//using up_buffer = std::unique_ptr<char[]>;
		//using up_buffer_w = std::unique_ptr<wchar_t[]>;

		inline auto
			up_buffer_size_suspicious = [](auto const& source)
			-> std::size_t
		{
			if (source.get() == nullptr)
				return 0U;
			return (sizeof(source.get()) / sizeof(source.get()[0]));
		};

		template< class CHAR  >
		inline std::unique_ptr<CHAR[]> up_buffer_copy(std::unique_ptr<CHAR[]> const& source)
		{
			const auto sze_ = up_buffer_size_suspicious(source);
			auto uptr_ = std::make_unique<CHAR[]>(sze_);
			std::copy(source.get(), source.get() + sze_, uptr_.get());
			return uptr_;
		}

		template< class char_type,
			typename UP = std::unique_ptr<char_type[]> >
			inline constexpr UP up_buffer_make(size_t const& sze_)
		{
			UP up = std::make_unique<char_type[]>(sze_ + 1);
			up[sze_] = char_type(0);
			return up;
		}

		template< class char_type,
			typename UP = std::unique_ptr<char_type[]>,
			typename SV = std::basic_string_view<char_type>
		>
			inline constexpr UP up_buffer_make(char_type const* sliteral_)
		{
			SV sview_{ sliteral_ };
			UP up = up_buffer_make<char_type>(sview_.size());

			std::copy(sview_.begin(), sview_.end(), up.get());
			// add the string terminator
			up[sview_.size()] = char_type(0);
			return up;
		}

		template< class char_type,
			typename UP = std::unique_ptr<char_type[]>,
			typename SV = std::basic_string_view<char_type>
		>
			inline constexpr UP up_buffer_make(char_type const* sliteral_, size_t length_)
		{
			UP up = up_buffer_make<char_type>(length_ + 1);
			std::copy(sliteral_, sliteral_ + length_, up.get());
			// add the string terminator
			up[length_] = char_type(0);
			return up;
		}

	} // unique_ptr_buffers namespace 

#pragma endregion



	/*
	unvarnished is synonim for literal
	*/
	namespace core::unvarnished {

		// using namespace ::dbj::unique_ptr_buffer;

		inline  std::unique_ptr<char[]>
			operator "" _buffer(const char* sliteral_, size_t length_)
		{
			return unique_ptr_buffers::up_buffer_make(sliteral_, length_);
		}

		inline  std::unique_ptr<wchar_t[]>
			operator "" _buffer(const wchar_t* sliteral_, size_t length_)
		{
			return unique_ptr_buffers::up_buffer_make(sliteral_, length_);
		}

		inline  std::pair<  std::size_t, std::unique_ptr<char[]> >
			operator "" _buffer_pair(const char* sliteral_, size_t length_)
		{
			return std::pair(
				length_,
				unique_ptr_buffers::up_buffer_make(sliteral_, length_)
			);
		}

		inline  std::pair<  std::size_t, std::unique_ptr<wchar_t[]> >
			operator "" _buffer_pair(const wchar_t* sliteral_, size_t length_)
		{
			return std::pair(
				length_,
				unique_ptr_buffers::up_buffer_make(sliteral_, length_)
			);
		}

	} // core::unvarnished 

#pragma endregion unique_ptr_buffer_type

#endif // DBJ_DEPRECATED_BUFFERS

	namespace literals {
		inline  std::vector< char >
			operator "" _v_buffer(const char* sliteral_, size_t)
		{
			return vector_buffer<char>::make(sliteral_);
		}

		inline  std::vector< wchar_t >
			operator "" _v_buffer(const wchar_t* sliteral_, size_t)
		{
			return vector_buffer<wchar_t>::make(sliteral_);
		}
	} // literals

} // dbj

#endif // !_DBJ_BUFFERS_INC
