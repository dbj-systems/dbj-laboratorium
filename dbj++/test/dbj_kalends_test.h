#pragma once

#include "../dbj++.h"
#include <cstdlib>
#include <chrono>
#include <type_traits>

namespace quick_dirty
{
	// to check the validity of dbj timers
	struct Timer final
	{
		typedef std::chrono::high_resolution_clock clock_;
		typedef std::chrono::seconds		seconds_;
		typedef std::chrono::milliseconds	miliseconds_;
		typedef std::chrono::microseconds	microseconds_;
		std::chrono::time_point<clock_> beg_;

		Timer() : beg_(clock_::now()) {}

		void start() { beg_ = clock_::now(); }

		template<typename U = Timer::miliseconds_ >
		auto elapsed() const {
			return std::chrono::duration_cast<U>
				(clock_::now() - beg_).count();
		}
	};

	template<typename F,
		typename U = Timer::miliseconds_
	>
		auto measure(F fun_to_test)
	{
		using namespace dbj::kalends;
		Timer timer_;
		fun_to_test();
		return timer_.elapsed<U>();
	};
}


// #include "dbj_kalends.h"
DBJ_TEST_SPACE_OPEN( dbj_kalends_testing )

DBJ_TEST_UNIT(dbj_timers_) {

	using namespace dbj::kalends;

	auto worker = [& ]() { dbj_sleep_seconds(1); };
	using worker_type = decltype(worker);

	auto t1 = measure< worker_type, dbj::kalends::MilliSeconds, timer_kind::win32>
		(worker);

	auto t2 = measure< worker_type, dbj::kalends::MilliSeconds, timer_kind::modern>
		(worker);

	auto t3 = quick_dirty::measure([&] { dbj_sleep_seconds(1); });

	dbj::console::print(
        "\n\nWIN32, Modern, Quick and dirty timer: ",
		t1,", ",t2,", ", t3, " miliseconds" );
}
#pragma region more chrono testing
// dbj.org 2018 NOV 19
/*
non template, recursive fibonacci aka "the worst"

int fib(int x) {
	if (x == 0)
		return 0;

	if (x == 1)
		return 1;

	return fib(x-1)+fib(x-2);
}
*/

using fibo_type = int64_t;

namespace iterative {
	/*
	this is the best runtime fibonacci
	non recursive
	compiler is free to optimize this in the best possible way
	on i5 CPU / 8GB ram x64 WIN10 this was the fastest for N=40
	*/
	fibo_type fib(fibo_type n)
	{
		fibo_type c, a = 1, b = 1;
		for (fibo_type i = 3; i <= n; i++) {
			c = a + b;
			a = b;
			b = c;
		}
		return b;
	}
}

namespace slowest {
	/*
	recursive and template but very bad implementation
	this is ridiculously slow. why?
	because this creates at compile time N deep call stack
	to be executed at runtime
	*/
	template<fibo_type  N>
	constexpr fibo_type fibonacci() { return fibonacci<N - 1>() + fibonacci<N - 2>(); }
	template<>
	constexpr fibo_type fibonacci<1>() { return 1; }
	template<>
	constexpr fibo_type fibonacci<0>() { return 0; }
}

namespace fast {
	using namespace std;
	template<fibo_type N>
	struct fibonacci 
		: integral_constant < 
		fibo_type,
		fibonacci<N - 1>{} + fibonacci<N - 2>{} > 
	{};

	template<> struct fibonacci<1> : integral_constant<fibo_type, 1> {};
	template<> struct fibonacci<0> : integral_constant<fibo_type, 0> {};
}

#define ST_2_(x) #x
#define TT(x)   ::dbj::console::print("\n", ST_2_(x) ,"\n\t-> ", (x))

struct ya_timer final {
	mutable 
		::std::chrono::time_point<::std::chrono::system_clock> 
		startP{}, endP{};

	auto start() const noexcept {
		return startP = ::std::chrono::system_clock::now();
	}

	auto stop() const noexcept {
		return endP = ::std::chrono::system_clock::now();
	}

	auto report() const noexcept 
	{
		using namespace std;
		__int64 nano = chrono::duration_cast
			<chrono::nanoseconds>(endP - startP).count();
		using the_type = decltype(nano);

		struct timings_as_floats final 
		{
			__int64 nano_seconds;
			double seconds = (nano_seconds / 1.0E9);
			double micro_seconds = (nano_seconds / 1.0E3);
			double milli_seconds = (nano_seconds / 1.0E6);
		};
		return timings_as_floats{ nano };
	}
};

template<typename F>
static void test(F fun_) {
	using namespace std;
	ya_timer timer_;
	timer_.start();
	fun_();
	timer_.stop();
	auto rez = timer_.report() ;
	dbj::console::print("\n\tMeasurement:\n"
		, "\t", rez.seconds, " sec,"
		, "\t", rez.micro_seconds, " micro sec,"
		, "\t", rez.milli_seconds, " mili sec,"
		, "\t", rez.nano_seconds, " nano sec");
}

DBJ_TEST_UNIT(more_chrono_testing)
{
	constexpr static const fibo_type N = 40;
	test(
		[&]() { TT(iterative::fib(N)); }
	);

	test(
		[&]() { TT(slowest::fibonacci<N>()); }
	);
	test(
		[&]() { TT(fast::fibonacci<N>{}()); }
	);
}

#undef ST
#undef TT

#pragma endregion
DBJ_TEST_SPACE_CLOSE