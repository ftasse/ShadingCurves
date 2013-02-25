#include <QResizeEvent>
#include <QGLWidget>
#include <QFileDialog>
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
}

void GraphicsView::edit_bspline()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->sketchmode() = GLScene::ADD_CURVE_MODE;
}

void GraphicsView::move_bsplines()
{
    GLScene *my_scene = (GLScene *) scene();
    my_scene->sketchmode() = GLScene::IDLE_MODE;
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
    cv::distanceTransform(curvesGrayIm,dt,CV_DIST_L2,CV_DIST_MASK_3);

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

    // transfer mesh
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
//    if (surfaces.size() > 0)
//    {
//        glw->ctrlWidget1->numV->setText(QString::number(glw->glwidget1->meshCtrl[0]->my_numV));
//        glw->ctrlWidget1->numF->setText(QString::number(glw->glwidget1->meshCtrl[0]->my_numF));
//        glw->ctrlWidget1->numVsub->setText(QString::number(glw->glwidget1->meshCtrl[0]->my_numV/1000));
//        glw->ctrlWidget1->numFsub->setText(QString::number(glw->glwidget1->meshCtrl[0]->my_numF/1000));
//    }

//    //Comment this when the above transfer method is working
//    std::ofstream batch_ofs("surfaces.txt");
//    batch_ofs << surfaces.size() << std::endl;
//    for (int i=0; i<surfaces.size(); ++i)
//    {
//        std::stringstream ss;
//        ss << "tmp_surface_" << i << ".off";
//        const char *fname = ss.str().c_str();
//        std::ofstream ofs(fname);
//        ofs << surfaces[i];
//        ofs.close();

//        batch_ofs << fname << std::endl;
//    }
//    glw->loadBatch1("surfaces.txt");

    glw->ctrlWidget1->meshMenu->setCurrentIndex(0);
    glw->ctrlWidget1->subdLevelSpinbox->setValue(2);
    glw->show();
}

void GraphicsView::applyShading()
{
    cv::Mat img, imgOrig, imgNew;

    GLScene *my_scene = (GLScene *) scene();
    glvs = new GLviewsubd(my_scene->getImageHeight(), my_scene->getImageWidth(), my_scene->getImage());
    glvs->offScreen = true;
    glvs->offMainWindow = true;
    glvs->clear = false;

    // transfer mesh
    std::vector<std::string> surfaces = my_scene->OFFSurfaces();
    for (unsigned int i=0; i<surfaces.size(); ++i)
    {
        std::istringstream is(surfaces[i]);

        glvs->loadFile(is);
    }

    glvs->indexMesh = -1;
    glvs->setSubdivLevel(4);
    img = glvs->img;  // This img (after some normalisation)
                      // should be used as luminance difference
    cv::imshow("LumDif Image", img);

    if (img.cols > 0)
    {
        // grab original image
        my_scene->getImage()->copyTo(imgOrig);

        cv::imshow("Original Image", imgOrig);

        //convert to Lab space
        cv::cvtColor(imgOrig, imgOrig, CV_BGR2Lab);
        cv::cvtColor(img, img, CV_BGR2Lab);

        imgNew = cv::Mat::zeros(imgOrig.size(), imgOrig.type());    //Apparent, imgOrig.type() specifies how many how many bytes are used for each pixel and not the color code (such as RGB or Lab)
        cv::cvtColor(imgNew, imgNew, CV_BGR2Lab);       //So we should convert the new image from the default BGR to Lab before further processing

        // apply luminance adjustment
        for( int y = 0; y < imgOrig.rows; y++ )
        {
            for( int x = 0; x < imgOrig.cols; x++ )
            {
//                imgNew.at<cv::Vec3b>(y,x)[0] = 0;

//                imgNew.at<cv::Vec3b>(y,x)[0] =
 //                   img.at<cv::Vec3b>(y,x)[0];

                imgNew.at<cv::Vec3b>(y,x)[0] =
                    imgOrig.at<cv::Vec3b>(y,x)[0] + img.at<cv::Vec3b>(y,x)[0] - 127;
            }
        }

        //convert back to BGR
        cv::cvtColor(imgNew, imgNew, CV_Lab2BGR);

        cv::imshow("Resulting Image", imgNew);

//        my_scene->setImage(imgNew);
    }

    delete glvs;
}
