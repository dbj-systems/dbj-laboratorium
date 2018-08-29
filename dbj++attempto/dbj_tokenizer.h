#pragma once
#include "pch.h"

// added bu JovanovD 23.09.17
/*
From: Thursday, 21 May 1998, 08:41:40
To: Saturday, 23 September 2017, 08:41:38
Difference is: 7064 days, 23 hours, 59 minutes and 58 seconds
Or 19 years, 4 months, 1 day excluding the end date
This is how my original code was old today.
*/
//*****************************************************************************/
//
//                  
//
//                 Copyright (c)  1997 - 2015 by Dusan B. Jovanovic (dbj@dbj.org) 
//                          All Rights Reserved
//
//        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF Dusan B. Jovanovic (dbj@dbj.org)
//
//        The copyright notice above does not evidence any
//        actual or intended publication of such source code.
//
//  $Author: DBJ  $
//  $Date: $
//  $Revision: $
//*****************************************************************************/

namespace dbj::samples {

namespace internal {

template< typename STYPE> struct tokenizer_engine;

// most common used tokenizer engines types
typedef tokenizer_engine< std::string > tokenizer_eng;
typedef tokenizer_engine< std::wstring > wtokenizer_eng;

template< typename STYPE>
struct tokenizer_engine final
{
	using string_type = STYPE;

	using pos_vector = std::vector< size_t >; 

	mutable pos_vector			vector_of_begins;
	mutable pos_vector			vector_of_ends;
	mutable STYPE				src_;
	mutable STYPE				tag_;

	// return not-a-position value for this string type
	auto NPOS() const noexcept { static auto lazy_retval = STYPE::npos; return lazy_retval; }
	// find tag position in the source begining at given position
	// for this string type
	size_t find_tag_position(size_t starting_position) const
	{
		return src_.find(tag_, starting_position);
	}
	// return substring from the source
	// starting from vector_of_begins[ord_] using
	// length vector_of_ends[ord_] - vector_of_begins[ord_]
	// 
	STYPE src_substr(size_t ord_) const
	{
		if (0 == vector_of_ends[ord_] - vector_of_begins[ord_])
			return STYPE();

		return src_.substr(
			vector_of_begins[ord_], 
			vector_of_ends[ord_] - vector_of_begins[ord_]
		);
	}

	// size() semantics for the STYPE used
	// 
	size_t tag_size() const noexcept { return tag_.size();	}
	// 
	size_t src_size() const noexcept { return src_.size();	}
	//
	//IMPLEMENTATION
	//
	void parseLine()
	{
		size_t tagSize_ = tag_size();
		size_t tagStartPos_ = 0, tagEndPos_ = 0;

		vector_of_begins.clear();
		vector_of_ends.clear();

		if (tagSize_ < 1)
			return;
		if (src_size() < 1)
			return;

		vector_of_begins.push_back(tagStartPos_);

		while ((tagEndPos_ = find_tag_position(tagStartPos_)) != NPOS())
		{
			vector_of_ends.push_back(tagEndPos_);
			tagStartPos_ = tagEndPos_ + tagSize_;
			vector_of_begins.push_back(tagStartPos_);
		}
		vector_of_ends.push_back(src_size());
	}

	//--------------------------------------------------
	// use this method when you want to reuse the only
	// instance of this class
	void reset(STYPE & mSrc_, STYPE & mTag_)
	{
		src_.swap(mSrc_);
		tag_.swap(mTag_);
		parseLine();
	}

	// 
	STYPE getWord(const size_t pos_) const
	{
		return pos_ > src_.size() ? STYPE() : this->src_substr(pos_);
	}

	// forbidden
	tokenizer_engine() = delete;

	explicit tokenizer_engine(const STYPE & mSrc_, const STYPE & mTag_)
	{
		reset((STYPE &)mSrc_, (STYPE &)mTag_);
	}

	~tokenizer_engine() = default;

	// begins and engs must be the same size
	size_t size() const throw()
	{
		return vector_of_begins.size();
	}

	//  copy
	tokenizer_engine & operator=(const tokenizer_engine &right) = delete;
	tokenizer_engine(const tokenizer_engine & right) = delete;

	// move
	tokenizer_engine & operator=(tokenizer_engine && right) = delete;
	tokenizer_engine(tokenizer_engine && right) = delete;

		}; // tokenizer_engine

	}; // internal

//--------------------------------------------------------------
template< typename ENGINE_T > class tokenizer;

typedef tokenizer<internal::tokenizer_eng>		stokenizer;
typedef tokenizer<internal::wtokenizer_eng>		wtokenizer;

//--------------------------------------------------------------
template< typename ENGINE_T >
class tokenizer final 
{
	using string_type = typename ENGINE_T::string_type;
	using pos_pair = std::pair<	size_t, size_t >;
	using pos_pair_vector = std::vector< pos_pair >;

	mutable  ENGINE_T * engine_{};
	// mutable	pos_pair_vector		vector_of_pos_pairs_;

	pos_pair_vector	&
		vector_of_pos_pairs(bool rebuild = false) const noexcept
	{
		auto & b_v = engine_->vector_of_begins;
		auto & e_v = engine_->vector_of_ends;

		auto build = [&]() -> pos_pair_vector 
		{
			pos_pair_vector retval{};
			for (size_t j; j < b_v.size(); j++) {
				retval.push_back(
					std::make_pair(
						b_v[j], e_v[j]
					)
				);
			}
			return retval;
		};
		// anchor
		static pos_pair_vector pairs_v = build();
			if (rebuild) pairs_v = build();
				return pairs_v ;
	}
public :
	
	tokenizer(
		const string_type & src_, const string_type & tag_
	) : engine_(new ENGINE_T{ src_, tag_ } ) {}

	~tokenizer() { if (engine_ != nullptr) { delete engine_; } engine_ = nullptr; }

	auto begin()  noexcept {
		return vector_of_pos_pairs().begin();
	}

	auto end()  noexcept {
		return vector_of_pos_pairs().end();
	}

	// size_t walker{ 0 };
	auto  operator ++ () {
		return vector_of_pos_pairs()++;
	}

	// return the word by its prdinal number
	string_type operator [] (size_t ord_) const
	{
		return  engine_->getWord(ord_);
	}
};

}; // dbj::samples 
   /* standard suffix for every other header here */
#pragma comment( user, __FILE__ "(c) 2017 by dbj@dbj.org | Version: " __DATE__ __TIME__ ) 
   /*
   Copyright 2017 by dbj@dbj.org

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http ://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   */


