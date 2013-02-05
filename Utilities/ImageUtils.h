#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define DATA_DIR "Data/"
#define IMAGE_EXT ".tiff"

std::string imageLocationWithID(std::string imageID);
cv::Mat loadImage(std::string fname);
cv::Mat loadImageWithID(std::string imageID);
void displayImageWithID(std::string imageID);

#endif // IMAGEUTILS_H
