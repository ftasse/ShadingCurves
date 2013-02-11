#include <GL/glu.h>
#include <QPainter>
#include <QPaintEngine>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsView>
#include <algorithm>

#include "GLScene.h"

static const unsigned int  SELECTION_BUFFER_SIZE = 10000;
static const unsigned int  NAME_STACK_SIZE       = 2;
static const unsigned int  IMAGE_NODE_ID = 0;
static const unsigned int  CPT_NODE_ID = 1;
static const unsigned int  SPLINE_NODE_ID = 2;

GLScene::GLScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_curSplineIdx = -1;
    m_sketchmode = IDLE_MODE;
    pointSize = 10.0;
}

GLScene:: ~GLScene()
{
}

void GLScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //if (event->button() == Qt::LeftButton)
    {
        if (selectedObjects.size() > 0)
        {
            for (int i=0; i<selectedObjects.size(); ++i)
            {
                uint nodeId = selectedObjects[i].first;
                uint targetId = selectedObjects[i].second;
                if (nodeId == CPT_NODE_ID)
                {
                    ControlPoint& cpt = m_splineGroup.controlPoint(targetId);
                    QPointF newP = sceneToImageCoords(event->scenePos());
                    cpt.setX(newP.x());
                    cpt.setY(newP.y());
                } else if (nodeId == SPLINE_NODE_ID)
                {
                    QPointF diff = sceneToImageCoords(event->scenePos() - event->lastScenePos());
                    BSpline& spline = m_splineGroup.spline(targetId);
                    for (int k=0; k<spline.count(); ++k)
                    {
                        ControlPoint& cpt = spline.pointAt(k);
                        cpt.setX(cpt.x()+diff.x());
                        cpt.setY(cpt.y()+diff.y());
                    }
                }
            }
            update();
            return;
        }
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void GLScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    uint nodeId, targetId;
    if (pick(event->scenePos().toPoint(), nodeId, targetId, NULL))
    {
        if (!(event->modifiers() & Qt::ControlModifier))
            selectedObjects.clear();
        if (nodeId != IMAGE_NODE_ID)
        {
            selectedObjects.push_back(std::pair<uint, uint>(nodeId, targetId));
            if (nodeId == SPLINE_NODE_ID)
            {
                m_curSplineIdx = targetId;
            }
        }
    }
    update();
}

void GLScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_sketchmode == ADD_CURVE_MODE)
    {
        int cptRef = registerPointAtScenePos(event->scenePos());
        if (cptRef < 0)
            return;

        if (m_curSplineIdx < 0)
        {
            createBSpline();
        }

        if (m_splineGroup.addControlPoint(m_curSplineIdx, cptRef))
        {
            update();
        }
    } else
    {
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

void GLScene::keyPressEvent(QKeyEvent *event)
{
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
                    qDebug("Delete cpt %d", targetId);
                    m_splineGroup.removeControlPoint(targetId);
                } else if (nodeId == SPLINE_NODE_ID)
                {
                    if (m_curSplineIdx == (int)targetId) m_curSplineIdx = -1;
                    qDebug("Delete spline %d", targetId);
                    m_splineGroup.removeSpline(targetId);
                }
            }
            selectedObjects.clear();
            update();
        }
    } else if (m_sketchmode == ADD_CURVE_MODE)
    {
        switch(event->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            {
                m_sketchmode = IDLE_MODE;
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

void  GLScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (painter->paintEngine()->type()
                    != QPaintEngine::OpenGL) {
                qWarning("OpenGLScene: drawBackground needs a "
                         "QGLWidget to be set as viewport on the "
                         "graphics view");
                return;
            }

    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    display();

    glGetDoublev(GL_MODELVIEW_MATRIX, m_modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, m_projection);
}

void GLScene::display()
{
    glInitNames();

    if (m_curImage.cols > 0)
    {
        glColor3d(1.0, 1.0, 1.0);
        glPushName(IMAGE_NODE_ID);
        glPushName(0);
        draw_image(m_curImage);
        glPopName();
        glPopName();
    }

    glLineWidth(pointSize/5.0);
    for (int i=0; i<m_splineGroup.num_splines(); ++i)
    {
        if (m_splineGroup.spline(i).count() == 0)
            continue;

        glPushName(SPLINE_NODE_ID);
        glPushName(i);
        glColor3d(0.0, 0.0, 1.0);
        if (selectedObjects.contains(std::pair<uint, uint>(SPLINE_NODE_ID, i)))
        {
            glColor3d(1.0, 0.0, 0.0);
        }

        draw_spline(m_splineGroup.spline(i).idx);
        glPopName();
        glPopName();
    }

    glEnable(GL_POINT_SMOOTH);
    glPointSize(pointSize);
    for (int i=0; i<m_splineGroup.num_controlPoints(); ++i)
    {
        if (m_splineGroup.controlPoint(i).count() == 0)
            continue;

        glPushName(CPT_NODE_ID);
        glPushName(i);
        glColor3d(0.0, 0.0, 1.0);
        if (selectedObjects.contains(std::pair<uint, uint>(CPT_NODE_ID, i)))
        {
            glColor3d(1.0, 0.0, 0.0);
        }


        draw_control_point(m_splineGroup.controlPoint(i).idx);
        glPopName();
        glPopName();
    }
}

void GLScene::draw_image(cv::Mat& image)
{
        glEnable( GL_TEXTURE_2D );

        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);

        // Set texture interpolation methods for minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Set texture clamping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum inputColourFormat;
        #ifdef GL_BGR
            inputColourFormat = GL_BGR;
        #else
            inputColourFormat = GL_BGR_EXT;
        #endif
        if (image.channels() == 1)  inputColourFormat = GL_LUMINANCE;

        glTexImage2D(GL_TEXTURE_2D, 0, 3,image.cols, image.rows, 0, inputColourFormat, GL_UNSIGNED_BYTE, image.data);

        float tex_width = imSize.width();
        float tex_height = imSize.height();

        //glBindTexture(GL_TEXTURE_2D, texture[index]);
        glPushMatrix();
        glLoadIdentity();
        glTranslated(width()/2 - tex_width/2, height()/2 - tex_height/2, -0.5f);
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
}

void GLScene::draw_control_point(int point_id)
{
    ControlPoint& cpt = m_splineGroup.controlPoint(point_id);
    QPointF pos = imageToSceneCoords(cpt);

    glBegin(GL_POINTS);
    glVertex3d(pos.x(), pos.y(), 0.5);
    glEnd();
}

void GLScene::draw_spline(int spline_id)
{
    BSpline& spline = m_splineGroup.spline(spline_id);
    int order = spline.degree() + 1;

    // check for incomplete curve
      if (spline.count() < order)
        return;

      int numKnots = spline.knotVectors().size();
      GLfloat *knots = new GLfloat[numKnots];
      for (int i = 0; i < numKnots; ++i)
      {
          knots[i] = spline.knotVectors()[i];
      }

      GLfloat *ctlpoints = new GLfloat[spline.count() * 3];
      for (int i = 0; i < spline.count(); ++i)
      {
        QPointF p = imageToSceneCoords(spline.pointAt(i));
        ctlpoints[i * 3 + 0] = (GLfloat)p.x();
        ctlpoints[i * 3 + 1] = (GLfloat)p.y();
        ctlpoints[i * 3 + 2] = 0.0f;
      }

      GLUnurbsObj *theNurb;
      theNurb = gluNewNurbsRenderer();


      #ifdef WIN32
        gluNurbsCallback(theNurb, GLU_ERROR, (void (__stdcall *)(void))(&nurbsError) );
      #else
        gluNurbsCallback(theNurb, GLU_ERROR, (GLvoid (*)()) (&nurbsError) );
      #endif


      gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);
      gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 5.0);

      gluBeginCurve(theNurb);
      gluNurbsCurve(theNurb, numKnots, knots, 3, ctlpoints, order, GL_MAP1_VERTEX_3);
      gluEndCurve(theNurb);

      gluDeleteNurbsRenderer(theNurb);
      delete [] ctlpoints;
      delete [] knots;
}

void GLScene::adjustDisplayedImageSize()
{
    cv::Mat& image = currentImage();
    imSize = QSizeF(image.cols, image.rows);
    if (imSize.width() > width())
    {
        imSize = QSizeF(width(), (width() * image.rows)/image.cols);
    }
    if (imSize.height() > height())
    {
        imSize = QSizeF((height() * image.cols)/image.rows, height());
    }
}

QPointF GLScene::sceneToImageCoords(QPointF scenePos)
{
    QPointF topLeft(width()/2 - imSize.width()/2, height()/2 - imSize.height()/2);
    QPointF scaling(imSize.width()/m_curImage.cols, imSize.height()/m_curImage.rows);
    QPointF imgPos = scenePos - topLeft;
    imgPos.setX(imgPos.x()/scaling.x());
    imgPos.setY(imgPos.y()/scaling.y());
    return imgPos;
}

QPointF GLScene::imageToSceneCoords(QPointF imgPos)
{
    QPointF topLeft(width()/2 - imSize.width()/2, height()/2 - imSize.height()/2);
    QPointF scaling(imSize.width()/m_curImage.cols, imSize.height()/m_curImage.rows);
    QPointF scenePos = imgPos;
    scenePos.setX(scenePos.x()*scaling.x());
    scenePos.setY(scenePos.y()*scaling.y());
    scenePos += topLeft;
    return scenePos;
}

bool GLScene::pick(const QPoint& _mousePos, unsigned int& _nodeIdx,
           unsigned int& _targetIdx, QPointF* _hitPointPtr )
{
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
    update();
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

    int pointIdx = m_splineGroup.addControlPoint(sceneToImageCoords(scenePos));
    selectedObjects.clear();
    selectedObjects.push_back(std::pair<uint, uint>(CPT_NODE_ID, pointIdx));
    update();
    return pointIdx;
}


bool GLScene::openImage(std::string fname)
{
    cv::Mat image = loadImage(fname);

    //Test if image was loaded
    if (image.cols > 0)
    {
        m_curImage = image;
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
    if (m_splineGroup.load(fname))
    {
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

void nurbsError(uint errorCode)
{
   const GLubyte *estring;

   estring = gluErrorString(errorCode);
   qDebug("Nurbs Error: %s\n", estring);
}
