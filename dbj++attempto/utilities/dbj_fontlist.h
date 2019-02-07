#pragma once
#include "pch.h"

namespace dbj::samples {


	namespace {

		/* repeated here to cut off some dependancies */
		template <typename Type>
		inline std::vector<Type> & remove_duplicates_(std::vector<Type> & vec, bool sort = false) {
			if (sort) {
				std::sort(std::begin(vec), std::end(vec));
			}
			auto iterator_following_the_last_removed_element =
				vec.erase(std::unique(std::begin(vec), std::end(vec)), std::end(vec));

			return vec;
		}

		typedef std::pair<LOGFONTW, DWORD>	FontPair;
		typedef std::vector<FontPair>		FontVec;

		// why extern "C" ?
		inline bool comp(FontPair& left, FontPair& right)
		 {
		     if (( L'@' != left.first.lfFaceName[0]) && ( L'@' == right.first.lfFaceName[0]))
		          return true;
		     if (( L'@' == left.first.lfFaceName[0]) && ( L'@' != right.first.lfFaceName[0]))
		          return false;

		     return ( wcscmp(left.first.lfFaceName, right.first.lfFaceName) < 0);
		 }
		
		 /* extern "C" */ 
		inline int CALLBACK enumerateFontsCallBack(
			 ENUMLOGFONTEXW *lpelf,
		     NEWTEXTMETRICEX *lpntm,
		     DWORD fontType, LPARAM lParam)
		 {
		     (void)lpntm;
		
		     FontVec* pFontVec = (FontVec*)lParam;
		     pFontVec->push_back(FontPair(lpelf->elfLogFont, fontType));
		     return TRUE;
		 }

		/**
		  return *all* fonts available
		  by default sort the result by puting names starting with "@" at the end
		  if argument is false do not sort
		*/
		inline FontVec enumerateFonts( bool sort = true )
		{
			FontVec     fonts_vector;
			HDC the_hdc = ::GetWindowDC(::GetConsoleWindow());

			fonts_vector.clear();

			LOGFONTW     lf;
			lf.lfFaceName[0] = L'\0' ;
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfPitchAndFamily = 0;
			::EnumFontFamiliesExW(the_hdc, &lf,
				(FONTENUMPROCW)enumerateFontsCallBack,
				(LPARAM)&fonts_vector, 0);

			  if (sort) std::sort(fonts_vector.begin(), fonts_vector.end(), comp);

			  return fonts_vector;

		}

		/**
		 by default remove duplicates before returning
		 if second optional argument is false do not remove duplicates
		*/
		inline 
		std::vector<std::wstring>  
			font_names(
				const FontVec & font_vec_, 
				bool remove_duplicates = true 
			) {
			std::vector<std::wstring> str_vec;
			for (auto el : font_vec_ ) {
				str_vec.push_back(el.first.lfFaceName);
			}

			if (remove_duplicates)
				str_vec = remove_duplicates_(str_vec);

			return str_vec;
		}

	} // anon
} // dbj