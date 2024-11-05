#include        <stdio.h>
#include        <rpc/rpc.h>
#include        "px25_globals.h"
#include        "debug.h"
#include        "sm.h"
#include        "group.h"

main(argc, argv)
int argc;
char **argv;
{
    FILE *fp;
    char pid_str[20];
    int debuglevel;
    char debug_file[BUFSIZ];

    char host[BUFSIZ];
    char link[BUFSIZ];
    char service[BUFSIZ];
    int active, max, type, level;
    int rc;
    char buffer[BUFSIZ];
    char status[BUFSIZ];
    char command[BUFSIZ];
    struct  BEST_ROUTE *br;
    char link_num[10];
    struct  GROUP g;

    int fatal = 0;
    int c;
    extern char *optarg;

    while ((c = getopt(argc, argv, "d:")) !=EOF)
    {
        switch (c)
        {

        case 'd':
            debuglevel  =  atoi(optarg);
            break;

        default:
            printf("%s : invalid flag.\n", argv[0]);
            exit(1);
        }
    }

    if ( debuglevel == -1 )
    {
        fatal++;
    }
    if ( fatal )
    {
        exit(1);
    }

    sprintf(pid_str, "%d", getpid());
    sprintf(debug_file, "/tmp/sm_test%05d.debug", getpid());

#ifdef  DEBUG
    initdebug(debuglevel, debug_file, argv[0], pid_str);
#endif

    while (1) {
        printf("Inserisci il comando\n");
        gets(command);

        if ( strcmp(command, "debug") == 0)
        {
            printf("DEBUG_LEVEL : ");
            fflush(stdout);
            gets(buffer);
            level = atoi(buffer);

            rc = sm_debug(level);
            debug((1, "sm_debug rc  = %d\n", rc));
        }
        else if ( strcmp(command, "enddebug") == 0)
        {
            rc = sm_enddebug();
            debug((1, "sm_enddebug rc  = %d\n", rc));
        }
        else if ( strcmp(command, "list") == 0 )
        {
            rc = sm_list_route();
            debug((1, "sm_list_route rc  = %d\n", rc));
        }
        else if (strcmp(command, "dump") == 0)
        {
            sprintf(buffer, "/tmp/sm%d", getpid());
            rc = sm_dump_route(buffer);
            debug((1, "sm_dump_route rc = %d\n", rc));

            if (( fp = fopen(buffer, "r")) == NULL )
            {
                perror("buffer");
                continue;
            }

            while ( fgets(status, BUFSIZ, fp) != NULL )
            {
                printf("%s", status);
            }

            fclose(fp);
            unlink(buffer);
        }
        else if (strcmp(command, "load") == 0)
        {
            rc = sm_load_route();
            debug((1, "sm_load_route rc = %d\n", rc));
        }
        else if ( strcmp(command, "add") == 0)
        {
            printf("Inserisci HOSTNAME : ");
            fflush(stdout);
            gets(host);

            printf("LINK : ");
            fflush(stdout);
            gets(link);

            printf("SERVICE : ");
            fflush(stdout);
            gets(service);

            printf("ACTIVE : ");
            fflush(stdout);
            gets(buffer);
            active = atoi(buffer);

            printf("MAX : ");
            fflush(stdout);
            gets(buffer);
            max = atoi(buffer);

            printf("ROUTE_TYPE : ");
            fflush(stdout);
            gets(buffer);
            type = atoi(buffer);

            rc = sm_create_route(host, link, service, active, max, type);
            debug((1, "sm_create_route rc = %d\n", rc));
        }
        else if ( strcmp(command, "best") == 0 )
        {
            printf("Inserisci HOSTNAME : ");
            fflush(stdout);
            gets(host);

            printf("ROUTE_TYPE : ");
            fflush(stdout);
            gets(buffer);
            type = atoi(buffer);

            if ( type == ASY_NUA )
            {
                printf("GROUP NAME : ");
                fflush(stdout);
                gets(buffer);
                strcpy(g.grp_name, buffer);

                printf("Group NUM :");
                fflush(stdout);
                gets(buffer);
                g.grp_num = atoi(buffer);

                if ((br = sm_get_best_route(host, type, &g)) !=
                    ((struct BEST_ROUTE *) NULL))
                {
                    printf("Found Best Route : host %s link %s serv %s\n",
                           br->hostname, br->link, br->service);
                }
                else
                {
                    printf("Best Route Not found sm_errno = %d\n", sm_errno);
                }
            }
            if ( type == X25_NUA )
            {
                printf("LINK : ");
                fflush(stdout);
                gets(buffer);
                strcpy(link_num, buffer);

                if (buffer[0] == '\n')
                {
                    link_num[0] = '\0';
                }

                if ((br = sm_get_best_route(host, type, link_num)) !=
                    ((struct BEST_ROUTE *) NULL))
                {
                    printf("Found Best Route : host %s link %s serv %s\n",
                           br->hostname, br->link, br->service);
                }
                else
                {
                    printf("Best Route Not found sm_errno = %d\n", sm_errno);
                }
            }
        }
        else if ( strcmp(command, "incr") == 0 )
        {
            printf("Inserisci HOSTNAME : ");
            fflush(stdout);
            gets(host);

            printf("LINK : ");
            fflush(stdout);
            gets(link);

            rc = sm_incr_route(host, link);
            debug((1, "sm_incr_route rc = %d\n", rc));
        }
        else if ( strcmp(command, "free") == 0 )
        {
            printf("Inserisci HOSTNAME : ");
            fflush(stdout);
            gets(host);

            printf("LINK : ");
            fflush(stdout);
            gets(link);

            rc = sm_free_route(host, link);
            printf("sm_free_route rc = %d\n", rc);
        }
        else if ( strcmp(command, "delete") == 0 )
        {
            printf("Inserisci HOSTNAME : ");
            fflush(stdout);
            gets(host);

            printf("LINK : ");
            fflush(stdout);
            gets(link);

            rc = sm_delete_route(host, link);
            debug((1, "sm_delete_route rc = %d\n", rc));
        }
        else if (strcmp(command, "exit") == 0)
        {
            break;
        }
        else
        {
            printf("Comando errato\n");
            printf("I comandi disponibili sono:\n");
            printf("list add incr free delete best debug enddebug exit dump load\n");
        }
    }
#ifdef  DEBUG
    enddebug();
#endif
}

