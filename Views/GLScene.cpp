#include <QPainter>
#include <QPaintEngine>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsView>
#include <algorithm>
#include <set>

#include "GLScene.h"

#ifdef _WIN32 || _WIN64
#include <windows.h>
#endif
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
                    BSpline& spline = m_splineGroup.spline(targetId);
                    for (int k=0; k<spline.count(); ++k)
                    {
                        cpt_ids.insert(spline.pointAt(k).idx);
                    }

                }
            }

            for (std::set<int>::iterator it = cpt_ids.begin(); it != cpt_ids.end(); ++it)
            {
                ControlPoint& cpt = m_splineGroup.controlPoint(*it);
                cpt.setX(cpt.x()+diff.x());
                cpt.setY(cpt.y()+diff.y());
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

        if (m_splineGroup.addControlPointToSpline(m_curSplineIdx, cptRef))
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
    if (event->key() == Qt::Key_S)
    {
        if (m_curSplineIdx >=0 )
        {
            // HENRIK, include distrance transform image
            cv::Mat curvesIm = curvesImage();
            cv::Mat curvesGrayIm;
            cv::cvtColor(curvesIm, curvesGrayIm, CV_RGB2GRAY);
            cv::normalize(curvesGrayIm, curvesGrayIm, 0.0, 1.0, cv::NORM_MINMAX);

            cv::Mat dt;
            cv::distanceTransform(curvesGrayIm,dt,CV_DIST_L2,CV_DIST_MASK_3);

            m_splineGroup.createSurface(m_curSplineIdx,dt);
            update();
        }
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
                    qDebug("Delete cpt %d", targetId);
                    m_splineGroup.removeControlPoint(targetId);
                } else if (nodeId == SPLINE_NODE_ID)
                {
                    if (m_curSplineIdx == (int)targetId) m_curSplineIdx = -1;
                    qDebug("Delete spline %d", targetId);
                    m_splineGroup.removeSpline(targetId);
                } else if (nodeId == SURFACE_NODE_ID)
                {
                    qDebug("Delete surface %d", targetId);
                    m_splineGroup.removeSurface(targetId);
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
                /*qWarning("OpenGLScene: drawBackground needs a "
                         "QGLWidget to be set as viewport on the "
                         "graphics view");
                return;*/
            }

    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRenderMode(GL_RENDER);
    display();

    glGetDoublev(GL_MODELVIEW_MATRIX, m_modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, m_projection);
}

void GLScene::display(bool only_show_splines)
{
    glInitNames();
    glEnable(GL_POINT_SMOOTH);
    glPointSize(pointSize);

    if (m_curImage.cols > 0)
    {
        glColor3d(1.0, 1.0, 1.0);
        draw_image(m_curImage);
    }

    if (!only_show_splines)
    for (int i=0; i<m_splineGroup.num_surfaces(); ++i)
    {
        if (m_splineGroup.surface(i).controlPoints().size() == 0)
            continue;

        Surface& surface = m_splineGroup.surface(i);
        draw_surface(surface.idx);
    }

    glLineWidth(pointSize/5.0);
    for (int i=0; i<m_splineGroup.num_splines(); ++i)
    {
        if (m_splineGroup.spline(i).count() == 0)
            continue;

        draw_spline(m_splineGroup.spline(i).idx, only_show_splines);
    }

    if (!only_show_splines)
    for (int i=0; i<m_splineGroup.num_controlPoints(); ++i)
    {
        if (m_splineGroup.controlPoint(i).count() == 0)
            continue;

        draw_control_point(m_splineGroup.controlPoint(i).idx);
    }
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

        glPopName();
        glPopName();
}

void GLScene::draw_control_point(int point_id)
{
    glPushName(CPT_NODE_ID);
    glPushName(point_id);

    glColor3d(0.0, 0.0, 1.0);
    if (selectedObjects.contains(std::pair<uint, uint>(CPT_NODE_ID, point_id)))
    {
        glColor3d(1.0, 0.0, 0.0);
    }

    ControlPoint& cpt = m_splineGroup.controlPoint(point_id);
    QPointF pos = imageToSceneCoords(cpt);

    glBegin(GL_POINTS);
    glVertex3d(pos.x(), pos.y(), 0.5);
    glEnd();

    glPopName();
    glPopName();
}

void GLScene::draw_spline(int spline_id, bool only_show_splines)
{
    glPushName(SPLINE_NODE_ID);
    glPushName(spline_id);

    BSpline& spline = m_splineGroup.spline(spline_id);
    int order = spline.degree() + 1;

    // check for incomplete curve
      if (spline.count() <= 1 || spline.count() < order)
        return;


      // Display normals
      /*glColor3f(0.5, 0.5, 0.0);
      glBegin(GL_LINES);
      if (!only_show_splines)
      for (int i = 0; i < spline.count(); ++i)
      {
          float t = spline.closestParamToPointAt(i);
          QPointF curvPos = spline.derivativeCurvePoint(t, 0);
          QPointF scenePos = imageToSceneCoords(curvPos);
          glVertex2f(scenePos.x(), scenePos.y());

          QPointF normal = imageToSceneCoords(curvPos + spline.inward_normal(i)*20.0);
          glVertex2f(normal.x(), normal.y());
      }
      glEnd();
      */

      glColor3d(0.0, 0.0, 1.0);
      if (selectedObjects.contains(std::pair<uint, uint>(SPLINE_NODE_ID, spline_id)))
      {
          glColor3d(1.0, 0.0, 0.0);
      }

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

    Surface& surface = m_splineGroup.surface(surface_id);
    QPoint uv_order = surface.degree() + QPoint(1, 1);

    // check for incomplete curve
    if (surface.controlPoints().size() < uv_order.x() || surface.controlPoints()[0].size() < uv_order.y())
        return;

      QPoint numKnots(surface.u_knotVectors().size(), surface.v_knotVectors().size());
      GLfloat *u_knots = new GLfloat[numKnots.x()];
      GLfloat *v_knots = new GLfloat[numKnots.y()];
      for (int i = 0; i < numKnots.x(); ++i)
      {
          u_knots[i] = surface.u_knotVectors()[i];
      }
      for (int i = 0; i < numKnots.y(); ++i)
      {
          v_knots[i] = surface.v_knotVectors()[i];
      }

      int num_cpts = surface.controlPoints().size() * surface.controlPoints()[0].size();
      GLfloat *ctlpoints = new GLfloat[num_cpts * 3];
      for (int u = 0; u <  surface.controlPoints().size() ; ++u)
      {
          for (int v= 0; v <  surface.controlPoints()[u].size(); ++v)
          {
                QPoint pos(u, v);
                int h = surface.controlPoints()[u].size();
                QPointF p = imageToSceneCoords(surface.pointAt(pos));
                ctlpoints[(u*h + v) * 3 + 0] = (GLfloat)p.x();
                ctlpoints[(u*h + v) * 3 + 1] = (GLfloat)p.y();
                ctlpoints[(u*h + v) * 3 + 2] = -0.8f;
          }
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

      int renderMode;
      glGetIntegerv(GL_RENDER_MODE, &renderMode);
      if (renderMode == GL_RENDER)
      {
        gluBeginSurface(theNurb);
        gluNurbsSurface(theNurb,  numKnots.x(), u_knots, numKnots.y(), v_knots, surface.controlPoints()[0].size()*3, 3, ctlpoints,
                      uv_order.x(), uv_order.y(), GL_MAP2_VERTEX_3);
        gluEndSurface(theNurb);
      }

      gluDeleteNurbsRenderer(theNurb);
      delete [] ctlpoints;
      delete [] u_knots;
      delete [] v_knots;

      //Display Control Polygon
      glColor3f(0.65,0.65,0.65);
      glLineWidth(2.0);
      for (int k=0; k<surface.controlPoints().size(); ++k)
      {
          glBegin(GL_LINE_STRIP);
          for (int l=0; l<surface.controlPoints()[k].size(); ++l)
          {
              QPoint pos(k, l);
              QPointF scenePos = imageToSceneCoords(surface.pointAt(pos));
              glVertex3f(scenePos.x(), scenePos.y(), -0.8f);
          }
          glEnd();
      }
      if (!m_splineGroup.spline(surface.connected_spline_id).is_closed())
      {
        glBegin(GL_LINE_STRIP);
        for (int k=0; k<surface.controlPoints().size(); ++k)
        {
            QPointF scenePos = imageToSceneCoords(surface.pointAt(QPoint(k, 0)));
            glVertex3f(scenePos.x(), scenePos.y(), -0.8f);
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        for (int k=0; k<surface.controlPoints().size(); ++k)
        {
            QPointF scenePos = imageToSceneCoords(surface.pointAt(QPoint(k, surface.controlPoints()[k].size()-1)));
            glVertex3f(scenePos.x(), scenePos.y(), -0.8f);
        }
        glEnd();
      }
      glLineWidth(1.0);

      glPopName();
      glPopName();

      //Display control points that are not connected to any bspline
      for (int k=0; k<surface.controlPoints().size(); ++k)
      {
          for (int l=0; l<surface.controlPoints()[k].size(); ++l)
          {
              QPoint pos(k, l);
              if (surface.pointAt(pos).count() == 0)
              {
                  draw_control_point(surface.pointAt(pos).idx);
              }
          }
      }


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

cv::Mat GLScene::curvesImage()
{
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRenderMode(GL_RENDER);
    display(true);

    cv::Mat img;
    img.create(imSize.height(), imSize.width(), CV_8UC3);

    //use fast 4-byte alignment (default anyway) if possible
    glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

    //set length of one complete row in destination data (doesn't need to equal img.cols)
    glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());


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
    QPointF topLeft(width()/2 - imSize.width()/2, height()/2 - imSize.height()/2);
    glReadPixels(topLeft.x(), topLeft.y(), imSize.width(), imSize.height(), inputColourFormat, GL_UNSIGNED_BYTE, img.data);
    cv::cvtColor(img, img, CV_BGR2RGB);
    cv::flip(img, img, 0);

    return img;
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
