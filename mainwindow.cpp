#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDesktopWidget>
#include <QGLWidget>

#include "Views/GLScene.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Setup
    ui->setupUi(this);
    scene = new GLScene ();
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setViewport(new QGLWidget( QGLFormat(QGL::SampleBuffers  | QGL::DirectRendering)));
    ui->graphicsView->setViewportUpdateMode( QGraphicsView::FullViewportUpdate);

    //Connections
    connect(ui->actionOpen_Image, SIGNAL(triggered()), ui->graphicsView, SLOT(loadImage()));
    connect(ui->actionSave_Image, SIGNAL(triggered()), ui->graphicsView, SLOT(saveImage()));
    connect(ui->actionOpen_Curves, SIGNAL(triggered()), ui->graphicsView, SLOT(loadCurves()));
    connect(ui->actionSave_Curves, SIGNAL(triggered()), ui->graphicsView, SLOT(saveCurves()));
    connect(ui->actionCreate_BSpline, SIGNAL(triggered()), ui->graphicsView, SLOT(create_bspline()));

    connect(ui->createCurveButton, SIGNAL(pressed()), ui->graphicsView, SLOT(create_bspline()));
    connect(ui->editCurveButton, SIGNAL(pressed()), ui->graphicsView, SLOT(edit_bspline()));
    connect(ui->moveCurveButton, SIGNAL(pressed()), ui->graphicsView, SLOT(move_bsplines()));
    connect(ui->pointSizeSlider, SIGNAL(valueChanged(int)), ui->graphicsView, SLOT(changeControlPointSize(int)));
    connect(ui->surfaceWidthSlider, SIGNAL(valueChanged(int)), ui->graphicsView, SLOT(changeSurfaceWidth(int)));
    connect(ui->showControlMeshBox, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(showControlMesh(bool)));
    connect(ui->showControlPointsBox, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(showControlPoints(bool)));

    connect(ui->actionDistance_transform, SIGNAL(triggered()), ui->graphicsView, SLOT(createDistanceTransformDEBUG()));

    connect(ui->actionView_surface_in_3D, SIGNAL(triggered()), ui->graphicsView, SLOT(show3Dwidget()));
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
