#ifndef GLSCENE_H
#define GLSCENE_H

#include <QGraphicsScene>
#include "Utilities/ImageUtils.h"

class GLScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GLScene(QObject *parent = 0);
    virtual ~GLScene();

    //Drawing functions
    void drawImage(cv::Mat& image);
    void drawControlPoint();
    void drawCurve();

    //IO functions
    bool openImage(std::string fname);
    void saveImage(std::string fname);
    bool openCurve(std::string fname);
    void saveCurve(std::string fname);

    cv::Mat& currentImage()
    {
        return m_curImage;
    }
    
protected:
    void drawBackground(QPainter *painter, const QRectF &rect);

signals:
    
public slots:

private:
    cv::Mat m_curImage;
    
};

#endif // GLSCENE_H
