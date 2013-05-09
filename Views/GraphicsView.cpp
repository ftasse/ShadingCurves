#include <QResizeEvent>
#include <QGLWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QBrush>
#include <fstream>
#include "../Utilities/SurfaceUtils.h"
#include "../Views/GraphicsView.h"
#include "../Views/GLScene.h"

#define BACKUP_TIME_INTERVAL 5*60*1000      //(in milliseconds)

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    superSampling = 1;
    surfSubdLevel = 2;
    imgShowAll = false;
    imgWriteAll = false;
    clipping = false;
    clipMin = 0;
    clipMax = 100;
    shade = MATLAB;
    blackOut = false;
    subdivTime = 0;
    flatImage = true;
    clrVsTxtr = true;
    multiSubd = 0;
    pathToData = "../imageshading/Data";

    backupTimer = new QTimer(this);
    connect(backupTimer, SIGNAL(timeout()), this, SLOT(backupProject()));
    backupTimer->start(BACKUP_TIME_INTERVAL);

#ifdef QT_DEBUG
    backupName = "projectBackup.xml";
#else
    backupName = "projectBackup_" + QDateTime::currentDateTime().toString("dd.MM.yyyy_hh.mm.ss") + ".xml";
#endif
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
    my_scene->updateDisplay();
}

void GraphicsView::changeCurveSubdLevels(int value)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->curveSubdLevels = value;
    for (int i=0; i<my_scene->num_splines(); ++i)
        my_scene->spline(i).subv_levels = my_scene->curveSubdLevels;
    my_scene->recomputeAllSurfaces();
}

void GraphicsView::changeDisplaySubdLevels(int value)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->drawingSubdLevels = value;
    for (int i=0; i<my_scene->num_splines(); ++i)
        my_scene->spline(i).display_points.clear();
    my_scene->updateGeometry();
}


void GraphicsView::changeGlobalThickness(int value)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->globalThickness = value;
    my_scene->applyBlackCurves();
    my_scene->updateDisplay();
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
    my_scene->updateGeometry();
}

void GraphicsView::showNormals(bool status)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->showNormals = status;
    my_scene->updateGeometry();
}

void GraphicsView::showControlPoints(bool status)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->showControlPoints = status;
    my_scene->updatePoints();
}

void GraphicsView::showCurves(bool status)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->showCurves = status;
    my_scene->updateGeometry();
}

void GraphicsView::showColors(bool status)
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->showColors = status;
    my_scene->updatePoints();
}

void GraphicsView::create_bspline()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->createBSpline();
    my_scene->brush = false;

    my_scene->modeText = "Add Curve Mode";
    emit setStatusMessage("");
}

void GraphicsView::edit_bspline()
{
    GLScene *my_scene = (GLScene *) scene();
    //my_scene->sketchmode() = GLScene::ADD_CURVE_MODE;
    //my_scene->brush = false;
    //my_scene->modeText = "Edit Curve Mode";
    //emit setStatusMessage("");
    my_scene->insertPointNextToSelected();
}

void GraphicsView::move_bsplines()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->sketchmode() = GLScene::IDLE_MODE;
    my_scene->brush = false;

    my_scene->modeText = "Move Curve Mode";
    emit setStatusMessage("");
}

void GraphicsView::loadImage()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open Image Files"),
                                    pathToData,
                                    tr("Image files (*.jpg *.jpeg *.png *.gif *.bmp)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        my_scene->openImage(fileName.toStdString());
    }
}

void GraphicsView::loadBackgroungImage()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open Background Image Files"),
                                    pathToData,
                                    tr("Background Image files (*.jpg *.jpeg *.png *.gif *.bmp)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        cv::Mat backgoundImage = cv::imread(fileName.toStdString());

        my_scene->splineGroup().colorMapping.clear();
        my_scene->changeResolution(backgoundImage.cols, backgoundImage.rows, false);

        my_scene->orgBlankImage = backgoundImage.clone();
        my_scene->currentImage() = my_scene->orgBlankImage.clone();
        my_scene->recomputeAllSurfaces();
    }
}

void GraphicsView::saveScreenshot()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save Screenshot"),
                                    pathToData,
                                    tr("Image files (*.png *.jpg *.jpeg *.gif *.bmp)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        QSize s = my_scene->sceneRect().size().toSize();
        my_scene->displayModeLabel->setVisible(false);
        my_scene->glWidget->makeCurrent();

        glPixelStorei(GL_PACK_ROW_LENGTH,    0);;
        glPixelStorei(GL_PACK_ALIGNMENT,     1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        QGLFramebufferObjectFormat format;
        format.setSamples(4);
        QGLFramebufferObject renderFbo(s.width(), s.height(), format);
        QPainter fboPainter(&renderFbo);
        fboPainter.setRenderHint(QPainter::Antialiasing);
        fboPainter.setRenderHint(QPainter::HighQualityAntialiasing);
        my_scene->render(&fboPainter);
        renderFbo.toImage().save(fileName);

        my_scene->displayModeLabel->setVisible(true);
    }
}

void GraphicsView::saveCurrentState()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save Current State"),
                                    pathToData,
                                    tr("Image files (*.png *.jpg *.jpeg *.gif *.bmp)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        cv::Mat img = my_scene->imageWithGeometry();
        cv::imwrite(fileName.toStdString().c_str(), img);
    }
}

void GraphicsView::saveImage()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save Image"),
                                    pathToData,
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
                                    pathToData,
                                    tr("SplineGroup files (*.curv)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        my_scene->clearCurrentSelections(false);
        my_scene->openCurves(fileName.toStdString());
    }

}

void GraphicsView::saveCurves()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save SplineGroup"),
                                    pathToData,
                                    tr("SplineGroup files (*.curv)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        my_scene->saveCurves(fileName.toStdString());
    }
}


void GraphicsView::loadProject()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open Image Shading Project Files"),
                                    pathToData,
                                    tr("Image Shading Project files (*.xml *.yml *.yaml)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();

        my_scene->clearCurrentSelections();

        my_scene->splineGroup().loadAll(fileName.toStdString());
        for (int i=0; i<my_scene->num_splines(); ++i)
            my_scene->spline(i).subv_levels = my_scene->curveSubdLevels;

        cv::resize(my_scene->orgBlankImage, my_scene->orgBlankImage, my_scene->splineGroup().imageSize);
        cv::resize(my_scene->currentImage(), my_scene->currentImage(), my_scene->splineGroup().imageSize);
        my_scene->curDisplayMode = 0;
        my_scene->changeDisplayModeText();
        my_scene->changeResolution(my_scene->splineGroup().imageSize.width, my_scene->splineGroup().imageSize.height);

        my_scene->curSplineRef() = -1;
        my_scene->selectedObjects.clear();
    }
}

void GraphicsView::saveProject()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save Image Shading Project"),
                                    pathToData,
                                    tr("Image Shading Project Files (*.xml *.yml *.yaml)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        GLScene *my_scene = (GLScene *) scene();
        my_scene->splineGroup().imageSize = cv::Size(my_scene->currentImage().cols, my_scene->currentImage().rows);
        my_scene->splineGroup().saveAll(fileName.toStdString());
    }
}

void GraphicsView::backupProject()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->splineGroup().imageSize = cv::Size(my_scene->currentImage().cols, my_scene->currentImage().rows);
    my_scene->splineGroup().saveAll(backupName.toStdString().c_str());
}

void GraphicsView::toggleBackupStatus(bool b)
{
    if (b)
    {
        bool ok;
        int i = QInputDialog::getInt(this, tr("Backup Timer"),
                                          tr("Backup Interval (mins):"), backupTimer->interval()/(60*1000), 1, 60, 1, &ok);
        if (ok)
        {
            backupTimer->start(i*60*1000);
            qDebug("Enable Project Backup (every %d mins)", i);
        }
    }
    else
    {
        backupTimer->stop();
        qDebug("Disable Project Backup");
    }
}

void GraphicsView::saveOff()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                    tr("Save to OFF"),
                                    pathToData,
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

//    cv::Mat edges;
//    cv::blur(dt_normalized, edges, cv::Size(10, 10));
//    cv::Canny(edges, edges, 2, 6, 3);
//    cv::imshow("Edges (Canny) in the DT", edges);

    cv::cvtColor(dt_normalized, dt_normalized, CV_GRAY2RGB);

    //Uncomment this to display image in an opencv windiw
    //cv::imshow("Distance transform", dt_normalized);

    QImage image(dt_normalized.data, dt_normalized.cols, dt_normalized.rows,
                 dt_normalized.step, QImage::Format_RGB888);

    dbw->setWindowTitle("Distance Transform");
    dbw->setImage(image);
    dbw->setAttribute( Qt::WA_DeleteOnClose );
    dbw->show();
}

void GraphicsView::showDistanceTransform3D()
{
    GLScene *my_scene = (GLScene *) scene();
    cv::Mat curvesGrayIm = my_scene->curvesImage();
    cv::normalize(curvesGrayIm, curvesGrayIm, 0.0, 1.0, cv::NORM_MINMAX);

    cv::Mat dt;
    cv::distanceTransform(curvesGrayIm,dt,CV_DIST_L2,CV_DIST_MASK_PRECISE);

    cv::Mat dt_normalized;
    cv::normalize(dt, dt_normalized, 0.0, 255.0, cv::NORM_MINMAX);
    dt_normalized.convertTo(dt_normalized, CV_8UC1);

    cv::convertScaleAbs(dt_normalized, dt_normalized, 200.0/255.0, 0);

    cv::cvtColor(dt_normalized, dt_normalized, CV_GRAY2RGB);

    dt_normalized.convertTo(dt_normalized, CV_32FC3);

    int theType = dt_normalized.type();
    theType = my_scene->getImage()->type();

    //DT to 3D
    std::stringstream str;
    str << "OFF" << std::endl;
    str << dt_normalized.cols * dt_normalized.rows << " " << (dt_normalized.cols - 1) * (dt_normalized.rows - 1) << " " << 0 << std::endl;
    for (int i = 0 ; i < dt_normalized.cols ; i++) // height
    {
        for (int j = 0 ; j < dt_normalized.rows ; j++)
        {
            str << i + 0.5 << " " << j + 0.5 << " " << dt_normalized.at<cv::Vec3f>(j,i)[0] - 100 << std::endl;
//            str << i << " " << j << " " << 10 << std::endl;
        }
    }
    for (int i = 0 ; i < dt_normalized.cols-1 ; i++)
    {
        for (int j = 0 ; j < dt_normalized.rows-1 ; j++)
        {
            str << "4" << " " << j + i * dt_normalized.rows << " " <<
                                 j + 1 + i * dt_normalized.rows << " " <<
                                 j + 1 + (i + 1) * dt_normalized.rows << " " <<
                                 j + (i + 1) * dt_normalized.rows << std::endl;
        }
    }
    str << 128 << " " << 128 << " " << 128 << std::endl;

//    std::cout << str.str();

    dt_normalized.convertTo(dt_normalized, CV_8UC3);
    theType = dt_normalized.type();


    glw = new MainWindow3D(dt_normalized.cols, dt_normalized.rows, my_scene->getImage());
//    glw = new MainWindow3D(dt_normalized.cols, dt_normalized.rows, &dt_normalized);
    glw->setWindowTitle("3D DT");
    std::istringstream is(str.str());
    glw->load1(is);
    glw->ctrlWidget1->checkFeature->setChecked(false);
    glw->ctrlWidget1->checkCtrl->setChecked(false);
    glw->ctrlWidget1->radioMeshHeight->setChecked(true);
    glw->ctrlWidget1->checkFrame->setChecked(false);

    glw->setAttribute( Qt::WA_DeleteOnClose );
    glw->show();
}

void GraphicsView::showCurvesImage3D()
{
    GLScene *my_scene = (GLScene *) scene();
    cv::Mat curvesGrayIm = my_scene->curvesImage();
    cv::normalize(curvesGrayIm, curvesGrayIm, 0.0, 255.0, cv::NORM_MINMAX);

    curvesGrayIm.convertTo(curvesGrayIm, CV_8UC1);
    cv::convertScaleAbs(curvesGrayIm,curvesGrayIm, -1, 255);
    cv::convertScaleAbs(curvesGrayIm,curvesGrayIm, 100.0/255.0, 0);
    cv::cvtColor(curvesGrayIm, curvesGrayIm, CV_GRAY2RGB);

    curvesGrayIm.convertTo(curvesGrayIm, CV_32FC3);

    int theType = curvesGrayIm.type();
    theType = my_scene->getImage()->type();

    //DT to 3D
    std::stringstream str;
    str << "OFF" << std::endl;
    str << curvesGrayIm.cols * curvesGrayIm.rows << " " << (curvesGrayIm.cols - 1) * (curvesGrayIm.rows - 1) << " " << 0 << std::endl;
    for (int i = 0 ; i < curvesGrayIm.cols ; i++) // height
    {
        for (int j = 0 ; j < curvesGrayIm.rows ; j++)
        {
            str << i + 0.5 << " " << j + 0.5 << " " << curvesGrayIm.at<cv::Vec3f>(j,i)[0] << std::endl;
//            str << i << " " << j << " " << 10 << std::endl;
        }
    }
    for (int i = 0 ; i < curvesGrayIm.cols-1 ; i++)
    {
        for (int j = 0 ; j < curvesGrayIm.rows-1 ; j++)
        {
            str << "4" << " " << j + i * curvesGrayIm.rows << " " <<
                                 j + 1 + i * curvesGrayIm.rows << " " <<
                                 j + 1 + (i + 1) * curvesGrayIm.rows << " " <<
                                 j + (i + 1) * curvesGrayIm.rows << std::endl;
        }
    }
    str << 128 << " " << 128 << " " << 128 << std::endl;

//    std::cout << str.str();

    curvesGrayIm.convertTo(curvesGrayIm, CV_8UC3);
    theType = curvesGrayIm.type();


    glw = new MainWindow3D(curvesGrayIm.cols, curvesGrayIm.rows, my_scene->getImage());
//    glw = new MainWindow3D(curvesGrayIm.cols, curvesGrayIm.rows, &dt_normalized);
    glw->setWindowTitle("3D DT");
    std::istringstream is(str.str());
    glw->load1(is);
    glw->ctrlWidget1->checkFeature->setChecked(false);
    glw->ctrlWidget1->checkCtrl->setChecked(false);
    glw->ctrlWidget1->radioMeshHeight->setChecked(true);
    glw->ctrlWidget1->checkFrame->setChecked(false);
    glw->setAttribute( Qt::WA_DeleteOnClose );
    glw->show();
}

void GraphicsView::show3Dwidget()
{
    GLScene *my_scene = (GLScene *) scene();
    std::vector<std::string> surfaces = my_scene->OFFSurfaces();
    GLviewsubd  *glvs;

    glw = new MainWindow3D(my_scene->getImageHeight(), my_scene->getImageWidth(), my_scene->getImage());
    glw->setWindowTitle("3D View");
    glw->ctrlWidget1->checkClear->setChecked(true);

    // transfer meshes
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

    glvs = glw->glwidget1;
    glvs->indexMesh = -1; //all meshes visible
    glvs->super = superSampling; //supersampling (1, 2, or 4)
    glvs->showImg = true;
    glvs->writeImg = true;
    glvs->clipping = clipping;
    glvs->clipMin = clipMin;
    glvs->clipMax = clipMax;
    glvs->shade = shade;
    glvs->blackOut = blackOut;
    glvs->flatImage = flatImage;
    glvs->clrVsTxtr = clrVsTxtr;

    glw->ctrlWidget1->subdLevelSpinbox->setValue(2);

    glw->setAttribute( Qt::WA_DeleteOnClose );
    glw->show();
}

void GraphicsView::applyShading()
{
    applyShading(imgShowAll, imgWriteAll);
}

void GraphicsView::applyShading(bool showImg, bool writeImg)
{

    GLScene *my_scene = (GLScene *) scene();
    std::vector<std::string> surfaces = my_scene->OFFSurfaces();

            QElapsedTimer timer, timer2, timer3;
            qDebug(); // << endl;// << "GVT: Timer started";
            timer.start();
            timer2.start();

    glvs = new GLviewsubd(my_scene->getImageHeight(), my_scene->getImageWidth(), my_scene->getImage());

    glvs->offScreen = true;
    glvs->clear = false;

    // transfer meshes
    for (unsigned int i=0; i<surfaces.size(); ++i)
    {
        std::istringstream is(surfaces[i]);
        glvs->loadFile(is);
    }

            qDebug() << "GVT: Loading surfaces to 3D: " << timer2.elapsed() << "ms";
            timer2.restart();

    glvs->indexMesh = -1; //all meshes visible
    glvs->super = superSampling; //supersampling (1, 2, or 4)
    glvs->showImg = showImg;
    glvs->writeImg = writeImg;
    glvs->clipping = clipping;
    glvs->clipMin = clipMin;
    glvs->clipMax = clipMax;
    glvs->shade = shade;
    glvs->blackOut = blackOut;
    glvs->flatImage = flatImage;
    glvs->clrVsTxtr = clrVsTxtr;
    glvs->setSubdivLevel(surfSubdLevel); // calls updateGL

    subdivTime = glvs->subdivTime;

    my_scene->surfaceImg = glvs->img.clone();
    my_scene->shadedImg = glvs->imgFillShaded.clone();

    delete glvs;
    int subdivisionShadingTime = timer2.elapsed();

    timer3.start();
    my_scene->applyBlackCurves();
    int blackCurvesTime = timer3.elapsed();

    int totalShadingTime = timer.elapsed();
    emit setStatusMessage(my_scene->modeText + " [" + my_scene->stats + "]" +
                          "  [Shading: " + QString::number(totalShadingTime) +
                          " ms (incl subdivision time " + QString::number(subdivTime)  + " ms)]");
    qDebug() << "GVT: Subdivision and shading: " << subdivisionShadingTime << "ms";
    qDebug() << "GVT: Adding Black Curves: " << blackCurvesTime << "ms";
    qDebug() << "GVTALL: Complete shading: " << totalShadingTime << "ms";
    emit setTimeOutput(QString::number(totalShadingTime));
    emit setTimeOutputSub(QString::number(subdivTime));

    // switch to result in my_scene
    my_scene->curDisplayMode = 3;
    my_scene->changeDisplayModeText();
    my_scene->updateDisplay();
}

void GraphicsView::runMultiSubd()
{
    int i, m, time, total, avrg, min, max;

    emit setMultiSubdOutputMin("wait");
    emit setMultiSubdOutputAvrg("wait");
    emit setMultiSubdOutputMax("wait");

    this->setCursor(Qt::WaitCursor);
    QApplication::processEvents();

    total = 0;
    min = 1000000;
    max = 0;
    m = (int)(pow(10.0, multiSubd) + 0.5);

    GLScene *my_scene = (GLScene *) scene();
    std::vector<std::string> surfaces = my_scene->OFFSurfaces();

    for (i = 0 ; i < m ; i++)
    {
        glvs = new GLviewsubd(my_scene->getImageHeight(), my_scene->getImageWidth(), my_scene->getImage());

        glvs->offScreen = true;
        glvs->clear = false;

        // transfer meshes
        for (unsigned int i=0; i<surfaces.size(); ++i)
        {
            std::istringstream is(surfaces[i]);
            glvs->loadFile(is);
        }

        glvs->indexMesh = -1; //all meshes visible
        glvs->super = superSampling; //supersampling (1, 2, or 4)
        glvs->showImg = imgShowAll;
        glvs->writeImg = imgWriteAll;
        glvs->clipping = clipping;
        glvs->clipMin = clipMin;
        glvs->clipMax = clipMax;
        glvs->shade = shade;
        glvs->blackOut = blackOut;
        glvs->flatImage = flatImage;
        glvs->clrVsTxtr = clrVsTxtr;
        glvs->setSubdivLevel(surfSubdLevel); // calls updateGL

        time = glvs->subdivTime;
        total += time;
        if (time > max)
        {
            max = time;
        }
        if (time < min)
        {
            min = time;
        }

        delete glvs;
    }

    avrg = (int)((double)total / (double)m);

    emit setMultiSubdOutputMin(QString::number(min));
    emit setMultiSubdOutputAvrg(QString::number(avrg));
    emit setMultiSubdOutputMax(QString::number(max));

    this->setCursor(Qt::ArrowCursor);
    QApplication::processEvents();
}

void GraphicsView::setBrush()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->brush = true;
    applyShading(false, false);
/*    QBrush brush(QColor(255,0,0),Qt::Dense3Pattern);
    my_scene->setForegroundBrush(brush);*/

    my_scene->modeText = "Brush mode";
    emit setStatusMessage("");
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
    applyShading();
}

void GraphicsView::setSuper2()
{
    superSampling = 2;
    applyShading();
}

void GraphicsView::setSuper4()
{
    superSampling = 4;
    applyShading();
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

void GraphicsView::setClipping(bool b)
{
    clipping = b;
}

void GraphicsView::setClipMin(int min)
{
    clipMin = min;
}

void GraphicsView::setClipMax(int max)
{
    clipMax = max;
}

void GraphicsView::setShadingLab()
{
    shade = CVLAB;
    applyShading();
}

void GraphicsView::setShadingHLS()
{
    shade = CVHLS;
    applyShading();
}

void GraphicsView::setShadingOwn()
{
    shade = OWN;
    applyShading();
}

void GraphicsView::setShadingMatlab()
{
    shade = MATLAB;
    applyShading();
}

void GraphicsView::setShadingYxy()
{
    shade = YXY;
    applyShading();
}

void GraphicsView::setShadingRGB()
{
    shade = RGB;
    applyShading();
}

void GraphicsView::setBlackOut(bool b)
{
    blackOut = b;
    applyShading();
}

void GraphicsView::setFlatImage(bool b)
{
    flatImage = b;
    applyShading();
}

void GraphicsView::setClrVsTxtr(bool b)
{
    clrVsTxtr = b;
    applyShading();
}

void GraphicsView::setMultiSubd(int m)
{
    multiSubd = m;
}
