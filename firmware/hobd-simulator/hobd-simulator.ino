

#include <stdint.h>

#define PIN_LED (6)

#define HOBD_MSG_HEADER_SIZE (3)
#define HOBD_MSG_CHECKSUM_SIZE (1)
#define HOBD_MSG_DATA_SIZE_MAX (251)
#define HOBD_MSG_SIZE_MIN (4)
#define HOBD_MSG_SIZE_MAX (255)

#define HOBD_MSG_TYPE_INVALID (0x00)
#define HOBD_MSG_TYPE_QUERY (0x72)
#define HOBD_MSG_TYPE_RESPONSE (0x02)
#define HOBD_MSG_TYPE_WAKE_UP (0xFE)

#define HOBD_MSG_SUBTYPE_INVALID (0xAA)
#define HOBD_MSG_SUBTYPE_WAKE_UP (0xFF)
#define HOBD_MSG_SUBTYPE_INIT (0x00)
#define HOBD_MSG_SUBTYPE_TABLE_SUBGROUP (0x72)
#define HOBD_MSG_SUBTYPE_TABLE (0x71)

#define HOBD_INIT_DATA (0xF0)

#define HOBD_TABLE_0 (0x00)
#define HOBD_TABLE_16 (0x10)
#define HOBD_TABLE_32 (0x20)
#define HOBD_TABLE_209 (0xD1)


typedef struct
{
    uint8_t type;
    uint8_t size;
    uint8_t subtype;
} hobd_msg_header_s;


typedef struct
{
    hobd_msg_header_s header;
    uint8_t data[0];
    // checksum
} hobd_msg_s;


typedef struct
{
    uint8_t table;
    uint8_t offset;
    uint8_t count;
} hobd_data_table_query_s;


typedef struct
{
    uint8_t table;
    uint8_t offset;
    uint8_t data[0];
} hobd_data_table_response_s;


typedef struct
{
    uint8_t data;
} hobd_data_init_s;


typedef struct
{
    uint16_t engine_rpm;
    uint8_t tps_volt;
    uint8_t tps_percent;
    uint8_t ect_volt;
    uint8_t ect_temp;
    uint8_t iat_volt;
    uint8_t iat_temp;
    uint8_t map_volt;
    uint8_t map_pressure;
    uint8_t reserved_0;
    uint8_t reserved_1;
    uint8_t battery_volt;
    uint8_t wheel_speed;
    uint16_t fuel_injectors;
} hobd_table_16_s;


typedef struct
{
    uint8_t gear;
    uint8_t reserved_0;
    uint8_t reserved_1;
    uint8_t reserved_2;
    uint8_t engine_on;
} hobd_table_209_s;


static hobd_table_16_s table_16_data;
static hobd_table_209_s table_209_data;

static uint8_t buffer[HOBD_MSG_SIZE_MAX];


static uint8_t obd_checksum(
        const uint8_t * const buffer,
        const uint16_t len )
{
    uint16_t cs = 0;
    uint16_t idx = 0;

    for( idx = 0; idx < len; idx += 1 )
    {
        cs += (uint16_t) buffer[ idx ];
    }

    if( cs > 0x0100 )
    {
        cs = (0x0100 - (cs & 0x00FF));
    }
    else
    {
        cs = (0x0100 - cs);
    }

    return (uint8_t) (cs & 0xFF);
}


void send_init_response( void )
{
    hobd_msg_s * const msg =
            (hobd_msg_s*) buffer;

    msg->header.type = HOBD_MSG_TYPE_RESPONSE;
    msg->header.size = HOBD_MSG_HEADER_SIZE + HOBD_MSG_CHECKSUM_SIZE;
    msg->header.subtype = HOBD_MSG_SUBTYPE_INIT;

    uint8_t * const checksum = &msg->data[0];

    (*checksum) = obd_checksum(buffer, HOBD_MSG_HEADER_SIZE);

    Serial1.write(buffer, msg->header.size);
}


void send_table_16_request( void )
{
    hobd_msg_s * const msg =
            (hobd_msg_s*) buffer;

    msg->header.type = HOBD_MSG_TYPE_QUERY;
    msg->header.size = HOBD_MSG_HEADER_SIZE + HOBD_MSG_CHECKSUM_SIZE;
    msg->header.subtype = HOBD_MSG_SUBTYPE_TABLE_SUBGROUP;

    hobd_data_table_query_s * const query =
            (hobd_data_table_query_s*) &msg->data[0];

    msg->header.size += sizeof(*query);

    query->table = HOBD_TABLE_16;
    query->offset = 0;
    query->count = 17;

    uint8_t * const checksum = &msg->data[sizeof(*query)];

    (*checksum) = obd_checksum(buffer, msg->header.size - 1);

    Serial1.write(buffer, msg->header.size);
}


void send_table_16_response( void )
{
    hobd_msg_s * const msg =
            (hobd_msg_s*) buffer;

    msg->header.type = HOBD_MSG_TYPE_RESPONSE;
    msg->header.size = HOBD_MSG_HEADER_SIZE + HOBD_MSG_CHECKSUM_SIZE;
    msg->header.subtype = HOBD_MSG_SUBTYPE_TABLE_SUBGROUP;

    hobd_data_table_response_s * const resp =
            (hobd_data_table_response_s*) &msg->data[0];

    const uint8_t data_size = (sizeof(*resp) + sizeof(table_16_data));
    msg->header.size += data_size;

    resp->table = HOBD_TABLE_16;
    resp->offset = 0;

    (void) memcpy(&resp->data[0], &table_16_data, sizeof(table_16_data));

    uint8_t * const checksum = &msg->data[data_size];

    (*checksum) = obd_checksum(buffer, msg->header.size - 1);

    Serial1.write(buffer, msg->header.size);
}


void send_table_209_request( void )
{
    hobd_msg_s * const msg =
            (hobd_msg_s*) buffer;

    msg->header.type = HOBD_MSG_TYPE_QUERY;
    msg->header.size = HOBD_MSG_HEADER_SIZE + HOBD_MSG_CHECKSUM_SIZE;
    msg->header.subtype = HOBD_MSG_SUBTYPE_TABLE_SUBGROUP;

    hobd_data_table_query_s * const query =
            (hobd_data_table_query_s*) &msg->data[0];

    msg->header.size += sizeof(*query);

    query->table = HOBD_TABLE_209;
    query->offset = 0;
    query->count = 6;

    uint8_t * const checksum = &msg->data[sizeof(*query)];

    (*checksum) = obd_checksum(buffer, msg->header.size - 1);

    Serial1.write(buffer, msg->header.size);
}


void send_table_209_response( void )
{
    hobd_msg_s * const msg =
            (hobd_msg_s*) buffer;

    msg->header.type = HOBD_MSG_TYPE_RESPONSE;
    msg->header.size = HOBD_MSG_HEADER_SIZE + HOBD_MSG_CHECKSUM_SIZE;
    msg->header.subtype = HOBD_MSG_SUBTYPE_TABLE_SUBGROUP;

    hobd_data_table_response_s * const resp =
            (hobd_data_table_response_s*) &msg->data[0];

    const uint8_t data_size = (sizeof(*resp) + sizeof(table_209_data));
    msg->header.size += data_size;

    resp->table = HOBD_TABLE_209;
    resp->offset = 0;

    (void) memcpy(&resp->data[0], &table_209_data, sizeof(table_209_data));

    uint8_t * const checksum = &msg->data[data_size];

    (*checksum) = obd_checksum(buffer, msg->header.size - 1);

    Serial1.write(buffer, msg->header.size);
}


void setup( void )
{
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);
    Serial1.begin(115200);

    (void) memset(&table_16_data, 0, sizeof(table_16_data));
    (void) memset(&table_209_data, 0, sizeof(table_209_data));

    delay(120);

    send_init_response();

    digitalWrite(PIN_LED, LOW);

    delay(50);
}

void loop( void )
{
    digitalWrite(PIN_LED, HIGH);

    send_table_16_request();

    delay(5);

    send_table_16_response();

    delay(50);

    send_table_209_request();

    delay(5);

    send_table_209_response();
    
    digitalWrite(PIN_LED, LOW);

    delay(200);

    table_209_data.engine_on = 1;
    table_209_data.gear = 1;

    table_16_data.engine_rpm += 10;
    table_16_data.wheel_speed = 1;
    table_16_data.tps_volt = 1;
    table_16_data.tps_percent = 1;
}
