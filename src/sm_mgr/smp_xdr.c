/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <rpc/rpc.h>
#include "smp.h"

bool_t
xdr_node(xdrs, objp)
XDR *xdrs;
node *objp;
{
    if (!xdr_string(xdrs, &objp->machine, MAX_STR))
    {
        return (FALSE);
    }
    if (!xdr_string(xdrs, &objp->link, MAX_STR))
    {
        return (FALSE);
    }
    if (!xdr_string(xdrs, &objp->service, MAX_STR))
    {
        return (FALSE);
    }
    if (!xdr_int(xdrs, &objp->active))
    {
        return (FALSE);
    }
    if (!xdr_int(xdrs, &objp->max))
    {
        return (FALSE);
    }
    if (!xdr_int(xdrs, &objp->route_type))
    {
        return (FALSE);
    }
    return (TRUE);
}