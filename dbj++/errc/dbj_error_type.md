
# DBJ ERRC

## Error structure

(c) 2019 by dbj@dbj.org

## Architecture

For the core concepts please refer to ["DBJ ERRC" project](https://github.com/DBJDBJ/dbj_error_concept).

Here we develeop a recomendation to the common error structure design and implementation.
This is not a dictate. This is pragmatic approach with no "overarching" complex abstractions.

My decision is not to use `std::error_code` concept. Why? It is simply unfeasibly complex.
And it is to be deprecated. Together with standard C++ exceptions.

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
+---------------------+
|  ID                 |
+---------------------+
|  Message            |
+---------------------+
```
Using idmessage type, we can compose a simple and usefull error structure.

 ```
 error structure
+-----------------------+
|  idmessage  error     |
+-----------------------+
|  idmessage  location  |
+-----------------------+
```
#### Is location optional?
Most of the time location information is file name and line number. Last 40+ years in computing have proven that point. In case of distributed systems one can imagine 
node id (or service id) and name being used.

In my opinion the location "half" of the error structure is usefull but optional. On the smaller scale it might be not necessary on larger scale it might be.

#### Location optionality design

Simply leave it to implementation. Developing a design that will impose optionality mechanism implementation will complicate the error structure, C implementation.

C++ implementaiton could use readily available std::optional, but. It would be rather non trivial to implement that mechanism in C. That would also contradict [P1](#principle_1).

#### What is ID?

ID might be POSIX value (from `std::errc` enumeration). Or it might be project/company wide set of values.

#### How do we manage ID's

To avoid a confusion, is it POSIX or is it not, best define a set of error structure in a separate ( company/project owned )name space that will ***logicaly*** extend `std::errc` values and provide appropriate messages.

#### UNICODE

This code will be using `char`. That is ultimately compiler and platform agnostic.

`char` based strings are UNICODE if they are UTF8 encoded.

If and when necessary we will implement tranformations to/from `wchar_t`  based code.

## Implementation

In the first instance this will be implemented as C++ code. 

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
} ;
```
As an reminder, this is how to easily define `vector<char>` buffer of certain size and pre-filled with '\0' aka string-terminator.
```cpp
idmessage::message_type  message( BUFSIZ, char(0));
```

#### idmessage is POD
```cpp
static_assert( std::is_standard_layout<idmessage>::value,
   "idmessage is not POD aka 'not standard layout'" );
```

The above must not assert. We shall leave it in a code to 
check future developments.

We will implent idmessage as a POD and function helpers to manage instance of it.

#### Creating instances of idmessage type
```cpp
    // value semnatics of function parameter passing
    idmessage idmessage_make
    ( idmessage::id_type , idmessage::message_type ) ;
```
Same as standard C++ we are coding in value semantics fashion.

Since we have came to the conslusion vactor<char> is the optimal solution for the `message_type` implementations, we will require small set of helpers to create from types that users might use to create idmessage type instances.
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

For e.g pointers `nullptr` means "it is not there". If we just deal with simple values , not pointers or references, and POD's, the question is how do we implement the "not there" concept.  
```cpp
// we declare id 0 as "not an id"
// we create and keep it as an unique 
// compile time value
inline constexpr idmessage::id_type not_id{ 0 } ;
```
By simple act of declaration we declare id 0 as "not an id". Ande we create and keep that "not an id" value as above.

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

// also POD
static_assert( std::is_standard_layout<error_type>::value, 
   " error_type not standard layout" );
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






