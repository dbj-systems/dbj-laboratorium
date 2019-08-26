#pragma once

namespace dbj::errc{

	struct error_type final 
	{
		using id_type = typename idmessage::id_type;
		using message_type = typename idmessage::message_type ;

	   idmessage      error;
	   idmessage      location;

	   static constexpr bool valid( error_type const & err_ ) {
		   return  idmessage::valid(err_.error);
	   }
    };

} // dbj::errs
