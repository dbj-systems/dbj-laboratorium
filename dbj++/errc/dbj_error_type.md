
# DBJ ERRC

## Error structure

(c) 2019 by dbj@dbj.org

## Architecture

For the core concepts please refer to ["DBJ ERRC" project](https://github.com/DBJDBJ/dbj_error_concept).

Here we develeop a recomendation to the common error structure design and implementation.
This is not a dictate. This is pragmatic approach with no "overarching" complex abstractions.

Decision is not to use `std::error_code` concept. It is simply unfeasibly complex.
And it is to be deprecated. Together with standard C++ exceptions.

*“.. Zero overhead exceptions are not expected for C++ 23 currently, but rather as an experimental feature. If C adopts them for C23, then all the major compilers will have them, but they still wouldn’t enter until C++ 26 as for obvious reasons C++ 23 can’t incorporate C23..”*

[Nial Douglass](https://www.linkedin.com/in/nialldouglas/) (aka '14ned' )
  
## Design

 ```
 error structure
+---------------------+
|  ID                 |
+---------------------+
|  Message            |
+---------------------+
|  Location           |
+---------------------+
```
Location information is file name and line number. In case of distributed systems one can imagine 
system node name being added or used.

ID might be value from `std::errc` enumeration. To avoid a confusion, is it or is it not,
best define a set of error structure in a separate name space that will match std:errc values 
and provide appropriate messages.

## Implementation

Why not using 'indentity by struct' concept?
```cpp
struct ID {
   using value_type = uint8_t ;
   uint8_t  val;
    
friend bool operator == ( ID left, ID right ){ 
   return left.val ==  right.val ;
};

friend bool operator < ( ID left, ID right ){ 
   return left.val <  right.val ;
}

};

struct message {
// we do not use string as char buffer
   using value_type = std::unique_ptr<char[]> ;
    value_type val;
};

using line_number_type = ID ;
using file_name_type = message;

struct location_type {
    line_number_type line ;
    file_name_type   file ;
};

struct error {
   ID id ;
   message msg;
   location_type location;
};

// utility
 template <typename ... A>
 friend auto buffer ( const char * fmt, A ... args) ;

// make error instance
error err{ 
{1234}, 
{ buffer("error 1234, %s(%d)", __FILE__, __LINE__)}
};

```

`error` type can not be completely finalized at compile time. 