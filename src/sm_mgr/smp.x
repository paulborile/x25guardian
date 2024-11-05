const MAX_STR = 32;

/* definizione della struttura  */
struct node {
	string machine<MAX_STR>;
	string link<MAX_STR>;
	string service<MAX_STR>;
	int active;
	int max;
	int route_type;
};

/* procedure rpc */
program RDBPROG {
	version RDBVERS {
				int CREATE_ROUTE(node) = 1;
				node GET_BEST_ROUTE(node) = 2;
				int FREE_ROUTE(node) = 3;
				int INCR_ROUTE(node) = 4; 
				int DELETE_ROUTE(node) = 5;
				int LIST_ROUTE(void) = 6;
				int INIT_DEBUG(int) = 7;
				int END_DEBUG(void) = 8;
/*				int EXIT(void) = 10;        */
				int DUMP(string) = 9;
				int LOAD(void) = 10;
		} = 1;
} = 0x20000001;


