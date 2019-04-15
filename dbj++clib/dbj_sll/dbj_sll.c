
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
*/


#include "dbj_sll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4505 )
#endif

#ifndef DBJ_UNUSED
#define DBJ_UNUSED(x) (void) sizeof((x))
#endif

/* apparently only *static extern* variables can use thread local storage */
#ifdef _MSC_VER
#define dbj_thread_local __declspec(thread) static
#define dbj_malloc(type, count) (type *)malloc( count * sizeof(type))
#else
#define dbj_thread_local static __thread 
#define dbj_malloc(type, count) malloc( count * sizeof(type))
#endif

#if 0
/*already in this lib*/
/*
strdup and strndup are defined in POSIX compliant systems as :
char *strdup(const char *str);
char *strndup(const char *str, size_t len);
*/
char * dbj_strdup(const char *s) {
	char *d = malloc(strlen(s) + 1);   // Space for length plus nul
	if (d == NULL) {
		errno = ENOMEM;
		return NULL;
	}         // No memory
	strcpy(d, s);                        // Copy the characters
	return d;                            // Return the new string
}
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





/********************************************************/
dbj_sll_node * dbj_sll_node_new()
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
dbj_sll_node * dbj_sll_tls_head() {
	dbj_thread_local dbj_sll_node
		dbj_sll_tls_head_ = { DBJ_SLL_HEAD_KEY, 0 , 0 };
	return &dbj_sll_tls_head_;
}

dbj_sll_node * dbj_sll_make_head() {
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
bool is_dbj_sll_head(dbj_sll_node * head_) {
	assert(head_);
	if (head_) {
		return ((head_->next == 0) && (head_->key == DBJ_SLL_HEAD_KEY));
	}
	return false;
}
bool is_dbj_sll_tls_empty() 
{ return (dbj_sll_tls_head()->next == 0); }

bool is_dbj_sll_empty(dbj_sll_node * head_) 
{ return head_->next == 0; }

dbj_sll_node * dbj_sll_next(dbj_sll_node * current_)
{
	assert(current_);
	return current_->next;
}
char const * dbj_sll_data(dbj_sll_node * current_)
{
	assert(current_);
	return current_->data;
}
unsigned long dbj_sll_key(dbj_sll_node * current_)
{
	assert(current_);
	return current_->key;
}
/********************************************************/
dbj_sll_node * dbj_sll_tail(dbj_sll_node * head_)
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
void dbj_sll_erase(dbj_sll_node * head_)
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
void dbj_sll_erase_with_head(dbj_sll_node * head_)
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
dbj_sll_node * dbj_sll_find(dbj_sll_node * head_, unsigned long k_)
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
dbj_sll_node * dbj_sll_foreach
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
size_t dbj_sll_count(dbj_sll_node * head_)
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
dbj_sll_node *
dbj_sll_append(dbj_sll_node * head_, const char * str_)
{
	dbj_sll_node * new_node = dbj_sll_node_new();
	new_node->data = _strdup(str_);
	new_node->key = dbj_hash((unsigned char *)str_);

	dbj_sll_node * tail_node = dbj_sll_tail(head_);
	tail_node->next = new_node;
	return new_node;
}

/********************************************************/
/* sll visitors                                         */
/********************************************************/
bool dbj_sll_node_dump_visitor(dbj_sll_node * node_)
{

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
bool dbj_sll_remove_tail_visitor(dbj_sll_node * node_)
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
dbj_sll_node * dbj_sll_remove_tail(dbj_sll_node * head_)
{
	if (is_dbj_sll_empty(head_)) return 0;
	return dbj_sll_foreach(head_, dbj_sll_remove_tail_visitor);
}

/********************************************************/

#ifdef DBJ_SLL_TESTING
void test_dbj_sll(const char * what_to_append, size_t how_many_times, bool verbose)
{
	dbj_sll_node * head_ = dbj_sll_make_head();

	while (1 < how_many_times--) dbj_sll_append(head_, what_to_append);

	if (verbose) {
		printf("\nDBJ SLL dump");
		dbj_sll_foreach(head_, dbj_sll_node_dump_visitor);
		printf("\n");
	}

	assert(0 == strcmp(dbj_sll_remove_tail(head_)->data, what_to_append));
	dbj_sll_erase(head_);
	if (verbose) {
		printf("\nHead after SLL erasure");
		dbj_sll_node_dump_visitor(head_);
	}
	assert(true == is_dbj_sll_empty(head_));

	DBJ_UNUSED unsigned long k1 = dbj_sll_append(head_, "Abra")->key;
	DBJ_UNUSED unsigned long k2 = dbj_sll_append(head_, "Ka")->key;
	DBJ_UNUSED unsigned long k3 = dbj_sll_append(head_, "Dabra")->key;

	(void)(sizeof(k1)); (void)(sizeof(k3)); // combat no warning hassle

	dbj_sll_node * node_ = dbj_sll_find(head_, k2);
	assert(0 == strcmp(node_->data, "Ka"));
	assert(false == is_dbj_sll_empty(head_));
	dbj_sll_erase_with_head(head_);
}
#endif /*DBJ_SLL_TESTING */

#ifdef _MSC_VER
#pragma warning( pop )
#endif
