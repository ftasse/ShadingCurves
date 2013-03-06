#include <QResizeEvent>
#include <QGLWidget>
#include <QFileDialog>
#include <QBrush>
#include <fstream>
#include "../Utilities/SurfaceUtils.h"
#include "../Views/GraphicsView.h"
#include "../Views/GLScene.h"

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setMouseTracking(true);
}

void GraphicsView::resizeEvent(QResizeEvent *event)
{
    if (scene())
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
    GLScene *my_scene = (GLScene *) scene();
    my_scene->adjustDisplayedImageSize();

    QGraphicsView::resizeEvent(event);
}

void GraphicsView::changeControlPointSize(int pointSize)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->pointSize = pointSize;
    my_scene->update();
}

void GraphicsView::showControlMesh(bool status)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->showControlMesh = status;
    my_scene->update();
}

void GraphicsView::showControlPoints(bool status)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->showControlPoints = status;
    my_scene->update();
}

void GraphicsView::create_bspline()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->createBSpline();
    my_scene->brush = false;
}

void GraphicsView::edit_bspline()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->sketchmode() = GLScene::ADD_CURVE_MODE;
    my_scene->brush = false;
}

void GraphicsView::move_bsplines()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->sketchmode() = GLScene::IDLE_MODE;
    my_scene->brush = false;
}

void GraphicsView::loadImage()
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
        GLScene *my_scene = (GLScene *) scene();
        my_scene->openImage(fileName.toStdString());
    }
}

void GraphicsView::saveImage()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save Image"),
                                    "",
                                    tr("Image files (*.jpg *.jpeg *.png *.gif *.bmp)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        my_scene->saveImage(fileName.toStdString());
    }
}

void GraphicsView::loadCurves()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open SplineGroup Files"),
                                    "",
                                    tr("SplineGroup files (*.curv)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        my_scene->openCurves(fileName.toStdString());
    }

}

void GraphicsView::saveCurves()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save SplineGroup"),
                                    "",
                                    tr("SplineGroup files (*.curv)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        my_scene->saveCurves(fileName.toStdString());
    }
}

void GraphicsView::changeSurfaceWidth(int surfaceWidth)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->setSurfaceWidth(surfaceWidth);
    my_scene->update();
}

void GraphicsView::saveOff()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save to OFF"),
                                    "",
                                    tr("OFF files (*.off)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        my_scene->saveOff(fileName.toStdString());
    }
}

// ******** DEBUG FUNCTIONS ********
void GraphicsView::createDistanceTransformDEBUG()
{
    dbw = new DebugWindow();

    GLScene *my_scene = (GLScene *) scene();
    cv::Mat curvesGrayIm = my_scene->curvesImage();
    cv::normalize(curvesGrayIm, curvesGrayIm, 0.0, 1.0, cv::NORM_MINMAX);

    cv::Mat dt;
    cv::distanceTransform(curvesGrayIm,dt,CV_DIST_L2,CV_DIST_MASK_PRECISE);

    cv::Mat dt_normalized;
    cv::normalize(dt, dt_normalized, 0.0, 255.0, cv::NORM_MINMAX);
    dt_normalized.convertTo(dt_normalized, CV_8UC1);
    cv::cvtColor(dt_normalized, dt_normalized, CV_GRAY2RGB);

    //Uncomment this to display image in an opencv windiw
    //cv::imshow("Distance transform", dt_normalized);

    QImage image(dt_normalized.data, dt_normalized.cols, dt_normalized.rows,
                 dt_normalized.step, QImage::Format_RGB888);

    dbw->setWindowTitle("Distance Transform");
    dbw->setImage(image);
    dbw->show();
}

void GraphicsView::show3Dwidget()
{
    GLScene *my_scene = (GLScene *) scene();
    glw = new MainWindow3D(my_scene->getImageHeight(), my_scene->getImageWidth(), my_scene->getImage());
    glw->setWindowTitle("3D View");
    glw->ctrlWidget1->checkClear->setChecked(true);

    // transfer meshes
    std::vector<std::string> surfaces = my_scene->OFFSurfaces();
    for (unsigned int i=0; i<surfaces.size(); ++i)
    {
        std::istringstream is(surfaces[i]);
        glw->load1(is);
        if (i == 0)
        {
            glw->ctrlWidget1->checkClear->setChecked(false);
        }
    }

    glw->ctrlWidget1->checkClear->setChecked(true);
    glw->ctrlWidget1->meshMenu->setCurrentIndex(0);
    glw->ctrlWidget1->subdLevelSpinbox->setValue(2);
    glw->show();
}

void GraphicsView::applyShading()
{
    applyShading(true, true);
}

void GraphicsView::applyShading(bool showImg, bool writeImg)
{

    GLScene *my_scene = (GLScene *) scene();
    glvs = new GLviewsubd(my_scene->getImageHeight(), my_scene->getImageWidth(), my_scene->getImage());
    glvs->offScreen = true;
//    glvs->offMainWindow = true;
    glvs->clear = false;

    // transfer meshes
    std::vector<std::string> surfaces = my_scene->OFFSurfaces();
    for (unsigned int i=0; i<surfaces.size(); ++i)
    {
        std::istringstream is(surfaces[i]);
        glvs->loadFile(is);
    }

    glvs->indexMesh = -1;
    glvs->showImg = showImg;
    glvs->writeImg = writeImg;
    glvs->setSubdivLevel(4); // calls updateGL

    my_scene->surfaceImg = glvs->img.clone();

//    cv::imshow("Shaded img 4 Henrik", glvs->img);

    delete glvs;
}

void GraphicsView::setBrush()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->brush = true;
    applyShading(false, false);
/*    QBrush brush(QColor(255,0,0),Qt::Dense3Pattern);
    my_scene->setForegroundBrush(brush);*/
}

void GraphicsView::changeBrushLightness(int type)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->brushType = type;
}

void GraphicsView::changeBrushSize(int size)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->brushSize = (float) size;
}

void GraphicsView::changeFreehand(bool freehand)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->freehand = freehand;
}
