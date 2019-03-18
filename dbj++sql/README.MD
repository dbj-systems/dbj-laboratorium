
# dbj++log
### syslog + local log 

> **`syslog` project started from is Copyright (c) 2008 Secure Endpoints Inc.**


> Disclaimer: <br/>
> This release is Proof Of Concept (POC). <br/>
> Use at your own risk.


Currently RFC3164 is foollowed. Plan is to develop a support for the newer one: RFC5424.

NOTE: `stderr` output is redirected to the local log file.
In case you do not need or want that, please ammend the simple code doing it.
Folder and file are under `%programdata%`. Names are here:

```cpp
/* dbj_util.h */
	static char const * DBJ_LOG_FILE_FOLDER = "dbj\\dbj++";
	static char const * DBJ_LOG_FILE_NAME = "dbj++.log";
```

## The API

Currently the user needs to initialise the logging "manually"

```cpp
    // must know the server IP address
    // It cab be coming from, the ini file, the app manifest
    // and  so on?
	init_syslog("192.168.0.202");
```

Next step is optional. But this is how you control the logging.

```cpp
/* 
first argument is the "tag" aka the "identity"
if NULL it will appear as "Anonymous", in the log

Second argument are the options defining the behaviour of 
the logging

if LOG_PERROR is among them every line sent to the syslog server
will be also written in the local log file.
*/
openlog(NULL, LOG_PERROR | LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
``` 
Other than  LOG_PERROR` special meaning the whole syslog behaviour is standard, as described on  numerous pages on the web.
`syslog.h` header in here is 99% stndard and you are free to browse through it to understand all the options.

there is also a `dbj_log_test.h` where we have a simple testing functions, from where you might start, using the syslog(), with this added functionality.

```cpp
/* 
    standard loging as ever before since 1983 
*/
	syslog(LOG_EMERG,	"%s", "LOG_EMERG"	);
	syslog(LOG_ALERT,	"%s", "LOG_ALERT"	);
	syslog(LOG_CRIT,	"%s", "LOG_CRIT"	);
	syslog(LOG_ERR,		"%s", "LOG_ERR"		);
	syslog(LOG_WARNING, "%s", "LOG_WARNING" );
	syslog(LOG_NOTICE,	"%s", "LOG_NOTICE"	);
	syslog(LOG_INFO,	"%s", "LOG_INFO"	);
	syslog(LOG_DEBUG,	"%s", argv[0]	);
```

### Async behaviour and Multi Threading

This is all asynchronous as much as windows sockets are asynchronous.
But all the impotrtant functions in here are capable of sequential working, 
in the presence of multiple threads.

### WARNING: Messages are composed following the RFC3164

Make sure you syslog server can understand them. Which is the case with almost all of them. 
This is simply because there are milions of devices in use (nad will be for a long time to come) who are emitting 
syslog messages in the RFC3164 format. Device on the network, not PC's or laptops.

> This is a static library. 

> Built with Microsoft (R) C/C++ Optimizing Compiler Version 19.15.26732.1 or better.
> (In 2019Q1, standard C++ was C++17)


-------------------------------------

<pre>
Copyright 2017,2018, 2019 by dbj@dbj.org, dbj.org, dbj.systems ltd.

Licensed under the GNU GPL License, Version 3.0 (the "License");
you may not use any of files in this project except in compliance with the License.
You may obtain a copy of the License in the file LICENSE enclosed in
this project.

https://www.gnu.org/licenses/gpl-3.0.html

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
</pre>
---------------------------------------------------------------------  

[![dbj();](http://dbj.org/wp-content/uploads/2015/12/cropped-dbj-icon-e1486129719897.jpg)](http://www.dbj.org "dbj")  

