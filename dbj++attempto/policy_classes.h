#pragma once
#include "pch.h"

/*
__interface msvc keyword
explained here: https://docs.microsoft.com/en-us/cpp/cpp/interface
*/

#ifndef implements
#define implements public
#endif

namespace dbj::samples {
	namespace philology {
		using std::string;
		using dbj::console::print;
		/*
		 interfaces to writing and language policies
		*/
		__interface IWriterPolicy {
			void write(const string &) const noexcept ;
		};
		__interface ILanguagePolicy {
			string translate(const string & ) const noexcept ;
		};
		// classes to be used
		// as default policies		
		struct WideWriterPolicy 
			: implements IWriterPolicy {
			void write(const string & msg) const noexcept 
			{ print(" ", msg); }
		};
		struct LanguagePolicyEnglish 
			: implements ILanguagePolicy {
			string translate(const string & msg) const noexcept 
			{ return msg; }
		};
		// german is also here as some folks will 
		// want to make it a default language
		struct LanguagePolicyGerman 
			: implements ILanguagePolicy {
			string translate(const string & msg) const noexcept 
			{ return msg; }
		};

		template <
			/* make English to be compiled as the default language */
			typename LanguagePolicy = LanguagePolicyEnglish,
			/* compile wide string writer as the defualt output policy */
			typename OutputPolicy = WideWriterPolicy
		>
			class HelloWorld : private OutputPolicy, private LanguagePolicy
		{
			using OutputPolicy::write;
			using LanguagePolicy::translate;

		public:
			// Behaviour method
			void run(string message) 
			{
				// Two policy methods
				write(translate(message));
			}
		};

/*
No inhertiance! No name lookup issues.
*/
template <
	typename LanguagePolicy = LanguagePolicyEnglish,
	typename OutputPolicy = WideWriterPolicy >
	class HelloWorld2
{
public:
	// Behaviour method
	void run( string msg)
	{
		// not made before this point; if ever.
		static LanguagePolicy language{};
		static OutputPolicy printer{};
		// Two policy methods
		printer.write( 
			language.translate(msg)
		);
	}
};
        


	} // namespace philology 
} //namespace dbj::samples 

/*Copyright 2017 by dbj@dbj.org

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