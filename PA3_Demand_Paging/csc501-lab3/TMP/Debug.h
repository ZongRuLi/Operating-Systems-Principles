#ifndef __DEBUG_H__
#define __DEBUG_H__


#include <stdio.h>

extern int debugLevel;

#ifdef DEBUG
#define DEBUG_ENABLED 1  // debug code available at runtime
#else
/** This macro controls whether all debugging code is optimized out of the
 *  executable, or is compiled and controlled at runtime by the
 *  <tt>debugLevel</tt> variable. The value (0/1) depends on whether
 *  the macro <tt>DEBUG</tt> is defined during the compile.
 */
#define DEBUG_ENABLED 1  // all debug code optimized out
#endif 

/** Print the file name, line number, function name and "HERE" */
#define HERE debug("HERE")

/** Expand a name into a string and a value
 *  @param name name of variable
 */
#define debugV(name) #name,(name)

/** Output the name and value of a single variable
 *  @param name name of the variable to print
 */
#define vDebug(fmt, name) debug("%s=(" fmt ")" , debugV(name))

/** Simple alias for <tt>lDebug()</tt> */
#define debug(fmt, ...) lDebug(1, fmt, ##__VA_ARGS__)

/** Print this message if the variable <tt>debugLevel</tt> is greater
 *  than or equal to the parameter.
 *  @param level the level at which this information should be printed
 *  @param fmt the formatting string (<b>MUST</b> be a literal
 */
#define lDebug(level, fmt, ...) \
  do { \
       if (DEBUG_ENABLED && (debugLevel >= level)) \
         kprintf( "DEBUG %s[%d] %s() " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
  } while(0)

#endif
