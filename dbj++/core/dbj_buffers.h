#pragma once

namespace dbj {
	// it is not very usefull to have buffers of 
// unlimited size in programs
// thus we will define the upper limit
// it is
// in posix terms BUFSIZ * 2 * 64 aka 64KB
	constexpr inline std::size_t DBJ_64KB = UINT16_MAX;

	// very doubtfull this can be decided
	// this should be equal to BUFSIZ
	// but that is invetned for setvbuf function, actually
	constexpr std::size_t BUFFER_OPTIMAL_SIZE{ 2 * UINT8_MAX }; 

/*
Basically do not use std::string as buffer; it is crazy slow

For compile timer char buffer-ing
my preffered buffer type is std::array<>
2019-02-11 dbj.org
*/
	using optimal_compile_time_buffer_type = ::std::array<char, BUFFER_OPTIMAL_SIZE >;
	using optimal_compile_time_wbuffer_type = ::std::array<wchar_t, BUFFER_OPTIMAL_SIZE >;

	inline constexpr auto optimal_buffer(void) noexcept
	{ return optimal_compile_time_buffer_type{ {char(0)} }; }

	inline constexpr auto optimal_wbuffer(void) noexcept
	{ return optimal_compile_time_wbuffer_type{ {wchar_t(0)} }; }

	/*
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
	};

	/*
	run-time, dynamic, self cleaning char buffer
	*/

	template<typename CHAR>
	struct unique_ptr_buffer final
	{
		static_assert(is_any_same_as_first_v<CHAR, char, wchar_t>,
			"\n\n" __FILE__  "\n\n\tdbj_char_buffer requires char or wchar_t only\n\n");

		using type			= unique_ptr_buffer;
		using char_type		= CHAR;
		using value_type	= std::unique_ptr<char_type[]>;
		using pair_type		= std::pair< size_t, value_type >;

		// String Terminator
		constexpr static inline char_type ST = char_type(0);

		unique_ptr_buffer() = delete;

		explicit unique_ptr_buffer(size_t size_arg_)
			: pair_(std::make_pair(size_arg_, std::make_unique<char_type[]>(size_arg_ + 1)))
		{
			// add the string terminator
			pair_.second[size_arg_] = ST;
		}

		unique_ptr_buffer(std::basic_string_view<char_type> sview_)
			: pair_(std::make_pair(sview_.size(), std::make_unique<char_type[]>(sview_.size() + 1)))
		{
			std::copy(sview_.begin(), sview_.end(), this->pair_.second.get());
			// add the string terminator
			this->pair_.second[this->pair_.first] = ST;
		}

		unique_ptr_buffer& operator = (std::basic_string_view<char_type> sview_)
		{
			pair_ = std::make_pair(sview_.size(), std::make_unique<char_type[]>(sview_.size() + 1));
			std::copy(sview_.begin(), sview_.end(), this->pair_.second.get());
			// add the string terminator
			this->pair_.second[this->pair_.first] = ST;
			return *this;
		}

		// std::uniqe_ptr can not be copied, ditto
		unique_ptr_buffer(const unique_ptr_buffer& other_)
		{
			const auto sz_ = other_.size();
			this->pair_.first = other_.pair_.first;
			this->pair_.second = std::make_unique<char_type[]>(sz_ + 1);
			type::copy(this->pair_, other_.pair_);
			this->pair_.second[sz_] = ST;
		}

		unique_ptr_buffer& operator = (unique_ptr_buffer const& other_) {
			const auto sz_ = other_.size();
			this->pair_.first = other_.pair_.first;
			this->pair_.second = std::make_unique<char_type[]>(sz_ + 1);
			type::copy(this->pair_, other_.pair_);
			this->pair_.second[sz_] = ST;
			return *this;
		}

		unique_ptr_buffer(unique_ptr_buffer&& other_) {
			const auto sz_ = other_.size();
			this->pair_.first = std::move(other_.pair_.first);
			this->pair_.second = std::move(other_.pair_.second);
			this->pair_.second[sz_] = ST; // ?
		}

		unique_ptr_buffer& operator = (unique_ptr_buffer&& other_)
		{
			const auto sz_ = other_.size();
			this->pair_.first = std::move(other_.pair_.first);
			this->pair_.second = std::move(other_.pair_.second);
			this->pair_.second[sz_] = ST; // ?
			return *this;
		}

		// interface

		char_type& operator [] ( size_t idx_ ) { return pair_.second[idx_]; }
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
	};



} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"

/* standard suffix for every dbj header */
#pragma comment( user, DBJ_BUILD_STAMP ) 