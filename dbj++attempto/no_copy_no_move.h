#pragma once
#include "pch.h"

namespace dbj::samples {

	/// <summary>
	/// inherit as private base this class
	/// to make the offspring(s) 
	/// no copy
	/// no move
	/// dbj.org created 
	/// </summary>
	class __declspec(novtable) no_copy_no_move
	{
	protected:
		no_copy_no_move() = default;
		virtual ~no_copy_no_move() {};
	private:
		// no copy
		no_copy_no_move(const no_copy_no_move&) = delete;
		no_copy_no_move& operator=(const no_copy_no_move&) = delete;
		// no move
		no_copy_no_move(no_copy_no_move&&) = delete;
		no_copy_no_move& operator=(no_copy_no_move&&) = delete;
	};

	DBJ_TEST_SPACE_OPEN(local_tests)

	struct D;
	extern const D not_d;
	struct D
	{
		enum class state_code : char { non_initialized ='?', constructed = '!'};
		state_code state{ state_code::non_initialized };
		// D() = delete;
		explicit D (state_code new_payload_ = state_code::constructed ) : state(new_payload_) {}
		D( const D & other_) : _ref_( other_ ), state( other_.state) {  }
		D & operator = (D  other_) { std::swap(state, other_.state);  return *this; }
		std::reference_wrapper<const D> _ref_{ not_d }; // OK
	};

	inline const D not_d ;

	DBJ_TEST_UNIT( member_ref_to_its_host ) {
		
		D d1, d2, d3;

		d1 = d2 = d3;

		auto s1 = d1.state;

	}

	struct A;
	extern const A empty_a ;
	struct A
	{
		char state ;
		const A& ref{ empty_a }; // OK
		A( ) : state((char)127) {}
		// copy
		A(const A & other_) : ref(other_), state(other_.state) { }
		A & operator = (const A  & other_) = delete; //  { state = other_.state;  return *this; }
		// move
		A(A && other_) : ref(other_) { std::swap(state, other_.state); }
		A & operator = (A && other_) { std::swap(state, other_.state);  return *this; }
	};
	
	inline const A empty_a{ }; // OK

	DBJ_TEST_UNIT(no_copy_no_move) {

		auto test = []() {
			// create a_3 ref a_2 ref a_1 ref empty_a
			A a_3{     }; a_3.state = '3';
			A a_2{ a_3 }; a_2.state = '2';
			A a_1{ a_2 }; a_1.state = '1';
			return a_3;
		};

		auto r1 = test();
	}

	DBJ_TEST_SPACE_CLOSE
}