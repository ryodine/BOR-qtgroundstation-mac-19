#ifndef WindowWorker_H_G
#define WindowWorker_H_G
#include "mainwindow.h"
#include "RoverCommunicator.h"

#include <QMainWindow>


namespace Ui {
class MainWindow;
}

namespace Houston {

class WindowWorker : public QObject {
    Q_OBJECT
public:
    WindowWorker(::MainWindow& mw, Communication& comms );
    ~WindowWorker();

signals:
    void logTextChanged(QString text);
    void setCommTimer(int num); 
    void percentRxUpdate(int num);
    void updatePhoto(QPixmap map);  

private:
    void threadfunc();
    void handleUIButton(Ui::GuiButtons button);
    void UIButtonDoneEvt(Ui::GuiButtons button);

    bool runThread = true;
    ::MainWindow& m_mw;
    Communication& m_comms;

    std::thread m_thread;

    std::mutex m_lastCommRequestMutex;
    Ui::GuiButtons m_lastCommRequest = Ui::NONE;


};

}
#endif