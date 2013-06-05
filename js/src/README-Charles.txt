This is a description of the modifications I (Charles) have made to enable 
parallel execution of loop iterations.  This code comes without guarantee
of correctness and a sincere apology for its quality.

For the most part, comments I have made within existing mozilla files are 
prefixed by CAL.  I have attempted to add comments containing my knowledge 
of how the interpreter works.  This knowledge is very, very incomplete.

All the important changes are in the following files:

* cmps530.h
    * Contains some data structure definitions and helper functions.
* cmps530-threading.cpp
    * Contains the interpret function executed in threads and some 
    associated data structures functions.
* jsinterp.cpp
    * The mozilla file containing the javascript interpreter.  The function 
      of interest is Interpret.
* interp-defines.cpp
    * I moved some defines that used to be in jsinterp.cpp here for
      convenience.

The javascript interpreter is a fairly simply fetch-decode-execute 
interpreter.  The interpret function is named Interpret (like you needed me 
to tell you that).  The first for loop is the main interpret loop.  Read 
the in-code comment for a description of how it works.

To build spidermonkey:
* If it doesn't exist already, mkdir build-cmps530
* wget https://hg.mozilla.org/mozilla-central/raw-file/default/python/mozboot/bin/bootstrap.py && python bootstrap.py
    * Install preqs
* autoconf2.13 # or autoconf-2.13
    * I've modified the config to compile with the -std=c++0x  option
      by adding it to CXXFLAGS in autoconf/compiler-opts.m4.  This is
      to be able to use std::thread provided by c++11.
* cd build-cmps530
* ../configure --disable-methodjit --disable-monoic --disable-optimize --disable-tests
* make
* ./run.sh

Here are some useful comments:

JS_THREADSAFE is not set.

Script stored in class jsscript.h -> JSScript
    * Contains type Bindings to hold bindings
    * notes() of type jssrcnote*

There are some useful constants at the top of jsinterp.cpp that can be
commented/uncommented as needed.

Opcodes are defined in jsopcode.tbl.  Pretty much the only documentation
of what instructions do are in this file and in their implementation in
jsinterp.cpp.  (I think there is additional info in the tests for
spidermonkey as well, but I never ventured there.)

My method of finding where things were implemented were a combination of grep and the ctrl+click feature of eclipse.

[Internals Documentation][internals]
[JSAPI User Guide][guide] (C API) 
[JSAPI Reference][reference]
[Main Page][main]
[New to Spider Monkey][new]
[JSAPI Cookbook][cookbook] 

[main]: https://developer.mozilla.org/en-US/docs/SpiderMonkey
[new]: https://wiki.mozilla.org/JavaScript:New_to_SpiderMonkey
[cookbook]: https://developer.mozilla.org/en-US/docs/SpiderMonkey/JSAPI_Cookbook
[reference]: https://developer.mozilla.org/en-US/docs/SpiderMonkey/JSAPI_Reference
[internals]: https://developer.mozilla.org/en-US/docs/SpiderMonkey/Internals
[guide]: https://developer.mozilla.org/en-US/docs/SpiderMonkey/JSAPI_User_Guide

