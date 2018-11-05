
# dbj++sql

### Yet Another API (YAPI)
#### Using SQLITE(3) from standard  C++

> Disclaimer: <br/>
> Immediatrly usable in your projects. <br/>
> Current release is Proof Of Concept (POC). <br/>
> Unless stated otherwise.


This is a static library. SQLITE 3 is linked in. 

Built with Microsoft (R) C/C++ Optimizing Compiler Version 19.15.26732.1 for x86

[Inspired by Keny Kerr](
https://visualstudiomagazine.com/articles/2014/02/01/using-sqlite-with-modern-c.aspx). 

-------------------------------------

Copyright 2018 by dbj@dbj.org
<pre>
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
</pre>
---------------------------------------------------------------------  

Just very recently I had a need for a local storage, aka database. Of course, it has to be small but complete, mature and functional.  <a href="https://sqlite.org/index.html" target="_blank" rel="noopener">SQLite </a>is one <a href="https://en.wikipedia.org/wiki/Embedded_database" target="_blank" rel="noopener">embedded</a>, but very complete and mature (almost full) SQL capable library for using local database (like) files. All you need, but without an RDBMS server, imposed on you. And without a licence fee too. But. Written in C

Not a standard C++, we all love and hate.  All of the SQLite is in one (very) large c file. And one equally large C header. To use it in your C++ project you need to build a library and use them. And then you either use its very large C set of functions making the API or ... you develop your own in standard C++ API. Or you possibly find some of the dozen or so, open source ones.

(for UWP and  .NET or C# jockeys, Microsoft is including the SQLite in a form of a <a href="https://docs.microsoft.com/en-us/windows/uwp/data-access/sqlite-databases">.NET assembly</a> )

Quick GitHub search revealed few, for my taste, either over engineered C++ solutions or not standard C++ (14, or 11 or even before).

In the search results, I spotted Kenny Kerr's <a href="https://kennykerr.ca/2014/04/15/sqlite-with-c/" target="_blank" rel="noopener">somewhat short but useful set of 3 articles</a> with working code.

V.s. Kenny's, very sober and pragmatic solution, my API is conceptually further from the raw C of the SQLite API. That is more comfortable but not that loved by SQLite jockeys who memorize hundred or so <code>sqllite3_*</code>  functions and use them perhaps all day every day.

Well, this API is for you, a casual or above casual C++ developer.  I was always wondering myself, why do people try to make a real RDBMS server out of humble SQLite.

Thus, I have also deliberately not provided some functionality to which "real SQL" people are used to when dealing with C/C++ api's, to their RDBMS servers.

Perhaps a chief example is <a href="https://www.sqlite.org/c3ref/bind_blob.html" target="_blank" rel="noopener">SQLite statement "binding"</a>.  Where  users of this API can have parameterized SQL statement to which they can "bind" the actual arguments

```sql
SELECT word FROM words WHERE word LIKE('?%')
```

<span style="font-size: 16px;">NOTE: as a sample DB, I am using an <a href="https://github.com/dwyl/english-words/" target="_blank" rel="noopener">English dictionary in a file</a>, which I have transformed in the SQLite 3 DB file. It has a single table</span>: <span style="font-size: 16px;"><code>words</code></span>,<span style="font-size: 16px;"> with a single text column named `word`.</span>

Above, the '?' mark is the placeholder to which one can "bind" the actual values. Replacing te '?' mark with a value.
My API does not provide a facility for "binding". I am sure you are more than capable to do this best yourself in your standard C++.

I hope you are still reading this and you are looking for simple and comfortable SQLite,  standard c++ wrapper.

My objective here is to develop the minimal but useful SQLite  API. After which, I will explain a bit about this API, we will go through a few examples and hopefully prove the usability of my approach.

(<a href="https://github.com/dbj-systems/dbj-laboratorium/tree/master/dbj%2B%2Bsql" target="_blank" rel="noopener">Resulting static lib is on GitHub</a> )
## The Logic 

Or lack of. This first thing I have done was to wrap this huge sqlite3 C API in the C++ namespace. Like so:

```cpp
#pragma once
/*
wrap the SQLite into the namespace
this obviously does not hide the #define's
2018-11-03 dbj@dbj.org
*/
namespace sqlite{
#include "sqlite/sqlite3.h"
}
```

I feel much safer dealing with large C API's this way. If I am really into using dozens of raw C, <code>sqlite3_*</code> functions I can always do

```cpp
using namespace sqlite ;
```

inside the namespace or a function where I do need them.

The whole API is in the file <code>dbj++sql.h</code> . The only one you need to include. On the bottom, you will find 3 testing functions. They will exist only if you hash define<code>DBJ_DB_TESTING</code> before you include the single required header

```cpp
// for testing
#define DBJ_DB_TESTING
// local or with the path
#include "dbj++sql.h"
```

I assume you know basic SQL.  In that case usage is really very simple.

```cpp
database db(db_file);
// create a table
db.execute("DROP TABLE IF EXISTS demo");
db.execute("CREATE TABLE demo_table ( Id int primary key, word nvarchar(100) not null )");
// populate the table
db.execute("INSERT INTO demo_table (Id, Name) values (1, 'London'), (2, 'Glasgow'), (3, 'Cardif')");
// select from the table
db.execute("SELECT word FROM demo_table WHERE word LIKE 'G%'");
```
I am sure, you can find the above  in a dbj++sql.h, and see this is all in the "in-memory database" where the DB file names is the reserved one `":memory:"` .

In there you can also find we are always catching the "special" exception `dbj::db::exception`. It contains the SQLite error messages and error codes.

```cpp
{ /* use the dbj++sql api in here */ }
catch (dbj::dbj::sql_exception const &amp; e)
{
wprintf(
L"dbj::db exception, code: %d, message: %S\n", e.code, e.message.c_str()
);
}
```

If you are new the SQLite you will soon realize it is very forgiving. It relatively rarely returns error codes amd thus throw of <code>dbj::db::exception</code> happens realtively rarely, vs some other "normal" c++ code.

## The Interface

is (almost) all in one class:

```cpp
// second argument for 
// the database method
// query_result
using result_row_user_type =
int(*)(
const size_t , const vector &amp; , const value_decoder &amp;
);

class database final
{
public:
explicit database( string_view storage_name );
/*
register a function to be executed 
from inside a query
*/
auto register_user_defined_function
(
string_view udf_name,
/* the signature of a function 
to be registered */
void(__cdecl * udf_)
(sqlite3_context *, int, sqlite3_value **)
) ;
/*
Call with query and an
// optional callback row user
To operate on rows of the the result table
*/
auto query_result(
char const * query_,
optional&lt;result_row_user_type&gt; row_user_ = nullopt
);
}; // database
} // dbj::db
```

Yes, there is only one method through which you can (and will) pass your SQL queries. If callback is sent it will be used. Here is one example:

```cpp
/*
dbj++sql callback sample
NOTE: it is called once per each row
*/
int example_callback(
[[maybe_unused]] const size_t row_id,
/* this is giving us column count and column names */
[[maybe_unused]] const std::vector &amp; col_names,
[[maybe_unused]] const dbj::db::value_decoder &amp; val_user
)
{
// showing the result for 
// SELECT word from words 
// 'automagic' transform to std::string
// of the column 0 value for this row
std::string   word_ = val_user(0);
printf("\n\t%d\t%s", row_id, word_);

// do not forget this return
// otherwise dbj++sql
// will throw an dbj::db::sql_exception
// with a coresponding message
return SQLITE_OK;
}
```

Above is an callback defined for the following demo code:

```cpp
using namespace dbj::db ;
try {
// full path to your db file
database db(db_file);
db.query_result("select word from words where word like 'bb%'", cb_);
}
catch (sql_exception const &amp; e) {
wprintf(L"SQLite Exception\t%d, %S\n", e.code, e.message.c_str());
}
```

But, wait the minute pardner? What's that <code>dbj::db::value_decoder &amp; val_user</code> in the signature of the callback, above?

That is our little C++ helper so we do not see any value getting SQLite C code, for getting actual values from the result columns. We just get them as if by magic. Already in the proper type.

```cpp
struct value_decoder final
{
struct transformer final 
{
/* if user needs  float, the user will handle that best */
operator double() const noexcept ;
operator long() const noexcept ;
operator long long() const noexcept;
/* 
SQLITE is by default UTF-8
wstring is primarily windows curiosity
multibyte chars even more so
windows users will use this and perform the 
required conversions themselves best
*/
operator std::string() const noexcept;

mutable sqlite3_stmt *	statement_;
mutable size_t	col_index_;
};
/* 
return the transformer for particular column
from the current result set
*/
transformer operator ()(size_t col_idx ) const noexcept;
// --------------------------------------
mutable sqlite3_stmt * statement_{};
}; // value_decoder
```

Before using the callback and from inside query_result method, we make and send as argument to it, instacne of the above struct. Its single functional call operator returns a <code>value_decoder::transformer</code> instance for the particular column, from the result set. The crucial line from a callnack above once more, But with explanation this time:

```cpp
std::string   word_ = val_user(0);
/* above actualy first returns a temporary object */
   value_decoder::transformer _temporary_ 
        = val_user(0) ;
/* then because compiler sees we expect std::string type 
   it looks for and finds a std::string conversion operator 
   and calls it */
   // (pseudo code)
   std::string   word_  
       = (_temporary_::&amp; std::string operator)() ;

```

This is actually a well know and old C++ tehnique. In here we meet head on the fact ve are dealing with a database. Internaly result set is of a unsigned char * type. Whatever type you ask for SQLite will dutifuylly return its representation in an desired type. Assumptionn is you know the structure of the database you are dealing with.

Development is going on, but interface is very unlikely to change.
What might change is more exceptions poping out in case you try and are allowed to do naughty things.

Enjoy standard C++.

### &copy; 2018 [![dbj();](http://dbj.org/wp-content/uploads/2015/12/cropped-dbj-icon-e1486129719897.jpg)](http://www.dbj.org "dbj")  

