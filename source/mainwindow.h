#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QTimer>

#include <huffmancoder/huffmancoder.h>

namespace Threading {
    extern int completeness;
}

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    void setPOFunc(HuffmanCoder::ProgressOutputFunction x){POFunc = x;}

private:
    Ui::MainWindow *ui;
    QString dir;

    HuffmanCoder::ProgressOutputFunction POFunc;

    HuffmanCoder hc;
    QElapsedTimer timer;
    QFutureWatcher<void> future_watcher;
    //QFuture<void> future;

    bool compressing;

    enum{COMPRESSION_MODE, DECOMPRESSION_MODE};

    QTimer progress_timer;

    void encode();
    void decode();

    void DisableUI();
    void EnableUI();

    void Compress();
    void Decompress();

private slots:
    void BrowseInput();
    void BrowseOutput();

    void CompressionDone();
    void DecompressionDone();
    void ThreadFinished();
    void SelectMode(int mode);
    void Go();

    void Progress();
};

#endif // MAINWINDOW_H
