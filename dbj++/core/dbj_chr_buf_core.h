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


		static void set_payload(
			std::reference_wrapper< std::shared_ptr<char> > storage_ref,
			char const* payload_
		) {
			_ASSERTE(payload_ != nullptr);
			storage_ref.get().reset(_strdup(payload_));
		}

		static void set_payload(
			std::reference_wrapper< std::shared_ptr<wchar_t> > storage_ref,
			wchar_t const* payload_
		) {
			_ASSERTE(payload_ != nullptr);
			storage_ref.get().reset(_wcsdup(payload_));
		}

		static size_t length(
			std::reference_wrapper< std::shared_ptr<char> > storage_ref
		) {
			_ASSERTE(storage_ref.get());
			return (std::strlen(storage_ref.get().get()));
		}

		static size_t length(
			std::reference_wrapper< std::shared_ptr<wchar_t> > storage_ref
		) {
			_ASSERTE(storage_ref.get());
			return (std::wcslen(storage_ref.get().get()));
		}

	}; // inner

} // dbj::chr_buf