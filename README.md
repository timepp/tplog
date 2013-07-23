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

<table>
	<tr>
		<td>[tplog]</td>
		<td></td>
		<td>files for wrapping log functionality into DLL</td>
	</tr>
	<tr>
		<td>[tplogview]</td>
		<td></td>
		<td>tplogview source</td>
	</tr>
	<tr>
		<td>[build]</td>
		<td></td>
		<td>vcprojs for building</td>
	</tr>
	<tr>
		<td>[include]</td>
		<td></td>
		<td>tplog main</td>
	</tr>
	<tr>
		<td></td>
		<td>[detail]</td>
		<td>tplog detail implementation</td>
	</tr>
	<tr>
		<td></td>
		<td>tplog.h</td>
		<td>tplog interface</td>
	</tr>
	<tr>
		<td>[testlog]</td>
		<td></td>
		<td>functional test</td>
	</tr>
	<tr>
		<td>tools</td>
		<td></td>
		<td>misc tool</td>
	</tr>
</table>
