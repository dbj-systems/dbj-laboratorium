#pragma once
#include <string>
#include <array>
#include <vector>
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

	namespace compile_time_buffers {

		using optimal_buffer_type = ::std::array<char, BUFFER_OPTIMAL_SIZE >;
		using optimal_wbuffer_type = ::std::array<wchar_t, BUFFER_OPTIMAL_SIZE >;

		template<size_t SZ_>
		inline constexpr auto narrow(void) noexcept
		{
			/*
			will not compile if SZ_ is out of boundaries
			*/
			return ::std::array<char, SZ_>{ {char(0)} };
		}

		inline constexpr optimal_buffer_type narrow(void) noexcept
		{
			return optimal_buffer_type{ {char(0)} };
		}

		template<size_t SZ_>
		inline constexpr auto wide(void) noexcept
		{
			return ::std::array<wchar_t, SZ_ >{ {wchar_t(0)} };
		}

		inline constexpr optimal_wbuffer_type wide(void) noexcept
		{
			return optimal_wbuffer_type{ {wchar_t(0)} };
		}
	} // compile_time_buffers

#pragma region vector char_type buffer
	/*
	for runtime buffering the most comfortable and in the same time fast
	solution is vector<char_type>

	only unique_ptr<char[]> is faster than vector of  chars, by a margin
	*/
	template<typename CHAR>
	struct vector_buffer final {

		static_assert( dbj::nanolib::is_any_same_as_first_v<CHAR, char, wchar_t>,
			"\n\n" __FILE__  "\n\n\tvector_buffer requires char or wchar_t only\n\n");

		using buffer_type		=  std::vector<CHAR>;
		using narrow			=  std::vector<char>;
		using wide				=  std::vector<wchar_t>;

		static
			buffer_type make(size_t count_)
		{
			DBJ_VERIFY(count_ > 0);
			DBJ_VERIFY(DBJ_64KB >= count_);
			std::vector<CHAR> retval_(count_ + 1);
			// terminate!
			retval_[count_] = CHAR(0);
			return retval_;
		}

		static
			buffer_type make(std::basic_string_view<CHAR> sview_)
		{
			DBJ_VERIFY(sview_.size() > 0);
			DBJ_VERIFY(DBJ_64KB >= sview_.size());
			buffer_type retval_(sview_.begin(), sview_.end());
			// terminate!
			retval_.push_back(CHAR(0));
			return retval_;
		}

		static
			buffer_type make(std::unique_ptr<CHAR[]> const& upc_)
		{
			return vector_buffer::make(std::basic_string_view<CHAR>(upc_.get()));
		}

		static
			buffer_type make(std::shared_ptr<CHAR[]> const& upc_)
		{
			return vector_buffer::make(std::basic_string_view<CHAR>(upc_.get()));
		}
	}; // vector_buffer

#pragma endregion

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

#pragma region unique_ptr_buffer_type

	/*
	Following is run-time, dynamic, self cleaning char buffer
	An light wrapup of unique_ptr<CHAR[]>

	I can't claim I am some performant code expert, but this is surprisingly
	fast, beside being more confortable to use vs naked std unique poiinter.

	Perhaps the only 'trick' I used it so keep a size of the buffer when created
	in a pair with the buffer created.
	*/

	template<typename CHAR>
	struct unique_ptr_buffer_type final
	{
		static_assert( dbj::nanolib::is_any_same_as_first_v<CHAR, char, wchar_t>,
			"\n\n" __FILE__  "\n\n\tdbj_char_buffer requires char or wchar_t only\n\n");

		using type			= unique_ptr_buffer_type;
		using char_type		= CHAR;
		using value_type	= std::unique_ptr<char_type[]>;
		using pair_type		= std::pair< size_t, value_type >;

		// String Terminator
		constexpr static inline char_type ST = char_type(0);

		unique_ptr_buffer_type() = delete;

		explicit unique_ptr_buffer_type(size_t size_arg_)
			: pair_(std::make_pair(size_arg_, std::make_unique<char_type[]>(size_arg_ + 1)))
		{
			// add the string terminator
			pair_.second[size_arg_] = ST;
		}

		unique_ptr_buffer_type(std::basic_string_view<char_type> sview_)
			: pair_(std::make_pair(sview_.size(), std::make_unique<char_type[]>(sview_.size() + 1)))
		{
			std::copy(sview_.begin(), sview_.end(), this->pair_.second.get());
			// add the string terminator
			this->pair_.second[this->pair_.first] = ST;
		}

		unique_ptr_buffer_type& operator = (std::basic_string_view<char_type> sview_)
		{
			pair_ = std::make_pair(sview_.size(), std::make_unique<char_type[]>(sview_.size() + 1));
			std::copy(sview_.begin(), sview_.end(), this->pair_.second.get());
			// add the string terminator
			this->pair_.second[this->pair_.first] = ST;
			return *this;
		}

		// std::uniqe_ptr can not be copied, ditto
		unique_ptr_buffer_type(const unique_ptr_buffer_type& other_)
		{
			const auto sz_ = other_.size();
			this->pair_.first = other_.pair_.first;
			this->pair_.second = std::make_unique<char_type[]>(sz_ + 1);
			type::copy(this->pair_, other_.pair_);
			this->pair_.second[sz_] = ST;
		}

		unique_ptr_buffer_type& operator = (unique_ptr_buffer_type  const& other_) {
			const auto sz_ = other_.size();
			this->pair_.first = other_.pair_.first;
			this->pair_.second = std::make_unique<char_type[]>(sz_ + 1);
			type::copy(this->pair_, other_.pair_);
			this->pair_.second[sz_] = ST;
			return *this;
		}

		unique_ptr_buffer_type(unique_ptr_buffer_type&& other_) {
			const auto sz_ = other_.size();
			this->pair_.first = std::move(other_.pair_.first);
			this->pair_.second = std::move(other_.pair_.second);
			this->pair_.second[sz_] = ST; // ?
		}

		unique_ptr_buffer_type& operator = (unique_ptr_buffer_type&& other_)
		{
			const auto sz_ = other_.size();
			this->pair_.first = std::move(other_.pair_.first);
			this->pair_.second = std::move(other_.pair_.second);
			this->pair_.second[sz_] = ST; // ?
			return *this;
		}

		// interface

		char_type& operator [] ( unsigned idx_ ) { 
//			DBJ_VERIFY( size() >= idx_ );
			return pair_.second[idx_]; 
		}

//		// for constant instances we do not allow changing them chars
//		char_type const & operator [] ( unsigned const & idx_) const { 
////			DBJ_VERIFY( size() >= idx_);
//			return pair_.second[idx_];
//		}

		const size_t size() const noexcept { return pair_.first; };
		value_type const& buffer() const noexcept { return pair_.second; };

		operator size_t () const noexcept { return pair_.first; };
		operator value_type  const& () const noexcept { return pair_.second; };

		// utilities
		static pair_type& copy(pair_type& left_, pair_type const& right_) {

			DBJ_VERIFY(left_.first == right_.first);
			value_type& left_p_ = left_.second;
			value_type const& right_p_ = right_.second;

			for (auto j = 0; j < left_.first; j++) {
				left_p_[j] = right_p_[j];
			}
			return left_;
		}

	private:
		mutable pair_type pair_;
	}; // unique_ptr_buffer_type

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

	} // core::unvarnished 

#pragma endregion unique_ptr_buffer_type


} // dbj

