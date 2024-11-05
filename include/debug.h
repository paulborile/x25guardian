/*
 * $Id: debug.h,v 1.1.1.1 1998/11/18 15:03:05 paul Exp $
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: debug
 *
 * Contents: header_file
 *
 * Author(s):
 *
 * $Log: debug.h,v $
 * Revision 1.1.1.1  1998/11/18 15:03:05  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/07/07  10:12:06  px25
 * Initial revision
 *
 *
 */

# ifndef D_SIL

extern	void	_debug(), enddebug();

# define D_FLAGS	0xF000	/* Debuggging flags mask */
# define D_SIL		0x8000	/* Silent: don't print mod_id and u_id */
# define D_TIME		0x4000	/* Print elapsed real time since initdebug */
# define D_DATE		0x2000	/* Print date&time */

/*
 * Conditional use of debug.
 * To use debug directly, call _debug.
 *
 * WARNING: in the source code, debug calls MUST have double parentheses
 *	    as:
 *	    debug(( 9, "Errno = %d\n", errno ));
 *	    AND the first "(" MUST immediatly follow the "debug" word, thus:
 *
 *	    debug(( 9, "Errno = %d\n", errno ));  -- CORRECT
 *	    debug (( 9, "Errno = %d\n", errno )); -- WRONG!!
 *	    debug( 9, "Errno = %d\n", errno );    -- WRONG!!
 */

# ifdef DEBUG
# define debug(stuff)	_debug stuff
# else
# define debug(stuff)
# endif

# endif
