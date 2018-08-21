#pragma once
#include "pch.h"

// https://ideone.com/mYh4Y3

namespace sean_parent {

	using namespace std;

	template<typename T>
	inline void draw(const T& x, ostream& out, size_t position) 
	{ out << string(position, ' ') << x << endl; }

	/// <summary>
	/// runtime polymorphic object
	/// </summary>
	class object_t {
	public:     
	
		template <typename T>     object_t(T x) : self_(make_shared<model<T>>(move(x))) 
	    { }          
	
	friend void draw(const object_t& x, ostream& out, size_t position) 
	{ x.self_->draw_(out, position); }        
	
	private:     
		struct concept_t { 
			virtual ~concept_t() = default;         
			virtual void draw_(ostream&, size_t) const = 0; 
		};     
		template <typename T>     
		struct model final : concept_t 
		{ 
			model(T x) : data_(move(x)) { }         
			void draw_(ostream& out, size_t position) const override { draw(data_, out, position); }                 
			T data_; 
		};
		shared_ptr<const concept_t> self_;
	}; // object_t

	/*
	the document of object_t instances
	*/
	using document_t = vector<object_t>; 

	inline void draw(const document_t& x, ostream& out, size_t position)
	{
		out << string(position, ' ') << "<document>" << endl;     
		for (const auto& e : x) draw(e, out, position + 2);     
		out << string(position, ' ') << "</document>" << endl;
	}
	
	/*
	undo/redo mechanism
	*/
	using history_t = vector<document_t>;

	inline void commit(history_t& x) 
	{ _ASSERTE(x.size()); x.push_back(x.back()); } 
	
	inline void undo(history_t& x)
	{ _ASSERTE(x.size()); x.pop_back(); }

	document_t& current(history_t& x) 
	{ _ASSERTE(x.size()); return x.back(); }

	/*testing */
	class my_class_t {     /* ... */ }; 
	void draw(const my_class_t&, ostream& out, size_t position) 
	{ out << string(position, ' ') << "my_class_t" << endl; }

	inline void driver() {

		using std::cout;
		document_t document;   

		document.emplace_back(0);    
		document.emplace_back(string("Hello!"));     
		// document.emplace_back(document);     
		document.emplace_back(my_class_t());          
		draw(document, cout, 0);

		history_t h(1);     
		current(h).emplace_back(0);     
		current(h).emplace_back(string("Hello!"));          
		draw(current(h), cout, 0);     
		cout << "--------------------------" << endl;          
		commit(h);          
		current(h)[0] = 42.5;         
		auto saving = async([=]() {
			this_thread::sleep_for(chrono::seconds(3));         
			cout << "-- 'save' --" << endl;         
			draw(document, cout, 0);     
		});     
		current(h)[1] = string("World");     
		current(h).emplace_back(current(h)[1]);     
		current(h).emplace_back(my_class_t());
	}
	
} // sean_parent

namespace dbj_samples {
#pragma region "Unknown IDE ONE Author"
	struct MyConcept
	{
		template <typename Impl>
		MyConcept(Impl imp)
			: pimpl(new impl<Impl>(imp))
		{}

		MyConcept(MyConcept const& other)
			: pimpl(other.pimpl->clone())
		{}
		MyConcept(MyConcept && other) noexcept
			: pimpl(std::move(other.pimpl))
		{}

		MyConcept& operator =(MyConcept const&) = delete;

		void display() const
		{
			pimpl->display();
		}

	private:
		struct impl_base
		{
			virtual void display() const = 0;
			virtual std::unique_ptr<impl_base> clone() const = 0;
		};
		template < typename Impl >
		struct impl : impl_base
		{
			impl(Impl i) : imp(i) {}

			void display() const
			{
				imp.display();
			}
			std::unique_ptr<impl_base> clone() const
			{
				return std::unique_ptr<impl_base>(new impl(*this));
			}

		private:
			Impl imp;
		};

		std::unique_ptr<impl_base> pimpl;
	};

	void display_thingy(MyConcept const& thingy)
	{
		thingy.display();
	}

	struct Thingy1
	{
		void display() const { std::cout << "Thingy1\n"; }
	};

	struct Thingy2
	{
		void display() const { std::cout << "Thingy2\n"; }
	};
#pragma region "Unknown IDE ONE Author"


} // namespace dbj_samples 

#if 0

DBJ_TEST_SPACE_OPEN(dbj_polymorph)

using namespace dbj_samples;

DBJ_TEST_UNIT(": dbj any wrapper ") {
	display_thingy(Thingy1());
	display_thingy(Thingy2());
}

DBJ_TEST_SPACE_CLOSE

#endif

/*

GOLANG Polymoirphism with no inheritance
------------------------------------------------------------------------
package main
import "fmt"

// DBJ: in GO interface is *somewhat* like C++ ABC
type Hobby interface {
	myStereotype() string
}

// if you need a big Foot Gun
// GO LANG is not going to stop you
// a proper way to have another interface with the
// same message name
// is to implement it in another module
type Foot_Gun interface {
myStereotype() string
}

// DBJ: just a struct
type Human struct {

}

// DBJ: Human implements the Hobby interface method
// there is no inheritance
func(h Human) myStereotype() string {
	return "I'm a Human, only an abstract concept, and I can have no hobby."
}

// DBJ: again there is no inheritance of types (aka structs)
type Man struct {
	Human //anonymous class to inherit Human behavior
	// after this Man has all the methods Human has implemented
}

// DBJ: but Man also implements "myStereotype" here
// and hides/replaces the one from Human
func(m Man) myStereotype() string {
	return "I'm a Man and I'm going fishing."
}

// DBJ: same for Woman type
type Woman struct {
	Human //anonymous class to inherit Human behavior
}

func(m Woman) myStereotype() string {
	return "I'm a Woman and I'm going shopping."
}

// DBJ:  the dog type
type Dog struct {
	//does not inherit any other type
}
// DBJ:  the dog type
// also implements myStereotype
func(m Dog) myStereotype() string {
	return "bow bow bow, I'm chasing sticks."
}

func main() {
    h : = new (Human)
	m : = new (Man)
	w : = new (Woman)
	d : = new (Dog)

	//an array of hobby instances - we don’t need to know whether human or dog
	// DBJ: all types that implement the Hobby method can be in the array
	// of Hobby's
	// DBJ: Think of this as "everyone who undesrtands the Hobby messages"
	hobbyArr : = [...]Hobby{ h, m, w, d } //array of 3 Humans and 1 dog.
	for n, _ : = range(hobbyArr) {

		fmt.Println("My hobby?  Well,", hobbyArr[n].myStereotype())  
		// appears as Hobby type, but behavior changes 
		// depending on actual instance implenetning the message
		// "mySteretype"

	}
}
*/
