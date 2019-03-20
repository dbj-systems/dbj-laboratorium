#pragma once

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"

#ifndef  UNICODE
#error dbj++ requires UNICODE
#endif // ! UNICODE

#include <algorithm>
#include <variant>
#include "dbj_commander.h"

namespace dbj::console {
#pragma region "colors and painter"
namespace inner {
				/* modification of catch.h console colour mechanism */
				enum class Colour : unsigned {
					None = 0,		White,	Red,	Green,		Blue,	Cyan,	Yellow,		Grey,
					Bright = 0x10,

					BrightRed = Bright | Red,	BrightGreen = Bright | Green,
					BrightBlue = Bright | Blue,	LightGrey = Bright | Grey,
					BrightWhite = Bright | White,

				};
				/* stop the colour to text attempts*/
				std::ostream& operator << (std::ostream& os, Colour const&); // no op

				class __declspec(novtable) Painter final {
				public:
					// default ctor
					explicit Painter( ) noexcept 
						: stdoutHandle(::GetStdHandle(STD_OUTPUT_HANDLE))
					{
						CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
						::GetConsoleScreenBufferInfo(stdoutHandle, &csbiInfo);
						originalForegroundAttributes = csbiInfo.wAttributes & ~(BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
						originalBackgroundAttributes = csbiInfo.wAttributes & ~(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					}

	const bool text( Colour _colourCode) const {
		switch (_colourCode) {
			case Colour::None:      return setTextAttribute(originalForegroundAttributes);
			case Colour::White:     return setTextAttribute(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
			case Colour::Red:       return setTextAttribute(FOREGROUND_RED);
			case Colour::Green:     return setTextAttribute(FOREGROUND_GREEN);
			case Colour::Blue:      return setTextAttribute(FOREGROUND_BLUE);
			case Colour::Cyan:      return setTextAttribute(FOREGROUND_BLUE | FOREGROUND_GREEN);
			case Colour::Yellow:    return setTextAttribute(FOREGROUND_RED | FOREGROUND_GREEN);
			case Colour::Grey:      return setTextAttribute(0);

			case Colour::LightGrey:     return setTextAttribute(FOREGROUND_INTENSITY);
			case Colour::BrightRed:     return setTextAttribute(FOREGROUND_INTENSITY | FOREGROUND_RED);
			case Colour::BrightGreen:   return setTextAttribute(FOREGROUND_INTENSITY | FOREGROUND_GREEN);
			case Colour::BrightBlue:   return setTextAttribute(FOREGROUND_INTENSITY | FOREGROUND_BLUE);
			case Colour::BrightWhite:   return setTextAttribute(FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
			default: throw "Exception in "  __FUNCSIG__ " : not a valid colour code?";
		}
	}
	/* modern interface */
	const bool text( std::variant<Colour> _colourCode) const {
		return this->text(std::get<Colour>(_colourCode));
	} 

	const bool text_reset() const { return this->text(Colour::None); }

	private:
		bool setTextAttribute( WORD _textAttribute) const 
		{
			bool rezult = ::SetConsoleTextAttribute(
				stdoutHandle, 
				_textAttribute | originalBackgroundAttributes
			);
			_ASSERTE(rezult);
			return rezult;
		}
		mutable HANDLE stdoutHandle;
		mutable WORD originalForegroundAttributes;
		mutable WORD originalBackgroundAttributes;
};
        // the one and only is hidden in here ----------------------------------------
		inline Painter painter_{};
		// ---------------------------------------------------------------------------
} // nspace inner
#pragma endregion "colors and painter"
#pragma region "commander setup"
	/*
	Here we use the dbj::cmd::Commander,  define the comand id's and functions to execute them etc..
	*/
	typedef enum painter_command_ {
		white = 0,		red,		green,
		blue,			cyan,		yellow,
		grey,			bright_red, bright_blue, 
		text_color_reset,			nop = -1
	}  painter_command ;

	using PainterCommandFunction = bool(void);
	using PainterCommander = dbj::cmd::Commander<painter_command, PainterCommandFunction >;

	// make the unique commander instance
	inline auto factory_of_commands = []() 
		-> PainterCommander & 
	{
		using namespace inner;
		static PainterCommander & commander_ = [&]() 
			-> PainterCommander &
		{
			static PainterCommander cmdr_{};
			// register command/function pairs
			cmdr_.reg({
	{ painter_command::nop,				[&]() {										return true;  }},
	{ painter_command::text_color_reset,[&]() {	painter_.text_reset(); 				return true;  }},
	{ painter_command::white,			[&]() { painter_.text(Colour::White);		return true;  }},
	{ painter_command::red,				[&]() { painter_.text(Colour::Red);			return true;  }},
	{ painter_command::green,			[&]() { painter_.text(Colour::Green);		return true;  }},
	{ painter_command::blue,			[&]() { painter_.text(Colour::Blue);		return true;  }},
	{ painter_command::bright_red,		[&]() { painter_.text(Colour::BrightRed);	return true;  }},
	{ painter_command::bright_blue,		[&]() { painter_.text(Colour::BrightBlue);	return true;  }}
				});
					return cmdr_;
			}(); // execute the once-init lambda in place	
		return commander_;
	}; // factory_of_commands


	inline const PainterCommander  & painter_commander() {
		static 	const PainterCommander & just_call_once = factory_of_commands();
		return  just_call_once ;
	}

	inline const PainterCommander  & painter_commander_instance = painter_commander();

#pragma endregion
} // dbj::console
