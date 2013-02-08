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
    scene = new GLScene ();
    ui->graphicsView->setScene(scene);

    //Connections
    connect(ui->actionOpen_Image, SIGNAL(triggered()), ui->graphicsView, SLOT(loadImage()));
    connect(ui->actionSave_Image, SIGNAL(triggered()), ui->graphicsView, SLOT(saveImage()));
    connect(ui->actionOpen_Curves, SIGNAL(triggered()), ui->graphicsView, SLOT(loadCurves()));
    connect(ui->actionSave_Curves, SIGNAL(triggered()), ui->graphicsView, SLOT(saveCurves()));
    connect(ui->actionCreate_BSpline, SIGNAL(triggered()), ui->graphicsView, SLOT(create_bspline()));

    connect(ui->createCurveButton, SIGNAL(pressed()), ui->graphicsView, SLOT(create_bspline()));
    connect(ui->moveCurveButton, SIGNAL(pressed()), ui->graphicsView, SLOT(move_bsplines()));
    connect(ui->pointSizeSlider, SIGNAL(valueChanged(int)), ui->graphicsView, SLOT(changeControlPointSize(int)));

}

MainWindow::~MainWindow()
{
    delete scene;
    delete ui;
}

void MainWindow::center()
{
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}
