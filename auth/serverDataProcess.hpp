// Add this header file
#ifndef SERVER_DATA_PROCESS_H
#define SERVER_DATA_PROCESS_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include<iostream>
#include<vector>
#include <string>     // Required for std::string

#ifdef __cplusplus
extern "C" {
#endif

#define typeframe unsigned long long int

typedef struct
{
    char Header[10];
    char Key[100];
    uint8_t devIdSeperator;
    uint8_t packetType;
    uint8_t getTypeId;
    uint8_t setTypeId;
    uint8_t dataSeperator;
    uint8_t footer;
    uint8_t valueSeperator;
    uint8_t checksum;
    /* data */
} DEV_Config_Status;

// Define enums and structs
typedef enum {
    Config_UUID,
    Config_SEED,
    Config_VALIDATE,
    CONFIG_COUNT
} ConfigParametersID;

typedef enum
{
    CONFIG_SET =0xDB ,
    CONFIG_GET =0xDA,
    CONFIG_VALIDATE=0xDC,
    CONFIG_SET_GET = 3,
    CONFIG_SET_GET_TEST = 4,
    CONFIG_TEST = 5,
    CONFIG_NOTHING = 6
} ConfigSetGet;

typedef struct
{
    ConfigParametersID key;
    char *keyString;
    char *defaultValue;
    uint8_t size;
    ConfigSetGet ConfigSet;
    bool (*callback)(void *input, void *output, ConfigSetGet SetGetTest);
    bool isAllocated;
} ConfigItem;

typedef struct {
    ConfigItem valueOfConfig[CONFIG_COUNT];
    int totalData;
} ConfigTable;

typedef enum{
    SET_METHOD_IS_NOT_ALLOWED = 0x01,
    GET_METHOD_IS_NOT_ALLOWED,
    ERROR_INVALID_INPUT_START,
    ERROR_INVALID_INPUT_END,
    ERROR_START_OUT_OF_RANGE,
    ERROR_END_OUT_OF_RANGE,
    ERROR_OPENING_DIR,
    ERROR_NOT_A_PROPER_FILE,
    ERROR_GETTING_FILE_INFO,
    ERROR_FILE_NOT_FOUND,
    ERROR_FILE_NOT_DELETED,
    ERROR_INVALID_TIME,
    ERROR_INVALID_ROI,
    ERROR_INVALID_SDK_KEY ,
    ERROR_INVALID_DEVICE_ID
    }eErrorCode;

extern ConfigTable configTable;
extern DEV_Config_Status deviceConPacket;
extern bool client_validated;  // Declare global flag

// Function declarations
bool parseData(uint8_t* dataBuffer, uint8_t* dataToSend);
int splitHexString(uint8_t* string, uint8_t delimiter, uint8_t opbuffer[][100], size_t bufsize);
std::vector<uint8_t> encrypt(const std::string &plaintext, const std::string &seed);
std::string zen_device_seed_generation(uint8_t length);
bool zen_uuid_manage(void* inputBuffer, void* outputBuffer, ConfigSetGet method);
bool zen_sdk_key_manage(void* inputBuffer, void* outputBuffer, ConfigSetGet method);
void print_hex(const uint8_t *buffer, size_t length);

#ifdef __cplusplus
}
#endif

#endif // SERVER_DATA_PROCESS_H
