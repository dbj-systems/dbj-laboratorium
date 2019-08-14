#pragma once

namespace dbj::chr_buf {
	/*
	everything is based on these two types
	*/
	using wide_type =  std::shared_ptr<wchar_t>;
	using narrow_type =  std::shared_ptr<char>;

	// it is not very usefull to have buffers of 
	// unlimited size in programs
	// thus we will define the upper limit
	// it is
	// in posix terms BUFSIZ * 2 * 64 aka 64KB
	constexpr std::size_t max_length = UINT16_MAX;

	using inside_1_and_max =
		::dbj::core::util::insider<size_t, 1, max_length, ::dbj::core::util::insider_error_code_throw_policy>;


	/*
	very simple core routines for assigning char pointers to smart pointers
	*/
	struct core final {


		static  narrow_type  assign( char const* payload_ ) 
		{
			_ASSERTE(payload_ != nullptr);
			narrow_type storage_ref;
			storage_ref.reset(_strdup(payload_));
			return storage_ref;
		}		
		
		static  wide_type  assign( wchar_t const* payload_ ) 
		{
			_ASSERTE(payload_ != nullptr);
			wide_type storage_ref;
			storage_ref.reset(_wcsdup(payload_));
			return storage_ref;
		}

		static  narrow_type  assign(
			std::reference_wrapper< narrow_type > storage_ref,
			char const* payload_
		) {
			_ASSERTE(payload_ != nullptr);
			storage_ref.get().reset(_strdup(payload_));
			return storage_ref;
		}

		static wide_type assign(
			std::reference_wrapper< wide_type > storage_ref,
			wchar_t const* payload_
		) {
			_ASSERTE(payload_ != nullptr);
			storage_ref.get().reset(_wcsdup(payload_));
			return storage_ref;
		}

		static size_t length(
			std::reference_wrapper< narrow_type > storage_ref
		) {
			_ASSERTE(storage_ref.get());
			return (std::strlen(storage_ref.get().get()));
		}

		static size_t length(
			std::reference_wrapper< wide_type > storage_ref
		) {
			_ASSERTE(storage_ref.get());
			return (std::wcslen(storage_ref.get().get()));
		}

	}; // inner

} // dbj::chr_buf