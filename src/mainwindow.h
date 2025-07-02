#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "zen_worker.h"

// Forward declarations for UI elements to keep header clean
class QPushButton;
class QTextEdit;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Override the window's close event to safely shut down the background thread
    void closeEvent(QCloseEvent *event) override;

private slots:
    // --- Slots to handle UI events ---
    void onStartClicked();
    void onStopClicked();

    // --- Slots to receive data from the background worker ---
    void updateImage(const QByteArray &imageData);
    void updateData(const QString &dataString);
    void appendLog(const QString &message);

private:
    void setupUi();

    // --- UI Elements ---
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QTextEdit   *m_logArea;
    QLabel      *m_imageLabel;

    // --- Worker Thread Management ---
    QThread     m_workerThread;
    ZenWorker   *m_worker;
};

#endif // MAINWINDOW_H
