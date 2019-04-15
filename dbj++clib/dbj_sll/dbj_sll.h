
/*
(c) Copyright 2018-2019 by dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http ://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Singly Linked List of strings
Specialised non-generic , one head per thread solution
Each node has a key which is a hash of the string it also has
*/

#pragma once

#ifndef DBJ_SLL
#define DBJ_SLL

#include <stdbool.h>
#include <stdio.h>

#ifdef __clang__
/*
http://clang.llvm.org/docs/UsersManual.html#controlling-diagnostics-in-system-headers
*/
#pragma clang system_header
#endif /* __clang__ */

#ifdef __cplusplus
extern "C" {
#endif

			typedef struct dbj_sll_node dbj_sll_node;

#define DBJ_SLL_HEAD_KEY ((unsigned long)-1)

			/* typedef */ struct dbj_sll_node {
				unsigned long key;
				/*
				 the above is usually the hash of the data bellow
				 this it is unsigned long
				 see the dbj_hash and the link in its comment
				*/
				char * data;
				dbj_sll_node * next;
			} /*dbj_sll_node*/;

			dbj_sll_node * dbj_sll_make_head();
			dbj_sll_node *
				dbj_sll_append(dbj_sll_node * head_, const char * str_);

			dbj_sll_node * dbj_sll_foreach
			(dbj_sll_node * head_, bool(*visitor)(dbj_sll_node *));

			bool dbj_sll_node_dump_visitor(dbj_sll_node * node_);

			void dbj_sll_erase(dbj_sll_node * head_);

			void dbj_sll_erase_with_head(dbj_sll_node * head_);

			dbj_sll_node * dbj_sll_find(dbj_sll_node * head_, unsigned long k_);

			dbj_sll_node * dbj_sll_remove_tail(dbj_sll_node * head_);

			bool is_dbj_sll_empty(dbj_sll_node * head_);

			bool is_dbj_sll_head(dbj_sll_node * head_);
			bool is_dbj_sll_tls_empty();
			char const * dbj_sll_data(dbj_sll_node * current_);
			unsigned long dbj_sll_key(dbj_sll_node * current_);
			size_t dbj_sll_count(dbj_sll_node * head_);

#ifdef DBJ_SLL_TESTING
			extern void test_dbj_sll(const char * what_to_append, size_t how_many_times, bool verbose);
#endif 

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // ! DBJ_SLL
