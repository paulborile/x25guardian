/*
 * $Id: sm.h,v 1.1.1.1 1998/11/18 15:03:08 paul Exp $
 *
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module:
 *
 * Contents:
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: sm.h,v $
 * Revision 1.1.1.1  1998/11/18 15:03:08  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.1  1995/09/18  14:30:21  px25
 * Added Error for Tty group not found.
 *
 * Revision 1.0  1995/07/07  10:10:14  px25
 * Initial revision
 *
 *
 */

#define SM_HOST "PX25_SM_HOST"

#if !defined(MAX_STR)
#include    "smp.h"
#endif

/*
 * Data type returned by sm_get_best_route
 */

struct  BEST_ROUTE
{
    char hostname[MAX_STR];
    char service[MAX_STR];
    char link[MAX_STR];
};

/*
 * functions and return codes
 */

#define ESM_INVALID_RTTYPE  2000
#define ESM_NO_SM_HOST          2001
#define ESM_NO_SM               2002
#define ESM_NO_ROUTE            2003
#define ESM_NO_GROUP_NAME       2004


#ifdef _LIBSM
int sm_errno;
int sm_create_route();
int sm_free_route();
int sm_delete_route();
int sm_list_route();
int sm_dump_route();
int sm_load_route();
struct  BEST_ROUTE *sm_get_best_route();
int sm_debug();
int sm_enddebug();
#else
extern int sm_errno;
extern int sm_create_route();
extern int sm_free_route();
extern int sm_delete_route();
extern int sm_list_route();
extern int   sm_dump_route();
extern int sm_load_route();
extern struct  BEST_ROUTE *sm_get_best_route();
extern int sm_debug();
extern int sm_enddebug();
#endif
