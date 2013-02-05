#include <QPainter>
#include <QPaintEngine>
#include <GL/glu.h>

#include "GLScene.h"

GLScene::GLScene(QObject *parent) :
    QGraphicsScene(parent)
{
}

GLScene:: ~GLScene()
{
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
