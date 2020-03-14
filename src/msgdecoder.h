//#include <machine/endian.h>
//#include <sys/types.h>
static inline uint32_t get_serial(const uint8_t *const packet)
{
    uint32_t serial;
    memcpy(&serial, &packet[4], sizeof(serial));
    //return __builtin_bswap32(serial);
    return serial;
}
static inline uint16_t get_vendor(const uint8_t *const packet)
{
    uint16_t vendor;
    memcpy(&vendor, &packet[2], sizeof(vendor));
    return __builtin_bswap16(vendor);
    //return vendor;
}
static inline uint16_t get_type(const uint8_t *const packet)
{
    uint16_t type;
    type = 0;
    memcpy(&type, &packet[8], sizeof(type));
    return type;
}
static inline uint32_t get_last(const uint8_t *const packet)
{ //this value is the counter from tha last period
    uint32_t last;
    last = 0;
    memcpy(&last, &packet[16], 3);
    return last;
}
static inline uint32_t get_current(const uint8_t *const packet)
{ //this is the diff since the last period
    uint32_t curent;
    curent = 0;
    memcpy(&curent, &packet[20], 3);
    return curent;
}
static inline float get_temp1(const uint8_t *const packet)
{
    uint16_t temp1;
    memcpy(&temp1, &packet[22], sizeof(temp1));
    return (float)((float)temp1) / 100;
}
static inline float get_temp2(const uint8_t *const packet)
{
    uint16_t temp2;
    memcpy(&temp2, &packet[24], sizeof(temp2));
    return (float)((float)temp2) / 100;
}
static inline uint16_t get_actHKZ(const uint8_t *const packet)
{
    uint16_t actHKZ;
    memcpy(&actHKZ, &packet[20], sizeof(actHKZ));
    return actHKZ;
}
static inline uint16_t get_prevHKZ(const uint8_t *const packet)
{
    uint16_t prevHKZ;
    memcpy(&prevHKZ, &packet[16], sizeof(prevHKZ));
    return prevHKZ;
}