#pragma once
/*
Copyright 2018 by dbj@dbj.org

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
#ifdef __clang__
/*
http://clang.llvm.org/docs/UsersManual.html#controlling-diagnostics-in-system-headers
*/
#pragma clang system_header
#endif /* __clang__ */

#ifdef __cplusplus
extern "C" {
#endif

/* apparently only *static extern* variables can use thread local storage */
#ifdef _MSC_VER
#define dbj_thread_local __declspec(thread) static
#define dbj_malloc(type, count) (type *)malloc( count * sizeof(type))
#else
#define dbj_thread_local static __thread 
#define dbj_malloc(type, count) malloc( count * sizeof(type))
#endif

	/* for the DBJ SLL key making */
	/* the djb2 from: http://www.cse.yorku.ca/~oz/hash.html */
	static unsigned long dbj_hash(unsigned char *str)
	{
		unsigned long hash = 5381;
		int c;

		while ((c = *str++))
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

		return hash;
	}

	/*
	Singly Linked List of strings
	Specialised non-generic , one head per thread solution
	Each node has a key which is a hash of the string it also has
	*/

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

	/********************************************************/
	static dbj_sll_node * dbj_sll_node_new()
	{
		dbj_sll_node * new_
			= (dbj_sll_node *)malloc(sizeof(dbj_sll_node));
		if (new_) {
			new_->key = 0;
			new_->data = 0;
			new_->next = 0;
		}
		else {
			errno = ENOMEM;
		}
		return new_;
	}
	/********************************************************/
	/*
	Special "TLS_HEAD" global per this thread
	sThread Local Storage (TLS) solution
	 */
	static dbj_sll_node * dbj_sll_tls_head() {
		dbj_thread_local dbj_sll_node
			dbj_sll_tls_head_ = { DBJ_SLL_HEAD_KEY, 0 , 0 };
		return &dbj_sll_tls_head_;
	}

	static dbj_sll_node * dbj_sll_make_head() {
		dbj_sll_node * _head_ = dbj_sll_node_new();
		if (_head_) {
			_head_->key = DBJ_SLL_HEAD_KEY;
			_head_->data = 0;
			_head_->next = 0;
		}
		/* head node has no data it is easy to delete */
		return _head_;
		/* ENOMEM has been set if NULL */
	}
	/********************************************************/
	static bool is_dbj_sll_head(dbj_sll_node * head_) {
		assert(head_);
		if (head_) {
			return ((head_->next == 0) && (head_->key == DBJ_SLL_HEAD_KEY));
		}
		return false;
	}
	static bool is_dbj_sll_tls_empty() { return (dbj_sll_tls_head()->next == 0); }
	static bool is_dbj_sll_empty(dbj_sll_node * head_) { return head_->next == 0; }
	static dbj_sll_node * dbj_sll_next(dbj_sll_node * current_)
	{
		assert(current_);
		return current_->next;
	}
	static const char * dbj_sll_data(dbj_sll_node * current_)
	{
		assert(current_);
		return current_->data;
	}
	static const unsigned long dbj_sll_key(dbj_sll_node * current_)
	{
		assert(current_);
		return current_->key;
	}
	/********************************************************/
	static dbj_sll_node * dbj_sll_tail(dbj_sll_node * head_)
	{
		/* get the head */
		dbj_sll_node * walker_ = head_;
		while (walker_) {
			if (!walker_->next) break;
			walker_ = walker_->next;
		}
		return walker_;
	}
	/********************************************************/
	/*  erase the whole list */
	static void dbj_sll_erase(dbj_sll_node * head_)
	{
		dbj_sll_node * current_ = head_->next;
		dbj_sll_node * temp_ = 0;
		while (current_) {
			temp_ = current_->next;
			if (current_->data) { free(current_->data); current_->data = 0; }
			free(current_);
			current_ = temp_;
		};
		/* make the head aware there are no nodes left  */
		head_->next = 0;
	}
	static void dbj_sll_erase_with_head(dbj_sll_node * head_)
	{
		dbj_sll_erase(head_);
		free(head_); /* head has no data */
		head_ = 0;
	}
	/********************************************************/
	/*
		Find by key is the deafult
		to find by string one can implement the visitor
	*/
	static dbj_sll_node * dbj_sll_find(dbj_sll_node * head_, int k_)
	{
		dbj_sll_node * walker_ = head_->next;
		while (walker_) {
			if (walker_->key == k_) return walker_;
			walker_ = walker_->next;
		}
		return NULL;
	}
	/********************************************************/
	/*
	 visitor function to each node
	 visitation is stopped when visitor returns true
	 remember: head is never used it is just an anchor
	 return the pointer to the last node visited
	 visitor function signature:

	 bool (*visitor)(dbj_sll_node *)

	NOTE: most (if not all) SLL operations
	can be implemented as visitors
	*/
	static dbj_sll_node * dbj_sll_foreach
	(dbj_sll_node * head_, bool(*visitor)(dbj_sll_node *))
	{
		dbj_sll_node * walker_ = head_->next;
		while (walker_) {
			if (visitor(walker_)) return walker_;
			walker_ = walker_->next;
		}
		/* not found */
		return walker_;
	}
	/********************************************************/
	static size_t dbj_sll_count(dbj_sll_node * head_)
	{
		size_t count_ = 0;
		/* get and skip the head */
		dbj_sll_node * walker_ = head_->next;
		while (walker_) {
			++count_;
			walker_ = walker_->next;
		}
		return count_;
	}
	/********************************************************/
	/*
	return the newly made and appended node
	str argument is copied and thus has to
	be freed sometimes later
	key is generated as the hash() of the str_argument
	*/
	static dbj_sll_node *
		dbj_sll_append(dbj_sll_node * head_, const char * str_)
	{
		dbj_sll_node * new_node = dbj_sll_node_new();
		new_node->data = dbj_strdup(str_);
		new_node->key = dbj_hash((unsigned char *)str_);

		dbj_sll_node * tail_node = dbj_sll_tail(head_);
		tail_node->next = new_node;
		return new_node;
	}

	/********************************************************/
	/* sll visitors                                         */
	/********************************************************/
	static bool dbj_sll_node_dump_visitor(dbj_sll_node * node_) {

		assert(node_);
		printf("\n\n%p", node_);
		printf("\n--------------------------------------");
		printf("\nKey: %ld", node_->key);
		printf("\nStr: [%s]", (node_->data ? node_->data : "NULL"));
		printf("\nNext: %p", (node_->next ? node_->next : 0x0));
		/* return false as a signal NOT to stop */
		return false;
	}
	/*
	this is where we can clearly see the disdvantage of singly linked lists
	vs doubly linked lists.

	but SLL's simplicity more than compesates for this
	SLL lists are short and todays machines are fast
	so the algorithms like the one bellow are OK in normal situations
	*/
	static bool dbj_sll_remove_tail_visitor(dbj_sll_node * node_)
	{
		if (!node_->next) return true; /* stop */

		/* is the next one the tail? */
		if (!node_->next->next) {
			/* next one is the tail */
			free(node_->next->data);
			free(node_->next);
			node_->next = 0;
			return true; /* stop */
		}
		return false; /* proceed */
	}

	/* return the new tail or 0 if list is empty */
	static dbj_sll_node * dbj_sll_remove_tail(dbj_sll_node * head_)
	{
		if (is_dbj_sll_empty(head_)) return 0;
		return dbj_sll_foreach(head_, dbj_sll_remove_tail_visitor);
	}

	/********************************************************/
#define DBJ_SLL_TESTING
#ifdef DBJ_SLL_TESTING
	static void test_dbj_sll()
	{
		dbj_sll_node * head_ = dbj_sll_make_head();
		dbj_sll_append(head_, "ONE");
		dbj_sll_append(head_, "TWO");
		dbj_sll_append(head_, "THREE");

		printf("\nDBJ SLL dump");
		dbj_sll_foreach(head_, dbj_sll_node_dump_visitor);
		printf("\n");

		assert(0 == strcmp(dbj_sll_remove_tail(head_)->data, "TWO"));
		dbj_sll_erase(head_);

		printf("\nHead after SLL erasure");
		dbj_sll_node_dump_visitor(head_);
		assert(true == is_dbj_sll_empty(head_));

		unsigned long k1 = dbj_sll_append(head_, "Abra")->key;
		unsigned long k2 = dbj_sll_append(head_, "Ka")->key;
		unsigned long k3 = dbj_sll_append(head_, "Dabra")->key;

		dbj_sll_node * node_ = dbj_sll_find(head_, k2);
		assert(0 == strcmp(node_->data, "Ka"));
		assert(false == is_dbj_sll_empty(head_));
		dbj_sll_erase_with_head(head_);
	}
#endif /*DBJ_SLL_TESTING */

#ifdef __cplusplus
} /* extern "C"  */
#endif

