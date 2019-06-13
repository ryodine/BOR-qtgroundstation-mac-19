#include "WindowWorker.h"
#include "ui_mainwindow.h"
#include <QSignalMapper>
#include <QPixmap>
#include <QByteArray>

using namespace Houston;

WindowWorker::WindowWorker(::MainWindow& mw, Communication& comms)
 : m_mw(mw)
 , m_comms(comms)
 , m_thread(&WindowWorker::threadfunc, this)
{
    QObject::connect(m_mw.getUI()->status_btn, &QPushButton::clicked,
                     this,                     [this]{ handleUIButton(Ui::STATUS); });
    QObject::connect(m_mw.getUI()->arm_btn,    &QPushButton::clicked,
                     this,                     [this]{ handleUIButton(Ui::ARM_LANDING); });
    QObject::connect(m_mw.getUI()->land_btn,   &QPushButton::clicked,
                     this,                     [this]{ handleUIButton(Ui::MANUAL_LAND); });
    QObject::connect(m_mw.getUI()->soil_btn,   &QPushButton::clicked,
                     this,                     [this]{ handleUIButton(Ui::SAMPLE); });
    QObject::connect(m_mw.getUI()->photo_btn,  &QPushButton::clicked,
                     this,                     [this]{ handleUIButton(Ui::TAKE_PHOTO); });
    QObject::connect(m_mw.getUI()->ping_btn,   &QPushButton::clicked,
                     this,                     [this]{ handleUIButton(Ui::PING); });
};

WindowWorker::~WindowWorker() {
    runThread = false;
    m_thread.join();
    printf("[i] Window worker thread concluded\n");
}

void WindowWorker::threadfunc() {
    while (runThread) {
        usleep(100000);
        
        // Emits
        emit setCommTimer(m_comms.secsSinceLastRead());
        emit logTextChanged(QString(m_comms.lastStatusMessage().getBodyString().c_str()));
        emit percentRxUpdate(m_comms.getRxPercent() * 100);

        std::lock_guard<std::mutex> lock(m_lastCommRequestMutex);
        if (m_lastCommRequest != Ui::NONE && !m_comms.isProcessingCommand()) {
            UIButtonDoneEvt(m_lastCommRequest);
        }
    }
}

void WindowWorker::handleUIButton(Ui::GuiButtons button) {
    m_comms.scheduleUICommand(button);
    std::lock_guard<std::mutex> lock(m_lastCommRequestMutex);
    m_lastCommRequest = button;
}

void WindowWorker::UIButtonDoneEvt(Ui::GuiButtons button) {
    m_lastCommRequest = Ui::NONE;
    printf("done with %d\n", button);
    if (button == Ui::TAKE_PHOTO) {
        std::vector<char> bytes = m_comms.getResultOfUICommand().getBodyBytes();
        QByteArray byteArray(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        QPixmap qp;
        if (qp.loadFromData(byteArray, "JPG")) {
            printf("Made Pixmap\n");
            emit updatePhoto(qp);
        } else {
            printf("Can't load image from pixmap\n");
        }
    }
}