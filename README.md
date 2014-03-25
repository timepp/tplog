#tplog

highly extensible c++ log library on windows system

tplog include: 

- tplog library; 
- tplogview, a GUI tool for viewing tplog pipe device output;
- testtplog, for tplog functional testing.

#License
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Junhui Tong
  tongjunhui@live.cn 

#dir structure

* [tplog]: tplog DLL wrapper [todo][1]
* [tplogview]: The GUI log viewer
* [build]: build related files
* [include]: tplog implementations [todo][2]
    - [detail]: implementation detail
	- tplog.h: main interface of writing log
	- tplog_util.h: other handy facility to help writing log
	- tplog_impl.h: file to make tplog singleton [todo][3]
	- tplog_reader.h: implementation for reading log from varies sources such as pipe/file etc
	- tplog_synctime.h: interface to get tplog's process-level synced time
* [testlog]: unit testing
* [doc]: documentations

[1]: rename to dll
[2]: rename to tplog
[3]: rename to tplog_singleton.h
