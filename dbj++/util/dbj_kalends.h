#pragma once

#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
#endif
#define STRICT
#define NOSERVICE
// avoid min/max macros 
#define NOMINMAX
#include <windows.h>
#endif

#ifndef _ASSERTE
#include <crtdbg.h>
#endif
#include <cstdint>
#include <chrono>

extern "C" inline void dbj_sleep_seconds(int seconds_) {
	std::this_thread::sleep_for(::std::chrono::seconds(seconds_));
}
/// <summary>
/// `Kalends` are dbj std chrono utilities
/// note: name etimology is here: https://en.wikipedia.org/wiki/Calends
/// </summary>
namespace dbj::kalends {

	/// <summary>
	/// all abstractions in this namespace do use 
	/// the same std chrono clock
	/// </summary>
	using Clock = typename  ::std::chrono::steady_clock;
	// std chrono concept is that (for example) a Second
	// is a duration in time, of a lengt of one second
	using Seconds = typename ::std::chrono::seconds;
	using MilliSeconds = typename ::std::chrono::milliseconds;
	using Microseconds = typename ::std::chrono::microseconds;
	using Nanoseconds = typename ::std::chrono::nanoseconds;

	using time_ticks_type = Clock::rep;
	using frequency_type = Clock::rep;

	/// <summary>
	/// unit duration is ticks as std chrono duration
	/// second template argument is std::nano
	/// this is the "core of the trick" for transforming
	/// ticks to standard std chrono time units aka durations
	/// NOTE: UnitDiration::rep is the type same as time_ticks_type
	/// </summary>
	using UnitDuration = typename Clock::duration;

	/// <summary>
	/// we use the UnitDuration to transform tick to
	/// different time units
	/// reminder: Time Unit is std chrono duration
	/// for example 1 second is a duration that has taken 1 second.
	/// </summary>
	template< typename Unit >
	inline typename Unit to_desired_unit(time_ticks_type ticks_) {
		// create unit duration from ticks
		UnitDuration dura{ ticks_ };
		// we let std lib create a duration we are transforming to
		return ::std::chrono::duration_cast<Unit>(dura);
	}

	/// <summary>
	/// transfrom from duration unit (aka "ticks") to desired other 
	/// std chrono duration , named as particular unit 
	/// </summary>
	template< typename Unit >
	inline typename Unit to_desired_unit(UnitDuration ticks_) {
		// we let std lib to create a duration we are transforming to
		return ::std::chrono::duration_cast<Unit>(ticks_);
	}


#pragma region rounding to time ticks type
	/// <summary>
	/// https://stackoverflow.com/questions/23374153/convert-double-to-int-in-c-without-round-down-errors
	/// </summary>
	/// <param name="ull_">floating type</param>
	/// <returns>rounded to time_ticks_type</returns>
	template<typename T>
	inline constexpr
		typename std::enable_if_t< std::is_floating_point_v< T >, time_ticks_type >
		round_to_tt_type(T  ull_) {
		return static_cast<time_ticks_type>(ull_ < 0 ? ull_ - 0.5 : ull_ + 0.5);
	}
	/// <summary>
	/// same but for integrals
	/// </summary>
	/// <param name="ull_">int type</param>
	/// <returns>narrowed to time_ticks_type</returns>
	template<typename T>
	inline constexpr
		typename std::enable_if_t< std::is_integral_v< T >, time_ticks_type >
		round_to_tt_type(T  int_) {
		return static_cast<time_ticks_type>(int_);
	}
#pragma endregion 

	enum class timer_kind : int { win32 = 0, modern = 1 };

	/// <summary>
	/// timer engine Interface 
	/// </summary>
	struct __declspec(novtable) ITimer
	{
		using pointer = std::shared_ptr<ITimer>;

		virtual time_ticks_type	start() = 0;
		virtual time_ticks_type	elapsed() = 0;
	};


	namespace internal {

		/*
		Taken form UnitTest++ then modified by dbj.org
		This is WIN32 implementation
		*/
		class  win32_timer_engine final : public ITimer
		{
		public:
			win32_timer_engine() noexcept
				: threadHandle(::GetCurrentThread())
				, startTime(0)
			{
// https://docs.microsoft.com/en-us/windows/desktop/ProcThread/multiple-processors
				_ASSERTE(threadHandle);
				DWORD_PTR systemMask{};
				::GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemMask);
				// Set a processor affinity mask for the current thread
				// to the same CPU always?
				::SetThreadAffinityMask(threadHandle, 1);
				::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency));
				::SetThreadAffinityMask(threadHandle, processAffinityMask);
			}

			// 
			virtual time_ticks_type start() override {
				this->startTime = get_time();
				return reconcile_and_round(this->startTime);
			}

			virtual time_ticks_type elapsed() override
			{
				// time_ticks_type elapsed_ticks = get_time() - startTime;
				return reconcile_and_round(get_time() - startTime);
				// round_to_tt_type((elapsed_ticks * Clock::period::den) / frequency);
			}

		private:
			/*
			IMPORTANT!
			Clock::period::den is required to reconcile win32 ticks
			to the ticks of the std chrono clock we use
			`den` is denominator part of the std::ratio
			for example if Clock is `steady_clock`
			`period` is std::chrono::steady_clock::period
			which happens to be the alias for std::nano
			*/
			constexpr time_ticks_type reconcile_and_round(time_ticks_type win32_ticks) noexcept {
				return round_to_tt_type((win32_ticks * Clock::period::den) / frequency);
			}

			time_ticks_type	get_time() const
			{
				_ASSERTE(threadHandle);
				LARGE_INTEGER curTime{};
				::SetThreadAffinityMask(threadHandle, 1);
				::QueryPerformanceCounter(&curTime);
				::SetThreadAffinityMask(threadHandle, processAffinityMask);
				return (time_ticks_type)curTime.QuadPart;
			}

			void*			threadHandle{};
			DWORD_PTR		processAffinityMask{};
			time_ticks_type startTime{};
			frequency_type  frequency{};
		}; // win32_timer_engine

		   /*
		   and now ladies and gentlemen the modern solution
		   #include <ctime>
		   #include <chrono>

		   int main()
		   {
		   std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		   std::time_t now_c = std::chrono::system_clock::to_time_t(now - std::chrono::hours(24));
		   std::cout << "24 hours ago, the time was "
		   << std::put_time(std::localtime(&now_c), "%F %T") << '\n';

		   std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		   std::cout << "Hello World\n";
		   std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		   std::cout << "Printing took "
		   << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
		   << "us.\n";
		   }
		   */

		using ::std::chrono::duration_cast;

		class modern_timer final : public ITimer
		{
		public:
			// Inherited via ITimer
			virtual time_ticks_type start() override {
				this->start_ = Clock::now();
				return this->start_.time_since_epoch().count();
			}

			virtual time_ticks_type elapsed() override {
				return (Clock::now() - start_).count();
			}
		private:
			Clock::time_point start_{ Clock::now() };
		};

	} // internal

	  /// <summary>
	  /// the only visible class
	  /// from this module
	  /// has its engine changeble 
	  /// at construction time by 
	  /// the factory method 
	  /// </summary>
	class  __declspec(novtable) the_timer final
	{
		mutable ITimer::pointer imp_{};

		explicit the_timer(ITimer::pointer && engine_)
		{
			imp_ = std::move(engine_);
		}

		the_timer() = default;

	public:
		~the_timer() = default;
		// no copy
		the_timer(const the_timer&) = delete;
		the_timer& operator=(const the_timer&) = delete;
		// move
		the_timer(the_timer&& other_) noexcept { std::swap(this->imp_, other_.imp_); }
		the_timer& operator=(the_timer&& other_) noexcept { std::swap(this->imp_, other_.imp_); return *this; }


		time_ticks_type start() { return imp_->start(); }

		time_ticks_type elapsed() { return imp_->elapsed(); }

		/// <summary>
		/// the factory method
		/// </summary>
		/// <returns>the_timer instance</returns>
		friend the_timer create_timer(timer_kind which_);

	}; // the timer

	   /// <summary>
	   /// the factory method
	   /// </summary>
	   /// <returns>the_timer instance</returns>
	inline the_timer create_timer(timer_kind which_) {

		if (which_ == timer_kind::win32)
			return  the_timer{
			ITimer::pointer{ new internal::win32_timer_engine{} }
		};
		//
		return the_timer{
			ITimer::pointer{ new internal::modern_timer{} }
		};
	}

	// for comfortable measurements

	template<typename T>
	inline auto unit_str(time_ticks_type t_) = delete;
	//{ return  std::to_string( to_desired_unit<T>(t_ ).count() ) + " nanoseconds"; }

	template<> inline auto unit_str< dbj::kalends::time_ticks_type >(time_ticks_type t_)
	{
		return std::to_string(t_) + " nanoseconds";
	}

	template<> inline auto unit_str< dbj::kalends::MilliSeconds >(time_ticks_type t_)
	{
		return std::to_string(to_desired_unit<dbj::kalends::MilliSeconds>(t_).count()) + " miliseconds";
	}

	template<> inline auto unit_str< dbj::kalends::Microseconds>(time_ticks_type t_)
	{
		return std::to_string(to_desired_unit<dbj::kalends::Microseconds>(t_).count()) + " microseconds";
	}

	template<> inline auto unit_str< dbj::kalends::Seconds >(time_ticks_type t_)
	{
		return std::to_string(to_desired_unit<dbj::kalends::Seconds>(t_).count()) + " seconds";
	}

	template<typename F,
		/* default unit is nanoseconds */
		typename U = dbj::kalends::time_ticks_type,
		timer_kind which_ = timer_kind::modern
	>
	inline auto measure(F fun_to_test)
		noexcept -> std::string
	{
		using namespace dbj::kalends;
		auto timer_ = create_timer(which_);
		timer_.start();
		fun_to_test();
		time_ticks_type esd = timer_.elapsed();
		return unit_str<U>(esd);
	};

	template<typename F	>
	inline  auto miliseconds_measure(F fun_to_test)
		noexcept -> std::string
	{
		return measure<F, dbj::kalends::MilliSeconds>(fun_to_test);
	};

	template<typename F	>
	inline  auto microseconds_measure(F fun_to_test)
		noexcept -> std::string
	{
		return measure<F, dbj::kalends::Microseconds>(fun_to_test);
	};

	template<typename F	>
	inline auto seconds_measure(F fun_to_test)
		noexcept -> std::string
	{
		return measure<F, dbj::kalends::Seconds>(fun_to_test);
	};

} // dbj::kalends

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"