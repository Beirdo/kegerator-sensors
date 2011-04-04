#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "local.h"

#define BASEDIR "/opt/poll-sensor/rrds"

typedef struct {
    sensor_t    type;
    char       *string;
} sensor_map_t;

sensor_map_t sensorMap[] = {
    { S_UNKNOWN,       "unknown" },
    { S_RECEIVER,      "receiver" },
    { S_ACCELEROMETER, "accelerometer" },
    { S_MASS,          "mass" },
    { S_TEMPERATURE,   "temperature" }
};
int sensorMapCount = sizeof(sensorMap) / sizeof(sensorMap[0]);

typedef struct {
    uint8_t     address;
    sensor_t    type;
    uint8_t    *version;
} sensortype_t;

typedef struct {
    uint8_t  address;
    uint8_t  length;    /* From address to end of CRC16 */
    uint8_t  get_set;   /* 1 = set, 0 = get */
    uint8_t  subaddress;
    uint8_t  data[1];   /* actually of size length - 6 */
    uint16_t crc16;
} message;

sensortype_t sensors[17];   /* 0-15 are sensors, 16 is the receiver */
int sensor_count;


#define MAX_BUF_LEN 32

int sensor_setup(void)
{
    int i;
    uint8_t buf[MAX_BUF_LEN];
    message *msg;
    int len;
    int index;

    memset( &sensors[0], 0, sizeof(sensors) );
    sensor_count = 0;

    for( i = 0, index = 0; i < 256; i++ )
    {
        if( i == 16 )
            i = 255;

        printf("Setup Poll - Sensor Address 0x%02X\n", i);
        len = sensor_send( buf, MAX_BUF_LEN, i, 0, 0, NULL, 0 );
        if( len > 0 )
        {
            buffer_dump("Received: ", buf, len);
            msg = (message *)buf;
            if( msg->address == i )
            {
                sensors[index].address = i;
                sensors[index].type = sensor_type_find(msg->data, len-6);
                sensors[index].version = sensor_version_find(msg->data, len-6);
                printf("Sensor Address 0x%02X - Type: %s, Firmware %s\n",
                       sensors[index].address, 
                       sensorMap[sensors[index].type].string,
                       sensors[index].version);
                index++;
                sensor_count++;
            }
        }
    }

    return( sensor_count );
}

int sensor_send( uint8_t *buf, uint8_t maxlen, uint8_t address, uint8_t get_set,
                 uint8_t subaddress, uint8_t *data, uint8_t len )
{
    message *msg;

    msg = (message *)buf;
    msg->address    = address;
    msg->length     = 6 + len;
    msg->get_set    = get_set;
    msg->subaddress = subaddress;
    if( data && len )
        memcpy(msg->data, data, len);

    len = serial_write(buf, 4 + len);
    if( len <= 0 )
        return( len );

    return( serial_read(buf, maxlen) );
}

void buffer_dump(char *pre, uint8_t *buf, int len)
{
    int i;

    for( i = 0; i < len; i++ )
    {
        if( i % 16 == 0 ) {
            printf( "\n%s", pre );
        }

        printf( "%02X ", buf[i] );
    }

    printf( "\n" );
}


sensor_t sensor_type_find(uint8_t *string, int len)
{
    char *space;
    int i;

    string[len] = '\0';

    space = strchr((char *)string, ' ');
    if( !space )
        return( S_UNKNOWN );

    len = space - (char *)string;

    for( i = 0; i < sensorMapCount; i++ )
    {
        if( sensorMap[i].string && 
            !strncmp((char *)string, sensorMap[i].string, len) )
            return( sensorMap[i].type );
    }

    return( S_UNKNOWN );
}

uint8_t *sensor_version_find(uint8_t *string, int len)
{
    char *space;

    string[len] = '\0';
    space = strchr((char *)string, ' ');
    return( (uint8_t *)++space );
}

void sensor_poll(void)
{
    uint8_t buf[MAX_BUF_LEN];
    int i;
    int len;

    for( i = 0; i < sensor_count; i++ )
    {
        switch( sensors[i].type )
        {
            case S_RECEIVER:
                len = sensor_send( buf, MAX_BUF_LEN, sensors[i].address, 0, 1, 
                                   NULL, 0 );
                if( len > 0 )
                    sensor_handle( S_RECEIVER, buf );
                break;
            case S_ACCELEROMETER:
                len = sensor_send( buf, MAX_BUF_LEN, sensors[i].address, 0, 1, 
                                   NULL, 0 );
                if( len > 0 )
                    sensor_handle( S_ACCELEROMETER, buf );

                len = sensor_send( buf, MAX_BUF_LEN, sensors[i].address, 0, 2, 
                                   NULL, 0 );
                if( len > 0 )
                    sensor_handle( S_ACCELEROMETER, buf );
                break;
            case S_MASS:
                len = sensor_send( buf, MAX_BUF_LEN, sensors[i].address, 0, 1, 
                                   NULL, 0 );
                if( len > 0 )
                    sensor_handle( S_MASS, buf );
                break;
            case S_TEMPERATURE:
                len = sensor_send( buf, MAX_BUF_LEN, sensors[i].address, 0, 1, 
                                   NULL, 0 );
                if( len > 0 )
                    sensor_handle( S_TEMPERATURE, buf );
                break;
            default:
                break;
        }
    }
}

void sensor_handle( sensor_t type, uint8_t *buf )
{
    char template[] = BASEDIR "/sensor_%01X_%1d.rrd";
    char filename[] = BASEDIR "/sensor_X_X.rrd";
    message *msg;
    int len;
    FILE *fp;
    struct timeval tv;
    uint16_t word;
    uint32_t dword;
    float temperature;
    int i;
    int j;

    msg = (message *)buf;

    sprintf(filename, template, msg->address, msg->subaddress);
    len = msg->length - 6;

    gettimeofday(&tv, NULL);

    fp = fopen( filename, "a" );
    if( !fp )
        return;

    switch( type )
    {
        case S_RECEIVER:
        case S_TEMPERATURE:
            if( len < 2 )
                return;
            word = (msg->data[0] << 8) | msg->data[1];
            temperature = ((float)word) / 256.0;
            fprintf(fp, "%ld,%7.3f\n", tv.tv_sec, temperature);
            break;
        case S_MASS:
            if( len < 2 )
                return;
            word = (msg->data[0] << 8) | msg->data[1];
            fprintf(fp, "%ld,%d\n", tv.tv_sec, word);
            break;
        case S_ACCELEROMETER:
            if( len < 10 )
                return;

            dword = 0;
            for( i = 0; i < 4; i++ ) {
                dword <<= 8;
                dword |= msg->data[i];
            }
            fprintf(fp, "%ld,", (long int)dword);
            for( i = 0; i < 3; i++ ) {
                word = 0;
                for( j = 0; j < 2; j++ ) {
                    word <<= 8;
                    word |= msg->data[4+(i*2)+j];
                }
                fprintf(fp, "%d,", word );
            }
            fprintf(fp, "\n");
            break;
        default:
            break;
    }

    fclose( fp );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
