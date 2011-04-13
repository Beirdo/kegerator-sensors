#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "local.h"

#define BASEDIR "/opt/poll-sensor/data"

typedef struct {
    sensor_t    type;
    char       *string;
    int         maxsubaddr;
} sensor_map_t;

sensor_map_t sensorMap[] = {
    { S_UNKNOWN,       "unknown",       1 },
    { S_RECEIVER,      "receiver",      1 },
    { S_ACCELEROMETER, "accelerometer", 2 },
    { S_MASS,          "mass",          1 },
    { S_TEMPERATURE,   "temperature",   1 }
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

sensortype_t sensor;    /* 0-15 are sensors, 16 is the receiver */
int sensor_count;


#define MAX_BUF_LEN 32

int sensor_setup(int sensornum)
{
    int i;
    uint8_t buf[MAX_BUF_LEN];
    message *msg;
    int len;

    memset( &sensor, 0, sizeof(sensor) );
    sensor_count = 0;

    i = sensornum;

    printf("\nSetup Poll - Sensor Address 0x%02X\n", i);
    len = sensor_send( buf, MAX_BUF_LEN, i, 0, 0, NULL, 0 );
    if( len > 0 )
    {
        buffer_dump("Received: ", buf, len);
        msg = (message *)buf;
        if( msg->address == i )
        {
            sensor.address = i;
            sensor.type = sensor_type_find(msg->data, len-6);
            sensor.version = sensor_version_find(msg->data, len-6);
            printf("Sensor Address 0x%02X - Type: %s, Firmware %s\n",
                   sensor.address, 
                   sensorMap[sensor.type].string,
                   sensor.version);
            sensor_count++;
        }
    } 

    serial_close();

    if( sensor_count )
    {
        if( sensor.type != S_MASS ) {
            printf("Not a mass sensor!\n");
            return( 0 );
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
            printf( "%s%s", (i ? "\n" : ""), pre );
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

uint16_t sensor_poll(int regnum)
{
    uint8_t buf[MAX_BUF_LEN];
    int len;
    uint16_t value;
    message *msg;

    serial_setup();

    len = sensor_probe( buf, MAX_BUF_LEN, sensor.address, regnum );
    serial_close();

    if( len < 0 )
        return(0xFFFF);
    
    msg = (message *)buf;
    value = (msg->data[0] << 8) | (msg->data[1]);

    return( value );
}

int sensor_probe( uint8_t *buf, int maxlen, int address, int subaddr )
{
    int len;

    printf( "\nPolling %s sensor %02X, subaddr %d\n", 
            sensorMap[sensor.type].string, address, subaddr );

    len = sensor_send( buf, maxlen, address, 0, subaddr, NULL, 0 );
    if( len > 0 )
    {
        buffer_dump("Received: ", buf, len);
        printf("\n");
    }

    return( len );
}

uint8_t sensor_poll_reg(int regnum)
{
    uint8_t buf[MAX_BUF_LEN];
    int len;
    uint8_t value;
    message *msg;

    serial_setup();

    len = sensor_probe( buf, MAX_BUF_LEN, sensor.address, regnum );
    serial_close();

    if( len < 0 )
        return(0xFF);
    
    msg = (message *)buf;
    value = msg->data[0];

    return( value );
}

void sensor_set_reg(int regnum, int value)
{
    uint8_t buf[MAX_BUF_LEN];
    int len;

    serial_setup();

    len = sensor_set( buf, MAX_BUF_LEN, sensor.address, regnum, value );
    serial_close();
}

int sensor_set( uint8_t *buf, int maxlen, int address, int subaddr, int value )
{
    int len;
    uint8_t data[2];

    printf( "\nSetting %s sensor %02X, subaddr %d to %d\n", 
            sensorMap[sensor.type].string, address, subaddr, value );

    switch( subaddr )
    {
        case 1:
        case 4:
        case 5:
            data[0] = (value >> 8);
            data[1] = (value & 0xFF);
            len = 2;
            break;
        case 2:
        case 3:
        default:
            data[0] = (value & 0xFF);
            len = 1;
            break;
    }

    len = sensor_send( buf, maxlen, address, 1, subaddr, data, len );
    if( len > 0 )
    {
        buffer_dump("Received: ", buf, len);
        printf("\n");
    }

    return( len );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
