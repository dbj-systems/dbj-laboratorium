
# DBJ ERRC

## Error structure

(c) 2019 by dbj@dbj.org

## Architecture

For the core concepts please refer to ["DBJ ERRC" project](https://github.com/DBJDBJ/dbj_error_concept).

Here we develeop a recomendation to the common error structure design and implementation.
This is not a dictate. This is pragmatic approach with no "overarching" complex abstractions.

My decision is not to use `std::error_code` concept. Why? It is simply unfeasibly complex.
And it is officialy to be deprecated. Together with standard C++ exceptions.

But the key issue is timing.

* ... Zero overhead exceptions are not expected for C++ 23 currently, but rather as an experimental feature. If C adopts them for C23, then all the major compilers will have them, but they still wouldn't enter until C++ 26 as for obvious reasons C++ 23 can't incorporate C23... *

[Nial Douglass](https://www.linkedin.com/in/nialldouglas/) (aka '14ned' )

#### Principles

<span id="principle_1"></span>
**P1**. Design and implement so that harmonisation between C and C++ runtime and ABI is possible in the future.

<span id="principle_2"></span>
**P2**. Without implementing compiler intrinsics P1 is not possible

<span id="principle_3"></span>
**P3**. Keep the design not "too far away" to allow for future tranformations between C++ and C code. This also adheres to ***"Changeability"*** as universal principle of a good architecture.

### Statement of Work

Today 2019,  august, me and no other can afford to wait for C++23. We need to ship standard C++ code soon. Here is the proposed solution.

## Design

Stay on the right side of simple. Use as the base type, Id + Message data type. For the lack of better word we will call it "idmessage".

 ```
 idmessage structure
+---------------------+                   +---------------------+
|  ID                 |                   | STD                 |
+---------------------+ ---- depends ---> | ISO C++             |
|  Message            |                   | Standard Library    |
+---------------------+                   +---------------------+
```
Same as eveything else in this design and implementation, idmessage structure
depends on the iso C++, stdnard library. This is implied dependancy and we shall no repeaat it on every diagram.

#### idmessage is not literal

Events provoking error states in any IT system are inherently run-time in nature. Thus the `idmessage` struture can not be requested to be created at compile-time, that is before system even starts to function.

### error_type_

Using idmessage type, we can compose a simple and usefull `error_type` structure.

 ```
 error_type structure
+-----------------------+
|  idmessage  error     |
+-----------------------+
|  idmessage  location  |
+-----------------------+
```

Same as `idmessage` `error_type` instances can be made only at **run-time**.

#### Is error optional?

`error` is not optional. The purpose of this structure is conveying the information on the error event happened somewhere 
in the system/code before this structure was intercepted and used.

Allowing the error be optional, will bring yet another unnecessary complexity into design and implementation.

#### Is location optional?
Most of the time location information is file name and line number. Last 40+ years in computing have proven that point. In case of distributed systems one can imagine 
node id (or service id) and name being used.

In my opinion the location "half" of the error structure is usefull but optional. On the smaller scale it might be not necessary on larger scale it might be.

##### Location optionality

C++ implementaiton could use readily available std::optional, but. It would be rather non trivial to implement that mechanism in C. That would also contradict [P1](#principle_1).

Developing a design that will impose optionality mechanism implementation will complicate the error structure, C implementation.Simply leave it to implementation. 
#### What is ID?
In here ID, logic of id is the same as [**POSIX error** id](https://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html#tag_15_03). In case of C++, it might be POSIX value (from `std::errc` enumeration). Or it might be project/company wide set of values.

#### How do we manage ID's

To avoid a confusion, is it POSIX or is it not error, best define a set of error structure in a separate ( company/project owned )name space that will ***logicaly*** extend `std::errc` values and provide appropriate messages.

### `error_type` instances

Instances of this type are used to signal run-time errors. Each has to have as a minimum "error ID" and a message.

Each company/application/team/project can and will have the repository of predefined pairs of error numbers and messages.

That allows for separate compile-time id and message solution amd is decoupled from this design. For example.
```cpp
// compile time storage of 
// error numbers and associated messages
// made for company_xyz_
namespace company_xyz {_
using namespace std;

  struct idm /*id message literal type*/ final 
      { unsigned id; string_view message; } ;
 
      constexpr auto base_error_id = 1000U;

 constexpr array predefined_errors = {
    idm{base_error_id + 1, "Error 1"},
    idm{base_error_id + 2, "Error 2"}
 };
}
```
 Above is in lieu of run time `error_type` instances creations, that will be necessary on many ocasions.
 Above will be simply used, but not always, as a reporistory when creating instances of `error_type`.

In distributed systems, it is common requirement, to have to receive and pass further,
 foreign/external error codes and mesages, not predefined by the host system.

#### UNICODE

This solution will be using `char` type. That is making the solution, ultimately compiler and platform agnostic.

On Windows platform,`char` based strings are UNICODE if they are UTF8 encoded. 

If and when necessary we will implement tranformations to/from `wchar_t`  based code. Or to/from ANSI/UNICODE.

Each and every compiler intrinsic infrastructure is using 'char' type. Example: __FILE__, universal predefined macro is `const char *`.

## Implementation

In the first instance this solution will be implemented as C++ code. If and when future compilers allow, C solution will be added, and C++ part extended.

The key issue is to decide do we implement this as compile time or run-time code.

C has no compile-time concept. Also to satisfy Architecture [P3](#principle_3), run-time it is going to be. 

C++ code used will confirm to [the current ISO C++ standard](https://isocpp.org/std/status).

Note: wherever and whenever possible the std lib using directive will be used

```cpp
namespace example {
   using namespace std ;
}
```

The top level namespace is `dbj`. This solution will be in the `dbj::errc` namespace. "errc" stands for "error concept".

```cpp
namespace dbj::errc {
   using namespace std ;
}
```

Unless otherwise stated,all code in this document is in the above namespace.

[C11](http://www.iso-9899.info/wiki/The_Standard#C11) is the C standard to which the code will be aiming to confirm, whenever possible.

#### idmessage
```cpp
struct idmessage final 
{
   // used from stdint.h
   // not optional type
   // https://en.cppreference.com/w/c/types/integer
   using id_type = uint_fast64_t ;

   // the only runtime buffer type that 
   // is (marginally) faster is unique<char[]>
   // vector<char> is infinitely easier to 
   // use and leads to much simpler code
   // note: it is important to use helper 
   // routines that will assure zero terminated
   // strings using this structure
   using message_type = vector< char > ;

   id_type        id ;
   message_type   message ;

	idmessage() = delete;
	// we declare id 0 as "not an id"
	// we create and keep it as an unique 
	// compile time value
	inline static constexpr id_type not_id{ 0 };
		
	static constexpr bool valid ( idmessage const & idm_  ) {  return idm_.id > not_id; }

} ;
```
This structure is not a type that can be used to make literals, because `std::vector<char>` is used. 
Also some or all compilers generate "non trivial destructor" for this kind of structures, which make them not suitable to create literals.

As an reminder, this is how to easily define `vector<char>` buffer of certain size and pre-filled with '\0' aka string-terminator.
```cpp
// example
idmessage::message_type  message( BUFSIZ, char(0));
```
Modern std implementations are performing this operation very fast, rendering fully sized and clean run time buffer of char's.

#### idmessage is not POD

C++20 is retiring the `tdd::is_pod<T>` trait.

We will implement idmessage function helpers to manage instance of it.

#### Creating instances of idmessage type
```cpp
    // value semnatics function parameter passing
    idmessage idmessage_make
    ( idmessage::id_type , idmessage::message_type ) ;
```
Same as standard C++ we are coding in value semantics fashion.

Since we have came to the conslusion `vactor<char>` is the optimal solution for the `message_type` implementations, we will require small set of helpers to create from types that users might use to create idmessage type instances.
```cpp
    // zero terminated strings
    idmessage idmessage_make
    ( idmessage::id_type , const char * ) ;
    // string views
    idmessage idmessage_make
    ( idmessage::id_type , string_view ) ;
    // strings
    idmessage idmessage_make
    ( idmessage::id_type , string ) ;
    // std arrays
    template<size_t N>
    idmessage idmessage_make
    ( idmessage::id_type , array<char,N> ) ;
    // unique ptr char array
    // has no copy semantics
    idmessage idmessage_make
    ( idmessage::id_type , unique_ptr<char[]> const &) ;
    // shared ptr char array
    idmessage idmessage_make
    ( idmessage::id_type , shared_ptr<char[]> ) ;
```
#### What is the valid idmessage instance?

For e.g pointers `nullptr`, conceptually also  means "it is not there". If we just deal with simple values , not pointers or references, and POD's, the question is how do we implement the "not there" concept.  
Instead in the `idmessage` type we have declared and defined the "not an id" value.
```cpp
// we declare id 0 as "not an id"
// we create and keep it as an unique 
// compile time value
inline static constexpr id_type not_id{ 0 };
		
static constexpr bool valid ( idmessage const & idm_  ) {  return idm_.id > not_id; }
```
By simple act of declaration we declare id value zero (0), as "not an id". Ande we create and keep that "not an id" value as above.

Why 0? Because this is the default value id get's when idmessage is default constructed.

Thus we are saying, default constructed `idmessage` is in an invalid state because it's id is `0`, which is the value of the system wide `not_id` (compile time) constant.
```cpp
   // implicit call to compiler generated default ctor
   // set's the id member to 0 aka 'not_id'
   idmessage invalid_{} ;
   // at this point, 
   // invalid_.id is 0
   // invalide_.message.size() is also 0
```
Thus we can produce an simple helper 
```cpp
 constexpr bool is_idmessage_valid ( idmessage const & idm_ ) 
 {
    return idm_.id > not_id && idm_.message.size() > 0 ;
 }
```
## error type structure

```cpp
struct error_type final {
   idmessage      error; 
   idmessage      location; 
};

```
#### optionality/validity mechanims

**Both error  and location, data members of error_type, can be non-initialized. And that is not legal.**

`error` field has to always exist.

Now we use idmessage type to create simple mechanism for the error_type validity check.
```cpp
 constexpr bool is_error_valid ( error_type const & err_ ) 
 {
    return is_idmessage_valid( err_.error ) ;
 }
```
### Creating instances of error_types






