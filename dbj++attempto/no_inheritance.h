#pragma once
#include "pch.h"

namespace dbj::samples {
	namespace docops {
		using string = std::string;
		/*
		__interface msvc keyword
		explained here: https://docs.microsoft.com/en-us/cpp/cpp/interface

		Operation type interface
		Operation type is not a document type
		*/
		__interface IOperation {
			/*
			Future extension: introduce operations on database stored documents
			enum class type { online, local, database };
			*/
			enum class type { online, local };
			bool open(string name);
		};

		typedef typename std::unique_ptr<IOperation> pointer;

		/*
		return or change/return the operation type
		this alows external configuration/setup
		see the main() bellow
		*/
		inline IOperation::type operations_type(
			const IOperation::type & the_type_ = IOperation::type::online
		)
		{
			static IOperation::type default_type = IOperation::type::online;
			static IOperation::type current_type = IOperation::type::online;
			return the_type_ != current_type ? current_type = the_type_ : current_type;
		}

		// IOperation implementations are completely hidden from users
		namespace {
			class LocalDocOp final : public IOperation {
				bool open(string name) { return true; }
			};
			class OnlineDocOp final : public IOperation {
				bool open(string name) { return true; }
			};
			/*
			Future extension:
			class DatabaseDocOp final : public IOperation {
			bool open(string name) { return true; }
			};
			*/
		}
		/* IOperation implementations are decoupled from users
			with the use of factory method bellow
		*/
		inline pointer operations_policy(
			/* if we call with no arguments the configured op type is made
				otherwise the argument is used to make a desired op type
			*/
			const IOperation::type & type_to_make = operations_type()
		) {
			/*	Future extension plan:
					if (operations_type() == IOperation::type::database)
				return make_unique<DatabaseDocument>(DatabaseDocument{});
			*/
			if (type_to_make == IOperation::type::local)
				return std::make_unique<LocalDocOp>(LocalDocOp{});

			return std::make_unique<OnlineDocOp>(OnlineDocOp{});
		}
} // docops

namespace documents {
	using string = std::string;
		/*
		IDocument interface describes the behaviour of documents
		Document hierarchy uses Documents Operations
		Types of documents do not equal type of operations on them
		thus we do not inherit from doc operations
		*/
		__interface IDocument {
			/*
			Future extension: introduce database stored documents
			enum class type { online, local, database };
			*/
			enum class type { online, local };
			bool open(string name);
		};
		/*
		Future expansion: introduce DataBase document
		That will not change the existing code bellow whatsoever.
		*/
		class Document : public IDocument {
			/*	Types of documents do not equal type of operations on them
				thus we do not inherit from doc operations
				Document hierarchy uses Documents Operations
				through this docops pointer
			*/
			docops::pointer docops = docops::operations_policy();
		public:
			bool open(string name) { return docops->open(name); }
		};

		class TextDoc final : public Document {};
		class BinaryDoc final : public Document {};

		/* a single method to open any kind of a document
			polymorphic behaviour is OK here
		*/
		inline int opendoc(Document & doc, string && name) { return doc.open(name); }

	}
} // dbj

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
