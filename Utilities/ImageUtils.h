#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define DATA_DIR "../imageshading/Data/"
#define IMAGE_EXT ".tiff"

std::string imageLocationWithID(std::string imageID);
cv::Mat loadImage(std::string fname);
cv::Mat loadImageWithID(std::string imageID);
void displayImageWithID(std::string imageID);

cv::Mat qimage2mat(const QImage& qimage);

//The mask has the same size as the image
void customFloodFill(cv::Mat &img, cv::Mat &mask, bool **marked, cv::Scalar color, cv::Point2i seed);

#endif // IMAGEUTILS_H
