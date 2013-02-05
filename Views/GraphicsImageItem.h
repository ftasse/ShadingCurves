#ifndef QGRAPHICSIMAGEITEM_H
#define QGRAPHICSIMAGEITEM_H

#include <QGraphicsPixmapItem>
#include "Utilities/ImageUtils.h"

class GraphicsImageItem : public QGraphicsPixmapItem
{
public:
    GraphicsImageItem(QGraphicsItem * parent = 0);
    ~GraphicsImageItem();

private:
    cv::Mat *currentImage;

protected:
    //virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    //virtual QRectF boundingRect() const;

public:
    void updateImage();
    void loadImage(std::string imageLocation);
};

#endif // QGRAPHICSIMAGEITEM_H
