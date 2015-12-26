#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QMessageBox>

int Threading::completeness = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    compressing=false;

    connect(ui->EXIT_BTN,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->INPUT_BTN,SIGNAL(clicked()),this,SLOT(BrowseInput()));
    connect(ui->OUTPUT_BTN,SIGNAL(clicked()),this,SLOT(BrowseOutput()));

    connect(ui->SELECT_MODE_BOX,SIGNAL(currentIndexChanged(int)),this,SLOT(SelectMode(int)));
    connect(ui->GO_BTN,SIGNAL(clicked()),this,SLOT(Go()));
    connect(&future_watcher,SIGNAL(finished()),this,SLOT(ThreadFinished()));

    connect(&progress_timer,SIGNAL(timeout()),this,SLOT(Progress()));

    progress_timer.setInterval(200);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    //hc.setOutputProgressFunc(this, &MainWindow::Progress);
    ui->textBrowser->setPlainText("Welcome!");

    ui->SELECT_MODE_BOX->insertItem(COMPRESSION_MODE,"Compress");
    ui->SELECT_MODE_BOX->insertItem(DECOMPRESSION_MODE,"Decompress");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::BrowseInput()
{
    QString str=QFileDialog::getOpenFileName(this,"Select file",dir);
    if(str.isEmpty()) return;

    int index = str.lastIndexOf("\\");
    if(index == -1) index = str.lastIndexOf("/");
    dir = str;
    dir.truncate(index);

    ui->INPUT_LINE->setText(str);
    ui->OUTPUT_LINE->setText(str + ".cmpr");
}

void MainWindow::BrowseOutput()
{
    QString str=QFileDialog::getSaveFileName(this,"Select file",dir,"*.cmpr");
    if(str.isEmpty()) return;

    int index = str.lastIndexOf("\\");
    if(index == -1) index = str.lastIndexOf("/");
    dir= str;
    dir.truncate(index);

    ui->OUTPUT_LINE->setText(str);
}

void MainWindow::Compress()
{
    compressing=true;

    ui->progressBar->setValue(0);

    DisableUI();
    repaint();
    // check if image, and perform special compression



    QString filename(ui->INPUT_LINE->text());
    if(filename.right(5)==".cmpr" && QMessageBox::information(this,"Confurm","The file is allready compressed.","Preceed","Cancel"))
    {
        ui->textBrowser->append("Compression canceled.");
        EnableUI();
        return;
    }
    ui->textBrowser->setPlainText("Reading from file...");
    repaint();
    if(! hc.setData(filename.toLatin1().data()))
    {
        ui->textBrowser->append("Error reding file!");
        ui->textBrowser->append(filename.toLatin1().data());
        EnableUI();
        return;
    }
    hc.setOutputProgressFunc(POFunc);
    ui->textBrowser->append(QString("File: ") + filename);
    ui->textBrowser->append(QString("Size: ") + QString::number(hc.getDataLenght()*sizeof(Byte)) + " B");
	//TODO: remove this restriction, or change it to a warning
    if(hc.getDataLenght() < 2000) // perform huffman coding if file is large
    {
        ui->textBrowser->append("Huffman coding is not necessary.");
        EnableUI();
        return;
    }
    // cancel huffman coding if result is larger
    ui->textBrowser->append("Compressing...");

    repaint();

    progress_timer.start();
    timer.start();
    QFuture<void> future = QtConcurrent::run(this, &MainWindow::encode);//encode();
    future_watcher.setFuture(future);
}

void MainWindow::CompressionDone()
{
    //future.waitForFinished();
    ui->textBrowser->append(QString("Time: ") + QString::number(timer.elapsed()) + " ms" );
    progress_timer.stop();
    compressing=false;
    ui->textBrowser->append(QString("Compressed to ") + QString::number(hc.getBitMass().getSizeInBytes() + hc.getHeaderSize()) + " B");
    // check if really compressed
    ui->textBrowser->append("Writing to file...");
    repaint();
    hc.writeToFile(ui->OUTPUT_LINE->text().toLatin1().constData());
    ui->textBrowser->append("Done.");
    ui->progressBar->setValue(100);

    EnableUI();
    //repaint();
}

void MainWindow::Decompress()
{

    ui->progressBar->setValue(0);

    DisableUI();
    repaint();



    QString filename(ui->INPUT_LINE->text());
    if(filename.right(5)!=".cmpr")
    {
        ui->textBrowser->append("The file is not copressed!");
        EnableUI();
        return;
    }
    ui->textBrowser->setPlainText("Reading from file...");
    repaint();
    if(!hc.readFromFile(filename.toLatin1().constData()))
    {
        ui->textBrowser->append("Error reding file!");
        EnableUI();
        return;
    }
    hc.setOutputProgressFunc(POFunc);
    ui->textBrowser->append(QString("File: ") + filename);


    ui->textBrowser->append("Decompressing...");
    repaint();
    progress_timer.start();
    timer.start();
    QFuture<void> future = QtConcurrent::run(this, &MainWindow::decode);//decode();
    future_watcher.setFuture(future);
}

void MainWindow::DecompressionDone()
{
    ui->textBrowser->append(QString("Time: ") + QString::number(timer.elapsed()) + " ms" );
    progress_timer.stop();
    ui->textBrowser->append("Writing to file...");
    repaint();
    hc.writeDataToFile(ui->OUTPUT_LINE->text().toLatin1().constData());
    ui->textBrowser->append("Done.");
    ui->progressBar->setValue(100);

    EnableUI();
    //repaint();
}


void MainWindow::Progress()
{
    ui->progressBar->setValue(Threading::completeness);
    //repaint();
}

void MainWindow::encode()
{
    hc.encode();
}

void MainWindow::decode()
{
    hc.decode();
}

void MainWindow::ThreadFinished()
{
    if(compressing) CompressionDone();
    else DecompressionDone();
}

void MainWindow::SelectMode(int mode)
{
    switch(mode)
    {
    case COMPRESSION_MODE:
        break;
    case DECOMPRESSION_MODE:
        break;
    }
}

void MainWindow::Go()
{
    switch(ui->SELECT_MODE_BOX->currentIndex())
    {
    case COMPRESSION_MODE:
        Compress();
        break;
    case DECOMPRESSION_MODE:
        Decompress();
        break;
    }
}

void MainWindow::DisableUI()
{
    this->setFocusPolicy(Qt::NoFocus);
    ui->EXIT_BTN->setDisabled(true);
    ui->INPUT_BTN->setDisabled(true);
    ui->INPUT_LINE->setDisabled(true);
    ui->OUTPUT_BTN->setDisabled(true);
    ui->OUTPUT_LINE->setDisabled(true);
    ui->SELECT_MODE_BOX->setDisabled(true);
    ui->GO_BTN->setDisabled(true);
}

void MainWindow::EnableUI()
{
    ui->EXIT_BTN->setEnabled(true);
    ui->INPUT_BTN->setEnabled(true);
    ui->INPUT_LINE->setEnabled(true);
    ui->OUTPUT_BTN->setEnabled(true);
    ui->OUTPUT_LINE->setEnabled(true);
    ui->SELECT_MODE_BOX->setEnabled(true);
    ui->GO_BTN->setEnabled(true);

    this->setFocusPolicy(Qt::StrongFocus);
}





