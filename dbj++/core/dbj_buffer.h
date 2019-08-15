#pragma once

//#include <system_error>
//#include <cassert>
//#include <memory>
//#include <string_view>
//#include <type_traits>
//#include <cstdint>
//#include <cstddef>
//#include <cstring>
//#include <vector>

#include "../dbj_gpl_license.h"

#define DBJ_COMFY_BUFFER
#ifdef DBJ_COMFY_BUFFER

namespace dbj {
	namespace chr_buf {
		/*
	--------------------------------------------------------------------------------

	2019 FEB	dbj@dbj.org

	dbj::chr_buf is for fast-and-dangerous code and bellow is a bit more
	comfortable api for	dynamic	char buffer.
	Slower than naked dbj::fmt::yanb_t<T> but still approx 2x to 3x faster vs vector<char>

	btw: I have the code for measuring the difference,in case you are hotly
	against *not* using vector<char>.

	Generaly the smaller the buffer the faster it is vs vector<char>.
	For *normal* buffer sizes (posix BUFSIZ or multiplies of it)
	using this type has a lot in favour.
*/
		// using smart_buf_char_t = char; //  typename smart_buf<char>;

		struct buffer final
		{
			using smart_buf_t	= narrow_type ;
			// yanb_t<CHAR>; aka
			using storage_t		= yanb ;
			using type			= buffer;
			using reference_type = type &;
			// this is the std::share_ptr<char>
			using pointer		=  narrow_type; //  typename smart_buf_t::pointer;

			using char_type		= typename smart_buf_t::element_type;
			using iterator		= char_type *;
			using citerator		= char_type const*;
		private:
			// the data is here
			storage_t data_{}; // size == 0
		public:

			// this is important
			bool valid() const noexcept { return this->data_.operator bool(); }

			// allocated but empty yields true
			bool empty() const noexcept {
				return (valid() && this->data_[0] != '\0');
			}

			iterator data() noexcept { return data_.data(); }
			citerator data() const noexcept { return data_.data(); }

			size_t const size() const noexcept { return this->data_.size(); }
			size_t size() noexcept { return data_.size(); }

			// default 
			buffer() = default;

			// sized but empty buffer
			explicit buffer(inside_1_and_max new_size) noexcept
			{
				data_.reset(::dbj::chr_buf::yanb_helper<char_type>::make(new_size));
			}

			// copy
			buffer(const buffer& another_) noexcept
			{
				this->assign(another_);
			}

			buffer& operator = (const buffer& another_) noexcept
			{
				if (&another_ != this) {
					this->assign(another_);
				}
				return *this;
			}
			// NOTE! it was found manualy implemented
			// move as in here, speeds up the moving by min 200%
			buffer(buffer&& another_) noexcept {
				std::swap(this->data_, another_.data_);
			}

			buffer& operator = (buffer&& another_) noexcept {
				if (&another_ != this) {
					std::swap(this->data_, another_.data_);
				}
				return *this;
			}

			// construct from native charr array
			// i.e. the string literal
			template < typename T, size_t N,
				std::enable_if_t< std::is_same_v<T, char>, int> = 0
			>
				buffer(const T(&charr)[N]) noexcept
			{
				assign(charr, charr + N);
			}

			void assign(const buffer& another_) noexcept
			{
				this->data_ = another_.data_;
			}

			void assign(char const* from_, char const* to_) noexcept
			{
				assert(from_ && to_);
				std::string sv_(from_, to_); // normalize ?
				this->data_.reset(sv_.c_str()); // take ownership?
			}

			void assign(char const* from_) noexcept
			{
				assert(from_);
				this->data_.reset(from_);
			}

			// notice the usage of the dbj::insider definition
			// as  argument type
			char_type & operator [] (inside_1_and_max idx_) const
			{
				return const_cast<char_type &>(this->data_[idx_] );
			}

			// to be removed
			[[deprecated]]
			char_type ** const address() const noexcept {
				char_type* p = (char_type*)std::addressof(data_[0]);
				return std::addressof(p);
			}

			iterator  begin() noexcept { return data_.data(); }
			iterator  end()   noexcept { return data_.data() + data_.size(); }
			citerator begin() const noexcept { return data_.data(); }
			citerator end()   const noexcept { return data_.data() + data_.size(); }

			char_type& front() noexcept { assert(valid()); return data_[0]; }
			char_type& back()  noexcept { assert(valid()); return data_[data_.size() - 1]; }

			buffer const& fill(char val_) noexcept
			{
				// I do allow for default ctor which leaves the instance 
				// in the invalid state, but I do not allow to use it
				// this all complicates the usage
				assert(this->valid());
				// always send the size_
				// data_[0] == '\0' is the state of 
				// alocated but empty buffer
				yanb_helper<char_type>::fill(data_, val_, data_.size());
				return *this;
			}

			// to avoid "never releasing smart pointer" syndrome
			// we wil ban creation of this class on the heap
			void* operator new(std::size_t sz) = delete;
			void* operator new[](std::size_t sz) = delete;
			void  operator delete(void* ptr, std::size_t sz) = delete;
			void  operator delete[](void* ptr, std::size_t sz) = delete;

		private:

			//  buffer friends live here

			friend std::string to_string(const reference_type from_) noexcept {
				return { from_.data() };
			}

			friend std::vector<char> to_vector(const reference_type from_) noexcept {
				std::string str_(from_.data());
				return { str_.begin(), str_.end() };
			}

			//  assignments are used to make "from" certain types
			friend void assign(reference_type target_,
				char const* from_, char const* to_)
			{
				target_.assign(from_, to_);
			}

			friend void assign(reference_type target_, std::string str_)
			{
				target_.assign(str_.c_str());
			}

			friend void assign(reference_type target_, dbj::string_view strvw_)
			{
				target_.assign(strvw_.data());
			}

			friend void assign(reference_type target_, std::vector<char> charvec_)
			{
				target_.assign(charvec_.data());
			}

			friend
				bool operator == (const reference_type left_, const reference_type right_)
				noexcept
			{
				if (left_.size() != right_.size())
					return false;

				return std::equal(
					left_.begin(), left_.end(),
					right_.begin(), right_.end()
				);
			}
#ifdef DBJ_BUFFERS_IOSTREAMS
			friend std::ostream& operator << (std::ostream& os, buffer const& cb_)
			{
				return os << cb_.data_.data();
			}
#endif
#ifdef _WIN32
		public:
			struct wide_copy_result {
				std::error_code ec{};
				size_t size{};
				std::unique_ptr<wchar_t[]> pointer{};
			};
		private:
			/*
			for the inevitable WIN32 aficionados
			we will deliver the wide copy of internal array,
			transformed and inside the unique_ptr inside the wide_copy_result
			as ever we do not use exceptions
			*/
			friend wide_copy_result wide_copy(reference_type source_) noexcept
			{
				auto const& source_size_ = source_.data_.size();
				auto& source_pointer_ = source_.data_;

				std::unique_ptr<wchar_t[]> wp =
					std::make_unique<wchar_t[]>(source_size_ + 1);

				size_t rezult_size;
				auto mbstowcs_errno = ::mbstowcs_s(
					&rezult_size,
					wp.get(), source_size_ + 1,
					source_pointer_.data(), source_size_
				);
				std::error_code ec{}; // OK state
				if (0 != mbstowcs_errno) {
					ec = std::make_error_code(std::errc::invalid_argument);
				}
				return { ec, source_size_ , std::move(wp) };
			}
#endif // _WIN32

			//  buffer friends end here
		}; // buffer

	} // buf
} // dbj

namespace dbj::fmt {
	inline char const* frm_arg(::dbj::chr_buf::buffer::type value) noexcept
	{
		return value.data();
	}
}

#endif