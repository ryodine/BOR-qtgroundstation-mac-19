#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
enum GuiButtons {
    STATUS,
    PING,
    ARM_LANDING,
    MANUAL_LAND,
    TAKE_PHOTO,
    SAMPLE,
    NONE
};
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Ui::MainWindow* getUI() {return ui;}

public slots:
    void on_actionExit_triggered();
    void setLogBox(QString value);
    void setCommTimer(int value);
    void setRxProgress(int value);
    void updatePhoto(QPixmap map);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
