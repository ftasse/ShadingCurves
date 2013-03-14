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
#include <set>
#include <QDebug>

#include "../Utilities/SurfaceUtils.h"

#include "GLScene.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include "glew/GL/glew.h"
#include <GL/glu.h>

static const unsigned int  SELECTION_BUFFER_SIZE = 10000;
static const unsigned int  NAME_STACK_SIZE       = 2;
static const unsigned int  IMAGE_NODE_ID = 0;
static const unsigned int  CPT_NODE_ID = 1;
static const unsigned int  SPLINE_NODE_ID = 2;
static const unsigned int  SURFACE_NODE_ID = 3;

GLScene::GLScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_curSplineIdx = -1;
    m_sketchmode = IDLE_MODE;
    pointSize = 10.0;
    showControlMesh = true;
    showControlPoints = true;
    showCurrentCurvePoints = true;
    showCurves = true;
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
    displayModeLabel->setGeometry(10, 10, 90, 20);
    displayModeLabel->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    QGraphicsProxyWidget* proxyWidget = addWidget(displayModeLabel);
    proxyWidget->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    shadingProfileView = NULL;
    /*shadingProfileView = new ShadingProfileView ();
    shadingProfileView->setVisible(false);
    addWidget(shadingProfileView);
    shadingProfileView->setGeometry(0, 0, 300, 400);

    connect(shadingProfileView, SIGNAL(controlPointAttributesChanged(int)), this, SLOT(updateConnectedSurfaces(int)));*/
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

    if (shadingProfileView != NULL)
    {
        bool showShadingProfile = false;
        for (int k=0; k<selectedObjects.size(); ++k)
            if (selectedObjects[k].first == CPT_NODE_ID)
            {
                shadingProfileView->setControlPoint(controlPoint(selectedObjects[k].second));
                showShadingProfile = true;
            }
        shadingProfileView->setVisible(showShadingProfile);
    }

    inPanMode = false;
    if (hasMoved)
    {
        for (int i=0; i<num_splines(); ++i)
        {
            spline(i).recompute();
        }

        recomputeAllSurfaces();
        hasMoved = false;
        event->accept();
        update();
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
            if (spline(m_curSplineIdx).num_cpts()>2 && spline(m_curSplineIdx).cptRefs.front() == cptRef)
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
                update_region_coloring();
            }
        }
    }
}

void GLScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QGraphicsScene::wheelEvent(event);
    if (event->isAccepted()) return;

    if (event->delta() > 0)
        m_scale *= 1.2f;
    else
        m_scale /= 1.2f;
    adjustDisplayedImageSize();
    event->accept();
    update();
}

void GLScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_T)
     {
         curDisplayMode = (curDisplayMode+1)%3;
         changeDisplayModeText();
         adjustDisplayedImageSize();
         update();
         return;

     } else if (event->key() == Qt::Key_W)
     {
        //Reset blank image
        m_curImage = cv::Scalar(255,255,255);
        m_splineGroup.colorMapping.clear();
        update();
        return;

    } else if (event->key() == Qt::Key_R)
   {
       recomputeAllSurfaces();
       m_curImage = cv::Scalar(255,255,255);
       update_region_coloring();
       update();
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

            if (shadingProfileView != NULL) shadingProfileView->setEnabled(false);
            selectedObjects.clear();
            m_splineGroup.garbage_collection();
            recomputeAllSurfaces();
        }
    } else if (m_sketchmode == ADD_CURVE_MODE)
    {
        switch(event->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            {
                m_sketchmode = IDLE_MODE;
                recomputeAllSurfaces();
                emit setStatusMessage("Move Curve Mode");
                break;
            }
            default:
                break;
        }

    } else
    {
        QGraphicsScene::keyPressEvent(event);
    }
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

    if (shadingProfileView !=NULL)
    {
        QRect geom = shadingProfileView->geometry();
        if (geom.x() != width()-geom.width() || geom.y() != height()-geom.height())
        {
            shadingProfileView->setGeometry(width()-geom.width(), height()-geom.height(), geom.width(), geom.height());
        }
    }
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

    if (!only_show_splines)
    for (int i=0; i<ellipses.size(); ++i)
    {
        draw_ellipse(ellipses[i].center, ellipses[i].size, ellipses[i].brush);
    }

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
    glPushName(SPLINE_NODE_ID);
    glPushName(spline_id);

    BSpline& bspline = spline(spline_id);
    if (bspline.num_cpts() <= 1)
        return;

    if (!only_show_splines) {
        // Display normals
        glBegin(GL_LINES);
        if (!only_show_splines && (!showCurrentCurvePoints || m_curSplineIdx == spline_id))
        {
            glColor3f(0.0, 1.0, 0.0);
            for (int i = 0; i < bspline.getPoints().size(); ++i)
            {
                QPointF curvPos = bspline.getPoints()[i];
                QPointF scenePos = imageToSceneCoords(curvPos);
                glVertex2f(scenePos.x(), scenePos.y());

                QPointF normal = imageToSceneCoords(curvPos + bspline.get_normal(i, true, true)*5.0);
                glVertex2f(normal.x(), normal.y());
            }

            glColor3f(1.0, 0.0, 0.0);
            for (int i = 0; i < bspline.getPoints().size(); ++i)
            {
                QPointF curvPos = bspline.getPoints()[i];
                QPointF scenePos = imageToSceneCoords(curvPos);
                glVertex2f(scenePos.x(), scenePos.y());

                QPointF normal = imageToSceneCoords(curvPos + bspline.get_normal(i, true, false)*5.0);
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
    QVector<ControlPoint> subDividePts = subDivide(points, 5, bspline.has_uniform_subdivision);
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

        QVector<QVector<int> > faceIndices = surf.getFaceIndices();
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

void GLScene::changeResolution(int resWidth, int resHeight)
{
    float xs = resWidth / ((float) currentImage().cols);
    float ys = resHeight / ((float) currentImage().rows);
    cv::resize(currentImage(), currentImage(), cv::Size(resWidth, resHeight));
    adjustDisplayedImageSize();
    m_splineGroup.scale(xs, ys);
    recomputeAllSurfaces();
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
    //qDebug("Recompute surfaces");

    // HENRIK, include distrance transform image
    cv::Mat curvesGrayIm = curvesImage();
    cv::normalize(curvesGrayIm, curvesGrayIm, 0.0, 1.0, cv::NORM_MINMAX);

    cv::Mat dt;
    cv::distanceTransform(curvesGrayIm,dt,CV_DIST_L2,CV_DIST_MASK_PRECISE);

    for (int i=0; i<num_splines(); ++i)
    {
        spline(i).computeSurfaces(dt);
    }
    update();
}

void GLScene::delete_all()
{
    for (int i=0; i<num_splines(); ++i)
    {
        m_splineGroup.removeSpline(i);
    }
    m_splineGroup.garbage_collection();

    curSplineRef() = -1;
    selectedObjects.clear();

    resetImage();
    update();
}

void GLScene::subdivide_current_spline(){
    if (m_curSplineIdx >=0 &&  m_curSplineIdx<m_splineGroup.num_splines() ) //Check validity
    {
        BSpline& spline = m_splineGroup.spline(m_curSplineIdx);
        bool has_uniform_subd = spline.has_uniform_subdivision;

        QVector<ControlPoint> org_points, new_points;
        org_points = spline.getControlPoints();

        new_points = subDivide(org_points,1, has_uniform_subd);

        while (spline.cptRefs.size() > 0)
            m_splineGroup.removeControlPoint(spline.cptRefs[0]);


        for (int i=0; i<new_points.size(); ++i)
        {
            int new_cpt_id = m_splineGroup.addControlPoint(new_points[i], 0.0);
            if (!m_splineGroup.addControlPointToSpline(m_curSplineIdx, new_cpt_id))
                break;
        }
        spline.recompute();
        m_splineGroup.garbage_collection();

        recomputeAllSurfaces();
    }
}

void GLScene::updateConnectedSurfaces(int cptRef)
{
    cv::Mat curvesGrayIm = curvesImage();
    cv::normalize(curvesGrayIm, curvesGrayIm, 0.0, 1.0, cv::NORM_MINMAX);
    cv::Mat dt;
    cv::distanceTransform(curvesGrayIm,dt,CV_DIST_L2,CV_DIST_MASK_PRECISE);

    ControlPoint& cpt = controlPoint(cptRef);
    for (int k=0; k<cpt.num_splines(); ++k)
    {
        spline(cpt.splineRefs[k]).recompute();
        spline(cpt.splineRefs[k]).computeSurfaces(dt);
    }
    update();
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

cv::Mat GLScene::curvesImage(bool only_closed_curves)
{
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
    glLineWidth(1.5);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);

    for (int i=0; i<num_splines(); ++i)
    {
        if (m_splineGroup.spline(i).num_cpts() == 0)   continue;
        if (only_closed_curves && !spline(i).has_loop())   continue;
        draw_spline(i, true, false);
    }

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

    cv::cvtColor(img, img, CV_BGR2RGB);
    cv::flip(img, img, 0);
    cv::cvtColor(img, img, CV_RGB2GRAY);   // cv::imwrite("curv_img_bef.png", img);
    cv::threshold( img, img, 250, 255,   CV_THRESH_BINARY); cv::imwrite("curv_img.png", img); //cv::imshow("Closed Curves", img);
    return img;
}


void GLScene::update_region_coloring()
{
    cv::Mat curv_img = curvesImage(false);   //cv::imwrite("curv_img.png", curv_img);
    cv::convertScaleAbs(curv_img, curv_img, -1, 255 );
    //cv::imshow("Closed Curves", curv_img);

    cv::Mat mask(m_curImage.rows+2, m_curImage.cols+2, curv_img.type(), cv::Scalar(0));
    cv::Mat mask_vals(mask, cv::Range(0, m_curImage.rows), cv::Range(0, m_curImage.cols));
    curv_img.copyTo(mask_vals);
    //cv::imshow("Mask", mask);

    cv::Mat result = m_curImage.clone(); //(m_curImage.cols, m_curImage.rows, m_curImage.type(), cv::Scalar(255,255,255));

    //Use cv::floodfill (this should be faster)
    /*for (int i=0; i<m_splineGroup.colorMapping.size(); ++i)
    {
        QPoint seed = m_splineGroup.colorMapping[i].first;
        QColor color = m_splineGroup.colorMapping[i].second;
        cv::floodFill(result, mask, cv::Point2i(seed.x(),seed.y()),cv::Scalar(color.blue(), color.green(), color.red()));
    }*/

    //Alternatively, use cv::drawContours
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours( mask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

    //Randomly set colors
    /*if (m_splineGroup.colorMapping.size() == 0 && contours.size() > 0)
    {
        int idx = 0;
        cv::RNG rng(12345);
        for( ; idx >= 0; idx = hierarchy[idx][0] )
        {
            cv::Scalar color( rand()&255, rand()&255, rand()&255 );
            cv::drawContours( result, contours, idx, color, 2, 8, hierarchy, 0, cv::Point() );
            cv::drawContours( result, contours, idx, color, CV_FILLED, 8, hierarchy, 0, cv::Point() );
        }
    }*/

    std::vector<bool> marked(contours.size()+1, false);
    std::vector<cv::Scalar> colors(contours.size());
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
                    //cv::drawContours( result, contours, k, color, 2, 8, hierarchy, 0, cv::Point() );
                    //cv::drawContours( result, contours, k, color, CV_FILLED, 8, hierarchy, 0, cv::Point() );
                    marked[k] = true;
                    colors[k] = color;
                }
                break;
            }
        }
        if (k == contours.size() && !marked[k])   //Point lies in the background
        {
            cv::floodFill(result, cv::Point2i(seed.x(),seed.y()),color);
            marked[k] = true;
        }
    }

    for( int k=0; k< contours.size(); k++ )
    {
        if (marked[k])
        {
            //cv::drawContours( result, contours, k, colors[k], 2, 8, hierarchy, 0, cv::Point() );
            cv::drawContours( result, contours, k, colors[k], CV_FILLED, 8, hierarchy, 0, cv::Point() );
        }
    }

    m_curImage = result.clone();
    update();
}

//Public Slots
void GLScene::resetImage()
{
    m_scale = 1.0f;
    m_translation = QPointF(0.0, 0.0);
    QSize prevSize(m_curImage.rows, m_curImage.cols);

    std::string blankImagePath = imageLocationWithID("blank.png");
    m_curImage = loadImage(blankImagePath);
    if (prevSize.width() > 0)
        cv::resize(m_curImage, m_curImage, cv::Size(prevSize.height(), prevSize.width()));

    curDisplayMode = 0;
    changeDisplayModeText();
    adjustDisplayedImageSize();
    update();
}

bool GLScene::openImage(std::string fname)
{
    cv::Mat image = loadImage(fname);

    //Test if image was loaded
    if (image.cols > 0)
    {
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
    cv::imwrite(fname, m_curImage);
}


bool GLScene::openCurves(std::string fname)
{
    if (fname.size() > 0)
        delete_all();
    if (m_splineGroup.load(fname))
    {
        update_region_coloring();
        update();
        return true;

    } else
    {
        return false;
    }
}


void GLScene::saveCurves(std::string fname)
{
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
        emit bspline_parameters_changed(true, bspline.generic_extent, bspline.is_slope, bspline.has_uniform_subdivision, bspline.has_inward_surface, bspline.has_outward_surface);

    } else
    {
        emit bspline_parameters_changed(false, 0.0, false, false, false, false);
    }
}

void GLScene::change_bspline_parameters(float extent, bool _is_slope, bool _has_uniform_subdivision, bool _has_inward, bool _has_outward)
{
    bool has_changed = false;
    if (curSplineRef() >= 0)
    {
        BSpline& bspline = spline(curSplineRef());
        if (fabs(bspline.generic_extent-extent) > 1)
        {
            bspline.change_generic_extent(extent);
            has_changed = true;
        }

        if (bspline.has_uniform_subdivision != _has_uniform_subdivision || bspline.is_slope != _is_slope || bspline.has_inward_surface != _has_inward || bspline.has_outward_surface != _has_outward)
        {
            bspline.change_bspline_type(_is_slope, _has_uniform_subdivision, _has_inward, _has_outward);
            has_changed = true;
        }
    }

    if (has_changed)
        recomputeAllSurfaces();
}
