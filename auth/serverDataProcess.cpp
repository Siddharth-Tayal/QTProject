#include "serverDataProcess.hpp"
#include <stdio.h>
#include <random>
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <cstdlib> // Required for std::string
using std::string;
using namespace std;
uint8_t splitTmpBuffer[30][100] = {};
uint8_t defaulROI[100] = {'4', '2', '0'};
uint8_t tempDataBuffer[1024] = {};
bool client_validated = false;   // 👈 real definition
string deviceId = "KvVTBQTA8JWq"; // fetch from the device
string sdkKey = "b71N6hMNDOYR0YzkyfZKjgga";        // is created with .so file
string deviceSeed = "";          // device seed is the random generated dependent on the algorithm

bool isDeviceIdValidated = false;
bool isSdkKeyValidated = false;
ConfigTable configTable = {
    {
        {Config_UUID, "UUID", (char *)defaulROI, 16, CONFIG_GET, zen_uuid_manage},
        {Config_SEED, "SEED", (char *)defaulROI, 16, CONFIG_GET, nullptr},
        {Config_VALIDATE, "Config_VALIDATE", (char *)defaulROI, 16, CONFIG_VALIDATE, zen_sdk_key_manage},
    },
    CONFIG_COUNT};

DEV_Config_Status deviceConPacket = {
    "$ZEN", // Header
    "1234", // Key
    0xD5,   // devIdSeperator
    0x00,   // packetType (this field exists in the struct but wasn't initialized)
    0xDA,   // getTypeId
    0xDC,   // setTypeId
    0xD7,   // dataSeperator
    0xD1,   // footer
    0xD9,   // valueSeperator
    0xEE    // checksum
};
std::string zen_device_seed_generation(uint8_t length)
{
    const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string seed;
    seed.reserve(length);

    // Use random device and engine
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<> dist(0, charset.size() - 1);

    for (uint8_t i = 0; i < length; ++i)
    {
        seed += charset[dist(rng)];
    }

    return seed;
}
void print_hex(const uint8_t *buffer, size_t length)
{
    printf("\n");
    for (size_t i = 0; i < length; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}
std::vector<int> generateKeySequence(const std::string &seed, size_t length)
{
    std::vector<int> key;
    unsigned int numeric_seed = 0;

    // Calculate the initial seed as the sum of ASCII values of the characters in the seed
    for (char c : seed)
    {
        numeric_seed += static_cast<unsigned int>(c);
    }

    // Generate the key sequence using a simple LCG
    for (size_t i = 0; i < length; ++i)
    {
        numeric_seed = (numeric_seed * 31 + 7) % 256; // Simple LCG
        key.push_back(numeric_seed);
    }

    return key;
}

std::string bytesToHex(const std::vector<uint8_t> &bytes)
{
    std::ostringstream hexStream;
    for (uint8_t byte : bytes)
    {
        hexStream << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    return hexStream.str();
}

std::vector<uint8_t> encrypt(const std::string &plaintext, const std::string &seed)
{
    std::vector<int> key = generateKeySequence(seed, plaintext.size());
    std::vector<uint8_t> ciphertext;
    for (size_t i = 0; i < plaintext.size(); ++i)
    {
        ciphertext.push_back(static_cast<uint8_t>(plaintext[i] ^ key[i]));
    }
    return ciphertext;
}

std::vector<uint8_t> hexToBytes(const std::string &hex)
{
    std::vector<uint8_t> bytes;
    std::istringstream hexStream(hex);
    std::string byteString;
    while (hexStream >> byteString)
    {
        bytes.push_back(static_cast<uint8_t>(std::stoi(byteString, nullptr, 16)));
    }
    return bytes;
}


std::string decrypt(const std::vector<uint8_t> &ciphertext, const std::string &seed)
{
    std::vector<int> key = generateKeySequence(seed, ciphertext.size());
    std::string decrypted;
    for (size_t i = 0; i < ciphertext.size(); ++i)
    {
        decrypted += static_cast<char>(ciphertext[i] ^ key[i]);
    }
    return decrypted;
}
// Parse data function is used to parse the request and sending the final response
bool parseData(uint8_t *dataBuffer, uint8_t *dataToSend)
{
    size_t buffer_length = strlen((char *)dataBuffer);

    for (size_t i = 0; i < buffer_length; ++i) {
        printf("%02X ", dataBuffer[i]);
    }
    printf("\n");
    memset(tempDataBuffer, 0, sizeof(tempDataBuffer));
    isSdkKeyValidated = false;
    isDeviceIdValidated = false;
    client_validated = false;
    // size_t buffer_length = strlen((char *)dataBuffer);
    // printf("Data buffer in hex: %s\n\n\n", dataBuffer);
    // print_hex(dataBuffer, buffer_length);
    memcpy(tempDataBuffer, dataBuffer, buffer_length);


    if (deviceSeed.length() == 7)
    {
        string encryptedHexStr = (char*)dataBuffer;
        // printf("Encrypted strings : %s\n",encryptedHexStr.c_str());
        // parse to the hex
        std::vector<uint8_t> encryptedBytes(dataBuffer, dataBuffer + buffer_length);
        // printf("Encrypted Bytes: ");
        // print_hex(encryptedBytes.data(), encryptedBytes.size());
        string decrypted = decrypt(encryptedBytes, deviceSeed);
        // printf("Decrypted hex : %s\n",decrypted);
        // printf("The decrypted string : %s\n", decrypted.c_str());
        memcpy(tempDataBuffer, decrypted.c_str(), decrypted.length());
        deviceSeed = "";
    }
    // printf("The data buffer temp : %s\n", tempDataBuffer);
    // Check for null character in the data buffer
    if (strstr((char *)tempDataBuffer, deviceConPacket.Header) == NULL)
    {
        char errTemp[100] = {};
        sprintf((char *)errTemp, "%02x%c%s", ERROR_INVALID_INPUT_START, deviceConPacket.valueSeperator, "ERROR_INVALID_INPUT_START");
        strcat((char *)dataToSend, errTemp);
        return false;
    }
    memset(splitTmpBuffer, 0, sizeof(splitTmpBuffer));

    // Check wheter the Request buffer contain $ZEN

    int recLength = splitHexString(tempDataBuffer, deviceConPacket.dataSeperator, splitTmpBuffer, 25);
    // printf("\nRecLength is %d\n", recLength);
    if (recLength < 3)
    {
        strcat((char *)dataToSend, "err:Data is less\n");
        return false;
    }

    // seed validation
    std::string seedStr = (char *)splitTmpBuffer[1];
    if (seedStr.length() != 7)
    {
        // printf("Invalid seed length: must be exactly 7 characters\n");
        return false;
    }

    // response packet formation
    sprintf((char *)dataToSend, "%s%c%c%c", deviceConPacket.Header, deviceConPacket.dataSeperator, splitTmpBuffer[2][0], deviceConPacket.dataSeperator);

    // printf("The split time buffer : %s", splitTmpBuffer[3]);

    uint8_t responseData[2000] = {};

    for (int i = 3; i < recLength - 1; i++)
    {
        uint8_t key = splitTmpBuffer[i][1] - 1;
        // printf("\n %d key ", key);

        if (key > -1 && key < CONFIG_COUNT)
        {
            // printf("\nConfig come is D = %d  H = %02x s = %s\n", key, key, configTable.valueOfConfig[key].keyString);
            sprintf((char *)responseData, "%c%c%c", splitTmpBuffer[i][0], splitTmpBuffer[i][1], deviceConPacket.devIdSeperator);
            if (configTable.valueOfConfig[key].callback != NULL)
            {
                bool status = configTable.valueOfConfig[key].callback(&splitTmpBuffer[i][3], (void *)responseData, (ConfigSetGet)splitTmpBuffer[2][0]);
                if (status == false)
                {
                    // return false;
                    strcat((char *)responseData, "err");
                    // return false;
                }
            }
            else
            {
                // sprintf((char*)responseData, "%c%c%c%s%c", splitTmpBuffer[3][0], splitTmpBuffer[3][1], deviceConPacket.devIdSeperator,configTable.valueOfConfig[key].defaultValue,deviceConPacket.dataSeperator);
                strcat((char *)responseData, configTable.valueOfConfig[key].defaultValue);
            }
            sprintf((char *)&responseData[strlen((char *)responseData)], "%c", deviceConPacket.dataSeperator);
            strcat((char *)dataToSend, (char *)responseData);
        }
        else
        {
            char erro[100] = {};
            sprintf((char *)erro, "%c%c%c%s%c", splitTmpBuffer[i][0], splitTmpBuffer[i][1], deviceConPacket.devIdSeperator, "err", deviceConPacket.dataSeperator);
            // printf(" error message is %s", erro);
            strcat((char *)dataToSend, erro);
        }
    }

    // checksum formation
    char checksum[10] = "34\xd1\r\n";
    strcat((char *)dataToSend, checksum);
    printf("The generated data to send is: %s\n", dataToSend);

    // print_hex(dataToSend, strlen((char *)dataToSend));
    const string seed = (char *)splitTmpBuffer[1];
    // printf("THe seed : %s\n", seed.c_str());
    const string dataBufferString = (char *)dataToSend;
    // encrypt function
    vector<uint8_t> encryptedData = encrypt(dataBufferString, seed);
    printf("The encrypted packet: ");
    print_hex(encryptedData.data(), encryptedData.size());
    memset(dataToSend, 0, sizeof(dataToSend)); // Clear the buffer
    memcpy(dataToSend, encryptedData.data(), encryptedData.size());
    printf("The final data to send : %s\n",dataToSend);
    printf("Data to send (encrypted): ");
    print_hex(dataToSend, encryptedData.size());
    print_hex(dataToSend, strlen((char *)dataToSend));
    return true;
}

extern "C" int splitHexString(uint8_t *string, uint8_t delimiter, uint8_t opbuffer[][100], size_t bufsize)
{
    uint8_t *start = string;
    uint8_t *end = NULL;
    int count = 0;

    while ((end = (uint8_t *)memchr(start, delimiter, strlen((char *)start))) != NULL)
    {
        size_t length = end - start; // Calculate substring length
        if (length > 0 && count < bufsize)
        {
            memset(opbuffer[count], 0, 100);        // Clear the target buffer
            memcpy(opbuffer[count], start, length); // Copy the substring
            count++;
        }
        start = end + 1; // Move past the delimiter
    }

    // Copy the remaining part after the last delimiter
    if (*start != '\0' && count < bufsize)
    {
        memset(opbuffer[count], 0, 100); // Clear the target buffer
        strcpy((char *)opbuffer[count], (char *)start);
        count++;
    }

    return count; // Return the number of substrings
}
// std::vector<int> generateKeySequence(const std::string &seed, size_t length) {
//     std::vector<int> key;
//     unsigned int numeric_seed = std::stoi(seed);
//     srand(numeric_seed); // Seed the random number generator
//     for (size_t i = 0; i < length; ++i) {
//         key.push_back(rand() % 256);
//     }
//     return key;
// }

// std::vector<int> generateKeySequence(const string &seed, size_t length)
// {
//     vector<int> key;
//     unsigned int numeric_seed = stoi(seed);
//     for (size_t i = 0; i < length; ++i)
//     {
//         numeric_seed = (numeric_seed * 31 + 7) % 256; // Simple LCG
//         key.push_back(numeric_seed);
//     }
//     return key;
// }

// // // Encrypt the string using generated key
// string encrypt(const string &plaintext, const string &seed)
// {
//     vector<int> key = generateKeySequence(seed, plaintext.length());
//     string encrypted;
//     for (size_t i = 0; i < plaintext.length(); ++i)
//     {
//         encrypted += plaintext[i] ^ key[i];
//     }
//     print_hex((uint8_t *)encrypted.c_str(), strlen(encrypted.c_str()));
//     printf("Encrypted : %s\n", encrypted);
//     return encrypted;
// }

bool zen_uuid_manage(void *inputBuffer, void *outputBuffer, ConfigSetGet method)
{
    // printf("zen_roi_manage called data %2x\n", method);
    if (method == deviceConPacket.getTypeId)
    {
        uint8_t *inputPointer = (uint8_t *)inputBuffer;
        std::string seedStr = (char *)splitTmpBuffer[1];
        // printf("received Inside the Roi= %s \n", inputPointer);
        if (true)
        {
            deviceSeed = zen_device_seed_generation(7);
            // printf("Device seed : %s\n",deviceSeed);
            strcat((char *)outputBuffer, deviceId.c_str());
            char tempBuffer[100] = {};
            sprintf((char *)tempBuffer, "%c%c%c%c%s", 0XD7, 0X10, 0X02, 0XD5, deviceSeed.c_str());
            strcat((char *)outputBuffer, tempBuffer);
        }
        else
        {
            char errTemp[100] = {};
            sprintf((char *)errTemp, "%02x%c%s", ERROR_INVALID_ROI, deviceConPacket.valueSeperator, "ERROR_INVALID_ROI");
            strcat((char *)outputBuffer, errTemp);
        }
        return true;
    }
    else if (method == deviceConPacket.setTypeId)
    {
        std::string seedStr = (char *)splitTmpBuffer[1];
        std::string receivedDeviceId = (char *)inputBuffer;
        printf("The extracted devicei id  key : %s",receivedDeviceId.c_str());

        if (receivedDeviceId.c_str() == deviceId)
        {
            isDeviceIdValidated = true;
            if (isSdkKeyValidated)
            {
                printf("We are inside the sdk key validation\n");
                client_validated = true;
                printf("Client Validated....\n");
                deviceSeed = "";
            }
            string messageResponse = "Good";
            strcat((char *)outputBuffer, messageResponse.c_str());
        }
        else
        {
            char errTemp[100] = {};
            sprintf((char *)errTemp, "%02x%c%s", ERROR_INVALID_DEVICE_ID, deviceConPacket.valueSeperator, "ERROR_INVALID_DEVICE_ID");
            strcat((char *)outputBuffer, errTemp);
        }
    }
    else
        return false;
    return true;
}

bool zen_sdk_key_manage(void *inputBuffer, void *outputBuffer, ConfigSetGet method)
{
    if (method == deviceConPacket.setTypeId)
    {

        std::string sdk_Key = (char *)inputBuffer;
        printf("The extracted sdk key : %s",sdk_Key.c_str());
        std::string seedStr = (char *)splitTmpBuffer[1];
        if (sdk_Key.c_str() == sdkKey)
        {
            // printf("Sdk key are matched");
            isSdkKeyValidated = true;
            if (isDeviceIdValidated)
            {                printf("We are inside the device id key validation\n");

                client_validated = true;
                deviceSeed = "";
                // printf("Client validated...\n");
            }
            string messageResponse = "Good";
            strcat((char *)outputBuffer, messageResponse.c_str());
        }
        else
        {
            char errTemp[100] = {};
            sprintf((char *)errTemp, "%02x%c%s", ERROR_INVALID_SDK_KEY, deviceConPacket.valueSeperator, "ERROR_INVALID_SDK_KEY");
            strcat((char *)outputBuffer, (char *)errTemp);
        }
    }
    else
    {
        return false;
    }

    return true;
}
