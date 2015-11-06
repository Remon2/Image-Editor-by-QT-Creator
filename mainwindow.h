/******************************************************************************
 * mainwindow.h
 * Header file for mainwindow.cpp, where you can find more details about the
 * purpose of these two files.
 *****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include<QLineEdit>

class QMdiSubWindow;
class QLabel;
class QSlider;
class QComboBox;
class ImgWin;

namespace Ui {
    class MainWindow;
    class ImgResizeDialog;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void imageEditable(bool);

private slots:
    // File operations
    void doOpen();
    void doSave();
    void doRevert();
    // Image manipulations
    void doNegate();
    void doSharpen();
    void doCrop();
    void doResize();
    void doSmooth();
    // Menu disabling functions
    void doChangeImage(QMdiSubWindow*);
    void removeWindowListItem(QAction* act);
    // Dialogs
    void doSliders();
    void doZoom();
    void doFillWindow();
    void doInfo();
    void doAbout();
    // Mouseover info
    void zoomChanged(int);
    void zoomBoxChanged(QString);
    void on_actionRedo_triggered();
    void on_actionUndo_triggered();
void on_actionRotate_triggered();
void on_actionEdge_detect_triggered();

private:
    ImgWin* getCurrent();
    QImage getImage();
    void setImage(QImage p);

    Ui::MainWindow *ui;
    QSlider *zoom_slider;
    QComboBox *zoom_box;
    QStack<QImage> undo;
    QStack<QImage> redo;
    QLineEdit * line;
};

#endif // MAINWINDOW_H
