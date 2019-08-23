#pragma once
DBJ_TEST_SPACE_OPEN(_dbj_defval_)

namespace dbj_gdi_plus 
{
	typedef long REAL;

	enum  class SmoothingMode { SmoothingModeAntiAlias };
	enum class  LineCap { LineCapRound };

} // dbj_gdi_plus

using namespace dbj_gdi_plus;

// test the const onstance behaviour
struct S final {
	mutable dbj::holder<REAL> width{ 0 };
	S(REAL def_width) : width(def_width) {}
};

inline const S konst_{ 1024 }; // initialized to 1024

inline dbj::holder<SmoothingMode> smoothnes
{ SmoothingMode::SmoothingModeAntiAlias };

inline dbj::holder<LineCap> linecap
{ LineCap::LineCapRound };

inline void test_dbj_defval_option_three() {
	// set the width to 512 for all the calls latter 
	auto	width_ = konst_.width(512);
	const	auto w1_ = konst_.width(); // 512
	auto	w2_ = konst_.width(); // 512

	_ASSERTE(width_ == 512);
	_ASSERTE(w2_ == w1_ && w2_ == width_ && w1_ == width_);

	//use the previosuly declared 
	// and instantiated def val holders
	auto lc_ = linecap();
	auto sness = smoothnes();
}

DBJ_TEST_SPACE_CLOSE