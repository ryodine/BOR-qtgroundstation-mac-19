#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <regex>
#include <string>
#include <sstream>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStringList headers;
    ui->status_table->setColumnCount(2);
    headers<<"Name"<<"Value";
    ui->status_table->setHorizontalHeaderLabels(headers);
    ui->status_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->status_table->horizontalHeader()->setStretchLastSection(true);
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
    std::string val = value.toLocal8Bit().constData();
    ui->response_box->document()->setPlainText(value);
    std::istringstream f(val);
    std::string line;

    int count = 0;
    for (int i = 0; i < val.size(); i++)
        if (val[i] == '\n') count++;
    
    ui->status_table->setRowCount(count);
    int ct = 0;
    while (std::getline(f, line)) {
        std::regex rgx("(.*): (.*)");
        std::smatch matches;
        if(std::regex_search(line, matches, rgx)) {
            if (matches.size() == 3) {
                //std::cout << matches[1] << "-" << matches[2] << std::endl;
                ui->status_table->setItem(ct, 0, new QTableWidgetItem(QString(matches.str(1).c_str())));
                ui->status_table->setItem(ct, 1, new QTableWidgetItem(QString(matches.str(2).c_str())));
            }
        }
        ct++;
    }
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