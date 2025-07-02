#ifndef ZEN_WORKER_H
#define ZEN_WORKER_H

#include <QObject>
#include <QByteArray>
#include <QString>

// Include your library's main header
#include "../network/thread_stream.hpp"

// This class runs in a background thread to handle all blocking operations
class ZenWorker : public QObject {
    Q_OBJECT

public:
    explicit ZenWorker(QObject *parent = nullptr);
    ~ZenWorker();

public slots:
    // This slot is called to start the UDP listener
    void startListener();
    // This slot is called to stop the UDP listener
    void stopListener();

signals:
    // Signal emitted when an image is received from the library
    void imageReceived(const QByteArray &imageData);
    // Signal emitted when data is received from the library
    void dataReceived(const QString &dataString);
    // Signal to log status messages to the UI
    void logMessage(const QString &message);

private:
    // This static pointer allows our C-style callbacks to find the C++ class instance
    static ZenWorker* s_instance;

    // --- Static C-style callbacks that wrap our signals ---
    // These are the functions that will be passed to zen_comm_init.
    static void image_callback_wrapper(const unsigned char *data, size_t length);
    static void data_callback_wrapper(const unsigned char *data, size_t length);
};

#endif // ZEN_WORKER_H
