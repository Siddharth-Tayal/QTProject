#ifndef THREAD_STREAM_HPP
#define THREAD_STREAM_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>
#include <cstddef>

using image_callback_t = void (*)(const unsigned char* data, size_t length);
using data_callback_t = void (*)(const unsigned char* data, size_t length);

struct zen_comm_config_t {
    uint16_t port;
    uint32_t buffer_size;
    uint32_t pool_size;
    image_callback_t image_callback;
    data_callback_t data_callback;
};

bool zen_comm_init(const zen_comm_config_t* config);
void zen_comm_start();
void zen_comm_stop();
void udp_server_thread();

#ifdef __cplusplus
}
#endif

#endif // THREAD_STREAM_HPP
