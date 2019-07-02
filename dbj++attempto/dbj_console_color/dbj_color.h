#pragma once

#include <stdio.h>

// helper for printf boolean display
#ifndef DBJ_BOOLALPHA
#define DBJ_BOOLALPHA(x) (x ? "true" : "false")
#endif

namespace dbj {
	namespace con
	{
#include "scrutil.h"
	}
}

#ifndef _MSC_VER
#define RED "\x1b[31m"
#define RESET "\x1b[0m"
#else
#define RED "\0x1b[31m"
#define RESET "\0x1b[0m"
#endif

namespace dbj::console_color
{
	using namespace ::dbj::con;

	template<typename  ... A>
	inline int prinf(
		scrAttributes atts_, const char* fmt, A ... args_
	)
	{
		auto white_on_black = []() { scrSetColors(scrWhite, scrBlack); return true; };
		static auto once = white_on_black();

		scrSetColorsWithAttr(atts_);
		return printf(fmt, args_...);
	}

	template< typename  ... A>
	inline int red(const char* fmt, A ... args_)
	{
		return prinf(scrAttributes{ scrBlack, scrRed }, fmt, args_ ...);
	}

	template< typename  ... A>
	inline int green(const char* fmt, A ... args_)
	{
		return prinf(scrAttributes{ scrBlack, scrGreen }, fmt, args_ ...);
	}

	template< typename  ... A>
	inline int blue(const char* fmt, A ... args_)
	{
		return prinf(scrAttributes{ scrBlack, scrBlue  }, fmt, args_ ...);
	}

	template< typename  ... A>
	inline int white(const char* fmt, A ... args_)
	{
		return prinf(scrAttributes{ scrBlack, scrWhite  }, fmt, args_ ...);
	}


extern "C" int test(int argc, char **argv)
{
	using namespace dbj::con;
	char name_[255];

	// Start
	scrSetColors(scrGreen, scrBlack);
	scrClear();

	// Show console info
	scrShowCursor(true);
	scrMoveCursorTo(0, 0);
	printf("scrutil Demo");
	scrMoveCursorTo(5, 10);
	printf("Max Rows: %d\n", scrGetMaxRows());
	scrMoveCursorTo(6, 10);
	printf("Max Columns: %d\n", scrGetMaxColumns());

	// Ask for the name
	scrMoveCursorTo(10, 10);
	printf("Your name:");
	scrMoveCursorTo(10, 50);
	fgets(name_, 255, stdin);

	// Show the name
	scrSetColors(scrYellow, scrRed);
	scrMoveCursorTo(scrGetMaxRows() - 7, 50);
	printf("Your name is: %s\n", name_);
	scrShowCursor(false);

	scrSetColors(scrWhite, scrBlack);
	return 0;
}
} // namespace dbj::color