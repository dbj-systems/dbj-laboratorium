#pragma once



namespace dbj::chr_buf 
{
	constexpr std::size_t DBJ_MAX_BUFF_LENGTH = ::dbj::DBJ_64KB;
    /*
	everything is based on these two types
	*/
	using wide_type		=  std::shared_ptr<wchar_t[]>;
	using narrow_type	=  std::shared_ptr<char[]>;

	using between_0_and_max =
		::dbj::core::util::insider<size_t, 0, DBJ_MAX_BUFF_LENGTH, ::dbj::core::util::insider_error_code_throw_policy>;
	/*
	very simple core routines for assigning char pointers to smart pointers
	*/
	struct core final {

		/*
		always use this method of making share_ptr of char's of certain size
		*/
		template<typename CHAR>
		static std::shared_ptr<CHAR> __CRTDECL make(between_0_and_max sze_ ) 
		{
			static_assert(std::is_same_v<char, CHAR> || std::is_same_v<wchar_t, CHAR>, "\n\n" __FILE__  "\n"  __FUNCSIG__ "\n\tcore requires char or wchar_t only\n\n");

			shared_ptr<CHAR[]> retval_(new CHAR[sze_ + 1]);
			// terminate!
			retval_[sze_] = CHAR(0);
			return retval_;
		}

		static  narrow_type  __CRTDECL assign( char const* payload_ )
		{
			DBJ_SAFE( _ASSERTE(payload_ != nullptr) );
			narrow_type storage_ref;
			storage_ref.reset(_strdup(payload_));
			return storage_ref;
		}		
		
		static  wide_type  __CRTDECL assign( wchar_t const* payload_ )
		{
			DBJ_SAFE( _ASSERTE(payload_ != nullptr) );
			wide_type storage_ref;
			storage_ref.reset(_wcsdup(payload_));
			return storage_ref;
		}

		static  narrow_type  __CRTDECL assign(
			std::reference_wrapper< narrow_type > storage_ref,
			char const* payload_
		) {
			DBJ_SAFE( _ASSERTE(payload_ != nullptr) );
			storage_ref.get().reset(_strdup(payload_));
			return storage_ref;
		}

		static wide_type __CRTDECL assign(
			std::reference_wrapper< wide_type > storage_ref,
			wchar_t const* payload_
		) {
			DBJ_SAFE( _ASSERTE(payload_ != nullptr) );
			storage_ref.get().reset(_wcsdup(payload_));
			return storage_ref;
		}

		static size_t __CRTDECL length(
			// std::reference_wrapper< narrow_type > storage_ref
			narrow_type const & storage_ref
		) {
			DBJ_SAFE( _ASSERTE(storage_ref.get()) );
			return (std::strlen(storage_ref.get()));
		}

		static size_t __CRTDECL length(
		//	std::reference_wrapper< wide_type > storage_ref
		        wide_type const & storage_ref
		) {
			DBJ_SAFE( _ASSERTE(storage_ref.get()) );
			return (std::wcslen(storage_ref.get()));
		}

	}; // inner

} // dbj::chr_buf