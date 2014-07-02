#pragma once

/** @FILE
 * compiler configuration
 * - Enforced warning level. Usage: Set '/Wall' in project configuration; place all 3rd party header files which may issue warning inside TP_NO_WARNING_AREA_BEGIN/END
 */

/// No warning aera. Use it to dismiss the 3rd party warnings.
#define TP_NO_WARNING_AREA_BEGIN()                    \
	__pragma(warning(push, 1))                        \
	__pragma(warning(disable: 4505 4548 4819 4917 4555 4350))        \
	__pragma(warning(disable: 6202 6328 6334 6385 6386 6387 6255 6031 6011 6400 6246 6401 6323 6211 6282)) \
	__pragma(warning(disable: 6054 6001 28183))

#define TP_NO_WARNING_AREA_END()                      \
	__pragma(warning(pop))

/// Jump out of the warning stack.
/// Some header files (such as regex_token_iterator.hpp in boost 1.34) have bugs that warning stack pushes and pops are not, 
/// balanced, leaving us a low warning level stack frame
#define TP_CLEAR_WARNING_STACK()         \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(default: 4193))


/// Be sure to use this macro immediately after include regex.hpp from boost1.34
#define TP_WARNING_WORKAROUND__BOOST_1_34_REGEX __pragma(warning(pop)) __pragma(warning(pop))

/// Enable warnings that are disabled by default and disable warnings we don't care about
#define TP_TUNE_WARNINGS \
__pragma(warning(default: 4061 4062 4191 4242 4254 4263 4264 4265 4266 4287 4296)) \
__pragma(warning(default: 4302 4365 4514 4545 4546 4547 4548 4549 4555 4571)) \
__pragma(warning(default: 4623 4625 4626 4640 4668 4682 4710 4711 4820 4826)) \
__pragma(warning(default: 4905 4906 4917 4928 4946)) \
\
__pragma(warning(disable: 4061 4127 4503 4514 4571 4623 4625 4626 4710 4820)) \
__pragma(warning(disable: 4505 4512 4711))

#define TP_CONFIGURE_WARNINGS \
	TP_CLEAR_WARNING_STACK \
	TP_TUNE_WARNINGS


#ifndef WIDESTRING
#define WIDESTRING2(str) L##str
#define WIDESTRING(str) WIDESTRING2(str)
#endif

#ifndef STRINGIZE
#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)
#endif

#ifndef COMPILE_MESSAGE
#define COMPILE_MESSAGE(msg) \
	__pragma(message(__FILE__ "(" STRINGIZE(__LINE__) ") :" msg))
#endif
