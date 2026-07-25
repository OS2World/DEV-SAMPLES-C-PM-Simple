/*********************************************************************
 *
 *  simple.h - Header file for Simple PM Window Demo
 *
 *  Provides utility macros used by the Simple PM Window application.
 *  The min() and max() macros are conditionally defined to avoid
 *  conflicts when these are already provided by the compiler or
 *  other included headers.
 *
 *  Authors:
 *    Original Author
 *    Martin Iturbide / OS2World (2023)
 *
 *  License: BSD 3-Clause — see LICENSE file.
 *
 *********************************************************************/

/* Resource IDs */
#define ID_WINDOW    255                /* Frame window resource ID     */

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
