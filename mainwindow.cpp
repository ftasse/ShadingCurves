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
    ui->graphicsView->setMouseTracking(false);

    addDockWidget(Qt::LeftDockWidgetArea, scene->shadingProfileView);
    scene->shadingProfileView->toggleViewAction()->setEnabled(true);
    scene->shadingProfileView->toggleViewAction()->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    ui->menuFile->addAction(scene->shadingProfileView->toggleViewAction());
    setStyleSheet("QMainWindow::separator { border-color: rgb(0, 0, 0); width: 1px; border-width: 1px; border-style: solid;}");

    ui->graphicsView->setViewport(new QGLWidget( QGLFormat(QGL::SampleBuffers  | QGL::DirectRendering)));
    ui->graphicsView->setViewportUpdateMode( QGraphicsView::FullViewportUpdate);

    showStatusMessage("Idle Mode");

    //Connections
    connect(ui->actionDelete_all_curves, SIGNAL(triggered()), scene, SLOT(delete_all()));
    connect(ui->actionOpen_Image, SIGNAL(triggered()), ui->graphicsView, SLOT(loadImage()));
    connect(ui->actionSave_Image, SIGNAL(triggered()), ui->graphicsView, SLOT(saveImage()));
    connect(ui->actionOpen_Curves, SIGNAL(triggered()), ui->graphicsView, SLOT(loadCurves()));
    connect(ui->actionSave_Curves, SIGNAL(triggered()), ui->graphicsView, SLOT(saveCurves()));
    connect(ui->actionCreate_BSpline, SIGNAL(triggered()), ui->graphicsView, SLOT(create_bspline()));
    connect(ui->actionChange_resolution, SIGNAL(triggered()), ui->graphicsView, SLOT(changeResolution()));

    connect(ui->createCurveButton, SIGNAL(pressed()), ui->graphicsView, SLOT(create_bspline()));
    connect(ui->editCurveButton, SIGNAL(pressed()), ui->graphicsView, SLOT(edit_bspline()));
    connect(ui->moveCurveButton, SIGNAL(pressed()), ui->graphicsView, SLOT(move_bsplines()));
    connect(ui->pointSizeSlider, SIGNAL(valueChanged(int)), ui->graphicsView, SLOT(changeControlPointSize(int)));
    connect(ui->showControlMeshBox, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(showControlMesh(bool)));
    connect(ui->showControlPointsBox, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(showControlPoints(bool)));
    connect(ui->ShowCurvesBox, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(showCurves(bool)));
    connect(ui->curveSubdivisionsBox, SIGNAL(valueChanged(int)), ui->graphicsView, SLOT(changeCurveSubdLevels(int)));

    connect(ui->inward_suface_box, SIGNAL(clicked(bool)), this, SLOT(change_inward_outward_direction()));
    connect(ui->outward_surface_box, SIGNAL(clicked(bool)), this, SLOT(change_inward_outward_direction()));
    connect(ui->slope_curve_box, SIGNAL(clicked(bool)), this, SLOT(change_slope_curve()));
    connect(ui->uniform_subdivision_curve_box, SIGNAL(clicked(bool)), this, SLOT(change_uniform_subdivision()));
    connect(ui->surfaceWidthSlider, SIGNAL(sliderReleased()), this, SLOT(change_bspline_parameters()));
    connect(scene, SIGNAL(bspline_parameters_changed(bool,float,bool,bool,bool,bool)), this, SLOT(update_bspline_parameters_ui(bool,float,bool,bool,bool,bool)));

    connect(ui->subdivideCurveButton, SIGNAL(clicked(bool)), scene, SLOT(subdivide_current_spline()));
    connect(ui->onlyShowCurvePointsBox, SIGNAL(toggled(bool)), scene, SLOT(toggleShowCurrentCurvePoints(bool)));

    connect(ui->actionDistance_transform, SIGNAL(triggered()), ui->graphicsView, SLOT(createDistanceTransformDEBUG()));
    connect(ui->actionDT_in_3D, SIGNAL(triggered()), ui->graphicsView, SLOT(showDistanceTransform3D()));
    connect(ui->actionCurves_in_3D, SIGNAL(triggered()), ui->graphicsView, SLOT(showCurvesImage3D()));

    connect(ui->actionView_surface_in_3D, SIGNAL(triggered()), ui->graphicsView, SLOT(show3Dwidget()));

    connect(ui->getShadingButton, SIGNAL(pressed()), ui->graphicsView, SLOT(applyShading()));

    connect(ui->setBrushButton, SIGNAL(pressed()), ui->graphicsView, SLOT(setBrush()));
    connect(ui->brushLightness, SIGNAL(valueChanged(int)), ui->graphicsView, SLOT(changeBrushLightness(int)));
    connect(ui->brushSizeSlider, SIGNAL(valueChanged(int)), ui->graphicsView, SLOT(changeBrushSize(int)));
    connect(ui->freehandBox, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(changeFreehand(bool)));
    connect(ui->radioContinuous, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(changeBrushTypeC(bool)));
    connect(ui->radioDiscrete, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(changeBrushTypeD(bool)));

    connect(ui->graphicsView, SIGNAL(setStatusMessage(QString)), this, SLOT(showStatusMessage(QString)));
    connect(scene, SIGNAL(setStatusMessage(QString)), this, SLOT(showStatusMessage(QString)));

    connect(ui->super1, SIGNAL(clicked()), ui->graphicsView, SLOT(setSuper1()));
    connect(ui->super2, SIGNAL(clicked()), ui->graphicsView, SLOT(setSuper2()));
    connect(ui->super4, SIGNAL(clicked()), ui->graphicsView, SLOT(setSuper4()));
    connect(ui->spinBoxSubd, SIGNAL(valueChanged(int)), ui->graphicsView, SLOT(setSurfSubdLevel(int)));
    connect(ui->imgShowAll, SIGNAL(clicked(bool)), ui->graphicsView, SLOT(setImgShowAll(bool)));
    connect(ui->imgWriteAll, SIGNAL(clicked(bool)), ui->graphicsView, SLOT(setImgWriteAll(bool)));
}

MainWindow::~MainWindow()
{
    delete scene;
    delete ui;
}

void MainWindow::change_slope_curve()
{
    if (ui->slope_curve_box->isChecked())
    {
        ui->inward_suface_box->setChecked(true);
        ui->outward_surface_box->setChecked(true);
        ui->uniform_subdivision_curve_box->setChecked(false);
    }
    change_bspline_parameters();
}

void MainWindow::change_uniform_subdivision()
{
    if (ui->uniform_subdivision_curve_box->isChecked())
    {
        ui->slope_curve_box->setChecked(false);
    }
    change_bspline_parameters();
}

void MainWindow::change_inward_outward_direction()
{
    if (!ui->inward_suface_box->isChecked() || !ui->outward_surface_box->isChecked())
    {
        ui->slope_curve_box->setChecked(false);
    }
    change_bspline_parameters();
}

void MainWindow::change_bspline_parameters()
{

    scene->change_bspline_parameters(ui->surfaceWidthSlider->value(),
                                     ui->slope_curve_box->isChecked(),
                                     ui->uniform_subdivision_curve_box->isChecked(),
                                     ui->inward_suface_box->isChecked(),
                                     ui->outward_surface_box->isChecked());
}

void MainWindow::update_bspline_parameters_ui(bool enabled, float extent, bool _is_slope, bool _has_uniform_subdivision, bool _has_inward, bool _has_outward)
{
    ui->surfaceWidthSlider->setEnabled(enabled);
    ui->slope_curve_box->setEnabled(enabled);
    ui->uniform_subdivision_curve_box->setEnabled(enabled);
    ui->inward_suface_box->setEnabled(enabled);
    ui->outward_surface_box->setEnabled(enabled);

    ui->surfaceWidthSlider->setValue(extent);
    ui->slope_curve_box->setChecked(_is_slope);
    ui->uniform_subdivision_curve_box->setChecked(_has_uniform_subdivision);
    ui->inward_suface_box->setChecked(_has_inward);
    ui->outward_surface_box->setChecked(_has_outward);
}

void MainWindow::center()
{
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}

void MainWindow::showStatusMessage(QString message)
{
    statusBar()->showMessage(message, 0);
}
