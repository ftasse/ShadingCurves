#include "ImageUtils.h"


std::string imageLocationWithID(std::string imageID)
{
    std::string location = DATA_DIR + imageID;
    if (imageID.find('.') == std::string::npos)
    {
        location += IMAGE_EXT;
    }
    return location;
}

cv::Mat loadImage(std::string fname)
{
    cv::Mat image = cv::imread(fname);
    if (image.cols == 0)
    {
        qDebug("Could not load image: %s", fname.c_str());
    }
    return image;
}

cv::Mat loadImageWithID(std::string imageID)
{
    std::string location = imageLocationWithID(imageID);
    return loadImage(location);
}

void displayImageWithID(std::string imageID)
{
    cv::Mat image = loadImageWithID(imageID);
    std::string windowID = imageLocationWithID(imageID);

    cv::namedWindow(windowID, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
    cv::imshow( windowID, image );
    while( 1 ) {
        if( cv::waitKey( ) == 27 ) break;
    }
    cv::destroyWindow( windowID );
}
