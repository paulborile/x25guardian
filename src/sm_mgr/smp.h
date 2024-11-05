/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <rpc/types.h>

#define MAX_STR 32

struct node {
    char *machine;
    char *link;
    char *service;
    int active;
    int max;
    int route_type;
};
typedef struct node node;
bool_t xdr_node();

#define RDBPROG ((u_long)0x20000001)
#define RDBVERS ((u_long)1)
#define CREATE_ROUTE ((u_long)1)
extern int *create_route_1();
#define GET_BEST_ROUTE ((u_long)2)
extern node *get_best_route_1();
#define FREE_ROUTE ((u_long)3)
extern int *free_route_1();
#define INCR_ROUTE ((u_long)4)
extern int *incr_route_1();
#define DELETE_ROUTE ((u_long)5)
extern int *delete_route_1();
#define LIST_ROUTE ((u_long)6)
extern int *list_route_1();
#define INIT_DEBUG ((u_long)7)
extern int *init_debug_1();
#define END_DEBUG ((u_long)8)
extern int *end_debug_1();
#define DUMP ((u_long)9)
extern int *dump_1();
#define LOAD ((u_long)10)
extern int *load_1();
