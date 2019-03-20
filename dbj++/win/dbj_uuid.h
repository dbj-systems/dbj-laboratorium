#pragma once

// For UUID
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

namespace dbj::util 
{

class uuid final {
	mutable UUID uuid_{};
	const char * uuid_string_ = NULL;
	mutable unsigned long hash_{ 0 };
public:
	explicit uuid() noexcept {
		RPC_STATUS ret_val = ::UuidCreate(&this->uuid_);
		DBJ_NOUSE(ret_val);
		_ASSERTE(ret_val == RPC_S_OK);
	}
	~uuid() {
		if (uuid_string_ != NULL) {
			::RpcStringFreeA((RPC_CSTR*)&uuid_string_);
			uuid_string_ = NULL;
		}
	}

	constexpr  const UUID & get() const noexcept {
		return this->uuid_;
	}

	constexpr const char * uuid_string() const noexcept {
		if (uuid_string_ == NULL) {
			::UuidToStringA(
				&this->get(), (RPC_CSTR*)&uuid_string_
			);
			_ASSERTE(uuid_string_ != NULL);
		}
		return uuid_string_;
	}

	constexpr unsigned long hash_code() const noexcept {
		if (this->hash_ == 0) {
			// compute hash from uuid_string_
			this->hash_ = dbj::util::hash(this->uuid_string());
		}
		return this->hash_;
	}
};
} // dbj::util