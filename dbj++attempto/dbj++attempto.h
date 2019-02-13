#pragma once
// common stuff for this project -- dbj++attempto
#include "pch.h"

namespace dbj::attempto {
	// type for testing
	class TT;
	
	// declare the empty special value
	extern const TT empty_tt;

	class TT final {

		// reference to empty myself
		static inline const TT & myself_ref = empty_tt;

		/* some payload on stack */
		std::array<char, BUFSIZ> payload{ {0} };

		static auto const instance_counter() noexcept {
			static auto id_ = 0U;
			return id_++;
		}

	public:
		/*
		NOTE! this is how you do it, must not 
		relly on the typeid name() result
		*/
		static auto const & type_id() noexcept {
			static auto id_	= (typeid(TT).hash_code());
			return id_;
		}

		auto const & instance_id() const noexcept {
			static auto id_ = instance_counter();
			return id_;
		}

		TT & assign_new_payload(char const * ss_) noexcept {
			payload.fill(0);
			std::memcpy(payload.data(), ss_, ::strlen(ss_));
			return *this;
		}

		explicit TT(const char * ss_ = "") noexcept {
			assign_new_payload(ss_);
		}

		TT & operator = (char const * ss_) noexcept		{
			return assign_new_payload(ss_);
		}

		bool operator == (TT const & another_) const noexcept {
			return this->payload == another_.payload;
		}

	private:

		friend void out(TT const & d_) noexcept
		{
			::dbj::console::prinf("%s { type_id: %d, instance_id: %d }", 
				typeid(TT).name(), d_.type_id(), d_.instance_id()
			);
		}
	};

	// define the empty special value
	inline const TT empty_tt;

} // namespace dbj::attempto
