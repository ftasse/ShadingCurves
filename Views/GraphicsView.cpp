#include <QResizeEvent>
#include <QGLWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QBrush>
#include <fstream>
#include "../Utilities/SurfaceUtils.h"
#include "../Views/GraphicsView.h"
#include "../Views/GLScene.h"

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    superSampling = 1;
    surfSubdLevel = 4;
    imgShowAll = true;
    imgWriteAll = true;
}

void GraphicsView::resizeEvent(QResizeEvent *event)
{
    if (scene())
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
    GLScene *my_scene = (GLScene *) scene();
    my_scene->adjustDisplayedImageSize();

    QGraphicsView::resizeEvent(event);
}

/*void GraphicsView::wheelEvent(QWheelEvent* event) {

    //Get the position of the mouse before scaling, in scene coords
    QPointF pointBeforeScale(mapToScene(event->pos()));

    //Get the original screen centerpoint
    QPointF screenCenter = currentCenterPoint;

    //Scale the view ie. do the zoom
    double scaleFactor = 1.15; //How fast we zoom
    GLScene *my_scene = (GLScene *) scene();
    if(event->delta() > 0) {
        //Zoom in
        scale(scaleFactor, scaleFactor);

    } else {
        //Zooming out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }

    //Get the position after scaling, in scene coords
    QPointF pointAfterScale(mapToScene(event->pos()));

    //Get the offset of how the screen moved
    QPointF offset = pointBeforeScale - pointAfterScale;

    //Adjust to the new center for correct zooming
    QPointF newCenter = screenCenter + offset;
    setCenter(newCenter);
}

//Set the current centerpoint in the
void GraphicsView::setCenter(const QPointF& centerPoint) {
    //Get the rectangle of the visible area in scene coords
    QRectF visibleArea = mapToScene(rect()).boundingRect();

    //Get the scene area
    QRectF sceneBounds = sceneRect();

    double boundX = visibleArea.width() / 2.0;
    double boundY = visibleArea.height() / 2.0;
    double boundWidth = sceneBounds.width() - 2.0 * boundX;
    double boundHeight = sceneBounds.height() - 2.0 * boundY;

    //The max boundary that the centerPoint can be to
    QRectF bounds(boundX, boundY, boundWidth, boundHeight);

    if(bounds.contains(centerPoint)) {
        //We are within the bounds
        currentCenterPoint = centerPoint;
    } else {
        //We need to clamp or use the center of the screen
        if(visibleArea.contains(sceneBounds)) {
            //Use the center of scene ie. we can see the whole scene
            currentCenterPoint = sceneBounds.center();
        } else {

            currentCenterPoint = centerPoint;

            //We need to clamp the center. The centerPoint is too large
            if(centerPoint.x() > bounds.x() + bounds.width()) {
                currentCenterPoint.setX(bounds.x() + bounds.width());
            } else if(centerPoint.x() < bounds.x()) {
                currentCenterPoint.setX(bounds.x());
            }

            if(centerPoint.y() > bounds.y() + bounds.height()) {
                currentCenterPoint.setY(bounds.y() + bounds.height());
            } else if(centerPoint.y() < bounds.y()) {
                currentCenterPoint.setY(bounds.y());
            }

        }
    }

    //Update the scrollbars
    centerOn(currentCenterPoint);
}*/

void GraphicsView::changeControlPointSize(int pointSize)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->pointSize = pointSize;
    my_scene->update();
}

void GraphicsView::changeResolution()
{
    GLScene *my_scene = (GLScene *) scene();
    std::stringstream dss;
    if (my_scene->targetImage().cols > 0)
        dss << my_scene->targetImage().cols << " " << my_scene->targetImage().rows;
    else
        dss << my_scene->currentImage().cols << " " << my_scene->currentImage().rows;

    bool ok;
    QString text = QInputDialog::getText(this, tr("New image resolution"),
                                         tr("Width, Height:"), QLineEdit::Normal,
                                         QString(dss.str().c_str()), &ok);
    if (ok && !text.isEmpty())
    {
        std::stringstream ss;
        ss << text.toStdString();

        int width, height;
        ss >> width >> height;
        if (width > 0 && height > 0)
        {
            my_scene->changeResolution(width, height);
        }
    }
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

void GraphicsView::showCurves(bool status)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->showCurves = status;
    my_scene->update();
}

void GraphicsView::create_bspline()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->createBSpline();
    my_scene->brush = false;

    emit setStatusMessage("Add Curve Mode");
}

void GraphicsView::edit_bspline()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->sketchmode() = GLScene::ADD_CURVE_MODE;
    my_scene->brush = false;

    emit setStatusMessage("Edit Curve Mode");
}

void GraphicsView::move_bsplines()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->sketchmode() = GLScene::IDLE_MODE;
    my_scene->brush = false;

    emit setStatusMessage("Move Curve Mode");
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
    applyShading(imgShowAll, imgWriteAll);
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
    glvs->super = superSampling; //supersampling (1, 2, or 4)
    glvs->showImg = showImg;
    glvs->writeImg = writeImg;
    glvs->setSubdivLevel(surfSubdLevel); // calls updateGL

    my_scene->surfaceImg = glvs->img.clone();
    my_scene->resultImg = glvs->imgFillShaded.clone();

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

    emit setStatusMessage("Brush Mode");
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

void GraphicsView::changeBrushTypeC(bool val)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->discreteB = !val;
}

void GraphicsView::changeBrushTypeD(bool val)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->discreteB = val;
}

void GraphicsView::setSuper1()
{
    superSampling = 1;
}

void GraphicsView::setSuper2()
{
    superSampling = 2;
}

void GraphicsView::setSuper4()
{
    superSampling = 4;
}

void GraphicsView::setImgShowAll(bool b)
{
    imgShowAll = b;
}

void GraphicsView::setImgWriteAll(bool b)
{
    imgWriteAll = b;
}

void GraphicsView::setSurfSubdLevel(int level)
{
    surfSubdLevel = level;
}
