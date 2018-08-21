#pragma once
#include "pch.h"

namespace dbj_samples {
	namespace keep {
		template <typename T>
		inline std::wstring	to_static_wstring(const T & t) {
			static std::wstring retval_; // prolong the life of returned value
			std::wostringstream out ;
			out << t;
			retval_.clear(); 
			retval_.append( out.str() ); //copy
			return retval_;
		}

		template<>
		inline std::wstring	to_static_wstring<std::wstring>(const std::wstring & t) {
			static std::wstring retval_; // prolong the life of returned value
			retval_.clear();
			retval_.append(t.data()); //copy
			return retval_;
		}
	}

	namespace inner {
		/*
		we have nicked the following from corecrt_wstdio.h
		*/
		template< size_t N>
		inline int dbj_snwprintf_s
		(
			wchar_t(&_Buffer)[N],
			size_t         const _BufferCount,
			size_t         const _MaxCount,
			wchar_t const* const _Format,
			...)
		{
			int _Result;
			::va_list _ArgList;
			__crt_va_start(_ArgList, _Format);
			_Result = _vsnwprintf_s_l(_Buffer, _BufferCount, _MaxCount, _Format, NULL, _ArgList);
			__crt_va_end(_ArgList);
			return _Result;
		}
	}

	template <typename ... Args>
	inline std::wstring bufprint(wchar_t const * const format, Args ... args) noexcept
	{
		/* NOTE: 512 happpens to be the BUFSIZ */
		wchar_t buffer[BUFSIZ * 2] = { L'0'};
		size_t buffer_size = dbj::countof(buffer);
		assert(-1 != dbj_snwprintf_s(buffer, buffer_size - 1, buffer_size - 1, format, (args) ...));

		return { buffer };
	}
} // dbj

namespace dbj_samples {
	namespace treeprint {
		namespace {
			constexpr wchar_t doubles[] = { L"║═╚" };
			constexpr wchar_t singles[] = { L"│─└" };
			constexpr wchar_t SS = L' '; // single space
			constexpr int V{ 0 }; 
			constexpr int H{ 1 }; 
			constexpr int K{ 2 };


			/*
			Print the binary tree made of NODE's
			NODE must have three methods:
			NODE * left  ();
			NODE * right ();
			NODE::DataType * data  ();
			Where NODE::DataType requirement is to be unicode output streamable

			The actual alorithm inspired by: http://www.randygaul.net/2015/06/15/printing-pretty-ascii-trees/
			*/
			constexpr size_t BUFSIZ_ = { 2056 };

			class BinaryTreePrinter final {

				wchar_t depth[BUFSIZ_] = {};
				int di = int(0);

				void Push(wchar_t c)
				{
					// depth[di++] = L' ';
					depth[di++] = c;
					depth[di++] = L' ';
					depth[di++] = L' ';
					depth[di] = 0;
				}

				void Pop()
				{
					// 3? why 3?
					depth[di -= 3] = 0;
				}
				/*
				this is the only data memeber
				*/
				mutable std::wstring out_buf_{ BUFSIZ };
			public:
				BinaryTreePrinter() {}

				~BinaryTreePrinter(){out_buf_.clear();}

				BinaryTreePrinter(BinaryTreePrinter&&) = default;
				BinaryTreePrinter & operator = (BinaryTreePrinter&&) = default;

				/*
				starting from an given node print into and return the result in an wstring
				*/
				template<typename NODE>
				std::wstring operator ()(NODE * tree, const int outwid_ = 0)
				{
					print_(tree, outwid_);
					out_buf_.append(L"\n");
					return out_buf_ ; // return the copy
				}
				/* save it for return and then flush the local output buffer */
				auto flush() {
					auto retval{ this->out_buf_.data() }; // the copy
					out_buf_.clear();
					return retval;
				}
			private:
				// print the tree into the local buffer
				template<typename NODE>
				std::wstring  & print_(NODE* tree, const int outwid_)
				{
					if (tree) {
						// assert(tree->data());
						// out_buf_ += bufprint(L"{%s}\n", dbj::conv::to_wstring(tree->data())	);
						out_buf_ += dbj::conv::to_wstring(tree->data());
						out_buf_ += L"\n";
					}
					if (tree->left()) {
						out_buf_ += bufprint(L"%s%c\n", depth, singles[V]);
						out_buf_ += bufprint(L"%s%c%c%c", depth, singles[K], singles[H], singles[H]);
						Push(singles[V]);
						print_(tree->left(), outwid_); // recurse left
						Pop();
					}
					if (tree->right()) {
						// this->out_buf_ << depth << singles[V] << L"\n";
						// this->out_buf_ << depth << singles[K] << singles[H] << singles[H];
						out_buf_ += bufprint( L"%s%c\n", depth, singles[V]);
						out_buf_ += bufprint( L"%s%c%c%c", depth, singles[K], singles[H], singles[H]);
						Push(SS);
						print_(tree->right(), outwid_);
						Pop();
					}

					return this->out_buf_;
				}

			}; // eof BinaryTreePrinter
		} // namespace

		template <typename TREE>
		inline std::wstring binary_tree_to_string ( TREE * root_ , unsigned outwid = 0 )
		{
			BinaryTreePrinter tpf{};
			return std::wstring( tpf(root_, outwid) );
		}

	} //namespace treeprint 
} //namespace dbj_samples 
#pragma comment( user, "(c) 2017 by dbj@dbj.org :: " __FILE__ " :: " __DATE__ " :: " __TIME__ ) 
  /*
  Copyright 2017 dbj@dbj.org

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
