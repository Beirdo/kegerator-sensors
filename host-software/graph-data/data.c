#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#define __USE_GNU
#include <string.h>
#include <unistd.h>
#include "local.h"

#define BASEDIR "/opt/poll-sensor"

char *readings[] = {
    "Readings", "Temperature", "Readings", "Mass", "Temperature"
};

char *label[] = {
    "Unknown", "deg C", "A/D Reading", "g", "deg C"
};

char *rawfile = NULL;
char *datafile = NULL;
char *plotfile = NULL;
char *graphfile = NULL;
char *graphuri = NULL;


void load_data(int sensor, int addr, int type, int days)
{
    char filename[1024];
    char line[1024];
    struct timeval tv;
    time_t starttime;
    FILE *fin, *fout;
    int32_t x_mean, x_max, x_min, x_count;
    int32_t y_mean, y_max, y_min, y_count;
    int32_t z_mean, z_max, z_min, z_count;
    int64_t x_accum, y_accum, z_accum;
    time_t binsize, currbin, nextbin;
    float xf_mean, xf_min, xf_max;

    if( days < 1 )
        days = 1;

    if( days > 9999 )
        days = 9999;

    sprintf(filename, BASEDIR "/data/sensor_%02X_%1d.dat", sensor, addr);
    rawfile = strdup(filename);

    sprintf(filename, BASEDIR "/data/sensor_%02X_%1d_%d.dat", sensor, addr, 
            days);
    datafile = strdup(filename);

    sprintf(filename, BASEDIR "/plot/sensor_%02X_%1d_%d.plot", sensor, addr, 
            days);
    plotfile = strdup(filename);

    sprintf(filename, BASEDIR "/graph/sensor_%02X_%1d_%d.png", sensor, addr, 
            days);
    graphfile = strdup(filename);

    sprintf(filename, "/graph/sensor_%02X_%1d_%d.png", sensor, addr, days);
    graphuri = strdup(filename);

    gettimeofday(&tv, NULL);
    binsize = days * 24 * 60 * 60;
    starttime = tv.tv_sec - binsize;

    if( days >= 1 && days < 7 )
        binsize /= 24;
    else if( days >= 7 && days < 30 )
        binsize /= 21;
    else if( days >= 30 && days < 365 )
        binsize /= 30;
    else
        binsize /= 24;

    currbin = starttime;
    nextbin = starttime + binsize;

    x_count = y_count = z_count = 0;
    x_accum = y_accum = z_accum = 0;
    x_min = y_min = z_min = INT32_MAX;
    x_max = y_max = z_max = INT32_MIN;

    fin  = fopen(rawfile,  "r");
    fout = fopen(datafile, "w");

    while( !feof(fin) )
    {
        char *nextch, *spc, *arg;
        time_t stamp;
        int32_t x, y, z;
        float   xf;

        arg = line;
        spc = fgets(arg, 1024, fin);
        spc = strchrnul(arg, ' ');

        if( *spc ) {
            *spc = 0;
            nextch = spc + 1;
        } else {
            nextch = spc;
        }
        while( *nextch == ' ' )
            nextch++;

        stamp = atol(arg);

        if( stamp < starttime )
            continue;

        if( stamp < nextbin )
        {
            arg = nextch;
            if( !*arg )
                continue;

            spc = strchrnul(arg, ' ');

            if( *spc ) {
                *spc = 0;
                nextch = spc + 1;
            } else {
                nextch = spc;
            }
            while( *nextch == ' ' )
                nextch++;

            if( type == S_RECEIVER || type == S_TEMPERATURE )
            {
                xf = strtof(arg, NULL);
                x  = (int32_t)(xf * 1000.0);
            } else {
                x = atol(arg);
            }

            if( type == S_ACCELEROMETER )
            {
                arg = nextch;
                if( !*arg )
                    continue;

                spc = strchrnul(arg, ' ');

                if( *spc ) {
                    *spc = 0;
                    nextch = spc + 1;
                } else {
                    nextch = spc;
                }
                while( *nextch == ' ' )
                    nextch++;

                y = atol(arg);

                arg = nextch;
                if( !*arg )
                    continue;

                spc = strchrnul(arg, ' ');

                if( *spc ) {
                    *spc = 0;
                    nextch = spc + 1;
                } else {
                    nextch = spc;
                }
                while( *nextch == ' ' )
                    nextch++;

                z = atol(arg);

                y_count++;
                y_accum += y;
                y_min = (y < y_min ? y : y_min);
                y_max = (y > y_max ? y : y_max);

                z_count++;
                z_accum += z;
                z_min = (z < z_min ? z : z_min);
                z_max = (z > z_max ? z : z_max);
            }

            x_count++;
            x_accum += x;
            x_min = ((x < x_min) ? x : x_min);
            x_max = ((x > x_max) ? x : x_max);
        } else {
            if( x_count ) {
                x_mean = x_accum / x_count;
                if( type == S_RECEIVER || type == S_TEMPERATURE )
                {
                    xf_mean = x_mean / 1000.0;
                    xf_min  = x_min  / 1000.0;
                    xf_max  = x_max  / 1000.0;
                    fprintf(fout, "%ld %.3f %.3f %.3f", currbin + (binsize/2), 
                            xf_mean, xf_min, xf_max);
                }
                else
                {
                    fprintf(fout, "%ld %d %d %d", currbin + (binsize/2), x_mean,
                            x_min, x_max);
                }
                if( y_count ) {
                    y_mean = y_accum / y_count;
                    fprintf(fout, " %d %d %d", y_mean, y_min, y_max);
                    z_mean = z_accum / z_count;
                    fprintf(fout, " %d %d %d", z_mean, z_min, z_max);
                }
                fprintf(fout, "\n");
            }

            currbin = nextbin;
            nextbin += binsize;

            x_count = y_count = z_count = 0;
            x_accum = y_accum = z_accum = 0;
            x_min = y_min = z_min = INT32_MAX;
            x_max = y_max = z_max = INT32_MIN;
        }
    }

    if( x_count ) {
        x_mean = x_accum / x_count;
        if( type == S_RECEIVER || type == S_TEMPERATURE )
        {
            xf_mean = x_mean / 1000.0;
            xf_min  = x_min  / 1000.0;
            xf_max  = x_max  / 1000.0;
            fprintf(fout, "%ld %.3f %.3f %.3f", currbin + (binsize/2), 
                    xf_mean, xf_min, xf_max);
        }
        else
        {
            fprintf(fout, "%ld %d %d %d", currbin + (binsize/2), x_mean,
                    x_min, x_max);
        }
        if( y_count ) {
            y_mean = y_accum / y_count;
            fprintf(fout, " %d %d %d", y_mean, y_min, y_max);
            z_mean = z_accum / z_count;
            fprintf(fout, " %d %d %d", z_mean, z_min, z_max);
        }
        fprintf(fout, "\n");
    }

    fclose(fin);
    fclose(fout);

    /* Generate the gnuplot file */
    fout = fopen(plotfile, "w");

    if( type == S_ACCELEROMETER ) {
        sprintf(line, ", \\\n'%s' using 1:5:6:7 with yerrorlines, \\\n"
                      "'%s' using 1:8:9:10 with yerrorlines",
                      datafile, datafile);
    } else {
        line[0] = '\0';
    }

    fprintf(fout, "set title \"%s at Sensor %d\"\n"
                  "set xlabel \"\\nTime (GMT)\"\n"
                  "set ylabel \"%s\"\n", readings[type], sensor, label[type]);
    fprintf(fout, "set xdata time\nset timefmt \"%%s\"\nset format x \"%s\"\n"
                  "set xtics rotate\nset grid\nset key left\n"
                  "set terminal png tiny size 240,200\n"
                  "set output \"%s\"\n"
                  "plot '%s' using 1:2:3:4 with yerrorlines%s\n",
                  (days == 1 ? "%H:%M" : "%m/%d\\n%H:%M"), graphfile, datafile,
                  line);
    fclose(fout);
}

void create_graph(void)
{
    int retval;
    char command[1024];

    sprintf(command, "gnuplot %s > /dev/null 2>&1 ", plotfile);

    retval = system(command);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
