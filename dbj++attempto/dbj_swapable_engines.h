#pragma once
#include "pch.h"

/*
  Swappable Engine Factory, I think a better variation to the 
  Factory Pattern
*/

namespace car_factory {

	// engine tag is public
	// thus clients can also use it
	// to order a car of particular kind
	enum class engine_tag : char { old = 'O', next = 'N' };

	namespace engine_workshop { // hidden from clients

		struct exception : public std::runtime_error {
			exception(const char * msg) 
				: std::runtime_error(msg) {}
		};

	// interface to swapable engines
		struct __declspec(novtable) IEngine
		{ 
			virtual  engine_tag tag() const noexcept = 0;
			virtual  bool start() const noexcept = 0;
			virtual ~IEngine() = default;

			IEngine() = default;
			// by its nature car engine is definitely not copyable 
			IEngine(const IEngine &) = delete;
			IEngine & operator = (const IEngine &) = delete;
			// and perhaps moveable
			IEngine(IEngine &&) = default;
			IEngine & operator = (IEngine &&) = default;
		};
		// switchable engines
		struct old final : public IEngine 
		{ 
			virtual engine_tag tag() const noexcept override  { return engine_tag::old; }
			virtual bool start() const noexcept override { return true; }
		};
		struct next final : public IEngine 
		{ 
			virtual engine_tag tag() const noexcept override { return engine_tag::next; }
			virtual bool start() const noexcept  override { return true; }
		};

		/*
		   bellow is a proof of concept
		   in reality making a new engine is long process
		   thus we should have engine production plant work on a separate thread
		   produce the engines at some predefined interval and place them on
		   the properly tagged shelves in the output warehouse
		   ordering method bellow should go to this warehouse, and wait only if there
		   are no new engines ready
		*/
		IEngine * make_new_engine( engine_tag tag_ ) {
			switch (tag_ ) {
			case engine_tag::old: return new old{};
			case engine_tag::next: return new next{};
			};
			throw exception( __FUNCSIG__ "Unknown engine tag");
		}

	} // engine_workshop

	// facade of the solution
	// this is not ABC
	class __declspec(novtable) Automobile final {

		using engine_type = typename engine_workshop::IEngine ;
		// car owns the engine
		// be carefull not to share 
		mutable engine_type * engine_{};
		// this ctor is visible to the factory method
		// we do not want this as a "converting constructor"
		// ditto we declare it as explicit
		explicit Automobile(engine_type * use_) : engine_(use_)
		{
			_ASSERTE(this->engine_);
		}
		// no default ctor
		// no nullptr engine
		Automobile() = delete;
		// the factory function declaration
		friend  Automobile assembly_line(engine_tag);
		//
		void insert_new_engine(engine_tag tag_) {
			if (this->engine_ != nullptr) delete this->engine_;
			this->engine_ = engine_workshop::make_new_engine(tag_);
		}
	public:
		// delegating to the engine
		// inheritance is evil
		bool start() const noexcept { return engine_->start(); }
		// it is easy to manage the pointer lifetime
		~Automobile() { delete engine_; }
		/*
		NOTE: 
		We must be very carefull for move or copy assignment
		since this can effectively and quietly
		change the engine "mid flight" 
		*/
		// copy
		Automobile(const Automobile& rhs_)  
		{ 
		_ASSERTE(rhs_.engine_ != nullptr );
		// two cars can not share the same engine, thus
		this->insert_new_engine(rhs_.engine_->tag());
		}
		Automobile& operator=(const Automobile&) = delete;
		// move
		Automobile(Automobile&& other_) noexcept { 
			_ASSERTE(other_.engine_ != nullptr);
			// we can not just simply
			// point to the other_ car engine
			// since it will be deleted after
			// we leave here
			this->insert_new_engine(other_.engine_->tag());
		}
		Automobile& operator=(Automobile&& other_) = delete;  
		// noexcept { std::swap(this->engine_, other_.engine_); return *this; }

		std::string description() const noexcept
		{
			return std::string{ "Automobile, with the engine tagged: '" } 
			+ char(this->engine_->tag()) + "'";
		}

	};

// factory inserts the engine on production line
// we do not return ABC so we enjoy the value semantics
inline Automobile assembly_line (engine_tag which_)
{
	// I could also use std::enable_if and std::is_move_constructible
	// in return value type to check the Automobile type movability
	static_assert( std::is_move_constructible_v<Automobile>  );

		return Automobile(
			engine_workshop::make_new_engine(which_)
		);
}

} // car_factory

namespace dbj::console {
	// void __cdecl dbj::console::out<class car_factory::Automobile>(class car_factory::Automobile)
	template<> inline void
		dbj::console::out<class car_factory::Automobile>
		(
			class car_factory::Automobile  automobile_
		)
	{
		console::PRN.printf(automobile_.description().c_str());
	}
}
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