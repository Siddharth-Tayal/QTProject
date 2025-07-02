#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QPixmap>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi();
    // 1. Create the worker object that contains our library logic
    m_worker = new ZenWorker();
    // 2. Move the worker to the background thread to prevent GUI freezing
    m_worker->moveToThread(&m_workerThread);

    // 3. Connect signals and slots for thread-safe communication
    // Connect buttons to worker slots
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);

    // Connect worker signals to UI update slots
    connect(m_worker, &ZenWorker::imageReceived, this, &MainWindow::updateImage);
    connect(m_worker, &ZenWorker::dataReceived, this, &MainWindow::updateData);
    connect(m_worker, &ZenWorker::logMessage, this, &MainWindow::appendLog);

    // Connect thread's finished signal for proper cleanup
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    // 4. Start the background thread's event loop
    m_workerThread.start();

    appendLog("Application ready. Click 'Start Listener' to begin.");
}

MainWindow::~MainWindow()
{
    // The closeEvent handles cleanup
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    appendLog("Window closing, shutting down worker thread...");
    // Ensure the listener is stopped
    onStopClicked();

    // Quit the thread's event loop and wait for it to finish
    m_workerThread.quit();
    m_workerThread.wait(3000); // Wait up to 3 seconds
    event->accept();
}

void MainWindow::onStartClicked()
{
    // Use QMetaObject::invokeMethod to call the slot in the worker's thread
    QMetaObject::invokeMethod(m_worker, "startListener", Qt::QueuedConnection);
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
}

void MainWindow::onStopClicked()
{
    QMetaObject::invokeMethod(m_worker, "stopListener", Qt::QueuedConnection);
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
}

void MainWindow::updateImage(const QByteArray &imageData)
{
    QPixmap pixmap;
    // Try to load the received data as an image
    if (pixmap.loadFromData(imageData))
    {
        m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        appendLog(QString("📷 Received image of %1 bytes.").arg(imageData.size()));
    }
    else
    {
        appendLog("⚠️ Failed to decode received image data.");
    }
}

void MainWindow::updateData(const QString &dataString)
{
    appendLog(QString("📥 Received data: %1").arg(dataString));
}

void MainWindow::appendLog(const QString &message)
{
    m_logArea->append(message);
}

void MainWindow::setupUi()
{
    // Create main widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_startButton = new QPushButton("Start Listener");
    m_stopButton = new QPushButton("Stop Listener");
    m_stopButton->setEnabled(false);
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    mainLayout->addLayout(buttonLayout);

    // Image display area
    m_imageLabel = new QLabel("Waiting for image stream...");
    m_imageLabel->setMinimumSize(640, 480);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setFrameShape(QFrame::StyledPanel);
    m_imageLabel->setStyleSheet("background-color: #2c3e50; color: #ecf0f1; font-size: 16px;");
    mainLayout->addWidget(m_imageLabel);

    // Log display area
    m_logArea = new QTextEdit();
    m_logArea->setReadOnly(true);
    mainLayout->addWidget(m_logArea);

    setCentralWidget(centralWidget);
    setWindowTitle("Zenithra UDP Image Streamer");
    resize(800, 700);
}
