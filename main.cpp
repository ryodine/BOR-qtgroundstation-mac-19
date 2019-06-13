#include "mainwindow.h"
#include <QApplication>
#include <thread>
#include "SerialScanner.h"
#include "SerialPort.h"
#include "RoverCommunicator.h"
#include "WindowWorker.h"

#include <QPixmap>

int main(int argc, char *argv[])
{
    Houston::IO::USBSerialScanner scanner;
    std::string ftdi_path;
    if (scanner.scan(1027, 24597, ftdi_path) == 1) {
        /*port.write("Action: status\r\n\r\n");
        std::vector<char> response;
        port.readUntilEnd(response);
        std::string res(response.begin(), response.end());
        printf("Got back:\n%s\n", res.c_str());*/
        Houston::Communication comms(ftdi_path, 9600);
        

        QApplication a(argc, argv);
        MainWindow w;
        Houston::WindowWorker worker(w, comms);

        QObject::connect(&worker, SIGNAL(logTextChanged(QString)),
                         &w,      SLOT(setLogBox(QString)));
        QObject::connect(&worker, SIGNAL(setCommTimer(int)),
                         &w,      SLOT(setCommTimer(int)));
        QObject::connect(&worker, SIGNAL(percentRxUpdate(int)),
                         &w,      SLOT(setRxProgress(int)));
        QObject::connect(&worker, SIGNAL(updatePhoto(QPixmap)),
                         &w,      SLOT(updatePhoto(QPixmap)));

        w.show();

        

        return a.exec();
        //return 1;
    } else {
        printf("[e] No FTDI usb serial device found. Can't start application\n");
        return 1;
    }
}
