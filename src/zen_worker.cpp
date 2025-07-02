#include "zen_worker.h"
#include <QDebug>

// Initialize the static instance pointer
ZenWorker* ZenWorker::s_instance = nullptr;

ZenWorker::ZenWorker(QObject *parent) : QObject(parent) {
    // When a ZenWorker object is created, it sets itself as the static instance.
    // This is a common pattern to bridge C-style callbacks with C++ objects.
    s_instance = this;
}

ZenWorker::~ZenWorker() {
    // Ensure cleanup is performed if the object is destroyed
    stopListener();
}

void ZenWorker::startListener() {
    emit logMessage("Initializing Zen Communication...");

    // Setup the UDP configuration, pointing to our static wrapper functions
    zen_comm_config_t udp_config = {
        .port = 5000,
        .buffer_size = 65536,
        .pool_size = 4, // Increased pool size for better performance
        .image_callback = &ZenWorker::image_callback_wrapper,
        .data_callback = &ZenWorker::data_callback_wrapper,
    };

    // Initialize your library
    if (!zen_comm_init(&udp_config)) {
        emit logMessage("❌ ERROR: Failed to initialize UDP communication!");
        return;
    }

    emit logMessage("✅ Zen Communication initialized. Starting listener thread...");

    // Start the library's main listening loop (which creates its own pthread)
    zen_comm_start();

    emit logMessage("✅ Listener is running.");
}

void ZenWorker::stopListener() {
    emit logMessage("🔌 Stopping Zen Communication...");
    zen_comm_stop();
    emit logMessage("🛑 Listener stopped.");
}

// --- Static Callback Implementations ---

void ZenWorker::image_callback_wrapper(const unsigned char *data, size_t length) {
    if (s_instance) {
        QByteArray imageData(reinterpret_cast<const char*>(data), length);
        emit s_instance->imageReceived(imageData);
    }
}

void ZenWorker::data_callback_wrapper(const unsigned char *data, size_t length) {
    if (s_instance) {
        QString textData = QString::fromUtf8(reinterpret_cast<const char*>(data), length);
        emit s_instance->dataReceived(textData);
    }
}
