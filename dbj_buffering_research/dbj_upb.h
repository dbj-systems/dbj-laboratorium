#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifndef __DBJ__SPB__
#define __DBJ__SPB__

#pragma region shared_pointer_buffer

namespace dbj {

	/*
	This type comes out as the fastest buffer type.
	Which is confusing.
	*/

	template<typename CHAR>
	struct  shared_pointer_buffer final
	{
		using type			= shared_pointer_buffer;
		using char_type		= CHAR;
		using pointer_type	= std::shared_ptr<char_type[]>;

	private:

		// String Terminator
		constexpr static inline char_type ST = char_type(0);

		
		// OPT 01 : replace std::pair with 
		struct sp_t final {
			size_t			size;
			pointer_type	pointer;

			char_type& operator [ ] (size_t idx_) {
				_ASSERTE( size > idx_ );
				return pointer[idx_] ; 
			}

			sp_t(size_t sze_) 
				: size( 1 + sze_)
			    , pointer( new char_type [ 1 + sze_ ]  )
			{
				pointer[sze_] = char_type(0) ;
			}

		} sp_;

	public:

		shared_pointer_buffer() = delete;

		explicit shared_pointer_buffer(size_t size_arg_)
			: sp_(	size_arg_ )
		{
		}

		shared_pointer_buffer(const shared_pointer_buffer& other_) = default;
		shared_pointer_buffer& operator = (shared_pointer_buffer  const& other_) = default;
		shared_pointer_buffer(shared_pointer_buffer&& other_) = default;
		shared_pointer_buffer& operator = (shared_pointer_buffer&& other_) = default;

		// interface

		char_type& operator [] (unsigned idx_) {
			_ASSERTE(sp_.size > idx_);
			return sp_.pointer[idx_] ;
		}

		//		// for constant instances we do not allow changing them chars
		//		char_type const & operator [] ( unsigned const & idx_) const { 
		////			DBJ_VERIFY( size() >= idx_);
		//			return pair_.second[idx_];
		//		}

		const size_t size() const noexcept { return sp_.size; };
		pointer_type const& buffer() const noexcept { return sp_.pointer; };

	}; // shared_pointer_buffer

} // dbj
#pragma endregion


#endif // !__DBJ__SPB__
