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
#include <QProcess>
#include <QThread>

#include "GLScene.h"
#include "../Utilities/SurfaceUtils.h"
#include "../Utilities/GLUtils.h"

#include <QGLWidget>

#ifndef max
    #define max(a,b) ((a) < (b) ? (a) : (b))
#endif

static const unsigned int  SELECTION_BUFFER_SIZE = 10000;
static const unsigned int  NAME_STACK_SIZE       = 2;
static const unsigned int  IMAGE_NODE_ID = 0;
static const unsigned int  CPT_NODE_ID = 1;
static const unsigned int  SPLINE_NODE_ID = 2;
static const unsigned int  SURFACE_NODE_ID = 3;
static const unsigned int  COLOR_NODE_ID = 4;

GLScene::GLScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_curSplineIdx = -1;
    m_sketchmode = IDLE_MODE;
    pointSize = 10.0;
    showControlMesh = true;
    displaySimpleSurfaces = false;
    showControlPoints = true;
    showCurrentCurvePoints = false;
    showCurves = true;
    showNormals = true;
    showColors = true;
    accumMouseChanges = QPointF(0.0,0.0);
    curveSubdLevels = DEFAULT_SUBDV_LEVELS;
    drawingSubdLevels = 5;
    globalThickness = 0;
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
    displayModeLabel->setGeometry(10, 10, 100, 30);
    displayModeLabel->setStyleSheet("background-color: rgba(255, 255, 255, 0); color: rgb(0, 0, 0);");
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
    ghostSurfacesEnabled = true;
    clipHeight = true;

    image_display_list = points_display_list = 0;
    colors_display_list =  texId = 0;
    curves_display_list = surfaces_display_list = 0;
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
            std::set<int> color_ids;
            QPointF diff = sceneToImageCoords(event->scenePos()) - sceneToImageCoords(event->lastScenePos());
            accumMouseChanges += diff;

            QPointF intdiff = accumMouseChanges.toPoint();
            bool significant = (std::abs(intdiff.x())>0 || std::abs(intdiff.y())>0);

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
                } else if (nodeId == COLOR_NODE_ID && significant)
                {
                    color_ids.insert(targetId);
                }
            }

            for (std::set<int>::iterator it = cpt_ids.begin(); it != cpt_ids.end(); ++it)
            {
                ControlPoint& cpt = m_splineGroup.controlPoint(*it);
                cpt.setX(cpt.x()+diff.x());
                cpt.setY(cpt.y()+diff.y());
            }

            for (std::set<int>::iterator it = color_ids.begin(); it != color_ids.end(); ++it)
            {
                std::pair<QPoint, QColor>& colorPoint = m_splineGroup.colorMapping[*it];
                colorPoint.first.setX(colorPoint.first.x()+intdiff.x());
                colorPoint.first.setY(colorPoint.first.y()+intdiff.y());
            }

            if (significant)
                accumMouseChanges = QPointF(accumMouseChanges.x() - intdiff.x(),
                                            accumMouseChanges.y() - intdiff.y());
            if (cpt_ids.size() > 0 || color_ids.size()>0)
                hasMoved = true;

            event->accept();
            updatePoints();
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
    accumMouseChanges = QPointF(0.0,0.0);

    if(event->button() == Qt::LeftButton && brush)
    {
        event->accept();
        return; //mouseMoveEvent(event);
    }

    if (event->button() == Qt::LeftButton && pick(event->scenePos().toPoint(), nodeId, targetId, NULL))
    {
        inPanMode  = false;

        if (!(event->modifiers() & Qt::ControlModifier) && (nodeId != IMAGE_NODE_ID))
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
        updateGeometry();
    }  else if (event->button() == Qt::RightButton)
    {
        inPanMode = true;
        event->accept();
    }
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
            updateGeometry();
        }
    } else
    {
        uint nodeId, targetId;
        QPointF hitPoint;
        if (event->button() == Qt::LeftButton && pick(event->scenePos().toPoint(), nodeId, targetId, &hitPoint))
        {
            event->accept();
            QPoint seed = hitPoint.toPoint();

            cv::Vec3b bgrPixel = displayImage()->at<cv::Vec3b>(seed.y(), seed.x());
            QColor image_color(bgrPixel[2], bgrPixel[1], bgrPixel[0]);

            if (nodeId == COLOR_NODE_ID && curDisplayMode==0)
            {
                image_color = m_splineGroup.colorMapping[targetId].second;
            }

            QColor color = QColorDialog::getColor(image_color, (QWidget*)this->activeWindow());
            if(color.isValid())
            {
                if (nodeId == COLOR_NODE_ID)
                {
                    m_splineGroup.colorMapping[targetId].second = color;
                } else
                {
                    m_splineGroup.colorMapping.push_back(std::pair<QPoint, QColor>(seed,color));
                    if (selectedObjects.size()>0 && selectedObjects.last().first == COLOR_NODE_ID)
                    {
                        selectedObjects.last().second = m_splineGroup.colorMapping.size()-1;
                    }
                }

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

    if (event->delta() > 0)
        m_scale *= 1.2f;
    else
        m_scale /= 1.2f;

    QPointF afterScaling = imageToSceneCoords(imgCoords);
    m_translation -= (afterScaling-beforeScaling);

    changeDisplayModeText();
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
        updateImage();
        return;

    } if (event->key() == Qt::Key_U)
    {
        curDisplayMode = (curDisplayMode-1); if (curDisplayMode < 0)    curDisplayMode = NUM_DISPLAY_MODES-1;
        changeDisplayModeText();
        adjustDisplayedImageSize();
        updateImage();
        return;

    } else if (event->key() == Qt::Key_W)
    {
        //Reset blank image
        m_curImage = orgBlankImage.clone();
        m_splineGroup.colorMapping.clear();
        updateDisplay();
        return;

    } else if (event->key() == Qt::Key_R)
    {
        recomputeAllSurfaces();
        return;
    } else if (event->key() == Qt::Key_S)
    {
        emit triggerShading();
        return;
    } else if (event->key() == Qt::Key_M)
    {
        qDebug("\n%s\n", memory_info().toStdString().c_str());
        return;
    } else if (event->key() == Qt::Key_C)
    {
        displaySimpleSurfaces = !displaySimpleSurfaces;
        updateGeometry();
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
        QVector<int> deleted_color_ids;
        if (selectedObjects.size() > 0)
        {
            for (int i=0; i<selectedObjects.size(); ++i)
            {
                uint nodeId = selectedObjects[i].first;
                uint targetId = selectedObjects[i].second;
                if (nodeId == CPT_NODE_ID)
                {
                    m_splineGroup.removeControlPoint(targetId);
                    //qDebug("Delete cpt %d", targetId);

                } else if (nodeId == SPLINE_NODE_ID)
                {
                    if (m_curSplineIdx == (int)targetId) m_curSplineIdx = -1;
                    //qDebug("Delete spline %d", targetId);
                    m_splineGroup.removeSpline(targetId);
                    currentSplineChanged();
                } /*else if (nodeId == SURFACE_NODE_ID)
                {
                    qDebug("Delete surface %d", targetId);
                    m_splineGroup.removeSurface(targetId);
                }*/ else if (nodeId == COLOR_NODE_ID)
                {
                    deleted_color_ids.push_back(targetId);
                    selectedObjects.erase(selectedObjects.begin()+i);
                    --i;
                }
            }

            if (deleted_color_ids.size()>0)
            {
                std::sort(deleted_color_ids.begin(), deleted_color_ids.end());
                int counter = 0;
                for (int k=0; k<deleted_color_ids.size(); ++k)
                {
                    m_splineGroup.colorMapping.erase(m_splineGroup.colorMapping.begin()+deleted_color_ids[k]-counter);
                    ++counter;
                }
            }
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

void GLScene::updateDisplay()
{
    glWidget->makeCurrent();
    buildDisplayImage();
    buildGeometry();
    update();
}

void GLScene::updateGeometry()
{
    glWidget->makeCurrent();
    buildGeometry();
    update();
}

void GLScene::updateImage()
{
    glWidget->makeCurrent();
    buildDisplayImage();
    update();
}

void GLScene::updatePoints()
{
    glWidget->makeCurrent();
    buildPoints();
    buildColorPoints();
    update();
}

void  GLScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    /*if (painter->paintEngine()->type() != QPaintEngine::OpenGL && painter->paintEngine()->type() != QPaintEngine::OpenGL2)
    {
        qWarning("OpenGLScene: drawBackground needs a QGLWidget to be set as viewport on the graphics view");
        return;
    }*/

    initialize();

    //Setup OpenGL view
    glClearColor(0.7, 0.7, 0.7, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1000.0, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRenderMode(GL_RENDER);
    draw();

    QTransform transform;
    QPointF scaling(m_scale*imSize.width()/(float)m_curImage.cols,
                    m_scale*imSize.height()/(float)m_curImage.rows);
    QPointF full_translation = QPointF(width()/2.0 - m_scale*imSize.width()/2.0,
                                       height()/2.0 - m_scale*imSize.height()/2.0) + m_translation;
    transform.translate(full_translation.x(), full_translation.y());
    transform.scale(scaling.x(), scaling.y());
    ellipseGroup->setTransform(transform);
}

void GLScene::buildGeometry(bool only_show_splines)
{
    buildPoints(only_show_splines);
    buildCurves(only_show_splines);
    buildSurfaces(only_show_splines);
    buildColorPoints();
}

void GLScene::buildPoints(bool only_show_splines)
{
    glNewList (points_display_list, GL_COMPILE);

    if (showCurves)
    {
        glPointSize(pointSize);
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
        }
    }

    glEndList ();
}

void GLScene::buildColorPoints()
{
    glNewList (colors_display_list, GL_COMPILE);

    if (showColors)
        for (uint i=0; i<m_splineGroup.colorMapping.size(); ++i)
            draw_color_point(i);

    glEndList ();
}

void GLScene::buildCurves(bool only_show_splines)
{
    glNewList (curves_display_list, GL_COMPILE);

    if (showCurves)
    {
        glLineWidth(pointSize/5.0);
        for (int i=0; i<num_splines(); ++i)
        {
            if (spline(i).num_cpts() <= 1)
                continue;
            else
                draw_spline(i, only_show_splines);
        }
    }

    glEndList ();
}

void GLScene::buildSurfaces(bool only_show_splines)
{
    glNewList (surfaces_display_list, GL_COMPILE);

    if (showCurves)
    {
        if (!only_show_splines)
            for (int i=0; i<m_splineGroup.num_surfaces(); ++i)
            {
                if (surface(i).controlMesh.size() == 0)
                    continue;
                draw_surface(i);
            }
    }

    glEndList ();
}

void GLScene::buildDisplayImage()
{
    glWidget->makeCurrent();
    if (texId>0)
    {
        glBindTexture(GL_TEXTURE_2D, texId);
        glPixelStorei(GL_UNPACK_ALIGNMENT, (displayImage()->step & 3) ? 1 : 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, displayImage()->cols, displayImage()->rows, 0,
                     BGRColourFormat(), GL_UNSIGNED_BYTE, (GLvoid*)displayImage()->data);
        //qDebug("%d %d: %s", displayImage()->cols, displayImage()->rows, glewGetErrorString(0));

        glNewList (image_display_list, GL_COMPILE);
        glColor3d(1.0, 1.0, 1.0);
        draw_image(*displayImage());
        glEndList ();
    }
}

void GLScene::initialize()
{
    //GLEW initialization
    static bool initialized = false;
    if (!initialized)
    {
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            /* Problem: glewInit failed, something is seriously wrong. */
            qWarning("GLEW Error: %s\n", glewGetErrorString(err));
        } else
        {
        }

        initialized = true;

        image_display_list = glGenLists(5);
        points_display_list = image_display_list+1;
        colors_display_list = image_display_list+2;
        curves_display_list = image_display_list+3;
        surfaces_display_list = image_display_list+4;

        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        buildDisplayImage();
        buildGeometry();
    }
}

void GLScene::draw()
{
    glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
    glHint( GL_POINT_SMOOTH_HINT, GL_FASTEST );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glPushMatrix();
    glTranslatef(m_translation.x(), m_translation.y(), -100.0f);
    glTranslatef(width()/2.0 - m_scale*imSize.width()/2.0, height()/2.0 - m_scale*imSize.height()/2.0, 0.0f);
    glScalef(m_scale, m_scale, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId);
    glCallList(image_display_list);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.0f);
    glCallList(surfaces_display_list);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 2.0f);
    glCallList(curves_display_list);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 3.0f);
    glCallList(colors_display_list);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 4.0f);
    glCallList(points_display_list);
    glPopMatrix();

    /*if (!only_show_splines)
    for (int i=0; i<ellipses.size(); ++i)
    {
        draw_ellipse(ellipses[i].center, ellipses[i].size, ellipses[i].brush);
    }*/

    glPopMatrix();

    glDisable(GL_DEPTH_TEST);
}

void GLScene::draw_image(cv::Mat& image)
{
    glPushName(IMAGE_NODE_ID);
    glPushName(0);

    float tex_width = imSize.width();
    float tex_height = imSize.height();

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

    glPopName();
    glPopName();
}

void GLScene::draw_color_point(int color_id)
{
    if (!showColors || color_id >= (int)m_splineGroup.colorMapping.size())
        return;

    std::pair<QPoint, QColor> colorPoint = m_splineGroup.colorMapping[color_id];
    QPoint pos = colorPoint.first;
    cv::Vec3b disp_color = displayImage()->at<cv::Vec3b>(colorPoint.first.y(), colorPoint.first.x()); //BGR
    float hw = pointSize/2.0;

    glPushName(COLOR_NODE_ID);
    glPushName(color_id);

    glColor3d(0,0,1);
    if (disp_color[0] ==255 && disp_color[1]==0 && disp_color[2] == 0) glColor3d(0,0,0.5);
    if (selectedObjects.contains(std::pair<uint, uint>(COLOR_NODE_ID, color_id)))
    {
        glColor3d(1.0, 0.0, 0.0);
        if (disp_color[0] == 0 && disp_color[1] ==0 && disp_color[2] ==255) glColor3d(0.5,0,0.0);
    }
    glEnable(GL_POLYGON_OFFSET_FILL); // Avoid Stitching!
    glPolygonOffset(1.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_QUADS);
    glVertex2f(pos.x()-hw, pos.y()-hw);
    glVertex2f(pos.x()+hw, pos.y()-hw);
    glVertex2f(pos.x()+hw, pos.y()+hw);
    glVertex2f(pos.x()-hw, pos.y()+hw);
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);

    glColor3ub(colorPoint.second.red(), colorPoint.second.green(), colorPoint.second.blue());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    glVertex2f(pos.x()-hw, pos.y()-hw);
    glVertex2f(pos.x()+hw, pos.y()-hw);
    glVertex2f(pos.x()+hw, pos.y()+hw);
    glVertex2f(pos.x()-hw, pos.y()+hw);
    glEnd();

    glPopName();
    glPopName();
}

void GLScene::draw_control_point(int point_id)
{
    ControlPoint& cpt = m_splineGroup.controlPoint(point_id);
    if (!showControlPoints)
        return;

    glPushName(CPT_NODE_ID);
    glPushName(point_id);

    glColor3d(0.0, 0.0, 1.0);
    if (selectedObjects.contains(std::pair<uint, uint>(CPT_NODE_ID, point_id)))
    {
        glColor3d(1.0, 0.0, 0.0);
    }

    QPointF pos = cpt;
    glBegin(GL_POINTS);
    glVertex2f(pos.x(), pos.y());
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
        if (!only_show_splines && showNormals && (!showCurrentCurvePoints || m_curSplineIdx == spline_id))
        {
            glBegin(GL_LINES);
            glColor3f(0.0, 1.0, 0.0);

            QVector<QPointF> normals = bspline.getNormals(true);
            QVector<ControlPoint> subdPts = bspline.getPoints();
            for (int i = 0; i < subdPts.size(); ++i)
            {
                QPointF curvPos = subdPts[i];
                QPointF scenePos = curvPos;
                glVertex2f(scenePos.x(), scenePos.y());

                QPointF normal = curvPos + normals[i]*5.0;
                glVertex2f(normal.x(), normal.y());
            }

            glColor3f(1.0, 0.0, 0.0);
            normals = bspline.getNormals(false);
            for (int i = 0; i < subdPts.size(); ++i)
            {
                QPointF curvPos = subdPts[i];
                QPointF scenePos = curvPos;
                glVertex2f(scenePos.x(), scenePos.y());

                QPointF normal = curvPos + normals[i]*5.0;
                glVertex2f(normal.x(), normal.y());
            }

            glEnd();
        }

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
            QPointF pt = lp.at(i);
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

    QVector<ControlPoint> subDividePts = bspline.getDisplayPoints(drawingSubdLevels);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < subDividePts.size(); ++i)
    {
        glVertex2f(subDividePts[i].x(), subDividePts[i].y());
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
        //        glColor3f(0.65,0.65,0.65);
        if (surf.direction == INWARD_DIRECTION)
        {
            glColor3f(0,0.45,0);
        }
        else
        {
            glColor3f(0.45,0,0);
        }
        glLineWidth(2.0);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

        if (!displaySimpleSurfaces)
        {
            QVector<QVector<int> > faceIndices = surf.faceIndices;
            for (int i=0; i<faceIndices.size(); ++i)
            {
                if (faceIndices[i].size() == 4)
                    glBegin(GL_QUADS);
                else
                    glBegin(GL_TRIANGLES);
                for (int m=0; m<faceIndices[i].size(); ++m)
                {
                    QPointF point = surf.vertices[faceIndices[i][m]];
                    glVertex2f(point.x(), point.y());
                }
                glEnd();
            }
        } else
        {
            glBegin(GL_LINE_STRIP);
            for (int i=0; i<surf.controlMesh.first().size(); ++i)
            {
                QPointF point = surf.vertices[surf.controlMesh.first()[i]];
                glVertex2f(point.x(), point.y());
            }
            glEnd();
        }

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glLineWidth(1.0);
    }

    glPopName();
    glPopName();
}

void GLScene::draw_ellipse(QPointF center, float size, QBrush brush)
{
    //FLORA: This is not used at the moment

    glPointSize(size); //glPointSize(size*m_scale);
    glEnable(GL_POINT_SMOOTH);
    QColor color = brush.color();
    glColor4i(color.red(), color.green(), color.red(), color.alpha());

    /*GLuint objectID;
    glGenTextures(1, &objectID);
    glBindTexture(GL_TEXTURE_2D, objectID);
    GLenum inputColourFormat = BGRColourFormat();
    glTexImage2D(GL_TEXTURE_2D, 0, 3,image.cols, image.rows, 0, inputColourFormat, GL_UNSIGNED_BYTE, image.data);*/

    //glEnable(GL_POINT_SPRITE);
    //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    QPointF pos = center;
    glBegin(GL_POINTS);
    glVertex2f(pos.x(), pos.y());
    glEnd();
}

void GLScene::clearCurrentSelections(bool clearImages)
{
    if (clearImages)
    {
        resultImg = cv::Mat();
        shadedImg = cv::Mat();
        surfaceImg = cv::Mat();
    }
    selectedObjects.clear();
    shadingProfileView->cpts_ids.clear();
    curSplineRef() = -1;
    currentSplineChanged();
    selectedPointChanged();
    shadingProfileView->updatePath();
}

void GLScene::adjustDisplayedImageSize()
{
    imSize = QSizeF(displayImage()->cols, displayImage()->rows);
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
    QPointF topLeft(width()/2.0 - m_scale*imSize.width()/2.0, height()/2.0 - m_scale*imSize.height()/2.0);
    topLeft += m_translation;
    QPointF scaling(m_scale*imSize.width()/(float)m_curImage.cols, m_scale*imSize.height()/(float)m_curImage.rows);
    QPointF imgPos = scenePos - topLeft;
    imgPos.setX(imgPos.x()/scaling.x());
    imgPos.setY(imgPos.y()/scaling.y());
    return imgPos;
}

QPointF GLScene::imageToSceneCoords(QPointF imgPos)
{
    QPointF topLeft(width()/2.0 - m_scale*imSize.width()/2.0, height()/2.0 - m_scale*imSize.height()/2.0);
    topLeft += m_translation;
    QPointF scaling(m_scale*imSize.width()/(float)m_curImage.cols, m_scale*imSize.height()/(float)m_curImage.rows);
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
    glInitNames();
    draw();

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

        //qDebug("Pick: %d %d %d", _nodeIdx, _targetIdx, hits);

        if (_hitPointPtr)
        {
            GLuint zscale=~(0u);
            GLdouble min_zz = ((GLdouble)min_z) / ((GLdouble)zscale);
            GLdouble max_zz = ((GLdouble)max_z) / ((GLdouble)zscale);
            GLdouble zz     = 0.5F * (min_zz + max_zz);
            GLdouble objX, objY, objZ;
            gluUnProject(x, y, zz, m_modelview, m_projection, viewport, &objX, &objY, &objZ);
            *_hitPointPtr = sceneToImageCoords( QPointF(objX, objY));
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
    spline(m_curSplineIdx).subv_levels = curveSubdLevels;
    currentSplineChanged();
}

void GLScene::insertPointNextToSelected()
{
    bool refresh = false;
    for (int i=0; i<selectedObjects.size(); ++i)
    {
        if (selectedObjects[i].first == CPT_NODE_ID)
        {
            int cpt_id = selectedObjects[i].second;
            ControlPoint& cpt = controlPoint(cpt_id);
            if (cpt.num_splines() > 0)
            {
                BSpline& curve = cpt.splineAt(0);
                int index = -1;
                for (int k=0; k<curve.cptRefs.size(); ++k)
                {
                    if (curve.cptRefs[k] == cpt_id)
                    {
                        index = k;
                        break;
                    }
                }

                if (index >=0)
                {
                    refresh = true;
                    ControlPoint newPoint;
                    if (curve.num_cpts() == 1)
                    {
                        newPoint = curve.pointAt(index) + ControlPoint(QPointF(5, 5));
                    } else
                    {
                        if (index > 0 && index < curve.num_cpts()-1)
                        {
                            ControlPoint movedPoint;
                            movedPoint = (index>0)?(1.0/3.0)*curve.pointAt(index-1)+(2.0/3.0)*curve.pointAt(index):
                                                   (1.0/3.0)*curve.pointAt(index)+(2.0/3.0)*curve.pointAt(index+1);

                            cpt.setX(movedPoint.x());
                            cpt.setY(movedPoint.y());
                            cpt.setZ(movedPoint.z());
                            for (int k=0; k<2; ++k)
                                cpt.attributes[k]  = movedPoint.attributes[k];
                        }

                        newPoint = (index<curve.num_cpts()-1)?(1.0/3.0)*curve.pointAt(index+1)+(2.0/3.0)*curve.pointAt(index):
                                         (1.0/3.0)*curve.pointAt(index)+(2.0/3.0)*curve.pointAt(index-1);
                    }

                    int new_cpt_id = splineGroup().addControlPoint(newPoint, newPoint.z());
                    for (int k=0; k<2; ++k)
                        controlPoint(new_cpt_id).attributes[k]  = newPoint.attributes[k];
                    if (index == curve.num_cpts()-1)
                        curve.cptRefs.insert(index, new_cpt_id);
                    else
                        curve.cptRefs.insert(curve.cptRefs.begin() + index+1, new_cpt_id);
                    controlPoint(new_cpt_id).splineRefs.push_back(curve.ref);
                }
            }
        }
    }
    if (refresh)
        recomputeAllSurfaces();
}

void GLScene::cleanMemory()
{
    //qDebug("\nCleanMemory\n%s", memory_info().toStdString().c_str());

    m_splineGroup.garbage_collection(true);
    shadingProfileView->cpts_ids.clear();

    for (int k=0; k<selectedObjects.size(); ++k)
    {
        if (selectedObjects[k].first == SPLINE_NODE_ID)
        {
            if (m_splineGroup.new_spline_indices.find(selectedObjects[k].second)!= m_splineGroup.new_spline_indices.end())
            {
                selectedObjects[k].second = m_splineGroup.new_spline_indices[selectedObjects[k].second];
                for (int i=0; i<spline(selectedObjects[k].second).num_cpts(); ++i)
                {
                    shadingProfileView->cpts_ids.push_back(spline(selectedObjects[k].second).cptRefs[i]);
                }
            }
            else
            {
                selectedObjects.erase(selectedObjects.begin() + k);
                --k;
            }
        } else if (selectedObjects[k].first == CPT_NODE_ID)
        {
            if (m_splineGroup.new_cpt_indices.find(selectedObjects[k].second)!= m_splineGroup.new_cpt_indices.end())
            {
                selectedObjects[k].second = m_splineGroup.new_cpt_indices[selectedObjects[k].second];
                shadingProfileView->cpts_ids.push_back(selectedObjects[k].second);
            }
            else
            {
                selectedObjects.erase(selectedObjects.begin() + k);
                --k;
            }
        }
    }

    int old_spline_idx = m_curSplineIdx;
    if (m_splineGroup.new_spline_indices.find(m_curSplineIdx)!= m_splineGroup.new_spline_indices.end())
    {
        m_curSplineIdx = m_splineGroup.new_spline_indices[m_curSplineIdx];
    } else
    {
        m_curSplineIdx = -1;
    }

    m_splineGroup.new_cpt_indices.clear();
    m_splineGroup.new_spline_indices.clear();
    m_splineGroup.new_surface_indices.clear();

    if (m_curSplineIdx != old_spline_idx)
    {
        currentSplineChanged();
    }

    selectedPointChanged();
    shadingProfileView->updatePath();
}

void GLScene::recomputeAllSurfaces()
{
    QTime t, t2;
    t.start(); t2.start();
    int tm;

    m_splineGroup.imageSize = cv::Size(currentImage().cols, currentImage().rows);
    int npoints=0, ncurves=0, nsurfaces=0, nslopecurves = 0;
    int curves_timing, coloring_timing, dt_timing, surfaces_timing, garbage_collection_timing, offscreen_rendering_timing;

    for (int k=0; k<num_surfaces(); ++k)
    {
        int curveRef = surface(k).splineRef;
        if (surface(k).ref >= 0 && curveRef>=0 && surface(k).controlMesh.size()>0)
        {
            if (spline(curveRef).num_cpts()<=1 ||
                    (surface(k).direction==INWARD_DIRECTION && !spline(curveRef).has_inward_surface) ||
                    (surface(k).direction==OUTWARD_DIRECTION && !spline(curveRef).has_outward_surface))
                m_splineGroup.removeSurface(k);
        }
    }

    cleanMemory();
    garbage_collection_timing = t.elapsed();
    t.restart();

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

    cv::Mat dt, curvesIm;
    curvesIm = curvesImage(false, 1.5);
    offscreen_rendering_timing = t.elapsed();
    t.restart();

    update_region_coloring(curvesIm);
    coloring_timing = t.elapsed();
    t.restart();

    if (nsurfaces > 0)
    {
        cv::Mat curvesGrayIm = curvesIm.clone();
        cv::normalize(curvesGrayIm, curvesGrayIm, 0.0, 1.0, cv::NORM_MINMAX);
        cv::distanceTransform(curvesGrayIm,dt,CV_DIST_L2,CV_DIST_MASK_PRECISE);
    }
    dt_timing = t.elapsed();
    t.restart();

    if (nsurfaces>0)
    {
        cv::Mat luminance;
        if (clipHeight)
        {
            cv::cvtColor(currentImage(), luminance, CV_BGR2Lab);
        }

        for (int i=0; i<num_splines(); ++i)
        {
            if (spline(i).num_cpts() > 1)
                spline(i).computeSurfaces(dt, luminance, clipHeight);
        }
    }
    surfaces_timing = t.elapsed();

    tm = t2.elapsed();
    char timings[1024];

    //Cleanup
    curvesIm.release();
    dt.release();

    qDebug("\n************************************************************");
    qDebug("Garbage Collection: %d ms\nSubdivide Curves: %d ms\nOffscreen Rendering: %d ms\nUpdate Region Coloring: %d ms\nCompute distance transform: %d ms\nCompute surfaces (incl tracing): %d ms",
           garbage_collection_timing, curves_timing, offscreen_rendering_timing, coloring_timing, dt_timing, surfaces_timing);

    if (interactiveShading)
    {
        t.restart();
        emit triggerShading();
        sprintf(timings, "Stats: %dx%d res, %d points, %d curves (incl %d slopes), %d surfaces, surfaces computation %d ms, shading %d ms", currentImage().cols, currentImage().rows, npoints, ncurves, nslopecurves, nsurfaces, tm, t.elapsed());
        // updateDisplay() called in ApplyShading
    }
    else
    {
        sprintf(timings, "Stats: %dx%d res, %d points, %d curves (incl %d slopes), %d surfaces, surfaces computation %d ms", currentImage().cols, currentImage().rows, npoints, ncurves, nslopecurves, nsurfaces, tm);
        updateDisplay();
    }
    //    sprintf(timings, "Stats: %dx%d res, %d points, %d curves (incl %d slopes), %d surfaces, surfaces computation %d ms", currentImage().cols, currentImage().rows, npoints, ncurves, nslopecurves, nsurfaces, t2.elapsed());
    stats = timings;
    emit setStatusMessage("");


#ifdef QT_DEBUG
    qDebug("\n[Debug Mode] %s", memory_info().toStdString().c_str());
#endif
    qDebug("\n%s", stats.toStdString().c_str());
    qDebug("************************************************************");
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
    updateGeometry();
}

void GLScene::subdivide_current_spline(){
    if (m_curSplineIdx >=0 &&  m_curSplineIdx<m_splineGroup.num_splines() ) //Check validity
    {
        BSpline& spline = m_splineGroup.spline(m_curSplineIdx);
        bool has_uniform_subd = spline.has_uniform_subdivision;
        bool has_loop = spline.has_loop();

        QVector<ControlPoint> new_points;
        QVector<int> new_cpt_refs;
        QVector<int> old_cpt_refs = spline.cptRefs;

        new_points = subDivide(spline.getControlPoints(),1, has_uniform_subd);
        if (has_uniform_subd)
        {
            new_points.pop_back(); new_points.pop_front();
        }

        for (int i=0; i<(!has_loop?new_points.size():new_points.size()-1); ++i)
        {
            int new_cpt_id = m_splineGroup.addControlPoint(new_points[i], 0.0);
            new_cpt_refs.push_back(new_cpt_id);
            for (int k=0; k<2; ++k)
                controlPoint(new_cpt_id).attributes[k] = new_points[i].attributes[k];
        }
        if (has_loop)
            new_cpt_refs.push_back(new_cpt_refs.first());

        ControlPoint& cpt = controlPoint(old_cpt_refs.first());
        for (int k=0; k<cpt.num_splines(); ++k)
        {
            if (cpt.splineRefs[k] != spline.ref)
            {
                BSpline& connected = m_splineGroup.spline(cpt.splineRefs[k]);
                for (int j=0; j<connected.num_cpts(); ++j)
                    if (connected.cptRefs[j]==cpt.ref)
                        connected.cptRefs[j] = new_cpt_refs.first();
                controlPoint(new_cpt_refs.first()).splineRefs.push_back(connected.ref);
                cpt.splineRefs.erase(cpt.splineRefs.begin()+k); --k;
            }
        }

        if (!has_loop)
        {
            ControlPoint& cpt2 = controlPoint(old_cpt_refs.last());
            for (int k=0; k<cpt2.num_splines(); ++k)
            {
                if (cpt2.splineRefs[k] != spline.ref)
                {
                    BSpline& connected = m_splineGroup.spline(cpt2.splineRefs[k]);
                    for (int j=0; j<connected.num_cpts(); ++j)
                        if (connected.cptRefs[j]==cpt2.ref)
                            connected.cptRefs[j] = new_cpt_refs.last();
                    controlPoint(new_cpt_refs.last()).splineRefs.push_back(connected.ref);
                    cpt2.splineRefs.erase(cpt2.splineRefs.begin()+k); --k;
                }
            }
        }

        for (int k=0; k<old_cpt_refs.size(); ++k)
        {
            ControlPoint& point = controlPoint(old_cpt_refs[k]);
            m_splineGroup.removeControlPoint(point.ref);
        }

        //Connect to curves
        for (int i=0; i<new_cpt_refs.size(); ++i)
        {
            if (!m_splineGroup.addControlPointToSpline(m_curSplineIdx, new_cpt_refs[i]))
                break;
        }

        recomputeAllSurfaces();
    }
}

void GLScene::applyBlackCurves()
{
    if (shadedImg.cols == 0)
        return;

    bool addBlackCurves = false;
    for (int i=0; i<num_splines(); ++i)
        if (spline(i).num_cpts() > 1 && (spline(i).thickness>0 || globalThickness>0))
        {
            addBlackCurves = true; break;
        }
    if (!addBlackCurves)
    {
        resultImg = shadedImg.clone();
        return;
    }
    glWidget->makeCurrent();
    GLuint imageWidth = shadedImg.cols,
            imageHeight = shadedImg.rows;

    GLenum inputColourFormat = BGRColourFormat();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint( GL_TEXTURE_COMPRESSION_HINT, GL_NICEST );

    //Setup for offscreen drawing if fbos are supported
    GLuint framebuffer, renderbuffer, depthbuffer;
    if (setupFrameBuffer(framebuffer, renderbuffer, depthbuffer, imageWidth, imageHeight))
    {
        //Drawing
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);    glLoadIdentity();
        glOrtho(0, imageWidth, imageHeight, 0, -1000.0, 1000.0);
        glMatrixMode(GL_MODELVIEW);     glLoadIdentity();

        //Render result image
        GLuint texId2;
        glGenTextures(1, &texId2);
        glBindTexture(GL_TEXTURE_2D, texId2);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8 ,shadedImg.cols, shadedImg.rows, 0, inputColourFormat, GL_UNSIGNED_BYTE, shadedImg.data);

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, imageHeight);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(imageWidth, imageHeight);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(imageWidth, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glDeleteTextures(1, &texId2);

        //Draw black curves
        int old_subdLevels  = drawingSubdLevels;
        bool resubdivide = (drawingSubdLevels!=5);
        drawingSubdLevels = 5;
        for (int i=0; i<num_splines(); ++i)
        {
            if (m_splineGroup.spline(i).num_cpts() <=1)   continue;
            int thickness = spline(i).thickness;
            if (thickness <= 0 && !spline(i).is_slope)  thickness = globalThickness ;
            if (thickness)
            {
                glLineWidth(thickness);

                if (resubdivide) spline(i).display_points.clear();
                draw_spline(i, true, false);
                if (resubdivide) spline(i).display_points.clear();
            }
        }
        drawingSubdLevels = old_subdLevels;

        cv::Mat img;
        img.create(imageHeight, imageWidth, CV_8UC3);
        glReadPixels(0, 0, imageWidth, imageHeight, inputColourFormat, GL_UNSIGNED_BYTE, img.data);
        cv::flip(img, img, 0);
        resultImg = img.clone();
    } else
    {
        resultImg = shadedImg.clone();
    }

    //Clean up offscreen drawing
    cleanupFrameBuffer(framebuffer, renderbuffer, depthbuffer);

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
        updateGeometry();
    }
}

int GLScene::registerPointAtScenePos(QPointF scenePos)
{
    unsigned int nodeId, targetId;
    QPointF hitPoint;

    if (pick(scenePos.toPoint(), nodeId, targetId, &hitPoint))
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

    int pointIdx = m_splineGroup.addControlPoint(hitPoint, 0.0);
    selectedObjects.clear();
    selectedObjects.push_back(std::pair<uint, uint>(CPT_NODE_ID, pointIdx));
    return pointIdx;
}

cv::Mat GLScene::curvesImageBGR(bool only_closed_curves, float thickness)
{
    glWidget->makeCurrent();
    GLuint imageWidth = m_curImage.cols,
            imageHeight = m_curImage.rows;

    cv::Mat img;

    //Setup for offscreen drawing if fbos are supported
    GLuint framebuffer, renderbuffer, depthbuffer;
    if (setupFrameBuffer(framebuffer, renderbuffer, depthbuffer, imageWidth, imageHeight))
    {
        //Drawing
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, imageWidth, imageHeight, 0, -1000.0, 1000.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);

        int old_subdLevels  = drawingSubdLevels;
        bool resubdivide = (drawingSubdLevels!=5);
        drawingSubdLevels = 5;
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
            if (resubdivide) spline(i).display_points.clear();
            draw_spline(i, true, false);
            if (resubdivide) spline(i).display_points.clear();
        }
        drawingSubdLevels = old_subdLevels;

        GLenum inputColourFormat = BGRColourFormat();
        img.create(imageHeight, imageWidth, CV_8UC3);

        //use fast 4-byte alignment (default anyway) if possible
        glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

        //set length of one complete row in destination data (doesn't need to equal img.cols)
        glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());

        glReadPixels(0, 0, img.cols, img.rows, inputColourFormat, GL_UNSIGNED_BYTE, img.data);
        cv::flip(img, img, 0);
    } else
    {
        img = cv::Mat(m_curImage.cols, m_curImage.rows, m_curImage.type(), cv::Scalar(255,255,255));
    }

    //Clean up offscreen drawing
    cleanupFrameBuffer(framebuffer, renderbuffer, depthbuffer);

    return img;
}

cv::Mat GLScene::curvesImage(bool only_closed_curves, float thickness)
{
    cv::Mat img = curvesImageBGR(only_closed_curves, thickness);

    cv::cvtColor(img, img, CV_BGR2RGB);
    cv::cvtColor(img, img, CV_RGB2GRAY);   //cv::imwrite("curv_img_bef.png", img);
    cv::threshold( img, img, 250, 255,   CV_THRESH_BINARY); //cv::imwrite("curv_img.png", img); //cv::imshow("Closed Curves", img);

    //qDebug("\n13 %.4f MB %s", img.cols*img.rows*img.elemSize()/(1024.0*1024.0), memory_info().toStdString().c_str());

    return img;
}


void GLScene::update_region_coloring(cv::Mat img)
{
    //curvesImageBGR(false, -1);;
    m_curImage = orgBlankImage.clone();

    cv::Mat curv_img;
    if (img.cols == 0)
        curv_img = curvesImage(false, 1.5);
    else    curv_img  = img.clone();

    //cv::imwrite("curv_img.png", curv_img);
    cv::convertScaleAbs(curv_img, curv_img, -1, 255 );
    //cv::imshow("Closed Curves", curv_img);

    cv::Mat mask(m_curImage.rows+2, m_curImage.cols+2, curv_img.type(), cv::Scalar(0));
    cv::Mat mask_vals(mask, cv::Range(1, m_curImage.rows+1), cv::Range(1, m_curImage.cols+1));
    curv_img.copyTo(mask_vals);
    //cv::imshow("Mask", mask);
    //cv::imwrite("mask.png", mask);

    cv::Mat result = m_curImage.clone();

    //Use cv::floodfill (this should be faster)
    for (int l=m_splineGroup.colorMapping.size()-1; l>=0; --l)
    {
        QPoint seed = m_splineGroup.colorMapping[l].first;
        QColor qcolor = m_splineGroup.colorMapping[l].second;
        cv::Scalar color(qcolor.blue(), qcolor.green(), qcolor.red());

        if (seed.y() < 0 || seed.y() >= result.rows || seed.x() < 0 || seed.x() >= result.cols)
            continue;

        /*cv::Vec3b def_color = orgBlankImage.at<cv::Vec3b>(seed.y(), seed.x());
        cv::Vec3b cur_color = result.at<cv::Vec3b>(seed.y(), seed.x());

        if (!(def_color[0] == cur_color[0] && def_color[1] == cur_color[1] && def_color[2] == cur_color[2]))
        {
            //if (!(def_color[0] == color[0] && def_color[1] == color[1] && def_color[2] == color[2]))

            m_splineGroup.colorMapping.erase(m_splineGroup.colorMapping.begin()+l);

            continue;
        }*/

        cv::floodFill(result, mask, cv::Point2i(seed.x(),seed.y()),color, 0, cv::Scalar(255,255,255), cv::Scalar(255,255,255));

        QVector<QPoint> neighbours;
        for (int i=0; i<result.rows; ++i)
        {
            for (int j=0; j<result.cols; ++j)
            {
                if (curv_img.at<uchar>(i,j) > 128)
                {
                    bool neighbouring = false;

                    for (int m=-1; m<=1; ++m)
                    {
                        for (int n=-1; n<=1; ++n)
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
            for (int k=0; k<3; ++k)
                result.at<cv::Vec3b>(neighbours[i].x(), neighbours[i].y())[k] = color[k];
        }
    }

    for (int i=0; i<result.rows; ++i)
    {
        for (int j=0; j<result.cols; ++j)
        {
            if (curv_img.at<uchar>(i,j) > 128)
            {
                bool stop = false;
                for (int m=-1; m<=1; ++m)
                {
                    for (int n=-1; n<=1; ++n)
                    {
                        if ((m!=0 || n!=0) && i+m>=0 && j+n>=0 && i+m<result.rows &&
                                j+n < result.cols && curv_img.at<uchar>(i+m,j+n) <128)
                        {
                            cv::Vec3b current = result.at<cv::Vec3b>(i+m,j+n);
                            for (int k=0; k<3; ++k)
                                result.at<cv::Vec3b>(i,j)[k] = current[k];
                            curv_img.at<uchar>(i,j) = 0;
                            stop = true;
                            break;
                        }
                    }
                    if (stop)   break;
                }
            }
        }
    }

    m_curImage = result.clone();
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
            surface_strings.push_back( surface(i).surfaceToOFF(color) );

            /*std::stringstream ss;
            ss << "Surface" << i << ".off";
            std::ofstream ofs(ss.str().c_str());
            ofs << surface_strings.back();
            ofs.close();
            */
        }

    QVector< QVector<int> > skippedSurfaces =  mergeSurfaces(mergedGroups, surface_strings);

    while (skippedSurfaces.size() > 0)
        skippedSurfaces = mergeSurfaces(skippedSurfaces, surface_strings);

    if (ghostSurfacesEnabled)
    {
        for (int i=0; i<num_splines(); ++i)
        {
            if (!spline(i).is_slope && spline(i).num_cpts() > 1)
            {
                if (spline(i).has_inward_surface != spline(i).has_outward_surface)
                {
                    if (!spline(i).has_inward_surface)
                        surface_strings.push_back(spline(i).ghostSurfaceString(INWARD_DIRECTION, currentImage()));
                    else
                        surface_strings.push_back(spline(i).ghostSurfaceString(OUTWARD_DIRECTION, currentImage()));
                }
            }
        }
    }

    char timing[50];
    sprintf(timing, " | Surf Streams(incl merging): %d ms", t.elapsed());
    //stats += timing;
    //emit setStatusMessage("");

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
                mergedSurface.sharpCorners.insert(surf.sharpCorners.begin(), surf.sharpCorners.end());
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

                for (std::set<int>::iterator it = surf.sharpCorners.begin(); it != surf.sharpCorners.end(); ++it)
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

        int intersectingVertices = 0;
        int vertex_id = -1;
        for (int i=0; i<mergedSurface.controlMesh.size(); ++i)
        {
            if (mergedSurface.controlMesh[i].first() == mergedSurface.controlMesh[i].last())
            {
                vertex_id = mergedSurface.controlMesh[i].first();
                intersectingVertices++;
            }
        }
        if (intersectingVertices == 1)  mergedSurface.sharpCorners.insert(vertex_id);


        mergedSurface.computeFaceIndices();

        Surface &surface1 = surface(mergedGroups[i].first());
        QPointF pixelPoint = surface1.vertices[surface1.controlMesh.first()[surface1.controlMesh.first().size()/2]];
        cv::Vec3b color = currentImage().at<cv::Vec3b>(pixelPoint.y(), pixelPoint.x());
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
    shadingProfileView->max_extent = max(m_curImage.cols, m_curImage.rows);

    if (texId>0)
        updateImage();
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

        updateImage();
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
        for (int i=0; i<num_splines(); ++i)
            spline(i).subv_levels = curveSubdLevels;
        recomputeAllSurfaces();
        curDisplayMode = 0;
        changeDisplayModeText();
        adjustDisplayedImageSize();
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
    {
        recomputeAllSurfaces();
    }
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
            applyBlackCurves();
            updateDisplay();
        }
    }

    if (has_changed)
        recomputeAllSurfaces();
}

void GLScene::setInteractiveShading(bool b)
{
    interactiveShading = b;
}

void GLScene::setGhostSurfacesEnabled(bool b)
{
    bool shading = (ghostSurfacesEnabled != b);
    ghostSurfacesEnabled = b;
    if (shading)
        emit triggerShading();
}

void GLScene::setClipHeight(bool b)
{
    clipHeight = b;
    recomputeAllSurfaces();
    if (!interactiveShading)
    {
        emit triggerShading();
    }
}

void GLScene::emitSetStatusMessage(QString message)
{
    emit setStatusMessage(message);
}

QString GLScene::memory_info()
{
    QString system_info("Memory Info: ");
    system_info.append(
                QString("\n  Number of cores: %1")
                .arg(QThread::idealThreadCount()));

#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)

    QProcess p;
    p.start("awk", QStringList() << "/MemTotal/ { print $2 }" << "/proc/meminfo");
    p.waitForFinished();
    QString memory = p.readAllStandardOutput();
    system_info.append(QString("\n  Total Physical Memory: %1 MB").arg(memory.toLong() / 1024.0));
    p.close();

#elif defined(__WIN32)

    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memory_status)) {
        system_info.append(
                    QString("\n  Total Physical Memory: %1 MB")
                    .arg(memory_status.ullTotalPhys / (1024 * 1024.0)));
        system_info.append(
                    QString("\n  Total Virtual Memory: %1 MB")
                    .arg(memory_status.ullTotalPageFile / (1024 * 1024.0)));
    } else{
        system_info.append("Unknown Total Physical Memory");
        system_info.append("Unknown Total Virtual Memory");
    }
#endif

    size_t phys=0, virt=0;
    getCurrentRSS(phys, virt);

    system_info.append(
                QString("\n  Physical Memory Used by Me: %1 MB")
                .arg(phys / (1024 * 1024.0)));
    system_info.append(
                QString("\n  Virtual Memory Used by Me: %1 MB")
                .arg(virt / (1024 * 1024.0)));

    return system_info;
}
