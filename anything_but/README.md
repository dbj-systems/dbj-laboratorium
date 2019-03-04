# nothing_but

**Value Handle to Avoid Implicit Conversions in standard C++**

## Motivation

Probably you indulge into recreational C++ and the following is 
thus, just a curiosity for you:

```cpp
    // these are MSVC typedef's
#ifndef _MSC_VER
    typedef unsigned char uint8_t;
    typedef signed char   int8_t;
#endif
 // CLANG/GNUC/G++/MSVC/UCRT default behaviour 
 // is : no warnings whatsoever here
 // assing signed to unsigned char
    uint8_t uc = int8_t('s');
 // assigned unsigned to signed char
    int8_t sc =  uc;
```

Or you take the above so seriously you actually do not use C++ on mission critical project?
Or inside some medical equipment perhaps, your company delivers.. 

You might take the C++ [implicit conversions](https://en.cppreference.com/w/cpp/language/implicit_conversion) so seriously that "even" the following is a very serious 
matter for you.

```cpp
   // implicit conversion of int to char
   char C = 64 ;
   // implicit conversion of double to float
   float F = 3.7;
```
Not because you happen to be "unreasonable", but becaue yuo need
 to deliver code where implicit conversion are simply not allowed. 
Just like for example exceptions, in many real-time projects are not allowed. They simply do not exist over there. Simply switched off.

And very likely, you have turned to the "[official sources](https://docs.microsoft.com/en-us/cpp/cpp/type-conversions-and-type-safety-modern-cpp?view=vs-2017)" just to come back dissapointed.

One options is to plan for usual extended test/debug/test/debug, cycles ad infinitum, of course.

### The suggestion

Before you discard (with the heavy heart) C++ for mission 
critical projects, completely, we might suggest you look into this ridiculously tiny single header?
Here is some code to tickle your fancy.

```cpp
// let's assume your code must not accidentaly mix
// signed and unsigned chars
    using just_signed   = dbj::util::nothing_but<signed char>;
    using just_unsigned = dbj::util::nothing_but<unsigned char>;
```
Just declarations first. As ever, make default initialized content, but with a twist: of exactly the types required.
```cpp
    just_signed s;
    just_unsigned u;
```

Now comes the interesting part. To actually assign anything to these types you must very conciously make those types first.
```cpp
    s =  int8_t('s');
    u = uint8_t('u');
```
You, or your team simply can not introduce a bug there. The following will simply not compile.
```cpp
s = 's'; // does not compile
u = 'u'; // does not compile
s = u;   // does not compile
s == u;  // does not compile
```
Just a perfect API to avoid those nasty little pests growing into bugs very difficult to find.

Ok, is that it? No it is not.

In case you might think only simple implicit conversion of fundamental types, can be targeted with this API, how about something 
a bit more involved?

#### Non Trivial Use-Case

Let's assume your C++, for the heart monitor module at one point needs exactly an array of 3 integers.
Not a "naturaly decayed" pointer and not `unsigned int` elements,
 but exactly as customer requested: an array of three ints, and nothing else 
but an array of  three ints.

Now, I am sure if really pressed you can devise some clever C++ solution.
 Provided you are allowed to use them "clever solutions" in your heart monitoring module.
Alternatively, perhaps we can interest you in the following snippet?

```cpp
    // using 'dbj nothing but' API
    using just_int   = dbj::util::nothing_but< int >;
    // here is the safe and sound static array 
    // just keep it in the scope please
    just_int safe_arr[]{1,2,3};
```

Now, this might seem like a "not a lot of code" to you, but we are just showing an API new to you. 
It can be used in a much more condensed manner. 

Above code looks almost too simple. It is easy to forget the safety service this API provides.

Now the really worn out prase: Your imagination is the limit. Very true here.

### Type's handled

We do handle all the arithmetic types, only.

- integral types	
    - bool
    - char
    - char16_t
    - char32_t
    - wchar_t
    - signed char
    - short int
    - int
    - long int
    - long long int
    - unsigned char
    - unsigned short int
    - unsigned int
    - unsigned long int
    - unsigned long long int
- floating point types
     - float
     - double
     - long double

These are the types where implicit conversions do happen, by default.

But what about compound types?  For example:

- References
- Pointers
- Arrays

Why not handling them too?  Simply because in case you need them 
you will naturaly use them as ever before, but combined with this API.

```cpp
     // 0 of a signed char type
     just_signed signed_char ; 
     // pointer to it
     just_signed * signed_char_pointer ; // null_ptr as ever_
     // reference to it
     just_signed & signed_char_ref = signed_char ;
     //  array of three elements of it
     just_signed signed_char_arr[3]_;
```

Please do note, how above, all the standard C++ 
default value initialization rules are respected.

In case of some serious bugs, singularities discovered, or edge case, we will reconsider the currently handled types. 

Going beyond arithmetics it is very inlikely the implicit conversion might be the problem.

## Dependencies

This API depends on C++ std lib only. 
We are developing using the Visual Studio 2017 15.9.X, 
but always checking it is equaly usable with 
both CLANG and GCC.

## Installation

This API is header-only: it consists entirely of one header file:

       dbj_nothing_but.h

##### No compilation necessary. No isntallation required. 

Just make it part of your project.


Every attempt has been made to make this into a cross-platform, 
header only, standard C++ library.

At time of this writting (2019Q1) standard C++ is C++17. 

This project builds and tests are passing with C++14 (/std:c++14)

This project builds and tests are passing with C++17 (/std:c++17)


### License

This is open source software. Distributed under the GPLv3 License, Version 1.0.

### Contact

Please report issues or [questions here](https://github.com/dbj-systems/dbj-laboratorium/issues).
 
You can contact me via twitter at @dbjdbj



### Contributing

Any feedback from users and stakeholders will be used to improve the library.

<hr/>
	Value Handle to Avoid Implicit Conversions in standard C++

	Copyright(C) 2019 Dušan B. Jovanović (dbj@dbj.org)

	This program is free software : you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < https://www.gnu.org/licenses/>.
</hr>

 &copy; 2019 by

[![dbj();](http://dbj.org/wp-content/uploads/2015/12/cropped-dbj-icon-e1486129719897.jpg)](http://www.dbj.org "dbj")  
