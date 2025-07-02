#include "thread_stream.hpp"
#include "../auth/serverDataProcess.hpp" // Correct path to header
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

constexpr size_t BUFFER_SIZE = 65567;
const uint8_t HEADER[10]={'$','Z','E','N',0xD7};

struct SaveArgs
{
    uint8_t data[BUFFER_SIZE];
    ssize_t length;
    int frame_id;
};
bool isServerRunning = false;
static zen_comm_config_t zen_comm_config;
static SaveArgs *buffer_pool = nullptr;
static int sockfd = -1;
static pthread_t listener_tid;
static volatile bool keep_running = false;

static std::string validated_client_ip = "";

// ---------- IMAGE PROCESS ----------
static void *process_data(void *arg)
{
    SaveArgs *args = static_cast<SaveArgs *>(arg);
    // if args-> data contain $ZENDATA then it will be treated as data_callback else image_callback
    if (strstr((char*)args->data, "$ZENDATA") == nullptr) {
        if (zen_comm_config.image_callback != nullptr)
        {
            zen_comm_config.image_callback(reinterpret_cast<const unsigned char *>(args->data), args->length);
        }
    }
    else{
        if(zen_comm_config.data_callback != nullptr){
            zen_comm_config.data_callback(reinterpret_cast<const unsigned char *>(args->data), args->length);
        }
    }
    return nullptr;
}

// ---------- LISTENER ----------
static void *zen_comm_listener(void *arg)
{
    (void)arg;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int frame_id = 0;
    client_validated = false;
    std::printf("Listening on UDP port %u\n", zen_comm_config.port);

    while (keep_running)
    {
        int index = frame_id % zen_comm_config.pool_size;
        SaveArgs *args = &buffer_pool[index];
        ssize_t len = recvfrom(sockfd, args->data, zen_comm_config.buffer_size, 0, reinterpret_cast<struct sockaddr *>(&client_addr), &addr_len);

        if (len < 0 || !keep_running)
        {
            if (keep_running) std::perror("recvfrom failed");
            break;
        }
        if (!client_validated || strstr((char*)args->data,(char*)HEADER)!=NULL)
        {
            client_validated=false;
            uint8_t dataToSend[1024] = {};
            parseData(args->data, dataToSend);
            printf("The client validated : %s\n", client_validated ? "true" : "false");
            if (client_validated){
                validated_client_ip = inet_ntoa(((struct sockaddr_in *)&client_addr)->sin_addr);
            }
            sendto(sockfd, dataToSend, strlen((char *)dataToSend), 0, (struct sockaddr *)&client_addr, addr_len);
            continue;
        }
        std::string sender_ip = inet_ntoa(client_addr.sin_addr);
        if (sender_ip != validated_client_ip)
        {
            client_validated = false ;
            validated_client_ip = "";
            continue;
        }

        args->length = len;
        args->frame_id = frame_id++;

        pthread_t tid;
        if (pthread_create(&tid, nullptr, process_data, args) != 0)
        {
            std::perror("Thread creation failed");
        }
        else
        {
            pthread_detach(tid);
        }
    }
    printf("UDP listener thread terminated.\n");
    return nullptr;
}

// ---------- INIT / START / STOP ----------
bool zen_comm_init(const zen_comm_config_t *config)
{
    if (isServerRunning) {
        fprintf(stderr, "Communication already initialized.\n");
        return true;
    }

    if (!config || config->buffer_size == 0 || config->pool_size == 0)
    {
        std::fprintf(stderr, "Invalid UDP config\n");
        return false;
    }

    zen_comm_config = *config;

    if (zen_comm_config.buffer_size > BUFFER_SIZE)
    {
        std::fprintf(stderr, "Buffer size too large\n");
        return false;
    }

    buffer_pool = static_cast<SaveArgs *>(std::calloc(zen_comm_config.pool_size, sizeof(SaveArgs)));
    if (!buffer_pool)
    {
        std::perror("Memory allocation failed");
        return false;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::perror("Socket creation failed");
        return false;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(zen_comm_config.port);

    if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0)
    {
        std::perror("Bind failed");
        close(sockfd);
        sockfd = -1;
        free(buffer_pool);
        buffer_pool = nullptr;
        return false;
    }
    isServerRunning = true;
    return true;
}

void zen_comm_start()
{
    if (!isServerRunning) {
        fprintf(stderr, "Communication not initialized.\n");
        return;
    }
    if (keep_running){
        fprintf(stderr, "Listener already running.\n");
        return;
    }

    keep_running = true;
    if (pthread_create(&listener_tid, nullptr, zen_comm_listener, nullptr) != 0)
    {
        std::perror("UDP listener thread failed");
        keep_running = false;
    }
}

void zen_comm_stop()
{
    if (!isServerRunning) return;

    keep_running = false;
    if (sockfd != -1)
    {
        shutdown(sockfd, SHUT_RDWR); // This unblocks recvfrom
        close(sockfd);
        sockfd = -1;
    }

    if (listener_tid) {
        pthread_join(listener_tid, nullptr); // Wait for the thread to finish
        listener_tid = 0;
    }

    if (buffer_pool)
    {
        std::free(buffer_pool);
        buffer_pool = nullptr;
    }

    client_validated = false;
    validated_client_ip.clear();
    isServerRunning = false;

    printf("🛑 Communication stopped. Session reset.\n");
}
