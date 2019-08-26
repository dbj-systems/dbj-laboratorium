#pragma once

#include "pch.h"

namespace dbj::errc  {

	struct error_type final {
		idmessage error;
		idmessage location;
	};

	constexpr bool is_error_valid(error_type const& err_)
	{
		return  idmessage::valid(err_.error);
	}

	/*
	struct simple_error_type 
	{
		const idmessage id_and_message;
		optional<idmessage> line_and_file;

		// make with no location
		static simple_error_type
			make(
				idmessage::id_type id_,
				const char* msg_
			) {
			DBJ_ASSERT(msg_);
			return simple_error_type{ { id_, msg_}, nullopt };
		}

		// with optional location
		static simple_error_type
			make(
				idmessage::id_type id_,
				const char* msg_,
				idmessage::id_type loc_id_,
				const char* loc_msg_
			) {
			DBJ_ASSERT(msg_ && loc_msg_);
			return simple_error_type{ { id_, msg_}, idmessage(loc_id_, loc_msg_) };
		}

		// add location to error instance
		static simple_error_type
			locate(
				simple_error_type err_,
				idmessage::id_type loc_id_,
				const char* loc_msg_
			) {
			DBJ_ASSERT( loc_msg_);
			return make(
				err_.id_and_message.id(),
				err_.id_and_message.data(),
				loc_id_,
				loc_msg_);
		}

		// return all the data in a flat structure 
		// for easy consuming in a structured binding declaration
		friend decltype(auto) flat(simple_error_type err_);
	}; // simple_error_type

	// return all the data in a flat structure 
// for easy consuming in a structured binding declaration
	 decltype(auto)
		flat(simple_error_type err_)
	{
		if (err_.line_and_file) {
			return make_tuple(
				err_.id_and_message.id(),
				err_.id_and_message.message(),
				(*err_.line_and_file).id(),
				(*err_.line_and_file).message()
			);
		}
		// can not return two different tuples
		// from the same function .. ditto
		return make_tuple(
			err_.id_and_message.id(),
			err_.id_and_message.message(),
			idmessage::id_type{},
			idmessage::message_type{}
		);
	}
	*/
} // eof dbj::errc ns