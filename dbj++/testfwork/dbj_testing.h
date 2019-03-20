#pragma once
/*
(c) dbj.org, see the license at file bottom

Wellcome. An C++17 micro standalone testing framework
No exceptions are thrown out. They are reported.

TU == Test Unit
TUNODE == TU node in the set where we keep them
TUSET  == set of TUNODE's

API

DBJ_TEST_SPACE_OPEN ( legal_cpp_name )
// above is optional

DBJ_TEST_UNIT( legal_cpp_name ) {

   // display expression, result type name and result value
   DBJ_TEST_ATOM( 4 * 256 ) ;

   // display expression and result value
   DBJ_ATOM_TEST( 'A' + 'B' ) ;
}

DBJ_TEST_SPACE_CLOSE
// has to pair the  DBJ_TEST_SPACE_OPEN
*/

#include "../core/dbj_synchro.h"
#include "../core/dbj_format.h"
#include "../util/dbj_string_util.h"

namespace dbj {
	namespace testing {

		using namespace std::string_view_literals;

		constexpr inline auto TITLE = "dbj++ Testing Framework [" __DATE__ "]"sv ;
		constexpr inline auto ALLDN = "dbj++ Testing Framework -- ALL TESTS DONE"sv ;
		// hard coded constants -- not good
		// what should we do :)
		constexpr inline auto DBJ_MAX_ALLOWED_TEST_UNITS = 1000U;

		// testunit is void function (void)
		using testunittype = void(*)();
		inline void __stdcall null_unit() {}

		namespace internal {

			using buffer = typename ::dbj::buf::yanb;

			struct TUNODE final 
			{
				size_t ID;
				testunittype TU;
				buffer description;

				TUNODE(
					size_t id_, 
					testunittype test_unit_, 
					buffer & desc_
				) noexcept
					: ID(id_)
					, TU(test_unit_)
					, description( desc_ )
				{
				}

				constexpr bool operator < (const TUNODE & other_ ) const noexcept
				{
					return ID < other_.ID;
				}
			}; // TUNODE

			typedef  std::set< TUNODE > TUSET;

			inline TUSET & tuset_instance()
			{
				auto initor_ = [&]() -> TUSET {
					// this is called only once
					// do some more complex initialization
					// here, if need be
					return {};
				}; 
				static TUSET single_instance{ initor_() };
				return single_instance;
			};

#if 0
			// in da set value and key are the same type
			// all is in da "node"
			inline  bool operator == (
				const TUSET::value_type & node_a_,
				const TUSET::key_type & node_b_) noexcept
			{
				return std::addressof(node_a_.TU) == std::addressof(node_b_.TU);
			}

			inline  TUSET::iterator find(const TUSET::key_type & tunode_) {
				return tuset_instance().find(tunode_);
			}

	       inline  bool found(const TUSET::key_type & tunode_) {
				return (
					tuset_instance().end() != find(tunode_)
					);
			}
#endif
		   // return next_id and string presentation of it
		   // in an struct { id,sid }
		   auto next_test_id () noexcept 
		   {
			   struct retval final {
				   size_t id;
				   buffer sid;
			   };

			   static size_t tid{ 0 };
			   buffer id_str =
				   dbj::fmt::to_buff("[TID:%03d]", tid++);
			   return retval{ tid, id_str };
		   }

				/// the actiual test unit function registration 
				/// happens here
			   inline  auto append
			   (testunittype tunit_, buffer const & description_)
				   noexcept -> testunittype
			   {
				   auto next_ = next_test_id();

				   buffer  full_desc =
					   dbj::fmt::to_buff("%s%s",
						   next_.sid.data(),
						   description_.data()
					   );
	   
				   /* vs insert(), std::set emplace() returns a rezult */
				   auto rez = tuset_instance().emplace( 
						   TUNODE( next_.id, tunit_ , full_desc )
					   );

				   _ASSERTE(full_desc);

				   // NOTE: rez.second is false if no insertion ocured
				   if (rez.second == false) {
					   ::dbj::core::trace("\nNot inserted %s, because found already", full_desc.data());
				   }
				   else {
					   ::dbj::core::trace("\nInserted test unit: %s", full_desc.data());
				   }
				   return tunit_; 
			   }

				inline  void unit_execute(testunittype tunit_) {
					tunit_();
				}

			struct adder final {
				inline auto operator ()(
					std::string_view msg_, 
					testunittype tunit_ 
					) const noexcept
				{
					// mt safe in any build
					dbj::sync::lock_unlock auto_lock;
					return internal::append(tunit_, msg_.data());
				}
#ifdef _DEBUG
				 adder ( )  noexcept {	
				 ::dbj::core::trace("\n\n%s\n%s(%d)\n", __func__, __FILE__, __LINE__);
				}
#endif // _DEBUG

			}; //  adder

			// https://dbj.org/c-play-it-only-once-sam/
			inline  const adder & 
				adder_instance() 
			{
			// mt safe in any build
			static adder singleton_{ };
			  return singleton_ ;
			}

		   } // internal  
	} // testing
} // dbj

#ifdef DBJ_TEST_UNIT
#error "DBJ_TEST_UNIT Already defined?"
#else

#define DBJ_TEST_SPACE_OPEN(x) namespace _dbj_testing_namespace_ {  
#define DBJ_TEST_SPACE_CLOSE }
/*
remember: anonymous namespace variableas are static by default
that is they are "internal linkage"
thus bellow we name the namespace, to avoid that 'phenomenon'
*/
#define DBJ_TEST_CASE_IMPL(description, name ) \
void name(); \
namespace __dbj_register__namespace_  { \
  inline auto DBJ_CONCAT( dbj_test_unit_function_ , __COUNTER__ ) \
           = ::dbj::testing::internal::adder_instance()( description, name ); \
} \
inline void name() 

#define DBJ_TEST_CASE( description, x ) \
DBJ_TEST_CASE_IMPL ( description , DBJ_CONCAT( __dbj_test_unit__, x ) )

#define DBJ_TEST_UNIT(x) DBJ_TEST_CASE( ::dbj::core::fileline(__FILE__, __LINE__).data() , x )

#endif

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"