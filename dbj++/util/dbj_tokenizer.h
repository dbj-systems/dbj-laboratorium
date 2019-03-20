#pragma once

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

#include "../core/dbj_traits.h"
#include <string>
#include <vector>

namespace dbj {

	namespace internal {

		template< typename STYPE> struct tokenizer_engine;

		// most common used pair_tokenizer engines types
		typedef tokenizer_engine< std::string > tokenizer_eng;
		typedef tokenizer_engine< std::wstring > wtokenizer_eng;

		template< typename STYPE>
		struct tokenizer_engine final
		{
			static_assert(
				std::is_same_v<STYPE, std::string > || std::is_same_v<STYPE, std::wstring >,
				"tokenizer_engine requires std::string or std::wstring type and no other"
				);

			using string_type = STYPE;
			using pos_vector = std::vector< size_t >;

			mutable pos_vector			vector_of_begins;
			mutable pos_vector			vector_of_ends;
			mutable STYPE				src_;
			mutable STYPE				tag_;

			// instance is in a "null mode" when both src_ and tag_ are empty
			// this is not a good desing, this is a cludge
			constexpr bool null_mode() const noexcept {
				return src_.empty() && tag_.empty();
			}

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
				_ASSERTE(false == this->null_mode());

				if (0 == vector_of_ends[ord_] - vector_of_begins[ord_])
					return STYPE();

				return src_.substr(
					vector_of_begins[ord_],
					vector_of_ends[ord_] - vector_of_begins[ord_]
				);
			}

			// 
			size_t tag_size() const noexcept { return tag_.size(); }
			// 
			size_t src_size() const noexcept { return src_.size(); }
			//
			//IMPLEMENTATION
			//
			void parseLine()
			{
				_ASSERTE(false == this->null_mode());

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

			// 
			STYPE getWord(const size_t pos_) const
			{
				return pos_ > src_.size() ? STYPE() : this->src_substr(pos_);
			}

#ifndef _DEBUG
			~tokenizer_engine() = default;
#else
			~tokenizer_engine() {
				if (false == this->null_mode()) {
					vector_of_begins.clear();
					vector_of_ends.clear();
					src_.clear();
					tag_.clear();
				}
			}
#endif

			// begins and ends must be the same size
			size_t size() const throw()
			{
				return vector_of_begins.size();
			}

			// forbidden
			tokenizer_engine() = delete;

			constexpr explicit tokenizer_engine(STYPE mSrc_, STYPE mTag_) noexcept
				: src_(mSrc_), tag_(mTag_)
			{
				if (false == this->null_mode())
					this->parseLine();
			}

			//  copy
			tokenizer_engine & operator=(const tokenizer_engine &right) = delete;
			tokenizer_engine(const tokenizer_engine & right) noexcept
				: src_(right.src_), tag_(right.tag_)
			{
				if (false == this->null_mode())
					this->parseLine();
			}
			// move
			tokenizer_engine & operator=(tokenizer_engine && right) = delete;
			tokenizer_engine(tokenizer_engine && right) = delete;

		}; // tokenizer_engine

	}; // internal

	//--------------------------------------------------------------
	template< typename ENGINE_T > class pair_tokenizer;

	typedef pair_tokenizer<internal::tokenizer_eng>		pair_stokenizer;
	typedef pair_tokenizer<internal::wtokenizer_eng>	pair_wtokenizer;

	//--------------------------------------------------------------
	template< typename ENGINE_T >
	class pair_tokenizer final
	{
		using string_type = typename ENGINE_T::string_type;
		using pos_pair = std::pair<	size_t, size_t >;
		using pos_pair_vector = std::vector< pos_pair >;

		// by giving empty arguments we effectively create
		// 'null' instance of the engine which has no
		// default ctor available
		mutable  ENGINE_T engine_{ string_type{} , string_type{} };
		// mutable	pos_pair_vector		vector_of_pos_pairs_;

		pos_pair_vector	&
			vector_of_pos_pairs(/*bool rebuild = false*/) const noexcept
		{
			auto & b_v = engine_.vector_of_begins;
			auto & e_v = engine_.vector_of_ends;

			auto build = [&]() -> pos_pair_vector
			{
				pos_pair_vector retval{};
				for (size_t j{ 0 }; j < b_v.size(); j++) {
					retval.push_back(
						std::make_pair(
							b_v[j], e_v[j]
						)
					);
				}
				return retval;
			};
			// anchor
			static pos_pair_vector pairs_v{ build() };
			// if (rebuild) pairs_v = build();
			return pairs_v;
		}
	public:

		pair_tokenizer() = delete;

		explicit pair_tokenizer
		(string_type src_, string_type tag_)
			: engine_(ENGINE_T{ src_, tag_ })
		{}

		~pair_tokenizer() {
			engine_.~ENGINE_T();
		}

		auto begin()  noexcept {
			return vector_of_pos_pairs().begin();
		}

		auto end()  noexcept {
			return vector_of_pos_pairs().end();
		}

		auto  operator ++ () {
			return vector_of_pos_pairs()++;
		}

		// return the word by its ordinal number
		string_type operator [] (pos_pair beg_end) const noexcept
		{
			return  word(beg_end);
		}

		// take pair of word begin and end and return whats in between
		string_type word(pos_pair beg_end) const noexcept {
			auto & beg = beg_end.first;
			auto & end = beg_end.second;
			return engine_.src_.substr(beg, end - beg);
		}
	};

	//--------------------------------------------------------------
	template< typename ENGINE_T > class word_tokenizer;

	typedef word_tokenizer<internal::tokenizer_eng>		word_stokenizer;
	typedef word_tokenizer<internal::wtokenizer_eng>	word_wtokenizer;

	template< typename ENGINE_T >
	class word_tokenizer final
	{
		using string_type = typename ENGINE_T::string_type;
		using word_vector_type = std::vector< string_type >;

		// by giving empty arguments we effectively create
		// 'null' instance of the engine which has no
		// default ctor available
		mutable  ENGINE_T engine_{ string_type{} , string_type{} };
		// mutable	pos_pair_vector		vector_of_pos_pairs_;

		word_vector_type	&
			words_vector() const noexcept
		{
			auto build = [&]() -> word_vector_type
			{
				word_vector_type retval{};
				for (size_t j{ 0 }; j < engine_.size(); j++) {
					retval.push_back(
						engine_.getWord(j)
					);
				}
				return retval;
			};
			// anchor
			static word_vector_type words_v{ build() };
			return words_v;
		}
	public:

		word_tokenizer() = delete;

		explicit word_tokenizer
		(string_type src_, string_type tag_)
			: engine_(ENGINE_T{ src_, tag_ })
		{}

		~word_tokenizer() {
			engine_.~ENGINE_T();
		}

		auto begin()  noexcept {
			return words_vector().begin();
		}

		auto end()  noexcept {
			return words_vector().end();
		}

		auto  operator ++ () {
			return words_vector()++;
		}

		size_t size() const noexcept { return words_vector().size(); }

		// return the word by its ordinal number
		string_type operator [] (size_t ord_) const noexcept
		{
			return  words_vector()[ord_];
		}
	};

} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"