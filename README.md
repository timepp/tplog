#tplog

highly extensible c++ log library on windows system

tplog include: 

 - tplog library; 
 - tplogview, a GUI tool for viewing tplog pipe device output;
 - testtplog, for tplog functional testing.


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
