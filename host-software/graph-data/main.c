#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <cgi.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "local.h"

int main( int argc, char **argv )
{
    char   *param;
    int     sensornum;
    int     subaddr;
    int     days;
    int     type;

    cgi_init();
    cgi_process_form();

    param = cgi_param("sensor");
    sensornum = param ? atoi(param) : 255;

    param = cgi_param("type");
    type = param ? atoi(param) : S_RECEIVER;

    param = cgi_param("subaddr");
    subaddr = param ? atoi(param) : 1;

    param = cgi_param("days");
    days = param ? atoi(param) : 1;

    load_data(sensornum, subaddr, type, days );
    create_graph();

    cgi_redirect(graphuri);

    cgi_end();
    return( 0 );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
