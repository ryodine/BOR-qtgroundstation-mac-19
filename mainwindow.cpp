#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::setLogBox(QString value) {
    ui->response_box->document()->setPlainText(value);
}

void MainWindow::setCommTimer(int value) {
    ui->arduino_counter->setText(QString(std::to_string(value).c_str()));
    if (value > 7) {
        ui->arduino_led->setStyleSheet(QString("background: #bc3232;"));
    } else if (value > 5) {
        ui->arduino_led->setStyleSheet(QString("background: #cc9500;"));
    } else {
        ui->arduino_led->setStyleSheet(QString("background: #268e00;"));
    }
}

void MainWindow::setRxProgress(int value) {
    if (value > 100) value = 100;
    if (value < 0)   value = 0;
    ui->rx_progress->setValue(value);
}

void MainWindow::updatePhoto(QPixmap map) {
    ui->photo->setPixmap(map);
    printf("Got map on UI\n");
}