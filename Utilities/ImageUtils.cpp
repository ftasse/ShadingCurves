#include "ImageUtils.h"
#include <queue>


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

void customFloodFill(cv::Mat& img, cv::Mat& mask, cv::Scalar color, cv::Point2i seed)
{
    std::queue<cv::Point2i> area;
    std::vector<cv::Point2i> masked;
    area.push(seed);
    for (int k=0; k<3; ++k)
        img.at<cv::Vec3b>(seed.y, seed.x)[k] = color[k];

    while (!area.empty())
    {
        cv::Point2i point = area.front();
        area.pop();

        for (int m=-1; m<=1; ++m)
            for (int n=-1; n<=1; ++n)
                if (((m!=0 || n!=0)&&(m==0 || n==0)) && point.x+m>=0 && point.x+m<img.cols && point.y+n>=0 && point.y+n<img.rows)
                {
                    cv::Vec3b cur = img.at<cv::Vec3b> (point.y+n, point.x+m);
                    if ((cur[0]!=color[0] || cur[1]!=color[1] || cur[2]!=color[2]) )
                    {
                        if (mask.at<uchar>(point.y, point.x) < 128)
                        {
                            area.push(cv::Point2i(point.x+m, point.y+n));
                            for (int k=0; k<3; ++k)
                                img.at<cv::Vec3b>(point.y+n, point.x+m)[k] = color[k];
                        } else
                        {
                            //(mask.at<uchar>(point.y+n, point.x+m) > 128)
                            masked.push_back(cv::Point2i(point.x+m, point.y+n));
                        }
                    }
                }
    }

    for (int i=0; i<masked.size(); ++i)
    {
        for (int k=0; k<3; ++k)
            img.at<cv::Vec3b>(masked[i].y, masked[i].x)[k] = color[k];
    }
}
