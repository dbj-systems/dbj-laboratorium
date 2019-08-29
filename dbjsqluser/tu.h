#pragma once

// #include <string>
#include <stdint.h>
#include <stdio.h>
#include <array>
#include <vector>
#include <chrono>
#include <cmath>

/*
from vcruntime.h
*/
#define _DBJ_STRINGIZE_(x) #x
#define _DBJ_STRINGIZE(x) _DBJ_STRINGIZE_(x)

#define _DBJ_WIDE_(s) L ## s
#define _DBJ_WIDE(s) _DBJ_WIDE_(s)

#define _DBJ_CONCATENATE_(a, b) a ## b
#define _DBJ_CONCATENATE(a, b)  _DBJ_CONCATENATE_(a, b)

#define _DBJ_EXPAND_(s) s
#define _DBJ_EXPAND(s) _DBJ_EXPAND_(s)

#ifdef _MSVC_VER
// https://developercommunity.visualstudio.com/content/problem/195665/-line-cannot-be-used-as-an-argument-for-constexpr.html
#define CONSTEXPR_LINE long(_DBJ_CONCATENATE(__LINE__,U)) 
#else
#define CONSTEXPR_LINE __LINE__
#endif

#define TU_REGISTER inline auto  _DBJ_CONCATENATE( dumsy_ , __COUNTER__ ) = ::tu::catalog

#ifdef _unused
#error _unused is already defined somewhere ...
#else
#define _unused(...)  static_assert( (void)noexcept( __VA_ARGS__ , true ) )
#endif

namespace tu {

	using namespace std;

	struct timer final {
		using buffer = array<char, 24>;
		using timepoint = typename std::chrono::system_clock::time_point;
		const timepoint start_ = std::chrono::system_clock::now();

		double nano() const {
			timepoint end_ = std::chrono::system_clock::now();
			return  static_cast<double>((end_ - start_).count());
		}

		double micro() const { return nano() / 1000.0; }

		double seconds() const { return micro() / 1000.0; }

		double decimal3(double arg) { return (std::round(arg * 1000)) / 1000; }

		enum class kind { nano, micro, second };

		friend buffer as_buffer(timer const& timer_, kind which_ = kind::micro) {
			buffer retval{ char{0} };
			double arg{};
			char const * unit_{};
			switch (which_) {
			case kind::nano: arg = timer_.nano(); unit_ = " nano seconds ";
				break;
			case kind::micro: arg = timer_.micro(); unit_ = " micro seconds ";
				break;
			default: //seconds
				arg = timer_.seconds(); unit_ = " seconds ";
			}
			std::snprintf(retval.data(), retval.size(), "%.3f%s", arg, unit_);
			return retval;
		}
	};

	constexpr inline static auto MSCVER = _MSC_VER; // 1921
	constexpr inline static auto MSCFULLVER = _MSC_FULL_VER; //192127702
	constexpr inline static auto MSCBUILD = _MSC_BUILD; // 2

	using tu_function = void (*)();
	using units_ = vector< tu_function >;

	inline void line() noexcept { printf("\n----------------------------------------------------------------------"); }

	struct testing_system final
	{
		inline static units_ units{};

		auto operator () (tu_function fun_) const {
			units.push_back(fun_);
			return fun_;
		}

		void start() const {
			line();
			printf("\n " _DBJ_STRINGIZE(MSCVER) " :\t\t %d", MSCVER);
			printf("\n " _DBJ_STRINGIZE(MSCFULLVER) " :\t\t %d", MSCFULLVER);
			printf("\n " _DBJ_STRINGIZE(MSCBUILD) " :\t\t %d", MSCBUILD);
			printf("\n\n Catalogue has %zd test units", units.size());
			line();
		}

		void end() const {
			printf("\n\n All tests done.\n\n");
		}

		void execute() const {
			start();
			for (auto& tu_ : units) {
				timer timer_{};
				tu_();
				line();
				printf("\nDone in: %s", as_buffer(timer_).data());
				line();
			}
			end();
		}

	};

	constexpr inline testing_system catalog;

} // tu