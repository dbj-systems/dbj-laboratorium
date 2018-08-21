#pragma once
#include "pch.h"

#if 0
/*
I looked into this: https://github.com/LoopPerfect/valuable
and I decided it is not necessary. 
Hence dbj::AnyNode Proof Of Concept (POC) 

2017 NOV 18   dbj.org created

*/

namespace dbj_samples {
	
	template<typename T>
	inline std::string address_as_string(T * p) {
		char buf[9] = { 0x0 };
		sprintf_s(buf, 9,"%p", static_cast<void *>(p));
		return std::string(buf);
	}

	/*
	Design of this binary tree is:
	extremely simple binary node
	all operations through visitors
	*/
	struct AnyNode final {

		typedef typename std::any handle_t;

		/*
		make visitors more inteligent
		*/
		enum class where_next : int { left = -1, nowhere = 0, right = 1 };
		/*
		encapsulate visitors inteligence
		compate two any nodes and return where visitor should go next
		left or right or nowhere
		this can be used in any kind of operation supported by visitor
		*/
		using DecisionLogic = AnyNode::where_next(const AnyNode &, const AnyNode &);
		/*
		AnyNode visitor signature
		*/
		using AnyVisitor = bool(AnyNode &);
		// typedef typename std::function<AnyVisitor> AnyNodeVisitor;

		// uid is absolutely not changeable
		const			std::string		uid{};

		mutable			handle_t		data{};
		mutable			std::string		name{};
		handle_t		left{};
		handle_t		right{};

		const type_info& data_type_info () const {
			return this->data.type();
		}

		// handle to node
		static AnyNode node(const handle_t & tp_) {
			try {
				return std::any_cast<AnyNode>(tp_);
			}
			catch (std::bad_any_cast & x) {
				dbj::trace("Exception at %s(%d) [%s]", __FILE__, __LINE__, x.what());
				throw dbj::Exception(x.what());
			}
		}

		static  const bool empty(const handle_t & tp_) {
			return !(tp_).has_value();
		}

		/* is this a leaf node */
		bool leaf() const {
			if (empty(this->left)) {
				if (empty(this->right))
					return true;
			}
			return false;
		}

		/*
		default ctor leaves data, left and right as empty 
		*/
		AnyNode()
			: uid{ address_as_string(this) }
		{
			// if not user defined name is uid 
			if (this->name.empty()) this->name = uid;
		}

		/*
		the only constructor requires data paramater only
		left_ and right_ are AnyNodes handler's
		*/
		template<typename DT>
		AnyNode(
			const DT			& dta_ ,
			const std::string   & name_ = std::string{},
			const handle_t		& left_ = handle_t{},
			const handle_t		& right_ = handle_t{}
		)
			: data{ dta_ },  // store data in std::any
			  name{ name_ },
			  left{ left_ }, right{ right_ },
			  uid{ address_as_string(this) }
		{
			// if not user defined name is uid 
			if (this->name.empty()) this->name = uid;
		}

		/* 
		   append node to this node into the direction 
		   given by the leader.

		   if direction is nowhere do nothing. this is 
		   potential for silent error which is a bad thing

		   note: remember that false return from here may stop the visitor traversal
		*/
		bool add(where_next leader, const AnyNode & node_to_be_inserted_)
		{
			if (leader == where_next::nowhere) {
				dbj::trace("%s(%d) : insert nowhere?", __FILE__, __LINE__ );
				return true;
			}
			if (leader == where_next::left) {
				this->left = node_to_be_inserted_;
			}
			if (leader == where_next::right) {
				this->right = node_to_be_inserted_;
			}
			return true;
		}
	}; // AnyNode

	 /*	3 kinds of binary tree traversal	*/

	auto preorder = [] ( AnyNode & root_, /* AnyNode::AnyVisitor*/ auto f) -> void {
		if (!f(root_)) return; // process, stop if false return
		if (!AnyNode::empty(root_.left ))  preorder(AnyNode::node(root_.left), f); // left subtree
		if (!AnyNode::empty(root_.right))  preorder(AnyNode::node(root_.right), f); // right subtree
	};

	auto inorder = [] ( AnyNode & root_, auto f) -> void {
		if (!AnyNode::empty(root_.left))  inorder(AnyNode::node(root_.left), f); // left subtree
		if (!f(root_)) return; // process, stop if false return
		if (!AnyNode::empty(root_.right))  inorder(AnyNode::node(root_.right), f); // right subtree
	};

	auto postorder = [] ( AnyNode & root_, auto f) ->void {
		if (!AnyNode::empty(root_.left))  postorder(AnyNode::node(root_.left), f); // left subtree
		if (!AnyNode::empty(root_.right))  postorder(AnyNode::node(root_.right), f); // right subtree
		if (!f(root_)) return; // process, stop if false return
	};

	/*
	 With AnyVisitor and AnyNode traversals we can do anything wee need on the tree made of AnyNodes.

	 AnyVisitor can be lambda, functor, function or anything else 
	 that is callable and confirms to AnyVisitor signature

	 BIG NOTE: data on this tree is anonymous, i.e. typeless so we can not use operators on it, yet.

	*/

	namespace anyvisitors {

		inline AnyNode::where_next no_logic(const AnyNode &, const AnyNode &) {
			return AnyNode::where_next::nowhere;
		}

		/*	left or right, only if current node is leaf   */
		inline AnyNode::where_next append_left (const AnyNode & current , const AnyNode & new_ ) {
			 if (! current.leaf() ) return  AnyNode::where_next::nowhere;
			 return AnyNode::where_next::left;
		}

		inline AnyNode::where_next append_right(const AnyNode & current, const AnyNode & new_) {
			if (!current.leaf()) return  AnyNode::where_next::nowhere;
			return AnyNode::where_next::right;
		}

		template< typename insertion_logic >
		class Inserter final {

			AnyNode node_to_be_inserted_{};

		public:

			explicit Inserter( const AnyNode & new_node )
				: node_to_be_inserted_( new_node )
			{		}

			/*
			this is the visitor function to be given to 
			tree traversal methods
			*/
			bool operator ()( AnyNode & n) {
				typename AnyNode::where_next leader = insertion_logic(n, node_to_be_inserted_);
					return n.add(leader, node_to_be_inserted_);
			}
		};

	} // anyvisitors

} // dbj

#ifdef DBJ_TESTING_ONAIR
#pragma region dbj testing
namespace tree_testing {

	template<typename T> struct Identity_ { using type = T; };

	template<typename T>
		using Identity = typename Identity_<T>::type;

		template<typename T>
		struct Transformer final {
			T operator () (std::any any_) const
			{
				try {
					return std::any_cast<T>(any_);
				}
				catch (std::bad_any_cast & x) {
					dbj::trace("Exception at %s(%d) [%s]", __FILE__, __LINE__, x.what());
					// throw dbj::Exception(x.what());
				}
				return T{};
			};
		};

	DBJ_TEST_CASE(dbj::FILELINE(__FILE__, __LINE__, ": dbj::AnyNode ")) {

		Transformer<const char *> node_data;

		auto printer = [&node_data](auto & n) -> bool {
			dbj::log::print("\n(name: '", n.name, "', \tuid:", n.uid, ", \tdata: ", node_data(n.data) , ")");
			return true;
		};

	 auto root = dbj::AnyNode("root payload", "ROOT", dbj::AnyNode("left payload", "L"), dbj::AnyNode("right payload", "R")	);

	 dbj::anyvisitors::Inserter< decltype(dbj::anyvisitors::append_right) >
		 to_right( dbj::AnyNode("yeat another payload", "LEAF" ));

			dbj::preorder(root, to_right );

			 dbj::log::print("\n\nPreorder");
			 dbj::preorder(root, printer );
			 dbj::log::print("\n\nInorder");
			 dbj::inorder(root, printer);
			 dbj::log::print("\n\nPostorder");
			 dbj::postorder(root, printer);
	}
}
#pragma endregion
#endif

#endif