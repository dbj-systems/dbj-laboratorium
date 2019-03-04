# nothing_but

####	Type Handle to Avoid Implicit Conversions in standard C++

Every attempt has been made to make this into a cross-platform, 
header only, standard C++ library.

At time of this writting (2019Q1) standard C++ is C++17. 

This project builds and tests are passing with C++14 (/std:c++14)

This project builds and tests are passing with C++17 (/std:c++17)

## Motivation

Probably you indulge into recreational C++ and the following is 
thus just a curiosity:

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
Or in a medical equioment perhaps. 

You might take the C++ [implicit conversions](https://en.cppreference.com/w/cpp/language/implicit_conversion) so seriously that "even" the following is a very serious 
matter for you

```cpp
   // implicit conversion of int to char
   char C = 64 ;
   // implicit conversion of double to float
   float F = 3.7;
```

### The suggestion

Before you discard (with the heavy heart) C++ completely we might suggest you look into this ridiculously tiny single header?
Here is some code to tickle your fancy.

```cpp
// let's assume your code must not accidentaly mix
// signed and unsigned chars
    using just_signed   = dbj::util::nothing_but<signed char>;
    using just_unsigned = dbj::util::nothing_but<unsigned char>;
```
Juat a declarations, make default initialized content of the exact type required.
```cpp
    just_signed s;
    just_unsigned u;
```

Now comes the interesting part. To actually assign anything to these types you must very conciously make those types first.
```cpp
    s =  int8_t('s');
    u = uint8_t('u');
```
Yuo or your team simply can not introduce a bug there. The following will simply not compile.
```cpp
s = 's'; // does not compile
u = 'u'; // does not compile
s = u;   // does not compile
s == u;  // does not compile
```
Just a perfect API to avoid those nasty little pests growing into bugs very difficult to find.

In case you might think only simple implicit conversion of fundamental types, can be targeted, how about something 
a bit more involved?

Let's assume your heart monitor module at one point needs exactly an array of 3 integers.
Not a "naturaly decayed" pointer and not `unsigned int` elements,
 but exactly as customer requested: an array of three ints, and nothing else 
but an array of  three ints.

Now, I am sure if really pressed you can devise some clever SFINAE based solution.
 Provided you are allowed to use them SFINAE solutions in your heart monitoring module.
Or perhasp we can interest you in the following snippet?

```cpp
  // this will not require array swapping
  // just keep it in the scope.
  using arr_ref = std::reference_wrapper<int [3]> ;
  // using dbj nothing but API
     using just_arry   = dbj::util::nothing_but< arr_ref >;
  // here is the safe and sound static array 
  // just keep it in the scope
     int iarr[]{1,2,3};
  // intermediate step not required
    arr_ref arf = std::ref(iarr);
  // give it to our super safe structure
    just_arry arry( arf );
```

Now, this might seem like a "lot of code" to you, but we are just showing an API new to you. 
It can be used in a much more condensed manner. 

```cpp
// function thorugh which we fully move by value the "just_arry"
// we do not use references just to show moving works 
// standard C++ value semnatics
 auto collector = [] ( just_arry  ja_ ) 
{ 
   // data() method is returning what is inside 
   // `dbj nothing but` structure, in this case
   // that is `arr_ref` type
   // which in turn has the method get() 
   // to deliver the reference on the
   // int[3]
   ja_.data().get()[0] = 42; return ja_; 
} ;
//
just_arry results = collector(arry) ;
_
```

Looks almost too simple. It is almost easy to forget the safety service this API provides.


## Dependencies

This API depends on C++ std lib only. 
We are developing using the Visual Studio 2017 15.9.X, but always checking it is usable with 
both CLANG and GCC.

## Installation

This API is header-only: it consists entirely of one header file:

       dbj_nothing_but.h

##### No compilation necessary.


### License

Distributed under the GPLv3 License, Version 1.0.

### Contact

Please report issues or [questions here](https://github.com/dbj-systems/dbj-laboratorium/issues).
 
You can contact me via twitter at @dbjdbj

---

### Contributing

Any feedback from users and stakeholders will be used to improve the library.

<hr/>
	Type Handle to Avoid Implicit Conversions in standard C++

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
