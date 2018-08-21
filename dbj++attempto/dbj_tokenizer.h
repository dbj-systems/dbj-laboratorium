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
#pragma once

#ifdef DBJ_TOKENIZER_USES_BSTR
#include <comutil.h>
#endif
//---------------------------------------------------------------------------------------
namespace dbj_samples {
/* fm == Foundation Mechanisms */
namespace fm {
		template< typename STYPE, typename CHARTYPE >
		class tokenizerT
		{
		protected:

			// 
			std::vector<size_t> vector_of_begins;
			// 
			std::vector<size_t> vector_of_ends;
			// internally we use std::wstring for implementation
			// UNICODE is supposed to be quicker under W2K and NT
			// 
			mutable STYPE src_;
			// 
			mutable STYPE tag_;
			// return not-a-position value for this string type
			// 
			size_t NPOS() const
			{
				return STYPE::npos;
			}
			// find tag position in the source begining at given position
			// for this string type
			// 
			size_t find_tag_position(size_t starting_position) const
			{
				return src_.find(tag_, starting_position);
			}
			// return substring from the source
			// starting from vector_of_begins[pos_] using
			// length vector_of_ends[pos_] - vector_of_begins[pos_]
			// 
			STYPE src_substr(size_t pos_) const
			{
				if (0 == vector_of_ends[pos_] - vector_of_begins[pos_])
					return STYPE();

				return src_.substr(vector_of_begins[pos_], vector_of_ends[pos_] - vector_of_begins[pos_]);
			}
			// size() semantics for the STYPE used
			// 
			size_t tag_size() const
			{
				return tag_.size();
			}
			// 
			size_t src_size() const
			{
				return src_.size();
			}
			//
			//IMPLEMENTATION
			//
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

			// namespace {
			using IDX_VECTOR = std::vector<size_t >;
			inline IDX_VECTOR & indices(bool reset = false) {

				auto ordered_int_sequence = [](size_t sz_) {
					IDX_VECTOR rezult;
					for (size_t j = 0; j != sz_; ++j)
						rezult.push_back(j);
					return rezult;
				};

				static IDX_VECTOR indices_{ ordered_int_sequence(vector_of_begins.size()) };
				if (reset) indices_ = ordered_int_sequence(vector_of_begins.size());
				return indices_;
			}
			// }
			//--------------------------------------------------
			// use this method when you want to reuse the only
			// instance of this class
			void reset(STYPE & mSrc_, STYPE & mTag_)
			{
				src_.swap(mSrc_);
				tag_.swap(mTag_);
				parseLine();
				indices(true); // reset the indices too
			}

			// 
			STYPE getWord(const size_t pos_) const
			{
				// dbjVERIFY( pos_ > -1 );
				// std::end() points after the end so we can't assert pos < vector_of_begins.size()
				return pos_ > src_.size() ? STYPE() : /* cast_to_stype */(src_substr(pos_));
			}

			// forbidden
			tokenizerT() {}
		public:

			typedef STYPE string_type; // DBJ 19NOV2000
			typedef std::size_t size_t;
			//
			// 
			explicit tokenizerT(const STYPE & mSrc_, const STYPE & mTag_)
			{
				reset(const_cast<STYPE & >(mSrc_), const_cast<STYPE & >(mTag_));
			}

			~tokenizerT()
			{   // 210598 JovanovD added
				// vector_of_begins.erase( vector_of_begins.begin(), vector_of_begins.end() ) ;
				// vector_of_ends.erase( vector_of_ends.begin(), vector_of_ends.end() ) ;
				// 020517 DBJ
				vector_of_begins.clear();
				vector_of_ends.clear();
			}


			auto begin() {
				return indices().begin();
			}

			auto end() {
				return indices().end();
			}
			// added by JovanovD 21.05.98
			//
			STYPE operator [] (size_t pos_) const
			{
				return getWord(pos_);
			}
			//
			size_t size() const throw()
			{
				return vector_of_begins.size();
			}

			// 
			tokenizerT & operator=(const tokenizerT &right)
			{
				std::swap(*this, right);
				return *this;
			}
			tokenizerT & operator=(tokenizerT && right)
			{
				std::swap(*this, right);
				return *this;
			}

			// copy
			tokenizerT(const tokenizerT & right)
			{   // 210598 JovanovD added
				std::swap(*this, right);
			}
			// move -- called on rvalues only. rvalue is temporary object
			// 250917 JovanovD added
			tokenizerT(tokenizerT && right) noexcept {
				std::swap(*this, right);
			}

			friend auto swap(tokenizerT & left, tokenizerT & right) {
				using std::swap;
				swap(left.vector_of_begins, right.vector_of_begins);
				swap(left.vector_of_ends, right.vector_of_ends);
				swap(left.src_, right.src_);
				swap(left.tag_, right.tag_);
				indices(true);
			}

		}; // tokenizerT

		   //--------------------------------------------------------------
		   // specializations necessary for tokenizerT<std::string,char> to work
		   // transform std::wstring to std::string
#if 0
		__forceinline
			std::string tokenizerT< std::string, char >::cast_to_stype(const std::wstring & wstr) const
		{
			return std::string(wstr.begin(), wstr.end());
		}
		__forceinline
			void tokenizerT< std::string, char >::reset(
				const std::string & mSrc_, const std::string & mTag_)
		{
			src_ = (wchar_t*)std::wstring(mSrc_.begin(), mSrc_.end()).data();
			tag_ = (wchar_t*)std::wstring(mTag_.begin(), mTag_.end()).data();
			parseLine();
		}
#endif
		//--------------------------------------------------------------
		// most common used tokenizing types
		typedef tokenizerT< std::string, char >               tokenizer;
		typedef tokenizerT< std::wstring, wchar_t >           wtokenizer;
		//--------------------------------------------------------------

	}; // fm
}; // dbjsys
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


