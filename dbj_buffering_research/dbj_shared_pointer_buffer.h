#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifndef __DBJ__UPB__
#define __DBJ__UPB__

#pragma region dbj_spb

namespace dbj {

	/*
	UPB: Unique Pointer Buffer
	*/

	template<typename CHAR>
	struct  dbj_spb final
	{
		using type			= dbj_spb;
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

		dbj_spb() = delete;

		explicit dbj_spb(size_t size_arg_)
			: sp_(	size_arg_ )
		{
		}

		// std::uniqe_ptr can not be copied, ditto
		dbj_spb(const dbj_spb& other_) = default;
		dbj_spb& operator = (dbj_spb  const& other_) = default;
		dbj_spb(dbj_spb&& other_) = default;
		dbj_spb& operator = (dbj_spb&& other_) = default;

		// interface

		char_type& operator [] (unsigned idx_) {
			return sp_[idx_] ;
		}

		//		// for constant instances we do not allow changing them chars
		//		char_type const & operator [] ( unsigned const & idx_) const { 
		////			DBJ_VERIFY( size() >= idx_);
		//			return pair_.second[idx_];
		//		}

		const size_t size() const noexcept { return sp_.size; };
		pointer_type const& buffer() const noexcept { return sp_.pointer; };

		operator size_t () const noexcept { return sp_.size; };
		operator pointer_type  const& () const noexcept { return sp_.pointer ; };

	}; // dbj_spb

} // dbj
#pragma endregion


#endif // !__DBJ__UPB__
