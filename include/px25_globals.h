/*
 * $Id: px25_globals.h,v 1.1.1.1 1998/11/18 15:03:05 paul Exp $
 * 
 * Project : PX25 - Router per Autorizzazioni POS
 *
 * Module: 
 *
 * Contents: 
 *
 * Author(s): C.U. S.r.l - P.Borile
 *
 * $Log: px25_globals.h,v $
 * Revision 1.1.1.1  1998/11/18 15:03:05  paul
 * Guardian : x25 Pos router
 *
 * Revision 1.2  1995/09/18  13:13:29  px25
 * Modified struct NUA to include new userdata to be replaced.
 * Added flags for QBIT in incoming packet
 *
 * Revision 1.1  1995/07/14  09:22:10  px25
 * Added defines for all env variables used
 * Added RESPAWN exit code
 *
 * Revision 1.0  1995/07/07  10:12:06  px25
 * Initial revision
 *
 *
 */

/*
 * Header for internal messages
 */

struct	PKT_HDR
{
	unsigned	char	pkt_code;
	unsigned	char	pkt_error;
	unsigned	char	flags;
	unsigned	char	pkt_len;
};

/* X25 Data packet size, used to set more bit	*/

#define	X25_DATA_PACKET_SIZE		128

/* Len of x25 pid, for argotel or easyway strings	*/

#define	X25PID_LEN					4

#define	MAX_USER_DATA_LEN						16

/*
 * Possible packet codes
 */

#define	X25_DATA_PACKET			1
#define	X25_CALL_REQ				2
#define	X25_ERROR					3
#define	X25_CONNECTION_END		4
#define	X25_CALL_REJECT			5
#define	X25_CALL_ACCEPT			6

#define	ASY_DATA_PACKET			129
#define	ASY_CALL_REQ				130
#define	ASY_ERROR					131
#define	ASY_CONNECTION_END		132
#define	ASY_CALL_REJECT			133
#define	ASY_CALL_ACCEPT			134

/*
 * Possible flags
 */

#define	MORE_BIT						1
#define	QBIT							2

/*
 * Possible pkt_errors
 */

#define	PORT_DISABLED				1

#define	NUA_LEN	24

/*
 * Data structures
 */

struct	NUA
{
	char	primary_nua[NUA_LEN];
	char	secondary_nua[NUA_LEN];
	unsigned	char	udata_tobe_replaced[MAX_USER_DATA_LEN];
	int	udata_tobe_replaced_len;
	int	nua_type;
};

/*
 * Define nua types in routing table
 */

#define	ASY_NUA		0
#define	X25_NUA		1
#define	SNA_NUA		2
#define	TTY_NUA		3

/*
 * Symbols for nua types
 */

#define	ASY_NUA_STRING		"ASY"
#define	X25_NUA_STRING		"X25"
#define	SNA_NUA_STRING		"SNA"
#define	TTY_NUA_STRING		"TTY"

#define  PX25_SYN_TABLE_PATH  	"PX25_SYN_TABLE_PATH"
#define	PX25_ASY_TABLE_PATH		"PX25_ASY_TABLE_PATH"
#define	PX25_RT_TABLE_PATH		"PX25_RT_TABLE_PATH"
#define	PX25_SUB_RT_TABLE_PATH	"PX25_SUB_RT_TABLE_PATH"
#define  PX25_X3_TABLE				"PX25_X3_TABLE"
#define	PX25_X29_DIR				"PX25_X29_DIR"

#define	PX25_GEN_TABLE			"PX25_GEN_TABLE"
#define	PX25_DSCOPE_DIR		"PX25_DSCOPE_DIR"
#define	PX25_SM_HOST			"PX25_SM_HOST"
#define	PX25_LOG_DIR			"PX25_LOG_DIR"
#define	PX25_DSCOPE_DIR		"PX25_DSCOPE_DIR"
#define	PX25_UTMP_FILE			"PX25_UTMP_FILE"

/*
 * Return codes for processes
 */

#define	EXIT_AND_DELETE_ROUTE	3
#define	RESPAWN			2
#define	FAILURE			1
#define	SUCCESS			0
