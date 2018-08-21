#pragma once
#include "pch.h"

namespace dbj_samples {
	namespace tree {
	/* 
	Binary Tree made more useful the easy and quick way thanks to modern C++

	(c) 2017 by dbj@dbj.org

	DT is data type stored		
	It has to be deletable, swappable and capable of display for output

	tree imp operates with naked pointers
	that makes it more obvious what is goin on
	also the default constructor is not allowed
	that makes no sense and will introduce an 'empty BinaryNode' singularity
	*/
	template< typename DT >
		class BinaryNode {
		public:
			typedef BinaryNode *	    NodePointer;
			typedef DT 					data_type;
			typedef BinaryNode<DT>		BinaryNodeType;
			typedef std::unique_ptr<BinaryNode> Guardian;
			/*
			each BinaryNode processing function has to conform to this footprint
			tree traversal stops on false returned
			*/
			typedef bool(*BinaryNodeVisitorType) (NodePointer);
		private:
			data_type					data_;
			NodePointer					left_;
			NodePointer					right_;

			/* default ctor */
			BinaryNode() : data_{}, left_(nullptr), right_(nullptr){	}
		public:
			/*
			Factory method.
			Following the http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2027.html#Perfect_Forwarding

			TODO:users should make_node_pointer only once when "starting" a tree,
			keeping the root node in std::unique_ptr
			*/
			static Guardian make_node(data_type && root_data) {
				return std::make_unique<BinaryNode>(std::forward<data_type>(root_data));
			}

			static data_type make_data( const data_type & data_ ) {
				return data_type{ data_ };
			}
			
	
			/*
			reminder: delete will call BinaryNode destructor on 
			left and right 
			thus this destructor will delete the whole subtree 
			under this instance
			*/
			~BinaryNode() {
				assert(this);
				if (left_)
					delete left_; left_ = nullptr;
				if (right_)
					delete right_; right_ = nullptr;
			}

			/* construct with data */
			BinaryNode( data_type && ndata_ ) 
				: left_  (),
				  right_ ()
			{
				data_ = data_type{ ndata_ };
			}

			/* copying 
			   this node must be empty
			*/
			BinaryNode(const BinaryNode & other_)
			{
				DBJ_ASSERT( !left_  );
				DBJ_ASSERT( !right_ );

				this->data_ = other_.data_;
				this->left_ = other_.left_;
				this->right_ = other_.right_;
			}
			/*  
			 assignment
			 this node might be empty or not
			 */
			const BinaryNode &  operator = (const BinaryNode & other_) {
				if (this == &other_) return;
				if (left_)	delete left_;
				if (right_) delete right_;
					this->data_  = other_.data_  ;
					this->left_  = other_.left_;
					this->right_ = other_.right_;
				return *this;
			}

			/* Moving */
			BinaryNode(BinaryNode && other_) 
			{
				swap(*this, other_);
			}

			BinaryNode & operator = (BinaryNode && other_)
			{
				if (this != &other_ )
					swap(*this, other_);
				return *this;
			}

			/*
			the std::swap, requires the type T to be MoveConstructible and MoveAssignable
			note: always use this idiom to overload std::swap for a class/struct
			*/
			friend void swap(BinaryNode & left_, BinaryNode & other_) 
			{
				if (left_ == other_) return;
				std::swap(left_.left_ ,		other_.left_ );
				std::swap(left_.right_ ,	other_.right_);
				std::swap(left_.data_,		other_.data_);
			}

			const bool operator == (const BinaryNode & other_) const {
				if ( this->data_ == other_.data_ )
					if (this->left_ == other_.left_)
						if (this->right_ == other_.right_)
							return true;
				return false;
			}

			const bool operator != (const BinaryNode & other_) const {
				return !(*this == other_);
			}

			const data_type		data()  const noexcept {  return this->data_;  }
			const NodePointer	left () const noexcept {  return this->left_;  }
			const NodePointer	right() const noexcept {  return this->right_; }

			/*	Variadic insert	*/
			template < typename ... Args >
			const void insert(Args&&... args)
			{
				const size_t size = sizeof...(args);
				data_type arglist[size] = { args... };
				for (auto & data_ : arglist) {
					(void)inserter(this, data_ );
				}
			}

			/* Streaming insert */
			BinaryNode & operator << (const data_type & ndata) {
				inserter(this,ndata);
				return const_cast<BinaryNode &>(*this);
			}

			/* methods bellow might be public but that will confuse users */
		private:
			/*
			BinaryNode insertion method creates balanced tree by default
			it uses operator '<=' on the data type used and stored
			it also uses BinaryNode<T>::BinaryNode_creator() to make_node_pointer the BinaryNode
			*/
			NodePointer inserter(NodePointer root, data_type ndata) 
			{
				if (!root) {
					return root = new BinaryNode( std::forward<data_type>(ndata));
				}
					// assert(root->data_);
				/*
				'store left if smaller or equal, store right if not'
				insertion criteria is defined by using '<=' operator on data
				this is e.g. 0, 1, 2, 3, 4 will repeatedly give the resulting
				tree that will	have them all in just "mental picture right" Nodes
				*/
				if ( ndata <= root->data_ )
					root->left_ = inserter(root->left_, ndata) ;
				else
					root->right_ = 	inserter(root->right_, ndata );

				return root ;
			}
		}; // BinaryNode

		
		//visit BinaryNodes in preorder
		template<typename PT, typename F>
		 inline
		void preorder(PT root_, F f) {
			if (root_ == nullptr) return;
			if (!f(root_)) return; // process
			preorder(root_->left(), f);     // left subtree
			preorder(root_->right(), f);    // right subtree
		}
		//visit BinaryNodes in inorder
		template<typename PT, typename F>
		 inline
			void inorder(PT root, F f) {
			if (root == nullptr) return;
			inorder(root->left(), f);       // left subtree
			if (!f(root)) return ; // process
			inorder(root->right(), f);      // right subtree
		}

		//visit BinaryNodes in postorder
		template<typename PT, typename F>
		 inline
			void postorder(PT root, F f) {
			if (root == nullptr) return;
			postorder(root->left(),  f);   // left subtree
			postorder(root->right(), f);   // right subtree
			if (!f(root)) return ; // process
		}

	} // tree
} // dbj

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