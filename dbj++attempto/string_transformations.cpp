#include "pch.h"

#include <iostream>

namespace string_transformations {

	/*
	https://docs.microsoft.com/en-us/uwp/cpp-ref-for-winrt/to-string
	from
	C:\Program Files (x86)\Windows Kits\10\Include\10.0.17134.0\cppwinrt\winrt\base.h
	*/
	namespace from_winrt {
		inline std::string to_string(std::wstring_view value)
		{
			if (value.empty()) return {}; // DBJ added

			int const size =
				WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int32_t>(value.size()), nullptr, 0, nullptr, nullptr);

			if (size == 0) { return{};	}

			std::string result(size, '?');
			DBJ_VERIFY(size,
				WideCharToMultiByte(
					CP_UTF8, 0, value.data(), static_cast<int32_t>(value.size()), result.data(), size, nullptr, nullptr
				)
			);
			return result;
		}
	}

} // string_transformations