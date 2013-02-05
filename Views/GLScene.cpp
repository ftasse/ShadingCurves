#include <QPainter>
#include <QPaintEngine>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <GL/glu.h>
#include <algorithm>

#include "GLScene.h"

GLScene::GLScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_curBsplineIndex = -1;
}

GLScene:: ~GLScene()
{
}

void GLScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    int cptRef = registerPointAtScenePos(event->scenePos());
    if (cptRef < 0)
        return;

    if (m_curBsplineIndex < 0)
    {
        createBSpline();
        m_curBsplineIndex = m_curves.size() - 1;
    }

    BSpline& bspline = m_curves[m_curBsplineIndex];
    std::vector<int>& cptRefs = bspline.controlPointsRefs();

    if (std::find(cptRefs.begin(), cptRefs.end(), cptRef) == cptRefs.end())
    {
        bspline.controlPointsRefs().push_back(cptRef);
        update();
    }
}

void  GLScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (painter->paintEngine()->type() != QPaintEngine::OpenGL) {
        qWarning("GLScene: drawBackground needs a "
                 "QGLWidget to be set as viewport on the "
                 "graphics view");
        return;
    }

    glClearColor(1.0, 1.0, 1.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1.0, 1.0, 0, -1.0, 1.0);

    drawImage(m_curImage);
}

void GLScene::drawImage(cv::Mat& image)
{
    glColor3d(1.0, 1.0, 1.0);
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

    GLenum inputColourFormat = GL_BGR;
    if (image.channels() == 1)  inputColourFormat = GL_LUMINANCE;

    glTexImage2D(GL_TEXTURE_2D, 0, 3,image.cols, image.rows, 0, inputColourFormat, GL_UNSIGNED_BYTE, image.data);

    //glBindTexture(GL_TEXTURE_2D, texture[index]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();

    glDeleteTextures(1, &texId);
}

void GLScene::createBSpline()
{
    BSpline bspline;
    bspline.cptsPtr = &m_cpts;
    m_curves.push_back(bspline);
}

int GLScene::registerPointAtScenePos(QPointF scenePos)
{
    qDebug("Selected: %.2f %.2f", scenePos.x(), scenePos.y());

    /*TODO Flora
        Need to transform these coordinates so that they are indespedent of the scene size
        Need to check if the point is already added
     */

    QGraphicsRectItem pointItem;
    pointItem.setRect(scenePos.x() - 5, scenePos.y() - 5, 10, 10);
    pointItem.setVisible(true);
    pointItem.setBrush(QBrush(Qt::black));
    addItem(&pointItem);
    pointItem.setPos(scenePos);

    ControlPoint cpt(scenePos.x(), scenePos.y());
    m_cpts.push_back(cpt);
    return m_cpts.size() - 1;
}

bool GLScene::openImage(std::string fname)
{
    cv::Mat image = loadImage(fname);

    //Test if image was loaded
    if (image.cols > 0)
    {
        m_curImage = image;
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

bool GLScene::openCurve(std::string fname)
{
    //TODO
    return false;
}

void GLScene::saveCurve(std::string fname)
{
    //TODO
}
