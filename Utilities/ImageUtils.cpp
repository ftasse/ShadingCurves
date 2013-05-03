#include "ImageUtils.h"
#include <queue>
#include <stack>


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

void customFloodFill(cv::Mat& img, cv::Mat& mask, bool **marked,  cv::Scalar color, cv::Point2i seed)
{
    std::vector<std::pair<int, int> > dirs(4);
    dirs[0] = std::pair<int, int>(0, 1);
    dirs[1] = std::pair<int, int>(0, -1);
    dirs[2] = std::pair<int, int>(1, 0);
    dirs[3] = std::pair<int, int>(-1, 0);

    std::queue<cv::Point2i> area;
    area.push(seed);
    marked[seed.y][seed.x] = true;
    for (int k=0; k<3; ++k)
        img.at<cv::Vec3b>(seed.y, seed.x)[k] = color[k];

    while (!area.empty())
    {
        cv::Point2i point = area.front();
        area.pop();

        for (int i=0; i<dirs.size(); ++i)
        {
            int m = dirs[i].first;
            int n = dirs[i].second;
            if (point.x+m>=0 && point.x+m<img.cols && point.y+n>=0 && point.y+n<img.rows)
            {
                if ( !marked[point.y+n][point.x+m] )
                {
                    marked[point.y+n][point.x+m] = true;
                    for (int k=0; k<3; ++k)
                        img.at<cv::Vec3b>(point.y+n, point.x+m)[k] = color[k];
                    if (mask.at<uchar>(point.y, point.x) < 128)
                    {
                        area.push(cv::Point2i(point.x+m, point.y+n));
                    }
                }
            }
        }
    }
}
