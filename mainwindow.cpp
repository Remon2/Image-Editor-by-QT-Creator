
#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "ui_imgresizedialog.h"

#include "zoomdialog.h"
#include "imgwin.h"
#include "imgabout.h"
#include "sliderdialog.h"
#include <QtCore/qmath.h>
#include <QMdiArea>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QComboBox>
#include <QLineEdit>
#include<QByteArray>
#include<QBuffer>
#include <QTextStream>
#include<QInputDialog>
#include<QStack>
#include<QPainter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    line = new QLineEdit();
    line->setText("90");
    line->setMaximumSize(50,30);
    line->setEnabled(false);
    ui->toolBar->addWidget(line);

    // Add a drop-down box to select common zoom sizes or to type in your own size
    zoom_box = new QComboBox();
    zoom_box->addItem("300");
    zoom_box->addItem("200");
    zoom_box->addItem("150");
    zoom_box->addItem("100");
    zoom_box->addItem("75");
    zoom_box->addItem("50");
    zoom_box->addItem("25");
    zoom_box->addItem("10");
    zoom_box->setInsertPolicy(QComboBox::NoInsert);
    zoom_box->setValidator(new QIntValidator(10,300,zoom_box));
    zoom_box->setEditable(true);
    zoom_box->setEnabled(false);

    ui->toolBar->addWidget(zoom_box);

    // Add a spacer on the toolbar
    QWidget *w = new QWidget();
    w->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    w->setMinimumSize(4, 4);
    ui->toolBar->addWidget(w);
    ui->toolBar->setIconSize(QSize(60,40));
    // Add a slider to control zoom as well
    zoom_slider = new QSlider(Qt::Horizontal);
    zoom_slider->setRange(10, 300);
    zoom_slider->setSingleStep(10);
    zoom_slider->setValue(100);
    zoom_slider->setTickInterval(40);
    zoom_slider->setTickPosition(QSlider::TicksRight);
    zoom_slider->setEnabled(false);
    zoom_slider->setMinimumWidth(40);
   // zoom_slider->setMaximumWidth(60);
    zoom_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Note, if you move the toolbar, the slider changes direction :)
    zoom_slider->connect(ui->toolBar, SIGNAL(orientationChanged(Qt::Orientation)), SLOT(setOrientation(Qt::Orientation)));

    ui->toolBar->addWidget(zoom_slider);

    // Also, you can resize to fit the window
    ui->toolBar->addAction(ui->actionFit_window);

    // Sync the combo box and slider
    connect(zoom_box, SIGNAL(editTextChanged(QString)), SLOT(zoomBoxChanged(QString)));
    connect(zoom_box, SIGNAL(activated(QString)), SLOT(zoomBoxChanged(QString)));
}

/******************************************************************************
 * ~MainWindow(): Destructor
 * Deletes the ui.
 *****************************************************************************/
MainWindow::~MainWindow()
{
    delete ui;
}

ImgWin* MainWindow::getCurrent()
{
    QMdiSubWindow *child = ui->mdiArea->activeSubWindow();
    if (!child)
        return 0;
    return (ImgWin*)(child->widget());
}


QImage MainWindow::getImage()
{
    ImgWin *cur = getCurrent();
    if (cur)
        return cur->getImage();
    return QImage();
}


void MainWindow::setImage(QImage p)
{
    getCurrent()->setImage(p);
}

void MainWindow::doOpen()
{
    QString file = QFileDialog::getOpenFileName(this, "Open Image", "",
        "Image Files (*.png *.jpg *.bmp *.gif);;Any Files (*.*)");

    if (file != "")
    {
        QImageReader *reader = new QImageReader(file);
        QImage img = reader->read();
        if (!img.isNull())
        {
            undo.push(img);
            line->setEnabled(true);
            ui->actionUndo->setEnabled(true);
            ui->actionRotate->setEnabled(true);
            ImgWin* win = new ImgWin;
            win->setImage(img);
            win->setWindowTitle(file);
            win->setReader(reader);

            // Display the image window
            QMdiSubWindow* subwin = ui->mdiArea->addSubWindow(win);
            subwin->showMaximized();

            // Add a item to the window menu, and give it to the imgwin to keep track of
            //win->setMenuItem(ui->menuWindow->addAction(file, subwin, SLOT(setFocus())));
            connect(win, SIGNAL(closing(QAction*)), this, SLOT(removeWindowListItem(QAction*)));

            // Mouse info
            connect(win, SIGNAL(mouseOverInfo(QPoint)), SLOT(imgMouseInfo(QPoint)), Qt::UniqueConnection);
        }
    }
}


void MainWindow::doSave()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                "/home",
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);


    bool ok;
    bool ok2;
    QTextStream(stdout) << dir << endl;
    QString type;
        QString text = QInputDialog::getText(this, tr("Save As"),
                                                 tr("File name:"), QLineEdit::Normal,
                                                 "", &ok);
        if(ok && !text.isEmpty()){
            type = QInputDialog::getText(this, tr("Save As"),
                                                     tr("File type:"), QLineEdit::Normal,
                                                     "", &ok2);
            if(ok2 && !type.isEmpty() && (type=="jpg" ||type=="JPG"||type=="png"||type=="PNG"||type=="jpeg"||type=="JPEG")){
             QString name = dir+"/"+text+"."+type;
             QTextStream(stdout) << name << endl;
             getImage().save(name);
             QMessageBox::information(
                     this,
                     tr("Image Saved"),
                     tr("Your image is saved successfully") );
            }
            else{
                QMessageBox::information(
                        this,
                        tr("Image Saved"),
                        tr("Invalid image type") );
            }
        }
        else{
            QMessageBox::information(
                    this,
                    tr("Image Saved"),
                    tr("Enter image name") );
           }


}

void MainWindow::doRevert()
{
    // Must happen before the message box, because it deactivates the subwindow
    ImgWin *win = getCurrent();
    if (QMessageBox::Ok == QMessageBox::question(this, "Reset file",
        "Do you want to reset? You will lose all your changes.",
        QMessageBox::Ok | QMessageBox::Cancel))
    {
        win->getReader()->setFileName(win->getReader()->fileName());
        win->setImage(win->getReader()->read());
        undo.push(win->getReader()->read());
        ui->actionUndo->setEnabled(true);
    }

}

void MainWindow::doNegate()
{
    QImage img = getImage();
    for(int i = 0; i < img.width(); ++i)
    {
        for(int j = 0; j < img.height(); ++j)
        {
            QRgb old_color = img.pixel(i, j);
            old_color = qRgb(0xFF - qRed(old_color), 0xFF - qGreen(old_color), 0xFF - qBlue(old_color));
            img.setPixel(i, j, old_color);
        }
    }
    setImage(img);
    undo.push(img);
    ui->actionUndo->setEnabled(true);
}


void MainWindow::doSharpen()
{
    QImage img = getImage();
    QImage out_img = img;
    /* We only want the inner pixels. Don't worry about the furthest edge.*/
    for(int i = 1; i < img.width() - 1; ++i)
    {
        for(int j = 1; j < img.height() - 1; ++j)
        {
            QRgb old_color = img.pixel(i, j);
            int red = qRed(old_color);
            int blue = qBlue(old_color);
            int green = qGreen(old_color);
            red = 5 * red - qRed(img.pixel(i-1,j)) -qRed(img.pixel(i+1,j)) -qRed(img.pixel(i,j-1)) -qRed(img.pixel(i,j+1));
            blue = 5 * blue - qBlue(img.pixel(i-1,j)) -qBlue(img.pixel(i+1,j)) -qBlue(img.pixel(i,j-1)) -qBlue(img.pixel(i,j+1));
            green = 5 * green - qGreen(img.pixel(i-1,j)) -qGreen(img.pixel(i+1,j)) -qGreen(img.pixel(i,j-1)) -qGreen(img.pixel(i,j+1));
            if(red > 0xFF) red = 0xEF;
            if(red < 0) red = 0;
            if(green > 0xFF) green = 0xEF;
            if(green < 0) green = 0;
            if(blue > 0xFF) blue = 0xEF;
            if(blue < 0 ) blue = 0;
            old_color = qRgb(red, green, blue);
            out_img.setPixel(i, j, old_color);
        }
    }
    setImage(out_img);
    undo.push(out_img);
ui->actionUndo->setEnabled(true);
}


void MainWindow::doSmooth()
{
    QImage img = getImage();
    QImage out_img = img;
    /* We only want the inner pixels. Don't worry about the furthest edge.*/
    for(int i = 1; i < img.width() - 1; ++i)
    {
        for(int j = 1; j < img.height() - 1; ++j)
        {
            QRgb old_color = img.pixel(i, j);
            int red = qRed(old_color);
            int blue = qBlue(old_color);
            int green = qGreen(old_color);
            red = (red * 4 + qRed(img.pixel(i-1,j)) * 2 + qRed(img.pixel(i+1,j)) * 2
                  + qRed(img.pixel(i,j-1)) * 2 + qRed(img.pixel(i,j+1)) * 2
                  + qRed(img.pixel(i-1,j-1)) + qRed(img.pixel(i+1,j+1))
                  + qRed(img.pixel(i-1,j+1)) + qRed(img.pixel(i+1, j-1)) ) / 16;
            blue = (blue * 4 + qBlue(img.pixel(i-1,j)) * 2 + qBlue(img.pixel(i+1,j)) * 2
                  + qBlue(img.pixel(i,j-1)) * 2 + qBlue(img.pixel(i,j+1)) * 2
                  + qBlue(img.pixel(i-1,j-1)) + qBlue(img.pixel(i+1,j+1))
                  + qBlue(img.pixel(i-1,j+1)) + qBlue(img.pixel(i+1, j-1)) ) / 16;
            green = (green * 4 + qGreen(img.pixel(i-1,j)) * 2 + qGreen(img.pixel(i+1,j)) * 2
                  + qGreen(img.pixel(i,j-1)) * 2 + qGreen(img.pixel(i,j+1)) * 2
                  + qGreen(img.pixel(i-1,j-1)) + qGreen(img.pixel(i+1,j+1))
                  + qGreen(img.pixel(i-1,j+1)) + qGreen(img.pixel(i+1, j-1)) ) / 16;
            if(red > 0xFF) red = 0xEF;
            if(red < 0) red = 0;
            if(green > 0xFF) green = 0xEF;
            if(green < 0) green = 0;
            if(blue > 0xFF) blue = 0xEF;
            if(blue < 0 ) blue = 0;
            old_color = qRgb(red, green, blue);
            out_img.setPixel(i, j, old_color);
        }
    }
    setImage(out_img);
    undo.push(out_img);
    ui->actionUndo->setEnabled(true);
}


void MainWindow::doZoom()
{
    ZoomDialog *zoom = new ZoomDialog(this);
    zoom->setTarget(getCurrent());
    zoom->show();
}


void MainWindow::doFillWindow()
{
    ImgWin *c = getCurrent();
    c->setScale(std::min(c->contentsRect().width() * 100.0 / getImage().width(),
                         c->contentsRect().height() * 100.0 / getImage().height()));
}


void MainWindow::doCrop()
{
    ImgWin* win = getCurrent();

    QRect sel = win->getSelection();
    if (sel.isEmpty())
    {
        QMessageBox::warning(this, "Crop", "Press select a region to crop.", QMessageBox::Ok);
        return;
    }

    setImage(getImage().copy(win->getSelection()));
    undo.push(getImage().copy(win->getSelection()));
    ui->actionUndo->setEnabled(true);
}

void MainWindow::doResize()
{
    QDialog* d = new QDialog(this);
    Ui::ImgResizeDialog* resize = new Ui::ImgResizeDialog;
    resize->setupUi(d);
    resize->widthSpin->setValue(getImage().size().width());
    resize->heightSpin->setValue(getImage().size().height());
    resize->widthSpin->setRange(1,1000000);
    resize->heightSpin->setRange(1,1000000);

    if (d->exec() == QDialog::Accepted)
    {
        setImage(getImage().scaled(QSize(resize->widthSpin->value(),resize->heightSpin->value())));
        undo.push(getImage().scaled(QSize(resize->widthSpin->value(),resize->heightSpin->value())));
        ui->actionUndo->setEnabled(true);
    }
}

void MainWindow::doAbout()
{
    QMessageBox::information(this, "About",
        "A very simple image editor, created for hci course.\n"
        "Copyright Ebtehal, Remon and Shaimaa.");


}


void MainWindow::doInfo()
{
    imgAbout *d = new imgAbout();
    QMdiSubWindow *child = ui->mdiArea->activeSubWindow();
    ImgWin *win = (ImgWin*)(child->widget());
    d->setup(win);
    d->show();


}

void MainWindow::doChangeImage(QMdiSubWindow* win)
{
    ui->statusBar->clearMessage();

    zoom_box->setDisabled(win == 0);
    zoom_slider->setDisabled(win == 0);
    zoom_slider->disconnect();
    connect(zoom_slider, SIGNAL(valueChanged(int)), SLOT(zoomChanged(int)));
    if (win)
    {
        ImgWin *c = getCurrent();

        zoom_slider->setValue(c->getScale());
        zoom_box->setEditText(QString::number(c->getScale()));
        c->connect(zoom_slider, SIGNAL(valueChanged(int)), SLOT(setScale(int)));
        zoom_slider->connect(c, SIGNAL(scaleChanged(int)), SLOT(setValue(int)));
    }

    emit imageEditable(win != 0);
}


void MainWindow::removeWindowListItem(QAction* act)
{
   // ui->menuWindow->removeAction(act);
}

void MainWindow::doSliders()
{
    SliderDialog *d = new SliderDialog(ui->mdiArea->activeSubWindow());
    QMdiSubWindow *child = ui->mdiArea->activeSubWindow();
    ImgWin *win = (ImgWin*)(child->widget());
    d->setTarget(win);
    d->setImage(win->getImage());
    undo.push(win->getImage());
    ui->actionUndo->setEnabled(true);
    d->exec();
}


void MainWindow::zoomChanged(int s)
{
    zoom_box->setEditText(QString::number(s));
}

void MainWindow::zoomBoxChanged(QString str)
{
    int n = str.toInt();
    zoom_slider->setValue(n);
}

void MainWindow::on_actionRedo_triggered()
{
    ui->actionUndo->setEnabled(true);
    if(!redo.isEmpty()){
        QImage temp = redo.pop();
        if(redo.isEmpty())
             ui->actionRedo->setEnabled(false);
        setImage(temp);
        undo.push(temp);
    }
}

void MainWindow::on_actionUndo_triggered()
{
    ui->actionRedo->setEnabled(true);
    if(!undo.isEmpty()){
    QImage temp = undo.pop();
    if(undo.isEmpty())
         ui->actionUndo->setEnabled(false);
    setImage(temp);
    redo.push(temp);
    }
}

QImage rotate(const QImage &p_image, qreal angle) {
    QMatrix matrix;
    matrix.translate(p_image.size().width() / 2, p_image.size().height() / 2);
    matrix.rotate(angle);
    matrix.translate(-p_image.size().width() / 2, -p_image.size().height() / 2);
    return p_image.transformed(matrix);
}

void MainWindow::on_actionRotate_triggered()
{
   QString angel= line->text();
   int ang = angel.toInt();
   QImage myImage = getImage();
//   myImage = rotate(myImage,ang);
//   setImage(myImage);
   QApplication::processEvents();

      QPixmap rotate(myImage.size());
      QPainter p(&rotate);
      p.setRenderHint(QPainter::Antialiasing);
      p.setRenderHint(QPainter::SmoothPixmapTransform);
      p.setRenderHint(QPainter::HighQualityAntialiasing);
      p.translate(rotate.size().width()/2 , rotate.size().height() / 2);
      p.rotate(ang);
      p.translate(-rotate.size().width() / 2, -rotate.size().height() / 2);

      p.drawPixmap(0, 0, QPixmap::fromImage(myImage));
      p.end();
      setImage(rotate.toImage());
      undo.push(rotate.toImage());
      ui->actionUndo->setEnabled(true);
}

void MainWindow::on_actionEdge_detect_triggered()
{
    QImage img = getImage();
    QImage out_img = img;
    for (int x = 0; x < img.width(); x++) {
                for (int y = 0; y < img.height(); y++) {
                    QRgb old_color = img.pixel(x, y);
                    int r = qRed(old_color);
                    int b = qBlue(old_color);
                    int g = qGreen(old_color);
                    int gray = (int) (r + g + b) / 3;

                    old_color = img.pixel(x+1, y);
                    r = qRed(old_color);
                    b = qBlue(old_color);
                    g = qGreen(old_color);
                    int grayRight = (int) (r + g + b) / 3;

                    old_color = img.pixel(x, y+1);
                    r = qRed(old_color);
                    b = qBlue(old_color);
                    g = qGreen(old_color);
                    int grayDown = (int) (r + g + b) / 3;

                    int vertEdge = abs(gray - grayRight);

                    int horzEdge = abs(gray - grayDown);


                    int combined = (int) qSqrt(vertEdge * vertEdge + horzEdge
                            * horzEdge);

                    out_img.setPixel(x, y, combined);
                }
            }
            setImage(out_img);
            undo.push(out_img);
            ui->actionUndo->setEnabled(true);
}
