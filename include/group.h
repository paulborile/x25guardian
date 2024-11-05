/*
 * $Id: group.h,v 1.1.1.1 1998/11/18 15:03:05 paul Exp $
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: 
 *
 * Contents: 
 *
 * Author(s): C.U. S.r.l - P.Borile, G. Priveato
 *
 * $Log: group.h,v $
 * Revision 1.1.1.1  1998/11/18 15:03:05  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.0  1995/09/18  13:12:18  px25
 * Initial revision
 *
 *
 */

/*
 * rt errno's
 */

#define	ESUB_RT_NO_ENV_VAR	4000
#define	ESUB_RT_UD_NOTFOUND	4001
#define	ESUB_RT_CANNOT_OPEN	4002

/*
 * structure to keep group data
 */

struct	GROUP
{
	char	grp_name[MAX_USER_DATA_LEN];
	int	grp_num;
};
