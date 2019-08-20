#pragma once

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

 generaly 'ct_' is a prefix of "compile  time" type name
 generaly 'rt_' is a prefix of "run-time time" type name
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

	using rt_0_and_max =
		::dbj::core::util::insider<size_t, 0, DBJ_MAX_BUFF_LENGTH, ::dbj::core::util::insider_error_code_throw_policy>;

	template<unsigned K>
	using ct_0_and_max = ::dbj::inside_inclusive_t< unsigned, K, 0, DBJ_MAX_BUFF_LENGTH >;

	/*
	usage of the above:

	compile time, failed to specialize alias template -->
	inline inclusive_0_and_max<DBJ_MAX_BUFF_LENGTH + 42 > index_test_;

	runtime -- this will throw the exception as soon as program "passes" through here
	inline between_0_and_max b0max = DBJ_MAX_BUFF_LENGTH + 2;
	*/

	/////////////////////////////////////////////////////////////////////////////////////////////////
/*

For compile timer char buffer-ing
my preffered buffer type is std::array<>
2019-02-11 dbj.org
*/
	namespace compile_time_buffers {

		using optimal_compile_time_buffer_type = ::std::array<char, BUFFER_OPTIMAL_SIZE >;
		using optimal_compile_time_wbuffer_type = ::std::array<wchar_t, BUFFER_OPTIMAL_SIZE >;

		inline constexpr auto optimal_buffer(void) noexcept
		{
			return optimal_compile_time_buffer_type{ {char(0)} };
		}

		inline constexpr auto optimal_wbuffer(void) noexcept
		{
			return optimal_compile_time_wbuffer_type{ {wchar_t(0)} };
		}
	}

#pragma region vector char_type buffer
	/*
	for runtime buffering the most comfortable and in the same time fast
	solution is vector<char_type>

	only unique_ptr<char[]> is faster than vector of  chars, by a margin
	*/
	template<typename CHAR>
	struct vector_buffer final {

		static_assert(is_any_same_as_first_v<CHAR, char, wchar_t>,
			"\n\n" __FILE__  "\n\n\tvector_buffer requires char or wchar_t only\n\n");

		using narrow	=  std::vector<char>;
		using wide		=  std::vector<wchar_t>;

		static
			std::vector<CHAR> make(size_t count_)
		{
			DBJ_VERIFY(count_ > 0);
			DBJ_VERIFY(DBJ_64KB >= count_);
			std::vector<CHAR> retval_(count_ + 1);
			// terminate!
			retval_[count_] = CHAR(0);
			return retval_;
		}

		static
			std::vector<CHAR> make(std::basic_string_view<CHAR> sview_)
		{
			DBJ_VERIFY(sview_.size() > 0);
			DBJ_VERIFY(DBJ_64KB >= sview_.size());
			std::vector<CHAR> retval_(sview_.begin(), sview_.end());
			// terminate!
			retval_.push_back(CHAR(0));
			return retval_;
		}

		static
			std::vector<CHAR> make(std::unique_ptr<CHAR[]> const& upc_)
		{
			return vector_buffer::make(std::basic_string_view<CHAR>(upc_.get()));
		}

		static
			std::vector<CHAR> _make(std::shared_ptr<CHAR[]> const& upc_)
		{
			return vector_buffer::make(std::basic_string_view<CHAR>(upc_.get()));
		}
	}; // vector_buffer

#pragma endregion vector char_type buffer

#pragma region string view  char_type buffer
	/*
	once made string view is fast,  almost as vector<char>
	it is also having std methods for comfort

	CAUTION! THIS IS OF LIMITED VALUE

	Following method deliver string view on one single shared char buffer of particular size

	NOTE! at runtime you can *not* change the data inside the string view instance,
	unless you are naughty:

		  const_cast<char&>(sview_[0]) = '!' ;
	*/
	template<typename CHAR>
	inline auto runtime_shared_string_view_buffer(size_t count_)
	{
		static_assert(is_any_same_as_first_v<CHAR, char, wchar_t>,
			"\n\n" __FILE__  "\n\n\tdbj_char_buffer requires char or wchar_t only\n\n");
		// string view has no facility to create char buffer of certain size
		// this is how we allocate smart buffer for it at runtime
		static auto shared_up = [&]() {
			auto up_ = std::make_unique<CHAR[]>(count_ + 1);
			up_[count_] = CHAR(0);
			return up_;
		}();

		return std::string_view(shared_up.get(), count_);
	}

	/*
	 here we create an "empty" string veiw of certain size at compile time

	 CAUTION! go easy on this, as this is naturally an stack memory affair
	 for an populated compile time string view please use the literal available
	 constexpr auto svw = "Literal"sv ;

	 CAUTION! Same as method above this delivers string view onto a series of
	 shared char buffer made on stack

	*/
	template<typename CHAR, size_t N>
	inline constexpr auto compile_time_shared_string_view_buffer()
	{
		static_assert(is_any_same_as_first_v<CHAR, char, wchar_t>,
			"\n\n" __FILE__  "\n\n\tdbj_char_buffer requires char or wchar_t only\n\n");
		// string view has no facility to create char buffer of certain size
		// this is how we allocate buffer for it at runtime
		static auto std_array_ = [&]() {
			// terminate, just to be sure
			std::array<CHAR, N + 1> arr_;
			arr_[N] = CHAR(0);
			return arr_;
		} ();

		return std::string_view(std_array_.data());
	}

#pragma region string view  char_type buffer


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

#pragma endregion unique_ptr copy

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
		static_assert(is_any_same_as_first_v<CHAR, char, wchar_t>,
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

		char_type& operator [] (size_t idx_) { return pair_.second[idx_]; }
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

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"

/* standard suffix for every dbj header */
#pragma comment( user, DBJ_BUILD_STAMP ) 