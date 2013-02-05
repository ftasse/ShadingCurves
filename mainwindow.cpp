#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDesktopWidget>

#include "Views/GLScene.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Setup
    ui->setupUi(this);
    center();

    //Connections
    connect(ui->actionOpen_Image, SIGNAL(triggered()), this, SLOT(loadImage()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::center()
{
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}

void MainWindow::loadImage()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open Image Files"),
                                    "",
                                    tr("Image files (*.jpg *.jpeg *.png *.gif *.bmp)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        loadImage(fileName.toStdString());
    }
}

void MainWindow::loadImage(std::string imageLocation)
{
    GLScene *scene = (GLScene *) ui->graphicsView->scene();
    if (scene->openImage(imageLocation))
    {
        qDebug("Image loaded: %d %d", scene->currentImage().cols, scene->currentImage().rows);
        resize(scene->currentImage().cols + 10, scene->currentImage().rows + 10);
        center();
    }
}
