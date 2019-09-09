#pragma once

namespace dbj::errc{

	/*
	meaning of the state type depends on the wider scope 
	*/
	struct state_type final 
	{
		using id_type = typename idmessage::id_type;
		using message_type = typename idmessage::message_type ;

	   idmessage      state;
	   idmessage      location;

	   /*
	   by valid we mean has "state" field in non initial state
	   */
	   constexpr bool valid( ) {
		   return  state.valid() ;
	   }
    };

} // dbj::errs
