#pragma once

// #define DBJ_BUFFER_TESTING

#include "../dbj_gpl_license.h"
#include "dbj_insider.h"

#include <system_error>
#include <cassert>
#include <memory>
#include <string_view>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <cstring>

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

namespace dbj {
	namespace buf {

#pragma region yanb (yet another buffer)

		/*
		Note: static_assert() will not kick-in before instantiation
		of the template definition. Thus we use SFINAE to stop making
		illegal types, from this template.

		BUT! 

		That yanb_t<T> template declaration bellow will leave the type defined as:
		yanb<char> --- creates --> yanb_t<char,1>
		yanb_t<char> -- as a type will  not exist! ever.
		just: yanb_t<char,1>

		SO!

		the correct way of using it "upstream" for SFINAE is:
		yanb_t<char>::type
		*/

		template<
			typename T,
			std::enable_if_t<
			std::is_same_v<char, T> ||
			std::is_same_v<wchar_t, T>
			, bool > = true
		>
			struct yanb_t final
		{
			using type = yanb_t;
			using data_type = T;
			using value_type = std::shared_ptr<data_type>;
			using value_type_ref = std::reference_wrapper<value_type>;

			struct inner final {

				static void set_payload(
					std::reference_wrapper< std::shared_ptr<char> > storage_ref,
					char const * payload_
				) {
					assert(payload_ != nullptr);
					storage_ref.get().reset(_strdup(payload_));
				}

				static void set_payload(
					std::reference_wrapper< std::shared_ptr<wchar_t> > storage_ref,
					wchar_t const * payload_
				) {
					assert(payload_ != nullptr);
					storage_ref.get().reset(_wcsdup(payload_));
				}
				/*--------------------------------------------------------*/
				static size_t length(
					std::reference_wrapper< std::shared_ptr<char> > storage_ref
				) {
					assert(storage_ref.get());
					return (std::strlen(storage_ref.get().get()));
				}

				static size_t length(
					std::reference_wrapper< std::shared_ptr<wchar_t> > storage_ref
				) {
					assert(storage_ref.get());
					return (std::wcslen(storage_ref.get().get()));
				}

			}; // inner

			type & reset(data_type const * payload_)
			{
				inner::set_payload(this->data_, payload_);
				return *this;
			}

			yanb_t(data_type const * payload_) {
				inner::set_payload(this->data_, payload_);
			}

			size_t size() noexcept { return inner::length(this->data_); }
			size_t size() const noexcept { return inner::length(this->data_); }

			data_type * data() noexcept { return this->data_.get(); }
			data_type const * data() const noexcept { return this->data_.get(); }

			operator data_type * () noexcept { return this->data_.get(); }
			operator data_type const * () const noexcept { return this->data_.get(); }

			operator bool() noexcept { return data_.operator bool(); }
			operator bool() const noexcept { return data_.operator bool(); }

			yanb_t() = default;
			yanb_t(yanb_t const &) = default;
			yanb_t& operator = (yanb_t const &) = default;
			yanb_t(yanb_t &&) = default;
			yanb_t& operator = (yanb_t &&) = default;

		private:
			value_type data_{};

#ifdef DBJ_BUFFERS_IOSTREAMS

			/*
			Careful, a big and recurring gotcha!
			One can use both std::cout and std::wcout in one program.
			Do not do that. The issue:

			std::cout << yanb_t<wchar_t>(L"Hello?")
			// prints: 0x000FFCD27, aka "the pointer address"

			std::cout does not use std::wostream
			std::wcout does not use std::ostream

			Bellow is a version for programs targeting std::cout.
			The wide version is trivial if you understand this one.
			*/
			friend  std::ostream &
				operator << (std::ostream & os, yanb_t<data_type> const & bufy_)
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

		}; // yanb_t<T>

		using yanb = yanb_t<char>;
		using yanwb = yanb_t<wchar_t>;

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

		// it is not very usefull to have buffers of 
		// unlimited size in programs
		// thus we will define the upper limit
		// it is
		// in posix terms BUFSIZ * 2 * 64 aka 64KB
		constexpr std::size_t max_length = UINT16_MAX;

		namespace {
			using namespace ::dbj::core::util;
			using inside_1_and_max =
				insider<size_t, 1,
				::dbj::buf::max_length
				, insider_error_code_throw_policy
				>;
		}

		// for pre C++17 kid's
		namespace inner {
			template<class T,
				std::enable_if_t<is_array_v<T> && std::extent_v<T> == 0, int> = 0>
				[[nodiscard]] inline std::unique_ptr<T> 
				dbj_make_unique(size_t Size_)
			{	// make a unique_ptr
				typedef std::remove_extent_t<T> E;
				return (
					  std::unique_ptr<T>( new E[Size_]() )
					);
			}
		}

		// this is basically just set of helpers
		// to use the buffer from above
		template<typename CHAR> struct smart_buf final
		{
			using type = smart_buf;
			using value_type = CHAR;

			using storage_t = yanb_t<CHAR>;

			using pointer = typename storage_t::value_type ;
			using ref_type = type & ;

			// always use this function to make  
			// buff of particular size
			// array is sized & zeroed
			static storage_t make(inside_1_and_max size_) noexcept {
				return 
				{
				(std::make_unique<value_type[]>(size_ + 1)).get()
				};
			}

			static bool
				empty(
					pointer buff_
				) noexcept
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
			static auto length( pointer buf_)
				noexcept -> inside_1_and_max
			{
				return std::strlen(buf_.get());
			}

			// the meta maker ;)
			static storage_t make(
				value_type const * first_, value_type const * last_
			) noexcept
			{
				assert(first_ && last_);
				std::basic_string_view<value_type> sv_ (
					first_, std::distance(first_, last_)
					);
				assert(sv_.size() > 0);
				return { sv_.data() }; 
			}
			// here we depend on a zero terminated string
			static storage_t make(value_type const * first_) noexcept
			{
				assert(first_);
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
			static  storage_t	make(std::basic_string< value_type > sv_ )  noexcept
			{
				return { sv_.c_str() };
			}
			/* from vector  */
			static  storage_t	make(std::vector< value_type > sv_ )  noexcept
			{
				return { sv_.data() };
			}
			/* from array  */
			template<size_t N>
			static  storage_t	make(std::array< value_type, N > sv_ )  noexcept
			{
				return { sv_.data() };
			}

			/* from An Other */
			static storage_t make( storage_t another_) noexcept
			{
				assert( true == another_);
				return { another_.data() };
			}

			/*
			NOTE! 
			stringlen on char array filled with 0's returns 0!
			not the allocated size of the array.
			if you do not send the size and if it is found to be 0
			and no fill will be done.
			*/
			static  storage_t &
				fill(
					storage_t & buff_,
				value_type val_,
				size_t N = 0
				) noexcept
			{
				assert(buff_);
				if ( buff_ )
				{
					const auto bufsiz_ = buff_.size();
					N = (N >= 0 ? N : bufsiz_);
					assert(N <= bufsiz_);
					::std::fill(buff_.data(), buff_.data() + N, val_);
				}
				return buff_;
			}
		}; // smart_buf<CHAR>

		using buff_type = typename smart_buf<char>::type;
		using buff_pointer = typename smart_buf<char>::pointer;
		using buff_ref_type = typename smart_buf<char>::ref_type;

		using wbuff_type = typename smart_buf<wchar_t>::type;
		using wbuff_pointer = typename smart_buf<wchar_t>::pointer;
		using wbuff_ref_type = typename smart_buf<wchar_t>::ref_type;

		/*
		assign array to instance of yanb_t
		note: "any" type will do as long as std::unique_ptr
		will accept it
		*/
		template<typename C, size_t N>
		inline auto	assign(yanb_t<C> & sp_, const C(&arr)[N])
			noexcept -> yanb_t<C> &
		{
			assert(sp_);
			sp_.reset(arr);
			return sp_;
		}

		// do not try this at home. ever.
		extern "C"	inline void	secure_reset(void *s, size_t n) noexcept
		{
			volatile char *p = (char *)s;
			while (n--) *p++ = 0;
		}


		/*----------------------------------------------------------------------------
		binary comparisons
		*/

		template<typename CHAR, typename BT = smart_buf<CHAR>  >
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
		
		using buff_type = typename smart_buf<char>::type;
		using buff_pointer = typename smart_buf<char>::pointer;
		using buff_ref_type = typename smart_buf<char>::ref_type;

		using wbuff_type = typename smart_buf<wchar_t>::type;
		using wbuff_pointer = typename smart_buf<wchar_t>::pointer;
		using wbuff_ref_type = typename smart_buf<wchar_t>::ref_type;

		*/
		inline std::ostream & operator << (std::ostream & os, 
			std::unique_ptr<char[]> & the_buffer_)
		{
			return os << the_buffer_.get();
		}

		inline std::wostream & operator << (std::wostream & os, 
			std::unique_ptr<wchar_t[]> &  the_buffer_)
		{
			return os << the_buffer_.get();
		}

		inline std::ostream & operator << (std::ostream & os, 
			yanb_t<char> & the_buffer_)
		{
			return os << the_buffer_.data();
		}

		inline std::wostream & operator << (std::wostream & os, 
			yanb_t<wchar_t> &  the_buffer_)
		{
			return os << the_buffer_.data();
		}

	} // buf
} // dbj

#ifdef DBJ_BUFFER_TESTING
#define TU(x) std::wcout << std::boolalpha << L"\nExpression: '" << (#x) << L"'\n\tResult: '" << (x) << L"'\n"
#include <iostream>

namespace {
	inline bool dbj_testing_dbj_unique_ptr_buffer()
	{

		auto test_1 = [] (auto C, auto specimen) 
		{
			using namespace ::dbj::buf;

			using T = decltype( std::decay_t< C > );
			TU(smart_buf<T>::make(BUFSIZ));
			TU(smart_buf<T>::make(specimen));
			TU(smart_buf<T>::make(std::basic_string<T>(specimen)));
			TU(smart_buf<T>::make(std::basic_string_view<T>(specimen)));

			auto buf = smart_buf<T>::make(BUFSIZ);
			TU(smart_buf<T>::fill(buf, C));

			auto sec = smart_buf<T>::make(buf);
			TU( sec == buf);

		};
			test_1('*', "narrow string");
			test_1(L'*', L"wide string");
		return true;
	} // testing_dbj_buffer

	static auto dbj_testing_dbj_buffer_result
		= dbj_testing_dbj_unique_ptr_buffer();
}

#endif // DBJ_BUFFER_TESTING
#undef TU
#undef DBJ_BUFFER_TESTING
