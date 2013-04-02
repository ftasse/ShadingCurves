#include <QPainter>
#include <QPaintEngine>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QColorDialog>
#include <QDialog>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <set>
#include <stdio.h>
#include <QDebug>

#include "../Utilities/SurfaceUtils.h"

#include "GLScene.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include "glew/GL/glew.h"
#include <GL/glu.h>

#include <QGLWidget>

static const unsigned int  SELECTION_BUFFER_SIZE = 10000;
static const unsigned int  NAME_STACK_SIZE       = 2;
static const unsigned int  IMAGE_NODE_ID = 0;
static const unsigned int  CPT_NODE_ID = 1;
static const unsigned int  SPLINE_NODE_ID = 2;
static const unsigned int  SURFACE_NODE_ID = 3;

bool isEqual(Point3d p, Point3d q)
{
    return (fabs(p.x()-q.x()) < 1e-05 && fabs(p.y()-q.y()) < 1e-05); // && fabs(p.z()-q.z()) < 1e-05
}

GLScene::GLScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_curSplineIdx = -1;
    m_sketchmode = IDLE_MODE;
    pointSize = 10.0;
    showControlMesh = true;
    showControlPoints = true;
    showCurrentCurvePoints = false;
    showCurves = true;
    curveSubdLevels = 5;
    hasMoved = false;
    brush = false;
    brushType = 0;
    brushSize = 10;
    freehand = false;
    discreteB = false;

    m_scale = 1.0f;
    m_translation = QPointF(0.0,0.0);
    inPanMode = false;

    ellipseGroup = new QGraphicsItemGroup ();
    addItem(ellipseGroup);

    displayModeLabel = new QLabel ();
    displayModeLabel->setAutoFillBackground(false);
    displayModeLabel->setGeometry(10, 10, 100, 20);
    displayModeLabel->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    QGraphicsProxyWidget* proxyWidget = addWidget(displayModeLabel);
    proxyWidget->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    shadingProfileView = NULL;
    shadingProfileView = new ShadingProfileView ();
    shadingProfileView->setWindowTitle("  Shading Profile");
    shadingProfileView->centralWidget->setEnabled(false);
    shadingProfileView->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    shadingProfileView->setAllowedAreas(Qt::LeftDockWidgetArea);
    shadingProfileView->setFloating(true);

    connect(shadingProfileView, SIGNAL(controlPointAttributesChanged(int)), this, SLOT(updateConnectedSurfaces(int)));

    interactiveShading = false;
}

GLScene:: ~GLScene()
{
}

void GLScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
    if (event->isAccepted()) return;

    if (event->buttons() == Qt::NoButton)
    {
        event->accept();
        return;
    }

    if(!inPanMode && brush){
        Qt::BrushStyle style;

        int val = 255;

        if(!freehand) {
            QPointF imgP = sceneToImageCoords(event->scenePos());
            val = surfaceImg.at<cv::Vec3b>(imgP.y(),imgP.x())[0];


            if(discreteB) {
                brushType = val / 32;
//                if(val<32)
//                    brushType = 0;
//                else if(val>=32&&val<64)
//                    brushType = 1;
//                else if(val>=64&&val<96)
//                    brushType = 2;
//                else if(val>=96&&val<128)
//                    brushType = 3;
//                else if(val>=128&&val<159)
//                    brushType = 4;
//                else if(val>=159&&val<191)
//                    brushType = 5;
//                else if(val>=191&&val<223)
//                    brushType = 6;
//                else
//                    brushType = 7;
            }
        }

        QColor c;
        if(discreteB)
            c = QColor(0,0,0);
        else
            c = QColor(val,val,val);

        if(discreteB) {
            if(brushType==0) style = Qt::SolidPattern;
            else if(brushType==1) style = Qt::Dense1Pattern;
            else if(brushType==2) style = Qt::Dense2Pattern;
            else if(brushType==3) style = Qt::Dense3Pattern;
            else if(brushType==4) style = Qt::Dense4Pattern;
            else if(brushType==5) style = Qt::Dense5Pattern;
            else if(brushType==6) style = Qt::Dense6Pattern;
            else if(brushType==7) style = Qt::Dense7Pattern;
            else if(brushType==8) {style = Qt::SolidPattern; c=QColor(255,255,255);}
        } else {
            style = Qt::SolidPattern;
            if(freehand)
                c = QColor(qMin(brushType*32,255),qMin(brushType*32,255),qMin(brushType*32,255));
        }

        QPointF pos = event->scenePos();
        //ellipses.push_back(Ellipse(sceneToImageCoords(pos), brushSize/m_scale, QBrush(c,style)));

        QPointF center =  pos - m_translation;
        float size = brushSize*m_scale;
        ellipseGroup->addToGroup(addEllipse(center.x()-size/2,center.y()-size/2,size,size,Qt::NoPen,
                   QBrush(c,style)));

        event->accept();
        update();
        return;
    }
    else {
        if (inPanMode)
        {
            m_translation += event->scenePos() - event->lastScenePos();
            event->accept();
            update();
        } else if (selectedObjects.size() > 0)
        {
            std::set<int> cpt_ids;
            QPointF diff = sceneToImageCoords(event->scenePos()) - sceneToImageCoords(event->lastScenePos());

            for (int i=0; i<selectedObjects.size(); ++i)
            {
                uint nodeId = selectedObjects[i].first;
                uint targetId = selectedObjects[i].second;
                if (nodeId == CPT_NODE_ID)
                {
                    cpt_ids.insert(targetId);
                } else if (nodeId == SPLINE_NODE_ID)
                {
                    BSpline& bspline = spline(targetId);
                    for (int k=0; k<bspline.cptRefs.size(); ++k)
                    {
                        cpt_ids.insert(bspline.cptRefs[k]);
                    }
                }
            }

            for (std::set<int>::iterator it = cpt_ids.begin(); it != cpt_ids.end(); ++it)
            {
                ControlPoint& cpt = m_splineGroup.controlPoint(*it);
                cpt.setX(cpt.x()+diff.x());
                cpt.setY(cpt.y()+diff.y());
            }

            if (cpt_ids.size() > 0)
                hasMoved = true;

            event->accept();
            update();
            return;
        }
    }
}

void GLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    if (event->isAccepted()) return;

    selectedPointChanged();

    shadingProfileView->cpts_ids.clear();


    if (shadingProfileView != NULL)
    {
        bool showShadingProfile = false;
        shadingProfileView->splineGroup = &m_splineGroup;
        for (int k=0; k<selectedObjects.size(); ++k)
        {
            if (selectedObjects[k].first == CPT_NODE_ID)
            {
                shadingProfileView->cpts_ids.push_back(selectedObjects[k].second);
                showShadingProfile = true;
            } else if (selectedObjects[k].first == SPLINE_NODE_ID)
            {
                for (int i=0; i< spline(selectedObjects[k].second).num_cpts(); ++i)
                {
                    shadingProfileView->cpts_ids.push_back(spline(selectedObjects[k].second).cptRefs[i]);
                }
                showShadingProfile = true;
            }
        }
        shadingProfileView->updatePath();
        shadingProfileView->centralWidget->setEnabled(showShadingProfile);
    }

    inPanMode = false;
    /*if (hasMoved)
    {
        recomputeAllSurfaces();
        hasMoved = false;
        event->accept();
    } else if (sketchmode() == ADD_CURVE_MODE)
        recomputeAllSurfaces();*/

    if (hasMoved)
    {
        recomputeAllSurfaces();
        if (hasMoved) hasMoved = false;
        event->accept();
    }
}

void GLScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted()) return;

    uint nodeId, targetId;

    if(event->button() == Qt::LeftButton && brush)
    {
        event->accept();
        return; //mouseMoveEvent(event);
    }

    if (event->button() == Qt::LeftButton && pick(event->scenePos().toPoint(), nodeId, targetId, NULL))
    {
        inPanMode  = false;
        if (!(event->modifiers() & Qt::ControlModifier))
            selectedObjects.clear();
        if (nodeId != IMAGE_NODE_ID)
        {
            selectedObjects.push_back(std::pair<uint, uint>(nodeId, targetId));
            if (nodeId == SPLINE_NODE_ID)
            {
                m_curSplineIdx = targetId;
                currentSplineChanged();
            }
        }
        event->accept();

    }  else if (event->button() == Qt::RightButton)
    {
        inPanMode = true;
        event->accept();
    }

    update();
}

void GLScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseDoubleClickEvent(event);
    if (event->isAccepted()) return;

    if (event->button() == Qt::LeftButton && m_sketchmode == ADD_CURVE_MODE)
    {
        event->accept();
        int cptRef = registerPointAtScenePos(event->scenePos());
        if (cptRef < 0)
            return;

        if (m_curSplineIdx < 0)
        {
            createBSpline();
        }

        if (m_splineGroup.addControlPointToSpline(m_curSplineIdx, cptRef))
        {
            if (spline(m_curSplineIdx).num_cpts()>=3 && spline(m_curSplineIdx).cptRefs.front() == cptRef)
            {
                spline(m_curSplineIdx).has_uniform_subdivision = true;
                spline(m_curSplineIdx).recompute();
                currentSplineChanged();
            }
            controlPoint(cptRef).attributes[0].extent = spline(m_curSplineIdx).generic_extent;
            controlPoint(cptRef).attributes[1].extent = spline(m_curSplineIdx).generic_extent;
            update();
        }
    } else
    {
        uint nodeId, targetId;
        if (event->button() == Qt::LeftButton && pick(event->scenePos().toPoint(), nodeId, targetId, NULL))
        {
            event->accept();
            QPointF topLeft = QPointF(width()/2.0 - imSize.width()/2.0, height()/2.0 - imSize.height()/2.0) + m_translation;
            QPointF scaling(imSize.width()/(float)displayImage()->cols, imSize.height()/(float)displayImage()->rows);
            QPointF imgPos = event->scenePos().toPoint() - topLeft;
            imgPos.setX(imgPos.x()/scaling.x());
            imgPos.setY(imgPos.y()/scaling.y());
            QPoint seed = imgPos.toPoint();

            cv::Vec3b bgrPixel = displayImage()->at<cv::Vec3b>(seed.y(), seed.x());
            QColor image_color(bgrPixel[2], bgrPixel[1], bgrPixel[0]);

            QColor color = QColorDialog::getColor(image_color, (QWidget*)this->activeWindow());
            if(color.isValid())
            {
                m_splineGroup.colorMapping.push_back(std::pair<QPoint, QColor>(seed,color));
                recomputeAllSurfaces();
            }
        }
    }
}

void GLScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QGraphicsScene::wheelEvent(event);
    if (event->isAccepted()) return;

    QPointF imgCoords = sceneToImageCoords(event->scenePos());

    QPointF beforeScaling = imageToSceneCoords(imgCoords);
    float old_scale = m_scale;

    if (event->delta() > 0)
        m_scale *= 1.2f;
    else
        m_scale /= 1.2f;
    adjustDisplayedImageSize();

    /*if (imSize.width() < width() && imSize.height() < height())
    {
        m_scale = old_scale;
    } else*/
    {
        QPointF afterScaling = imageToSceneCoords(imgCoords);

        //if (imSize.width() > width() && imSize.height() > height())
        {
            m_translation -= (afterScaling-beforeScaling);

        } //else   m_translation  = QPointF(0.0,0.0);
    }
    adjustDisplayedImageSize();
    event->accept();
    update();
}

void GLScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_T)
     {
         curDisplayMode = (curDisplayMode+1)%NUM_DISPLAY_MODES;
         changeDisplayModeText();
         adjustDisplayedImageSize();
         update();
         return;

     } if (event->key() == Qt::Key_U)
    {
        curDisplayMode = (curDisplayMode-1); if (curDisplayMode < 0)    curDisplayMode = NUM_DISPLAY_MODES-1;
        changeDisplayModeText();
        adjustDisplayedImageSize();
        update();
        return;

    } else if (event->key() == Qt::Key_W)
     {
        //Reset blank image
        m_curImage = orgBlankImage.clone();
        m_splineGroup.colorMapping.clear();
        update();
        return;

    } else if (event->key() == Qt::Key_R)
   {
       recomputeAllSurfaces();
       update();
       return;
   } else if (event->key() == Qt::Key_S)
    {
        emit triggerShading();
        return;
    }

    if (event->key() == Qt::Key_Up)
    {
        m_translation.setY(m_translation.y() + 5.0f);
        update();
        return;
    } else if (event->key() == Qt::Key_Down)
    {
        m_translation.setY(m_translation.y() - 5.0f);
        update();
        return;
    } else if (event->key() == Qt::Key_Left)
    {
        m_translation.setX(m_translation.x() + 5.0f);
        update();
        return;
    } else if (event->key() == Qt::Key_Right)
    {
        m_translation.setX(m_translation.x() - 5.0f);
        update();
        return;
    } else if (event->key() == Qt::Key_Space)
    {
        m_translation = QPointF(0.0, 0.0);
        update();
        return;

    }

    if (event->key() == Qt::Key_Delete)
    {
        if (selectedObjects.size() > 0)
        {
            for (int i=0; i<selectedObjects.size(); ++i)
            {
                uint nodeId = selectedObjects[i].first;
                uint targetId = selectedObjects[i].second;
                if (nodeId == CPT_NODE_ID)
                {
                    m_splineGroup.removeControlPoint(targetId);
                    qDebug("Delete cpt %d", targetId);

                } else if (nodeId == SPLINE_NODE_ID)
                {
                    if (m_curSplineIdx == (int)targetId) m_curSplineIdx = -1;
                    qDebug("Delete spline %d", targetId);
                    m_splineGroup.removeSpline(targetId);
                    currentSplineChanged();
                } else if (nodeId == SURFACE_NODE_ID)
                {
                    qDebug("Delete surface %d", targetId);
                    m_splineGroup.removeSurface(targetId);
                }
            }

            if (shadingProfileView != NULL){
                shadingProfileView->cpts_ids.clear();
                shadingProfileView->updatePath();
                shadingProfileView->centralWidget->setEnabled(false);
            }
            selectedObjects.clear();
            m_splineGroup.garbage_collection();
            if (m_curSplineIdx+1 > num_splines())
                m_curSplineIdx = -1;
            recomputeAllSurfaces();
        }
        return;
    } else
    {
        switch(event->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            {
                m_sketchmode = IDLE_MODE;

                for (int i=0; i<num_splines(); ++i)
                {
                    spline(i).recompute();
                }

                recomputeAllSurfaces();
                modeText = "Idle Mode";
                emit setStatusMessage("");
                break;
            }
            default:
                break;
        }
        return;

    }

        QGraphicsScene::keyPressEvent(event);

}

void  GLScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    /*if (painter->paintEngine()->type() != QPaintEngine::OpenGL && painter->paintEngine()->type() != QPaintEngine::OpenGL2)
    {
        qWarning("OpenGLScene: drawBackground needs a QGLWidget to be set as viewport on the graphics view");
        return;
    }*/

    //GLEW initialization
    static bool initialized = false;
    if (!initialized)
    {
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            /* Problem: glewInit failed, something is seriously wrong. */
            qWarning("GLEW Error: %s\n", glewGetErrorString(err));
        }
        initialized = true;
    }

    //Setup OpenGL view
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1000.0, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRenderMode(GL_RENDER);
    display();

    QTransform transform;
    QPointF scaling(imSize.width()/(float)m_curImage.cols,
                    imSize.height()/(float)m_curImage.rows);
    QPointF full_translation = QPointF(width()/2.0 - imSize.width()/2.0,
                                       height()/2.0 - imSize.height()/2.0) + m_translation;
    transform.translate(full_translation.x(), full_translation.y());
    transform.scale(scaling.x(), scaling.y());
    ellipseGroup->setTransform(transform);

    /*if (shadingProfileView !=NULL)
    {
        QRect geom = shadingProfileView->geometry();
        if (geom.x() != width()-geom.width() || geom.y() != height()-geom.height())
        {
            shadingProfileView->setGeometry(width()-geom.width(), height()-geom.height(), geom.width(), geom.height());
        }
    }*/
}

void GLScene::display(bool only_show_splines)
{
    glPushMatrix();
    glTranslatef(m_translation.x(), m_translation.y(), 0.0f);

    glInitNames();
    glEnable(GL_POINT_SMOOTH);
    glPointSize(pointSize);


    if (displayImage()->cols > 0)
    {
        glColor3d(1.0, 1.0, 1.0);
        draw_image(*displayImage());
    }

    /*if (!only_show_splines)
    for (int i=0; i<ellipses.size(); ++i)
    {
        draw_ellipse(ellipses[i].center, ellipses[i].size, ellipses[i].brush);
    }*/

    if (!showCurves)
        return;

    if (!only_show_splines)
    for (int i=0; i<m_splineGroup.num_surfaces(); ++i)
    {
        if (surface(i).controlMesh.size() == 0)
            continue;
        draw_surface(i);
    }

    glLineWidth(pointSize/5.0);
    for (int i=0; i<num_splines(); ++i)
    {
        bool is_selected = selectedObjects.contains(std::pair<uint, uint>(SPLINE_NODE_ID, i));
        if (!only_show_splines && (!showCurrentCurvePoints || i == curSplineRef() || is_selected))
        {
          BSpline& bspline = spline(i);
          for (int j=bspline.num_cpts()-1; j>=0; --j)
          {
              draw_control_point(bspline.cptRefs[j]);
          }
        }

        if (spline(i).num_cpts() == 0)
            continue;
        else
            draw_spline(i, only_show_splines);
    }

    glPopMatrix();
}

void GLScene::draw_image(cv::Mat& image)
{
        glPushName(IMAGE_NODE_ID);
        glPushName(0);

        glEnable( GL_TEXTURE_2D );

        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);

        // Set texture interpolation methods for minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Set texture clamping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum inputColourFormat;
        #ifdef GL_BGR
            inputColourFormat = GL_BGR;
        #else
            #ifdef GL_BGR_EXT
                inputColourFormat = GL_BGR_EXT;
            #else
                #define GL_BGR 0x80E0
                inputColourFormat = GL_BGR;
            #endif
        #endif
        if (image.channels() == 1)  inputColourFormat = GL_LUMINANCE;

        glTexImage2D(GL_TEXTURE_2D, 0, 3,image.cols, image.rows, 0, inputColourFormat, GL_UNSIGNED_BYTE, image.data);

        float tex_width = imSize.width();
        float tex_height = imSize.height();

        //glBindTexture(GL_TEXTURE_2D, texture[index]);
        glPushMatrix();
        glTranslated(width()/2.0 - tex_width/2.0, height()/2.0 - tex_height/2.0, -100.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, tex_height);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(tex_width, tex_height);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(tex_width, 0.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);
        glEnd();
        glPopMatrix();

        glDeleteTextures(1, &texId);

        glPopName();
        glPopName();

        glPopMatrix();
}

void GLScene::draw_control_point(int point_id)
{
    ControlPoint& cpt = m_splineGroup.controlPoint(point_id);

    if (!showControlPoints)
        return;
    glPushName(CPT_NODE_ID);
    glPushName(point_id);

    glPointSize(pointSize);
    glColor3d(0.0, 0.0, 1.0);
    if (selectedObjects.contains(std::pair<uint, uint>(CPT_NODE_ID, point_id)))
    {
        glColor3d(1.0, 0.0, 0.0);
    }

    QPointF pos = imageToSceneCoords(cpt);
    glBegin(GL_POINTS);
    glVertex3d(pos.x(), pos.y(), 0.5f);
    glEnd();

    glPopName();
    glPopName();
}

void GLScene::draw_spline(int spline_id, bool only_show_splines, bool transform)
{

    BSpline& bspline = spline(spline_id);
    if (bspline.num_cpts() <= 1)
        return;

    glPushName(SPLINE_NODE_ID);
    glPushName(spline_id);

    if (!only_show_splines) {
        // Display normals
        glBegin(GL_LINES);
        if (!only_show_splines && (!showCurrentCurvePoints || m_curSplineIdx == spline_id))
        {
            glColor3f(0.0, 1.0, 0.0);
            QVector<QPointF> normals = bspline.getNormals(true);
            for (int i = 0; i < bspline.getPoints().size(); ++i)
            {
                QPointF curvPos = bspline.getPoints()[i];
                QPointF scenePos = imageToSceneCoords(curvPos);
                glVertex2f(scenePos.x(), scenePos.y());

                QPointF normal = imageToSceneCoords(curvPos + normals[i]*5.0);
                glVertex2f(normal.x(), normal.y());
            }

            glColor3f(1.0, 0.0, 0.0);
            normals = bspline.getNormals(false);
            for (int i = 0; i < bspline.getPoints().size(); ++i)
            {
                QPointF curvPos = bspline.getPoints()[i];
                QPointF scenePos = imageToSceneCoords(curvPos);
                glVertex2f(scenePos.x(), scenePos.y());

                QPointF normal = imageToSceneCoords(curvPos + normals[i]*5.0);
                glVertex2f(normal.x(), normal.y());
            }
        }
        glEnd();

        /*
        QVector<QPointF> points;
        for (int i=0; i< spline.connected_cpts.size(); ++i)
        {
            points.push_back(spline.pointAt(i));
        }
        glColor3f(0.5f, 0.5f, 0.5f);
        QVector<QPointF> lp = limitPoints(points);
        glBegin(GL_POINTS);
        for (int i = 0; i < lp.count(); ++i) {
            QPointF pt = imageToSceneCoords(lp.at(i));
            glVertex2f(pt.x(),pt.y());
        }
        glEnd();
        */

        glColor3d(0.0, 0.0, 1.0);
        if (selectedObjects.contains(std::pair<uint, uint>(SPLINE_NODE_ID, spline_id)))
        {
            glColor3d(1.0, 0.0, 0.0);
        }
    } else
    {
        glColor3d(0.0, 0.0, 0.0);
    }

    QVector<ControlPoint> points = bspline.getControlPoints();
    QVector<ControlPoint> subDividePts = subDivide(points, curveSubdLevels, bspline.has_uniform_subdivision);
    if (bspline.has_uniform_subdivision && points.size() >= 4) {
        subDividePts.pop_back();
        subDividePts.pop_front();
    }

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < subDividePts.size(); ++i)
    {
        if (transform)   subDividePts[i] = imageToSceneCoords(subDividePts[i]);
        glVertex3f(subDividePts[i].x(), subDividePts[i].y(), 0.0);
    }
    glEnd();

    glPopName();
    glPopName();
}

void GLScene::draw_surface(int surface_id)
{
    glPushName(SURFACE_NODE_ID);
    glPushName(surface_id);

    glColor3d(0.0, 1.0, 1.0);
    if (selectedObjects.contains(std::pair<uint, uint>(SURFACE_NODE_ID, surface_id)))
    {
        glColor3d(1.0, 1.0, 0.0);
    }

    Surface& surf = surface(surface_id);

    // check for incomplete curve
    if (surf.controlMesh.size() <= 1)
        return;

    //Display Control Polygon
    if (showControlMesh)
    {
        glColor3f(0.65,0.65,0.65);
        glPushMatrix();
        glTranslatef(0.0,0.0,-500.0f);
        glLineWidth(2.0);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

        QVector<QVector<int> > faceIndices = surf.faceIndices;
        for (int i=0; i<faceIndices.size(); ++i)
        {
            glBegin(GL_POLYGON);
            for (int m=0; m<faceIndices[i].size(); ++m)
            {
                QPointF point = imageToSceneCoords(surf.vertices[faceIndices[i][m]]);
                glVertex3f(point.x(), point.y(), surf.vertices[faceIndices[i][m]].z());
            }
            glEnd();
        }

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glLineWidth(1.0);
        glPopMatrix();
    }

    glPopName();
    glPopName();
}

void GLScene::draw_ellipse(QPointF center, float size, QBrush brush)
{
    //FLORA: This is not used at the moment

    glPointSize(size*m_scale);
    glEnable(GL_POINT_SMOOTH);
    QColor color = brush.color();
    glColor4i(color.red(), color.green(), color.red(), color.alpha());

    /*GLuint objectID;
    glGenTextures(1, &objectID);
    glBindTexture(GL_TEXTURE_2D, objectID);
    GLenum inputColourFormat;
    #ifdef GL_BGR
        inputColourFormat = GL_BGR;
    #else
        #ifdef GL_BGR_EXT
            inputColourFormat = GL_BGR_EXT;
        #else
            #define GL_BGR 0x80E0
            inputColourFormat = GL_BGR;
        #endif
    #endif
    glTexImage2D(GL_TEXTURE_2D, 0, 3,image.cols, image.rows, 0, inputColourFormat, GL_UNSIGNED_BYTE, image.data);*/

    //glEnable(GL_POINT_SPRITE);
    //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    QPointF pos = imageToSceneCoords(center);
    glBegin(GL_POINTS);
    glVertex3d(pos.x(), pos.y(), 0.5f);
    glEnd();
}

void GLScene::adjustDisplayedImageSize()
{
    imSize = m_scale*QSizeF(displayImage()->cols, displayImage()->rows);
}

void GLScene::changeResolution(int resWidth, int resHeight, bool update)
{
    float xs = resWidth / ((float) currentImage().cols);
    float ys = resHeight / ((float) currentImage().rows);
    cv::resize(currentImage(), currentImage(), cv::Size(resWidth, resHeight));
    cv::resize(orgBlankImage, orgBlankImage, cv::Size(resWidth, resHeight));
    adjustDisplayedImageSize();
    m_splineGroup.scale(xs, ys);

    if (shadingProfileView !=NULL) shadingProfileView->updatePath();
    if (update) recomputeAllSurfaces();
}

QPointF GLScene::sceneToImageCoords(QPointF scenePos)
{
    QPointF topLeft(width()/2.0 - imSize.width()/2.0, height()/2.0 - imSize.height()/2.0);
    topLeft += m_translation;
    QPointF scaling(imSize.width()/(float)m_curImage.cols, imSize.height()/(float)m_curImage.rows);
    QPointF imgPos = scenePos - topLeft;
    imgPos.setX(imgPos.x()/scaling.x());
    imgPos.setY(imgPos.y()/scaling.y());
    return imgPos;
}

QPointF GLScene::imageToSceneCoords(QPointF imgPos)
{
    QPointF topLeft(width()/2.0 - imSize.width()/2.0, height()/2.0 - imSize.height()/2.0);
    topLeft += m_translation;
    QPointF scaling(imSize.width()/(float)m_curImage.cols, imSize.height()/(float)m_curImage.rows);
    QPointF scenePos = imgPos;
    scenePos.setX(scenePos.x()*scaling.x());
    scenePos.setY(scenePos.y()*scaling.y());
    scenePos += topLeft;
    return scenePos;
}

bool GLScene::pick(const QPoint& _mousePos, unsigned int& _nodeIdx,
           unsigned int& _targetIdx, QPointF* _hitPointPtr )
{
    glGetDoublev(GL_MODELVIEW_MATRIX, m_modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, m_projection);

    GLint w = width(), h = height(), x = _mousePos.x(), y = h - _mousePos.y();
    GLint viewport[4] = {0,0,w,h};
    GLuint selectionBuffer[ SELECTION_BUFFER_SIZE ], nameBuffer[ NAME_STACK_SIZE ];

    glSelectBuffer( SELECTION_BUFFER_SIZE, selectionBuffer );
    glRenderMode(GL_SELECT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix((GLdouble) x, (GLdouble) y, pointSize, pointSize, viewport);
    glMultMatrixd(m_projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(m_modelview);

    glClear(GL_DEPTH_BUFFER_BIT);
    // do the picking
    display();
    int hits = glRenderMode(GL_RENDER);

    // restore GL state
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixd(m_projection);
    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixd(m_modelview);

    // process hit record
    if ( hits > 0 )
    {
        GLuint *ptr = selectionBuffer, num_names, z, min_z=~(0u),  max_z=0;
        for (int i=0; i<hits; ++i)
        {
            num_names = *ptr++;
            if ( num_names != NAME_STACK_SIZE )
            {
                qDebug( "GLScene::pick() : namestack error\n");
                return false;
            }
            if ( (z = *ptr++) < min_z )
            {
                min_z = z;
                max_z = *ptr++;
                for (unsigned int j=0; j<num_names; ++j)
                    nameBuffer[j] = *ptr++;
            }
            else ptr += 1+num_names;
        }

        _nodeIdx   = nameBuffer[0];
        _targetIdx = nameBuffer[1];

        //qDebug("Pick: %d %d", _nodeIdx, _targetIdx);

        if (_hitPointPtr)
        {
            GLuint zscale=~(0u);
            GLdouble min_zz = ((GLdouble)min_z) / ((GLdouble)zscale);
            GLdouble max_zz = ((GLdouble)max_z) / ((GLdouble)zscale);
            GLdouble zz     = 0.5F * (min_zz + max_zz);
            GLdouble objX, objY, objZ;
            gluUnProject(x, y, zz, m_modelview, m_projection, viewport, &objX, &objY, &objZ);
            *_hitPointPtr = QPointF(objX, objY);
        }
        return true;
    }  else if (hits < 0)
        qDebug("GLScene::pick() : selection buffer overflow\n");

    return false;
}


void GLScene::createBSpline()
{
    m_curSplineIdx = m_splineGroup.addBSpline();
    m_sketchmode = ADD_CURVE_MODE;
    currentSplineChanged();
    update();
}

void GLScene::recomputeAllSurfaces()
{
    QTime t, t2;
    t.start(); t2.start();

    m_splineGroup.imageSize = cv::Size(currentImage().cols, currentImage().rows);
    int npoints=0, ncurves=0, nsurfaces=0, nslopecurves = 0;
    int curves_timing, coloring_timing, dt_timing, surfaces_timing;

    for (int i=0; i<num_cpts(); ++i)
        if (controlPoint(i).num_splines()>0)    ++npoints;

    for (int i=0; i<num_splines(); ++i)
        if (spline(i).num_cpts() > 1)
        {
            ncurves++;
            if (spline(i).is_slope) nslopecurves++;
            if (spline(i).has_inward_surface)  nsurfaces++;
            if (spline(i).has_outward_surface)  nsurfaces++;
            spline(i).recompute();
        }

    for (int i=0; i<num_splines(); ++i)
        if (spline(i).num_cpts() > 1)
            spline(i).computeControlPointNormals();

    m_splineGroup.computeJunctions();
    curves_timing = t.elapsed();
    t.restart();

    update_region_coloring();
    coloring_timing = t.elapsed();
    t.restart();

    // HENRIK, include distrance transform image
    cv::Mat curvesGrayIm = curvesImage();
    cv::normalize(curvesGrayIm, curvesGrayIm, 0.0, 1.0, cv::NORM_MINMAX);

    cv::Mat dt;
    cv::distanceTransform(curvesGrayIm,dt,CV_DIST_L2,CV_DIST_MASK_PRECISE);
    dt_timing = t.elapsed();
    t.restart();

    cv::Mat luminance;
    cv::cvtColor(currentImage(), luminance, CV_BGR2Lab);

    for (int i=0; i<num_splines(); ++i)
    {
        if (spline(i).num_cpts() > 1)
            spline(i).computeSurfaces(dt, luminance);
    }
    surfaces_timing = t.elapsed();

    char timings[1024];
    sprintf(timings, "Stats: %dx%d res, %d points, %d curves (incl %d slopes), %d surfaces | Surf Perf: %d ms", currentImage().cols, currentImage().rows, npoints, ncurves, nslopecurves, nsurfaces, t2.elapsed());
    stats = timings;
    emit setStatusMessage("");

    qDebug("\n************************************************************\n%s", stats.toStdString().c_str());
    qDebug(" Subdivide Curves: %d ms\n Update Region Coloring: %d ms\n Compute distance transform: %d ms\n Compute surfaces (incl tracing): %d ms", curves_timing, coloring_timing, dt_timing, surfaces_timing);
    std::cout << std::flush;

    if (interactiveShading)
    {
        emit triggerShading();
    }

    update();
}

void GLScene::delete_all()
{
    for (int i=0; i<num_splines(); ++i)
    {
        m_splineGroup.removeSpline(i);
    }
    m_splineGroup.colorMapping.clear();
    m_splineGroup.garbage_collection();

    curSplineRef() = -1;
    selectedObjects.clear();

    shadingProfileView->cpts_ids.clear();
    shadingProfileView->updatePath();
    shadingProfileView->centralWidget->setEnabled(false);

    resetImage();
    update();
}

void GLScene::subdivide_current_spline(){
    if (m_curSplineIdx >=0 &&  m_curSplineIdx<m_splineGroup.num_splines() ) //Check validity
    {
        BSpline& spline = m_splineGroup.spline(m_curSplineIdx);
        bool has_uniform_subd = spline.has_uniform_subdivision;

        QVector<ControlPoint> new_points;

        new_points = subDivide(spline.getControlPoints(),1, has_uniform_subd);

        while (spline.cptRefs.size() > 0)
            m_splineGroup.removeControlPoint(spline.cptRefs[0]);


        for (int i=0; i<new_points.size(); ++i)
        {
            int new_cpt_id = m_splineGroup.addControlPoint(new_points[i], 0.0);
            for (int k=0; k<2; ++k)
                controlPoint(new_cpt_id).attributes[k] = new_points[i].attributes[k];
            if (!m_splineGroup.addControlPointToSpline(m_curSplineIdx, new_cpt_id))
                break;
        }

        shadingProfileView->cpts_ids.clear();
        for (int i=0; i<spline.num_cpts(); ++i)
        {
            shadingProfileView->cpts_ids.push_back(spline.pointAt(i).ref);
        }
        shadingProfileView->updatePath();

        //m_splineGroup.garbage_collection();
        recomputeAllSurfaces();
    }
}

void GLScene::updateConnectedSurfaces(int cptRef)
{
    recomputeAllSurfaces();
}

void GLScene::toggleShowCurrentCurvePoints(bool status)
{
    if (showCurrentCurvePoints != status)
    {
        showCurrentCurvePoints = status;
        update();
    }
}

int GLScene::registerPointAtScenePos(QPointF scenePos)
{
    unsigned int nodeId, targetId;
    if (pick(scenePos.toPoint(), nodeId, targetId))
    {
        if (nodeId == CPT_NODE_ID)
        {
            return targetId;
        }
    } else if (selectedObjects.size()>0 && selectedObjects.front().first == CPT_NODE_ID)
    {
        return selectedObjects.front().second;
    } else
    {
        return -1;
    }

    int pointIdx = m_splineGroup.addControlPoint(sceneToImageCoords(scenePos), 0.0);
    selectedObjects.clear();
    selectedObjects.push_back(std::pair<uint, uint>(CPT_NODE_ID, pointIdx));
    update();
    return pointIdx;
}

cv::Mat GLScene::curvesImageBGR(bool only_closed_curves, float thickness)
{
    glWidget->makeCurrent();
    GLuint imageWidth = m_curImage.cols,
           imageHeight = m_curImage.rows;

    //Setup for offscreen drawing if fbos are supported
    GLuint framebuffer, renderbuffer;
    GLenum status;
    glGenFramebuffersEXT(1, &framebuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
    glGenRenderbuffersEXT(1, &renderbuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, imageWidth, imageHeight);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                     GL_RENDERBUFFER_EXT, renderbuffer);
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
        qDebug("Could not draw offscreen");

    //Drawing
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, imageWidth, imageHeight);
    glOrtho(0, imageWidth, imageHeight, 0, -1000.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRenderMode(GL_RENDER);

    //if (thickness < 0.0)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
    }

    int old_curveSubdLevels  = curveSubdLevels;
    curveSubdLevels = 5;
    for (int i=0; i<num_splines(); ++i)
    {
        if (m_splineGroup.spline(i).num_cpts() == 0)   continue;
        if (only_closed_curves && !spline(i).has_loop())   continue;

        if (thickness < 0.0)
        {
            if (spline(i).thickness <= 0)
                continue;
            glLineWidth(spline(i).thickness);
        }
        else
            glLineWidth(thickness);
        draw_spline(i, true, false);
    }
    curveSubdLevels = old_curveSubdLevels;

    cv::Mat img;
    img.create(imageHeight, imageWidth, CV_8UC3);
    GLenum inputColourFormat;
    #ifdef GL_BGR
        inputColourFormat = GL_BGR;
    #else
        #ifdef GL_BGR_EXT
            inputColourFormat = GL_BGR_EXT;
        #else
            #define GL_BGR 0x80E0
            inputColourFormat = GL_BGR;
        #endif
    #endif
    glReadPixels(0, 0, imageWidth, imageHeight, inputColourFormat, GL_UNSIGNED_BYTE, img.data);

    //Clean up offscreen drawing
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDeleteRenderbuffersEXT(1, &renderbuffer);

    cv::flip(img, img, 0);

    return img;
}

cv::Mat GLScene::curvesImage(bool only_closed_curves)
{
    cv::Mat img = curvesImageBGR(only_closed_curves, 1.5);

    cv::cvtColor(img, img, CV_BGR2RGB);
    cv::cvtColor(img, img, CV_RGB2GRAY);   //cv::imwrite("curv_img_bef.png", img);
    cv::threshold( img, img, 250, 255,   CV_THRESH_BINARY); //cv::imwrite("curv_img.png", img); //cv::imshow("Closed Curves", img);
    return img;
}


void GLScene::update_region_coloring()
{
    //curvesImageBGR(false, -1);;
    m_curImage = orgBlankImage.clone();

    cv::Mat curv_img = curvesImage(false);   //cv::imwrite("curv_img.png", curv_img);
    cv::convertScaleAbs(curv_img, curv_img, -1, 255 );

    //cv::imshow("Closed Curves", curv_img);

    cv::Mat mask(m_curImage.rows+2, m_curImage.cols+2, curv_img.type(), cv::Scalar(0));
    cv::Mat mask_vals(mask, cv::Range(1, m_curImage.rows+1), cv::Range(1, m_curImage.cols+1));
    curv_img.copyTo(mask_vals);
    //cv::imshow("Mask", mask);
    //cv::imwrite("mask.png", mask);

    cv::Mat result = m_curImage.clone(); //(m_curImage.cols, m_curImage.rows, m_curImage.type(), cv::Scalar(255,255,255));

    //Use cv::floodfill (this should be faster)
    for (int l=m_splineGroup.colorMapping.size()-1; l>=0; --l)
    {
        QPoint seed = m_splineGroup.colorMapping[l].first;
        QColor qcolor = m_splineGroup.colorMapping[l].second;
        cv::Scalar color(qcolor.blue(), qcolor.green(), qcolor.red());

        cv::Vec3b def_color = orgBlankImage.at<cv::Vec3b>(seed.y(), seed.x());
        cv::Vec3b cur_color = result.at<cv::Vec3b>(seed.y(), seed.x());

        if (!(def_color[0] == cur_color[0] && def_color[1] == cur_color[1] && def_color[2] == cur_color[2]))
        {
            //if (!(def_color[0] == color[0] && def_color[1] == color[1] && def_color[2] == color[2]))

            m_splineGroup.colorMapping.erase(m_splineGroup.colorMapping.begin()+l);

            continue;
        }

        cv::floodFill(result, mask, cv::Point2i(seed.x(),seed.y()),color, 0, cv::Scalar(255,255,255), cv::Scalar(255,255,255));

        QVector<QPoint> neighbours;
        for (int i=0; i<result.rows; ++i)
            {
                for (int j=0; j<result.cols; ++j)
                {
                    if (curv_img.at<uchar>(i,j) > 128)
                    {
                        bool neighbouring = false;

                        for (int m=-2; m<=2; ++m)
                        {
                            for (int n=-2; n<=2; ++n)
                            {

                                if ((m!=0 || n!=0) && i+m>=0 && j+n>=0 && i+m<result.rows && j+n < result.cols && mask.at<uchar>(i+m+1,j+n+1) <128)
                                {
                                    cv::Vec3b current = result.at<cv::Vec3b>(i+m,j+n);
                                    if (current[0] == color[0] && current[1] == color[1] && current[2] == color[2])
                                    {
                                        neighbouring = true;
                                        break;
                                    }
                                }
                            }
                            if (neighbouring)   break;
                        }
                        if (neighbouring)
                        {
                           neighbours.push_back(QPoint(i,j));
                           curv_img.at<uchar>(i,j) = 0;
                        }
                    }
                }
            }
        for (int i=0; i<neighbours.size(); ++i)
        {
            for (int k=0; k<3; ++k) result.at<cv::Vec3b>(neighbours[i].x(), neighbours[i].y())[k] = color[k];
        }
    }

    //Alternatively, use cv::drawContours
    /*std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(curv_img, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    srand(time(NULL));
    for(uint k = 0; k< contours.size(); k++ )
    {
        cv::Point centroid;
        for (int l=0; l<contours[k].size(); ++l)
        {
            centroid += contours[k][l];
        }
        centroid.x /= contours[k].size();
        centroid.y /= contours[k].size();
        cv::Vec3b bgrPixel = result.at<cv::Vec3b>(centroid.x, centroid.y);

        cv::Scalar color( bgrPixel );

        cv::drawContours( result, contours, k, color, 1, 8);
    }*/

    /*QVector<int> bsplineRefs;
    for (int i=0; i<num_splines(); ++i) if (spline(i).num_cpts() > 1) bsplineRefs.push_back(spline(i).ref);

    for (int i=m_splineGroup.colorMapping.size()-1; i>=0; --i)
    {
        QPoint seed = m_splineGroup.colorMapping[i].first;
        if (seed.x() < 0 || seed.y() < 0 || seed.x() >= m_curImage.cols || seed.y() >= m_curImage.rows)
            continue;

        QColor qcolor = m_splineGroup.colorMapping[i].second;
        cv::Scalar color = cv::Scalar(qcolor.blue(), qcolor.green(), qcolor.red());
        for (int k=0; k<bsplineRefs.size(); ++k)
        {
            QVector<ControlPoint> points =
            cv::pointPolygonTest(contours[k], cv::Point2f(seed.x(), seed.y()), false) > 1e-5
        }
    }*/

    /**/

    //Randomly set colors
    /*
    srand(time(NULL));
    if (m_splineGroup.colorMapping.size() == 0 && contours.size() > 0)
    {
        int idx = 0;
        for( ; idx >= 0; idx = hierarchy[idx][0] )
        {
            cv::Scalar color( rand()&255, rand()&255, rand()&255 );
            cv::drawContours( result, contours, idx, color, CV_FILLED, 8, hierarchy, 0);
            qDebug("Idx: %d", idx);
        }
    }*/

    /*std::vector<bool> marked(contours.size()+1, false);

    for (int i=m_splineGroup.colorMapping.size()-1; i>=0; --i)
    {
        QPoint seed = m_splineGroup.colorMapping[i].first;
        if (seed.x() < 0 || seed.y() < 0 || seed.x() >= m_curImage.cols || seed.y() >= m_curImage.rows)
            continue;

        QColor qcolor = m_splineGroup.colorMapping[i].second;
        cv::Scalar color = cv::Scalar(qcolor.blue(), qcolor.green(), qcolor.red());

        uint k = 0;
        for( ; k< contours.size(); k++ )
        {
            if (cv::pointPolygonTest(contours[k], cv::Point2f(seed.x(), seed.y()), false) > 1e-5)
            {
                if (!marked[k])
                {
                    cv::drawContours( result, contours, k, color, CV_FILLED, 8, hierarchy, 0, cv::Point() );
                    marked[k] = true;
                }
                break;
            }
        }
        if (k == contours.size() && !marked[k])   //Point lies in the background
        {
            cv::floodFill(result, cv::Point2i(seed.x(),seed.y()),color);
            marked[k] = true;
        }
    }*/

    m_curImage = result.clone();
    update();
}


bool cmp_junctions (CurveJunctionInfo i, CurveJunctionInfo j)
{
    return (i.cptRef<j.cptRef);
}

std::vector<std::string> GLScene::OFFSurfaces()
{
    QTime t;
    t.start();
    std::vector<std::string> surface_strings;

    QVector< QVector<int> > mergedGroups;
    QVector< QVector<int> > mergedGroups_JIds;
    QVector<bool> surface_is_merged(num_surfaces(), false);

    createMergeGroups(mergedGroups, mergedGroups_JIds, surface_is_merged);

    for (int i=0; i< num_surfaces(); ++i)
        if (surface(i).vertices.size() > 0 && !surface_is_merged[i])
        {
            Surface &surface1 = surface(i);
            QPointF pixelPoint = surface1.vertices[surface1.controlMesh.first()[surface1.controlMesh.first().size()/2]];
            cv::Vec3b color = currentImage().at<cv::Vec3b>(pixelPoint.y(), pixelPoint.x());
            /*if (color[0] == 255 && color[1] == 255 && color [2] == 255)
            {
                if (bspline.thickness > 0)  { currentImage().at<cv::Vec3b>(bspline.getPoints()[1].y(), bspline.getPoints()[1].x()); }
            }*/
            surface_strings.push_back( surface(i).surfaceToOFF(color) );

            //qDebug("%s", surface_strings.back().c_str());
        }

    QVector< QVector<int> > skippedSurfaces =  mergeSurfaces(mergedGroups, surface_strings);

    while (skippedSurfaces.size() > 0)
        skippedSurfaces = mergeSurfaces(skippedSurfaces, surface_strings);

    char timing[50];
    sprintf(timing, " | Surf Streams(incl merging): %d ms", t.elapsed());
    stats += timing;
    emit setStatusMessage("");

    return surface_strings;
}

void GLScene::createMergeGroups(QVector< QVector<int> > &mergedGroups,
                                QVector< QVector<int> > &mergedGroups_JIds,
                                QVector<bool> &surface_is_merged)
{
    QVector<CurveJunctionInfo> junctionInfos  = m_splineGroup.junctionInfos;

    for (int k=0; k<junctionInfos.size(); ++k)
    {
        CurveJunctionInfo& junctionInfo = junctionInfos[k];
        BSpline&  bspline = spline(junctionInfo.splineRef1);
        BSpline& otherSpline = spline(junctionInfo.splineRef2);
        if (bspline.num_cpts() > 1 && otherSpline.num_cpts() > 1)
        {
            if (junctionInfo.valid)
            {
                std::pair<int, int> merging;
                for (int l=0; l<otherSpline.num_surfaces(); ++l)
                {
                    if (otherSpline.surfaceAt(l).vertices.size() > 1 && (otherSpline.surfaceAt(l).direction == INWARD_DIRECTION) == (junctionInfo.spline2Direction==0))
                    {
                        surface_is_merged[otherSpline.surfaceAt(l).ref] = true;
                        merging.second = otherSpline.surfaceAt(l).ref;
                    }
                }
                for (int l=0; l<bspline.num_surfaces(); ++l)
                {
                    if (bspline.surfaceAt(l).vertices.size() > 1 && (bspline.surfaceAt(l).direction == INWARD_DIRECTION) == (junctionInfo.spline1Direction==0))
                    {
                        surface_is_merged[bspline.surfaceAt(l).ref] = true;
                        merging.first = bspline.surfaceAt(l).ref;
                    }
                }

                bool already_added = false;
                for (int m=0; m<mergedGroups.size(); ++m)
                {
                    bool connected1 = false, connected2 = false;
                    for (int n = 0; n<mergedGroups[m].size(); ++n)
                    {
                        if (mergedGroups[m][n] == merging.first)
                            connected1 = true;
                        if (mergedGroups[m][n] == merging.second)
                            connected2 = true;
                    }
                    if (!connected1 && !connected2)
                        continue;
                    else if (connected1 && connected2)
                        already_added = true;
                    else if (connected1)
                    {
                        mergedGroups[m].push_back(merging.second);
                        mergedGroups_JIds[m].push_back(k);
                        already_added = true;
                    }
                    else if (connected2)
                    {
                        mergedGroups[m].push_back(merging.first);
                        mergedGroups_JIds[m].push_back(k);
                        already_added = true;
                    }
                    if (already_added)  break;
                }
                if (!already_added)
                {
                    mergedGroups.push_back(QVector<int>());
                    mergedGroups_JIds.push_back(QVector<int>());
                    mergedGroups.last().push_back(merging.first);
                    mergedGroups_JIds.last().push_back(k);
                    mergedGroups.last().push_back(merging.second);
                    mergedGroups_JIds.last().push_back(k);

                }
            }
        }
    }

    //Merging: (Join suitable merge groups and then create the different merged surfaces
    while (true)
    {
        bool joinMergedGroups = false;
        int m1, m2;
        for (int m=0; m<mergedGroups.size(); ++m)
        {
            for (int l=0; l<mergedGroups[m].size(); ++l)
            {
                for (int n=m+1; n<mergedGroups.size(); ++n)
                {
                    if (std::find(mergedGroups[n].begin(), mergedGroups[n].end(), mergedGroups[m][l]) != mergedGroups[n].end())
                    {
                        m1 = m;
                        m2 = n;
                        joinMergedGroups = true;
                        break;
                    }
                }
                if (joinMergedGroups)   break;
            }
            if (joinMergedGroups) break;
        }
        if (joinMergedGroups)
        {
            for (int n=0; n<mergedGroups[m2].size(); ++n)
            {
                if (std::find(mergedGroups[m1].begin(), mergedGroups[m1].end(), mergedGroups[m2][n]) == mergedGroups[m1].end())
                {
                    mergedGroups[m1].push_back(mergedGroups[m2][n]);
                    mergedGroups_JIds[m1].push_back(mergedGroups_JIds[m2][n]);
                }
            }
            mergedGroups.erase(mergedGroups.begin()+m2);
            mergedGroups_JIds.erase(mergedGroups_JIds.begin()+m2);
        } else
            break;
    }
}

QVector< QVector<int> > GLScene::mergeSurfaces(QVector< QVector<int> > &mergedGroups,
                            std::vector<std::string> &surface_strings)
{
    QVector< QVector<int> > skippedSurfaces;
    for (int i=0; i<mergedGroups.size(); ++i)
    {
        QVector<int> surf_ids = mergedGroups[i];
        QVector<int> skipped;
        Surface mergedSurface;
        int cptRefFront = -1;
        int cptRefBack = -1;

        int k=0;
        for (QVector<int>::iterator it = surf_ids.begin(); it!=surf_ids.end(); ++it)
        {
            Surface& surf = surface(*it);

           if (it == surf_ids.begin() )
            {
                mergedSurface.vertices = surf.vertices;
                mergedSurface.controlMesh = surf.controlMesh;
                mergedSurface.sharpCorners = surf.sharpCorners;
                cptRefFront = spline(surf.splineRef).cptRefs.front();
                cptRefBack = spline(surf.splineRef).cptRefs.back();
            } else
            {
                bool prepend = false;
                bool reverse = false;

                bool junction_is_first = false;
                bool close = false;

                QVector<int> cptRefs1 = spline(surf.splineRef).cptRefs;

                if (cptRefFront == cptRefs1.first())
                {
                    reverse = true;
                    prepend = true;
                    junction_is_first = true;
                    if (cptRefBack == cptRefs1.last())
                        close = true;
                    cptRefFront = cptRefs1.last();

                } else if (cptRefFront == cptRefs1.last())
                {
                    prepend = true;
                    if (cptRefBack == cptRefs1.first()) close = true;
                    cptRefFront = cptRefs1.first();
                } else if (cptRefBack == cptRefs1.first())
                {
                    junction_is_first = true;
                    if (cptRefFront == cptRefs1.last()) close = true;
                    cptRefBack = cptRefs1.last();
                } else if (cptRefBack == cptRefs1.last())
                {
                    reverse = true;
                    if (cptRefFront == cptRefs1.first()) close = true;
                    cptRefBack = cptRefs1.first();
                } else
                {
                    qDebug("Could connect surface %d to merged surface.", surf.ref);
                    skipped.push_back(surf.ref);
                    continue;
                }

                QVector<int> new_vert_ids;Surface &surface1 = surface(mergedGroups[i].first());
                QPointF pixelPoint = surface1.vertices[surface1.controlMesh.first().size()/2];
                for (int j=0; j<surf.vertices.size(); ++j)
                {
                    new_vert_ids.push_back(mergedSurface.addVertex(surf.vertices[j]));
                }

                for (QSet<int>::iterator it = surf.sharpCorners.begin(); it != surf.sharpCorners.end(); ++it)
                {
                    mergedSurface.sharpCorners.insert(new_vert_ids[*it]);
                }

                int direction = (surf.direction == OUTWARD_DIRECTION)?1:0;
                BSpline& bspline = spline(surf.splineRef);

                bool isSharp = false;
                bool hasZeroHeight = false;
                if (bspline.pointAt(0).num_splines() > 2)
                    isSharp = true;
                else if (bspline.pointAt(bspline.num_cpts()-1).num_splines()>2)
                    isSharp = true;
                if (junction_is_first && bspline.start_has_zero_height[direction])
                        hasZeroHeight = true;
                else if (!junction_is_first && bspline.end_has_zero_height[direction])
                        hasZeroHeight = true;

                if (isSharp)
                {
                    if (bspline.pointAt(0).num_splines()>2)
                    {
                        mergedSurface.sharpCorners.insert(new_vert_ids[surf.controlMesh.last().first()]);
                    }
                    if (bspline.pointAt(bspline.num_cpts()-1).num_splines()>2)
                    {
                        mergedSurface.sharpCorners.insert(new_vert_ids[surf.controlMesh.last().last()]);
                    }
                }

                for (int l=0; l<surf.controlMesh.size(); ++l)
                {
                    QVector<int> row = surf.controlMesh[l];
                    if (bspline.start_has_zero_height[direction])
                    {
                        if (prepend && reverse)
                            row.prepend(surf.controlMesh.first().first());
                        else if (!prepend && !reverse)
                            row.prepend(surf.controlMesh.first().first());
                    }
                    if (bspline.end_has_zero_height[direction])
                    {
                        if (prepend && !reverse)
                            row.append(surf.controlMesh.first().last());
                        else if (!prepend && reverse)
                            row.append(surf.controlMesh.first().last());
                    }

                    if (reverse)   std::reverse(row.begin(), row.end());

                    for (int j=0; j<row.size(); ++j)
                    {
                        if (prepend)
                        {
                            mergedSurface.controlMesh[l].prepend(new_vert_ids[row[row.size()-1-j]]);
                        }
                        else
                        {
                            mergedSurface.controlMesh[l].append(new_vert_ids[row[j]]);
                        }
                    }
                }

            }
            ++k;
        }

        for (int l=0; l<mergedSurface.controlMesh[0].size(); ++l)
        {
            bool isSharp = true;
            for (int i=1; i<mergedSurface.controlMesh.size(); ++i)
            {
                if (mergedSurface.controlMesh[i][l] != mergedSurface.controlMesh[i-1][l])
                {
                    isSharp = false;
                    break;
                }
            }
            if (isSharp) mergedSurface.sharpCorners.insert(mergedSurface.controlMesh[0][l]);
        }

        mergedSurface.computeFaceIndices();

        Surface &surface1 = surface(mergedGroups[i].first());
        QPointF pixelPoint = surface1.vertices[surface1.controlMesh.first()[surface1.controlMesh.first().size()/2]];
        cv::Vec3b color = currentImage().at<cv::Vec3b>(pixelPoint.y(), pixelPoint.x());
        /*if (color[0] == 255 && color[1] == 255 && color [2] == 255)
        {
            if (bspline.thickness > 0)  { currentImage().at<cv::Vec3b>(bspline.getPoints()[1].y(), bspline.getPoints()[1].x()); }
        }*/
        surface_strings.push_back( mergedSurface.surfaceToOFF(color) );

        if (skipped.size()>0)   skippedSurfaces.push_back(skipped);

        /*std::stringstream ss;
        ss << "MergedSurface_"<<i<<".off";
        std::ofstream ofs(ss.str().c_str());
        ofs << surface_strings.back();
        ofs.close();*/
    }
    return skippedSurfaces;
}

//Public Slots
void GLScene::resetImage()
{
    m_scale = 1.0f;
    m_translation = QPointF(0.0, 0.0);
    QSize prevSize(m_curImage.rows, m_curImage.cols);

    std::string blankImagePath = imageLocationWithID("blank.png");
    m_curImage = loadImage(blankImagePath);
    orgBlankImage = m_curImage.clone();

    if (prevSize.width() > 0)
        cv::resize(m_curImage, m_curImage, cv::Size(prevSize.height(), prevSize.width()));

    curDisplayMode = 0;
    changeDisplayModeText();
    adjustDisplayedImageSize();

    shadingProfileView->min_height = -100;
    shadingProfileView->max_height = 100;
    shadingProfileView->min_extent = 1;
    shadingProfileView->max_extent = std::max(m_curImage.cols, m_curImage.rows);

    update();
}

bool GLScene::openImage(std::string fname)
{
    cv::Mat image = loadImage(fname);

    //Test if image was loaded
    if (image.cols > 0)
    {
        changeResolution(image.cols, image.rows);
        m_targetImage = image;
        curDisplayMode = 1;
        changeDisplayModeText();
        adjustDisplayedImageSize();

        update();
        return true;
    }
    else
        return false;
}


void GLScene::saveImage(std::string fname)
{

    if (fname.find(".") == std::string::npos)
        fname += ".png";
    if (resultImg.cols > 0)
        cv::imwrite(fname, resultImg);
    else
        cv::imwrite(fname, m_curImage);
}


bool GLScene::openCurves(std::string fname)
{
    if (fname.size() > 0)
        delete_all();
    if (m_splineGroup.load(fname))
    {
        recomputeAllSurfaces();
        update();
        return true;

    } else
    {
        return false;
    }
}


void GLScene::saveCurves(std::string fname)
{
    if (fname.find(".") == std::string::npos)
        fname += ".curv";
    m_splineGroup.save(fname);
}

// HENRIK, save CPs to OFF
void GLScene::saveOff(std::string fname)
{
    m_splineGroup.saveOFF(fname);
}

void GLScene::currentSplineChanged()
{
    if (curSplineRef() >= 0)
    {
        BSpline& bspline = spline(curSplineRef());
        emit bspline_parameters_changed(true, bspline.generic_extent, bspline.is_slope, bspline.has_uniform_subdivision, bspline.has_inward_surface, bspline.has_outward_surface, bspline.thickness);

    } else
    {
        emit bspline_parameters_changed(false, 0.0, false, false, false, false, 0);
    }
}

void GLScene::selectedPointChanged()
{
    bool point_selected = false;
    bool isSharp = false;
    for (int i=0; i<selectedObjects.size(); ++i)
    {
        if (selectedObjects[i].first == CPT_NODE_ID)
        {
            ControlPoint& cpt = controlPoint(selectedObjects[i].second);
            point_selected = true;
            isSharp = cpt.isSharp;
        }
    }

    emit point_parameters_changed(point_selected, isSharp);
}

void GLScene::change_point_parameters(bool isSharp)
{
    bool has_changed = false;

    for (int i=0; i<selectedObjects.size(); ++i)
    {
        if (selectedObjects[i].first == CPT_NODE_ID)
        {
            ControlPoint& cpt = controlPoint(selectedObjects[i].second);
            if (cpt.isSharp != isSharp)
            {
                cpt.isSharp = isSharp;
                has_changed = true;
            }
        }
    }

    if (has_changed)
        recomputeAllSurfaces();
}

void GLScene::change_bspline_parameters(float extent, bool _is_slope, bool _has_uniform_subdivision, bool _has_inward, bool _has_outward, int  _thickness)
{
    bool has_changed = false;
    if (curSplineRef() >= 0)
    {
        BSpline& bspline = spline(curSplineRef());
        if (extent>=0.0f && fabs(bspline.generic_extent-extent) > 1)
        {
            bspline.change_generic_extent(extent);
            has_changed = true;
        }

        if (bspline.has_uniform_subdivision != _has_uniform_subdivision || bspline.is_slope != _is_slope || bspline.has_inward_surface != _has_inward || bspline.has_outward_surface != _has_outward)
        {
            bspline.change_bspline_type(_is_slope, _has_uniform_subdivision, _has_inward, _has_outward);
            has_changed = true;
        }

        if (_thickness != bspline.thickness)
        {
            bspline.thickness = _thickness;
            has_changed = true;
        }
    }

    if (has_changed)
        recomputeAllSurfaces();
}

void GLScene::setInteractiveShading(bool b)
{
    interactiveShading = b;
}
