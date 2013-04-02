#include "../Views/glew/GL/glew.h"
//#include <QtGui>
//#include <QtOpenGL>
#include "glviewsubd.h"
#include <assert.h>
//#include <GL/glu.h>
#include "tostring.h"

using namespace std;

GLviewsubd::GLviewsubd(GLuint iW, GLuint iH, cv::Mat *timg, QWidget *parent, QGLWidget *shareWidget) : GLviewport(parent, shareWidget)
{
    setAcceptDrops(true);

//    meshCtrl.clear();
//	meshCurr.clear();
//	meshSubd.clear();
//    meshOld.clear();

	mesh_enabled = true;
    flat_mesh_enabled = false;
    smooth_mesh_enabled = false;
	edged_mesh_enabled = false;
	culled_mesh_enabled = false;
	curvM_mesh_enabled = false;
	curvG_mesh_enabled = false;
	curvTG_mesh_enabled = false;
    height_mesh_enabled = true;
    feature_lines_enabled = false;
	IP_mesh_enabled = false;
    triang_enabled = false;
	triang2_enabled = false;
    ctrl_enabled = false;
    old_enabled = false;
    frame_enabled = false;
	probeOnCtrl = true;
    transf = true;
	clear = true;

    Lab_enabled = false;

    writeImg = true;
    showImg = true;

    subType = CC;

	verInd = -1;
	verIndSub = -1;

	lastChangeX = 0;
	lastChangeY = 0;
	lastChangeZ = 0;

	smoothCoef = 0;

	indexMesh = -1;

	curvRatio1 = 0.0;
	curvRatio2 = 0.0;

    stripeDensityLevel = 3;
    lapSmValue = 10;

    imageWidth = iW;
    imageHeight = iH;

//    this->setFixedSize(iW, iH);

    numberPaintCalls = 0;

    offScreen = false;
    offMainWindow = false;

    inputImg = timg;

    super = 1; //supersampling (1, 2, or 4)

    clipping = true;
    clipMax = 100;
    clipMin = 0;
    shade = CVLAB;
    blackOut = false;

    flatImage = true;
    clrVsTxtr = true;

    subdivTime = 0;

//    //COLOUR CONVERSION TESTS
//    double L, a, b, R, G, B, Y, x, y;

//    RGB2LAB(1,1,1,L,a,b);
//    cout << "1 1 1 RGB2LAB = " << L << " " << a << " " << b << endl;
//    RGB2LAB(0,0,0,L,a,b);
//    cout << "0 0 0 RGB2LAB = " << L << " " << a << " " << b << endl;
//    RGB2LAB(1,0,0,L,a,b);
//    cout << "1 0 0 RGB2LAB = " << L << " " << a << " " << b << endl;
//    RGB2LAB(0,1,0,L,a,b);
//    cout << "0 1 0 RGB2LAB = " << L << " " << a << " " << b << endl;
//    RGB2LAB(0,0,1,L,a,b);
//    cout << "0 0 1 RGB2LAB = " << L << " " << a << " " << b << endl;

//    matlabRGB2LAB(1,1,1,L,a,b);
//    cout << "1 1 1 RGB2LAB = " << L << " " << a << " " << b << endl;
//    matlabRGB2LAB(0,0,0,L,a,b);
//    cout << "0 0 0 RGB2LAB = " << L << " " << a << " " << b << endl;
//    matlabRGB2LAB(1,0,0,L,a,b);
//    cout << "1 0 0 RGB2LAB = " << L << " " << a << " " << b << endl;
//    matlabRGB2LAB(0,1,0,L,a,b);
//    cout << "0 1 0 RGB2LAB = " << L << " " << a << " " << b << endl;
//    matlabRGB2LAB(0,0,1,L,a,b);
//    cout << "0 0 1 RGB2LAB = " << L << " " << a << " " << b << endl;

//    LAB2RGB(0,0,0,R,G,B);
//    cout << "0 0 0 LAB2RGB = " << R << " " << G << " " << B << endl;

//    LAB2RGB(32.3026,79.1967,-113.364,R,G,B);
//    cout << "32.3026 79.1967 -113.364 LAB2RGB = " << R << " " << G << " " << B << endl;
//    LAB2RGB(52.3026,79.1967,-113.364,R,G,B);
//    cout << "52.3026 79.1967 -113.364 LAB2RGB = " << R << " " << G << " " << B << endl;
//    LAB2RGB(100,79.1967,-113.364,R,G,B);
//    cout << "100 79.1967 -113.364 LAB2RGB = " << R << " " << G << " " << B << endl;

//    matlabLAB2RGB(0,0,0,R,G,B);
//    cout << "0 0 0 LAB2RGB = " << R << " " << G << " " << B << endl;

//    matlabLAB2RGB(32.3026,79.1967,-113.364,R,G,B);
//    cout << "32.3026 79.1967 -113.364 LAB2RGB = " << R << " " << G << " " << B << endl;
//    matlabLAB2RGB(52.3026,79.1967,-113.364,R,G,B);
//    cout << "52.3026 79.1967 -113.364 LAB2RGB = " << R << " " << G << " " << B << endl;
//    matlabLAB2RGB(100,79.1967,-113.364,R,G,B);
//    cout << "100 79.1967 -113.364 LAB2RGB = " << R << " " << G << " " << B << endl;

//    RGB2YXY(1,1,1,L,a,b);
//    cout << "1 1 1 RGB2Yxy = " << L << " " << a << " " << b << endl;

//    R = 0;
//    G = 0;
//    B = 0;
//    RGB2YXY(R,G,B,Y,x,y);
//    cout << R << " " << G << " " << B << " " << "RGB2Yxy = " << Y << " " << x << " " << y << endl;
//    YXY2RGB(Y,x,y,R,G,B);
//    cout << Y << " " << x << " " << y << " " << "Yxy2RGB = " << R << " " << G << " " << B << endl;

//    R = 0;
//    G = 0;
//    B = 0;
//    RGB2YXY(R,G,B,Y,x,y);
//    cout << R << " " << G << " " << B << " " << "RGB2Yxy = " << Y << " " << x << " " << y << endl;
//    Y += 0.1;
//    YXY2RGB(Y,x,y,R,G,B);
//    cout << Y << " " << x << " " << y << " " << "Yxy2RGB = " << R << " " << G << " " << B << endl;

//    R = 0;
//    G = 0;
//    B = 0;
//    RGB2YXY(R,G,B,Y,x,y);
//    cout << R << " " << G << " " << B << " " << "RGB2Yxy = " << Y << " " << x << " " << y << endl;
//    Y -= 0.1;
//    YXY2RGB(Y,x,y,R,G,B);
//    cout << Y << " " << x << " " << y << " " << "Yxy2RGB = " << R << " " << G << " " << B << endl;
}

GLviewsubd::~GLviewsubd()
{
    unsigned int i, j;

    for (i = 0 ; i < meshSubd.size() ; i++)
    {
//        meshCurr[i] = NULL;
        for (j = 1 ; j < meshSubd[i].size() ; j++)
        {
            delete meshSubd[i][j]; // TO DO - should clear more memory
            meshSubd[i][j] = NULL;
        }
    }
//    meshSubd.clear();

    for (i = 0 ; i < meshCtrl.size() ; i++)
    {
        delete meshOld[i];
//        meshOld[i] = NULL;
    }

//	if (clear)
//	{
        for (i = 0 ; i < meshCtrl.size() ; i++)
        {
            delete meshCtrl[i];
//            meshCtrl[i] = NULL;
        }
//		meshCtrl.clear();
//		meshCurr.clear();
//        meshOld.clear();
//		setRotZero();
//	}
}

void GLviewsubd::subdivide(void)
{
//    if (meshCurr.size() > 0 && meshCurr[0]->my_level < 6)
//    {
//        emit subdivLevelChanged(meshCurr[0]->my_level + 1);
//    }
}

void GLviewsubd::setSubdivLevel(int newLevel)
{
	unsigned int i, j, oldLevel;
    unsigned int newL = newLevel;
    bool        showMessageCF, showMessageCCint;

    QElapsedTimer timer;
    timer.start();

    showMessageCF = false;
    showMessageCCint = false;

    if (meshCurr.size() > 0 && meshCurr[0]->my_level != newL)
	{
        oldLevel = meshCurr[0]->my_level;
//cout << "setSubdivLevel with old level " << oldLevel << " new level " << newLevel << endl;

        if ((meshCurr[0]->my_level > newL) || (meshSubd[0].size() > newL))
		{
			// just call the right mesh from the vector of meshes
			for (j = 0 ; j < meshCurr.size() ; j++)
			{
				meshCurr[j] = meshSubd[j][newLevel];
			}
		}
		else // if (meshCurr->my_level < newLevel)
		{
			for (i = 0 ; i < newL - oldLevel; i++)
			{
				for (j = 0 ; j < meshCurr.size() ; j++)
				{
					nextMesh = new Mesh();
                    meshCurr[j]->CatmullClark(nextMesh);

                    if (!showMessageCF && !showMessageCCint)
                    {
                        meshSubd[j].push_back(nextMesh);
                        meshCurr[j] = nextMesh;
                    }
				}
			}
		}

//        emit subdivLevelChanged(newLevel);
//        cout << "subdivide End " << endl;
	}
    subdivTime = timer.elapsed();
    qDebug() << "3D: Subdivision: " << subdivTime << "ms";

	updateAll();
}

void GLviewsubd::setClr(int newColor)
{

    cout << "setColor() " << newColor << endl;

	if (clr != newColor)
	{
		clr = newColor;
		buildCurvGMesh();
        buildHeightMesh();
		updateGL();
	}
}

void GLviewsubd::setIndMesh(int newInd)
{
	if (indexMesh != newInd - 1)
	{
		indexMesh = newInd - 1;

		verInd = -2;
		verIndSub = -2;

		lastChangeX = 0;
		lastChangeY = 0;
		lastChangeZ = 0;

		emit poiChanged();
		emit indexChanged(verInd + 1); // shifted by 1 because of 'Select...'

        buildAll();

        focusView();
	}
}

void GLviewsubd::buildAll(void)
{
    if (!offScreen)
    {
        if (mesh_enabled)
        {
            if (flat_mesh_enabled)
            {
                buildFlatMesh();
            }
            else if (smooth_mesh_enabled)
            {
                buildSmoothMesh();
            }
            else if (edged_mesh_enabled)
            {
                buildEdgedMesh();
            }
            else if (culled_mesh_enabled)
            {
                buildCulledMesh();
            }
            else if (curvG_mesh_enabled)
            {
                buildCurvGMesh();
            }
            else if (height_mesh_enabled)
            {
                buildHeightMesh();
            }
            else if (IP_mesh_enabled)
            {
                buildIPMesh();
            }
        }

        if (feature_lines_enabled)
        {
            buildFeatureLines();
        }

        if (ctrl_enabled)
        {
            buildCtrl();
            buildPoi();
            buildPoiSub();
            buildPoiBound();
        }

        if (old_enabled)
        {
            buildOld();
        }

        if (frame_enabled)
        {
            buildFrame();
        }

        if (Lab_enabled)
        {
            buildLab();
        }
    }
}

void GLviewsubd::initializeGL(void)
{
    QElapsedTimer timer;
    timer.start();

    static const int res = 1023;
    PointPrec		col[3];
    GLfloat			texture[5][res][4], alpha;

    alpha = 0.0;

//    if (joinTheDarkSide)
//    {
//        glClearColor(0, 0, 0, 0);
//    }
//    else
//    {
//        glClearColor(1, 1, 1, 0);
//    }

    glClearColor(col_back[0], col_back[1], col_back[2], col_back[3]);

    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
//    glDepthMask(GL_FALSE);
//    glEnable(GL_CULL_FACE);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse1);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  lightDiffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
    glEnable (GL_LIGHT1);
    glLightfv(GL_LIGHT2, GL_POSITION, lightPosition2);
    glLightfv(GL_LIGHT2, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE,  lightDiffuse1);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecular);
//    glEnable(GL_LIGHT2);

    glLoadIdentity();
//    glOrtho(0, imageWidth, imageHeight, 0, -1000.0, 1000.0);

//    swapBuffers();

    glGenTextures(7, textID); // 7th for image texture

    for (int j = 0; j < 5; ++j) // number of colour systems
    {
        for (int i = 0; i < res; ++i)
        {
            genColor(j, (float)i / res - 0.5, -0.5, 0.5, col);

            texture[j][i][0] = col[0];
            texture[j][i][1] = col[1];
            texture[j][i][2] = col[2];
            texture[j][i][3] = alpha;
        }

        glBindTexture(GL_TEXTURE_1D, textID[j]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glDisable(GL_TEXTURE_GEN_S);

        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, res,
                     0, GL_RGBA, GL_FLOAT, texture[j]);
    }

    //image texture
    glBindTexture(GL_TEXTURE_2D, textID[6]);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S , GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight,
                 0, GL_BGR, GL_UNSIGNED_BYTE, inputImg->data);

    qDebug() << "3D: GL initialisation: " << timer.elapsed() << "ms";

}

void GLviewsubd::paintGL(void)
{

    if (offScreen)
    {
        QElapsedTimer timer;
        timer.start();

        double      tmp, val, valMin, valMax, half;

//        half = 0.501960813999176; // grey level with lum difference zero
        half = 0.5;

        valMin = 0.4999; // if grey value (lum. dif. ~ 0) is in this interval, do not apply luminance difference
        valMax = 0.5001;

//        numberPaintCalls++;
//        cout << "PaintGL OFFscreen called: " << numberPaintCalls << endl;

        int origClr, origView[4];

        makeCurrent();

        //Setup for offscreen drawing if fbos are supported
        GLuint framebuffer, renderbuffer, depthbuffer;
        GLenum status;

        glGenFramebuffersEXT(1, &framebuffer);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
        glGenRenderbuffersEXT(1, &renderbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB16F, super*imageWidth, super*imageHeight); // GL_RGB32F does not work (on Z930) for high resolutions
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                         GL_RENDERBUFFER_EXT, renderbuffer);
        //depth buffer
        glGenRenderbuffersEXT(1, &depthbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, super*imageWidth, super*imageHeight);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                         GL_RENDERBUFFER_EXT, depthbuffer);

        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
            qDebug("Could not draw offscreen");

        //Setup view
        glClearColor(half, half, half, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        glGetIntegerv(GL_VIEWPORT, origView);
        glViewport(0, 0, super*imageWidth, super*imageHeight);

        glOrtho(0, super*imageWidth, 0, super*imageHeight, -1000.0, 1000.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glRenderMode(GL_RENDER);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        //draw stuff here
        origClr = clr;
        clr = 3;
        buildHeightMesh();
        glCallList(height_mesh_list);
        clr = origClr;

        //create image
        img.create(super*imageHeight, super*imageWidth, CV_32FC3);
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
        glReadPixels(0, 0, super*imageWidth, super*imageHeight, inputColourFormat, GL_FLOAT, img.data);

//        tmp = img.at<cv::Vec3f>(0,0)[0]; // for an empty image, use this value as `zero'
//        cout << "Zero level: " << tmp << endl;

        if (flatImage)
        {
            glClearColor(1.0, 1.0, 1.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //draw stuff here
            buildFrame();
            glCallList(frame_list);
            buildFlatMesh();
            glCallList(flat_mesh_list);
            //create image
            imgFill.create(super*imageHeight, super*imageWidth, CV_32FC3);
            glReadPixels(0, 0, super*imageWidth, super*imageHeight, inputColourFormat, GL_FLOAT, imgFill.data);
        }

        //Clean up offscreen drawing
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glViewport(0, 0, origView[2], origView[3]);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glDeleteRenderbuffersEXT(1, &renderbuffer);
        glClearColor(col_back[0], col_back[1], col_back[2], col_back[3]);
        offScreen = false;

        qDebug() << "3D: OpenGL rendering: " << timer.elapsed() << "ms";
        timer.restart();

//        cout << imgFill.at<cv::Vec3f>(0,0)[0] << " " << imgFill.at<cv::Vec3f>(0,0)[1] <<" " << imgFill.at<cv::Vec3f>(0,0)[2] << endl;

        //process images
//        cv::cvtColor(imgFill, imgFill, CV_BGR2RGB);

//        cv::imshow("Img: Lum Dif", img);
//        cv::imshow("Img: Filled", imgFill);


        cv::flip(img, img, 0); // img is grey with surface height

        if (flatImage)
        {
            cv::flip(imgFill, imgFill, 0); // imgFill is top view of image with flat surfaces on top
        }
        else
        {
            inputImg->copyTo(imgFill); // set imgFill to background image
            imgFill.convertTo(imgFill, CV_32FC3);
            imgFill *= 1.0 / 255.0;
        }


//        if (showImg)
//        {
//            cv::imshow("Img: Original", imgShaded);
//            cv::imshow("Img: Fill " + to_string(super), imgFill);
//            cv::imshow("Img: LumDif " + to_string(super), img);
//        }

//        if (writeImg)
//        {
//            cv::imwrite("ImgOrig.tif", imgShaded);

//            //need to convert these to 8bit images first!
////            cv::imwrite("ImgLumDif" + QString::number(super) + "x" + QString::number(super), img);
////            cv::imwrite("ImgFill" + QString::number(super) + "x" + QString::number(super), imgFill);
//        }
//        imgShaded.convertTo(imgShaded, CV_32FC3);

        cv::Mat imgPyrDown, imgPyrDown2, imgFillPyrDown, imgFillPyrDown2;

        switch (super)
        {
            case 1:
                break;
            case 2:
                cv::pyrDown(img, imgPyrDown, cv::Size(imageWidth, imageHeight));
                img = imgPyrDown;
                if (flatImage)
                {
                    cv::pyrDown(imgFill, imgFillPyrDown, cv::Size(imageWidth, imageHeight));
                    imgFill = imgFillPyrDown;
                }
                break;
            case 4:
                cv::pyrDown(img, imgPyrDown, cv::Size(2*imageWidth, 2*imageHeight));
                cv::pyrDown(imgPyrDown, imgPyrDown2, cv::Size(imageWidth, imageHeight));
                img = imgPyrDown2;
                if (flatImage)
                {
                    cv::pyrDown(imgFill, imgFillPyrDown, cv::Size(2*imageWidth, 2*imageHeight));
                    cv::pyrDown(imgFillPyrDown, imgFillPyrDown2, cv::Size(imageWidth, imageHeight));
                    imgFill = imgFillPyrDown2;
                }
                break;
        }

        if (img.cols > 0)
        {
            imgFill.copyTo(imgFillShaded);

            qDebug() << "3D: IP: Prepare images: " << timer.elapsed() << "ms";
            timer.restart();

            if (shade == OWN)
            {
                // apply luminance adjustment
                double  rr, gg, bb, L, a, b;
                int x, y;
                #pragma omp parallel for default(none) private(x, y, val, L, a, b, bb, gg, rr, tmp) shared(valMin, valMax, half)
                for( y = 0; y < imgFillShaded.rows; y++ )
                {
                    for( x = 0; x < imgFillShaded.cols; x++ )
                    {
                        val = img.at<cv::Vec3f>(y,x)[0];

                        if (val < valMin || val > valMax)
                        {
                            RGB2LAB(imgFillShaded.at<cv::Vec3f>(y,x)[2],
                                    imgFillShaded.at<cv::Vec3f>(y,x)[1],
                                    imgFillShaded.at<cv::Vec3f>(y,x)[0],
                                    L, a, b);

                            tmp = L + 200*(val - half); //0.50196081399917603

                            if (clipping)
                            {
                                if (tmp > clipMax)
                                {
                                    tmp = clipMax;
                                }
                                else if (tmp < clipMin)
                                {
                                    tmp = clipMin;
                                }
                            }
        //                    imgFillShaded.at<cv::Vec3f>(y,x)[0] = tmp;

                            //convert manually back to BGR

                            LAB2RGB(tmp, a, b, rr, gg, bb);
                            // seems to give results similar to Henrik' Matlab code
    //                         lab2rgb<double,double>(tmp, a, b, rr, gg, bb);
                            // results are a bit different
    //                         lab2rgbVer2(tmp, a, b, rr, gg, bb);

                            //black out out-of-range pixels
                            if (blackOut)
                            {
                                if (bb < 0 || bb > 1 || gg < 0 || gg > 1 || rr < 0 || rr > 1)
                                {
                                    bb = 0;
                                    gg = 0;
                                    rr = 0;
                                }
                            }

                            imgFillShaded.at<cv::Vec3f>(y,x)[0] = bb;
                            imgFillShaded.at<cv::Vec3f>(y,x)[1] = gg;
                            imgFillShaded.at<cv::Vec3f>(y,x)[2] = rr;
                        }
                    }
                }
            }
            else if (shade == CVLAB) // use openCV Lab
            {
                cv::cvtColor(imgFillShaded, imgFillShaded, CV_BGR2Lab);

                // apply luminance adjustment
                int x, y;
                #pragma omp parallel for default(none) private(x, y, val, tmp) shared(valMin, valMax, half)
                for( y = 0; y < imgFillShaded.rows; y++ )
                {
                    for( x = 0; x < imgFillShaded.cols; x++ )
                    {
                        tmp = imgFillShaded.at<cv::Vec3f>(y,x)[0] + 200*(img.at<cv::Vec3f>(y,x)[0] - half); //0.50196081399917603, 0.498039215803146

                        if (clipping)
                        {
                            if (tmp > clipMax)
                            {
                                tmp = clipMax;
                            }
                            else if (tmp < clipMin)
                            {
                                tmp = clipMin;
                            }
                        }
                        imgFillShaded.at<cv::Vec3f>(y,x)[0] = tmp;
                    }
                }
                //convert back to BGR
                cv::cvtColor(imgFillShaded, imgFillShaded, CV_Lab2BGR);
//                cv::cvtColor(imgFillShaded, imgFillShaded, CV_BGR2RGB); // why is this necessary here???
            }
            else if (shade == CVHLS) // OpenCV HLS
            {
                cv::cvtColor(imgFillShaded, imgFillShaded, CV_BGR2HLS);
                clipMin = 0;
                clipMax = 1;

                // apply luminance adjustment
                int x, y;
                #pragma omp parallel for default(none) private(x, y, val, tmp) shared(valMin, valMax, half)
                for( y = 0; y < imgFillShaded.rows; y++ )
                {
                    for( x = 0; x < imgFillShaded.cols; x++ )
                    {
                        tmp = imgFillShaded.at<cv::Vec3f>(y,x)[1] + 2*(img.at<cv::Vec3f>(y,x)[0] - half); //0.50196081399917603

                        if (clipping)
                        {
                            if (tmp > clipMax)
                            {
                                tmp = clipMax;
                            }
                            else if (tmp < clipMin)
                            {
                                tmp = clipMin;
                            }
                        }
                        imgFillShaded.at<cv::Vec3f>(y,x)[1] = tmp;
                    }
                }

                //convert back to BGR
                cv::cvtColor(imgFillShaded, imgFillShaded, CV_HLS2BGR);
            }
            else if (shade == YXY) // Yxy
            {
                clipMin = 0;
                clipMax = 1;

                // apply luminance adjustment
                double  rr, gg, bb, L, a, b;
                int x, y;

                #pragma omp parallel for default(none) private(x, y, val, L, a, b, bb, gg, rr, tmp) shared(valMin, valMax, half)
                for( y = 0; y < imgFillShaded.rows; y++ )
                {
                    //paralellising the inner loop gives slower results
                    //#pragma omp parallel for default(none) private(x, val, L, a, b, bb, gg, rr, tmp) shared(y, valMin, valMax)
                    for( x = 0; x < imgFillShaded.cols; x++ )
                    {
                        val = img.at<cv::Vec3f>(y,x)[0];

                        if (val < valMin || val > valMax)
                        {
                            RGB2YXY(imgFillShaded.at<cv::Vec3f>(y,x)[2],
                                    imgFillShaded.at<cv::Vec3f>(y,x)[1],
                                    imgFillShaded.at<cv::Vec3f>(y,x)[0],
                                    L, a, b);

//                            tmp = L + 2*(img.at<cv::Vec3f>(y,x)[0] - 0.5); //0.50196081399917603

                            // L in [0,1], tmp should lie in [0,log10(2)]
                            tmp = log10(L + 1) + log10(2)*2*(val - half);

                            // clip
                            if (clipping)
                            {
                                if (tmp > log10(2))
                                {
                                    tmp = log10(2);
                                }
                                else if (tmp < 0)
                                {
                                    tmp = 0;
                                }
                            }

                            //convert back tmp
                            tmp = pow(10.0, tmp) - 1;

                            //convert manually back to BGR
                            YXY2RGB(tmp, a, b, rr, gg, bb);

                            //black out already in YXY2RGB fucntion

                            imgFillShaded.at<cv::Vec3f>(y,x)[0] = bb;
                            imgFillShaded.at<cv::Vec3f>(y,x)[1] = gg;
                            imgFillShaded.at<cv::Vec3f>(y,x)[2] = rr;
                        }
                    }
                }
            }
            else if (shade == RGB)
            {
                // apply luminance adjustment
                double  R, G, B;
                int x, y;

                #pragma omp parallel for default(none) private(x, y, val, R, G, B, tmp) shared(valMin, valMax, half)
                for( y = 0; y < imgFillShaded.rows; y++ )
                {
                    //paralellising the inner loop gives slower results
                    //#pragma omp parallel for default(none) private(x, val, L, a, b, bb, gg, rr, tmp) shared(y, valMin, valMax)
                    for( x = 0; x < imgFillShaded.cols; x++ )
                    {
                        val = img.at<cv::Vec3f>(y,x)[0];

                        if (val < valMin || val > valMax)
                        {
                            B = imgFillShaded.at<cv::Vec3f>(y,x)[0];
                            G = imgFillShaded.at<cv::Vec3f>(y,x)[1];
                            R = imgFillShaded.at<cv::Vec3f>(y,x)[2];
                            if (val < valMin)
                            {
                                //make darker
                                tmp = 2 * val;
                                B *= tmp;
                                G *= tmp;
                                R *= tmp;
                            }
                            else
                            {
                                //make brighter
                                tmp = 2 * (1 - val);
                                B = 1 - B;
                                G = 1 - G;
                                R = 1 - R;
                                B *= tmp;
                                G *= tmp;
                                R *= tmp;
                                B = 1 - B;
                                G = 1 - G;
                                R = 1 - R;
                            }
                            imgFillShaded.at<cv::Vec3f>(y,x)[0] = B;
                            imgFillShaded.at<cv::Vec3f>(y,x)[1] = G;
                            imgFillShaded.at<cv::Vec3f>(y,x)[2] = R;
                        }
                    }
                }
            }
            else // matlab
            {
                // apply luminance adjustment
                double  rr, gg, bb, L, a, b;
                int x, y;

                #pragma omp parallel for default(none) private(x, y, val, L, a, b, bb, gg, rr, tmp) shared(valMin, valMax, half)
                for( y = 0; y < imgFillShaded.rows; y++ )
                {
                    //paralellising the inner loop gives slower results
                    //#pragma omp parallel for default(none) private(x, val, L, a, b, bb, gg, rr, tmp) shared(y, valMin, valMax)
                    for( x = 0; x < imgFillShaded.cols; x++ )
                    {
                        val = img.at<cv::Vec3f>(y,x)[0];

                        if (val < valMin || val > valMax)
                        {
                            matlabRGB2LAB(imgFillShaded.at<cv::Vec3f>(y,x)[2],
                                          imgFillShaded.at<cv::Vec3f>(y,x)[1],
                                          imgFillShaded.at<cv::Vec3f>(y,x)[0],
                                          L, a, b);

                            tmp = L + 200*(val - half); //0.50196081399917603

                            if (clipping)
                            {
                                if (tmp > clipMax)
                                {
                                    tmp = clipMax;
                                }
                                else if (tmp < clipMin)
                                {
                                    tmp = clipMin;
                                }
                            }
        //                    imgFillShaded.at<cv::Vec3f>(y,x)[0] = tmp;

                            //convert manually back to BGR
                            matlabLAB2RGB(tmp, a, b, rr, gg, bb);

                            //black out already in matlabLAB2RGB fucntion

                            imgFillShaded.at<cv::Vec3f>(y,x)[0] = bb;
                            imgFillShaded.at<cv::Vec3f>(y,x)[1] = gg;
                            imgFillShaded.at<cv::Vec3f>(y,x)[2] = rr;
                        }
                    }
                }
            }

            qDebug() << "3D: IP: Change luminance: " << timer.elapsed() << "ms";
            timer.restart();

//            cv::cvtColor(imgFillShaded, imgFillShaded, CV_BGR2RGB);

            //show always
            if (showImg)
            {
                cv::imshow("Img: Lum Dif", img);
                cv::imshow("Img: Filled and shaded result", imgFillShaded);
            }

            img *= 255;
            img.convertTo(img, CV_8UC3);
            imgFill *= 255;
            imgFill.convertTo(imgFill, CV_8UC3);
//            imgShaded *= 255;
//            imgShaded.convertTo(imgShaded, CV_8UC3);
            imgFillShaded *= 255;
            imgFillShaded.convertTo(imgFillShaded, CV_8UC3);

            if (writeImg)
            {
                cv::imwrite("ImgLumDifPyrDown" + to_string(super) + ".tif", img);
//                cv::cvtColor(imgFill, imgFill, CV_BGR2RGB);
                cv::imwrite("ImgFillPyrDown" + to_string(super) + ".tif", imgFill);
//                cv::imwrite("ImgResult" + to_string(super) + ".tif", imgShaded);
                cv::imwrite("ImgFillResult" + to_string(super) + ".tif", imgFillShaded);
            }
        }
//        updateGL();

        qDebug() << "3D: IP: Show/write images: " << timer.elapsed() << "ms";
    }
    else
    {
        makeCurrent();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        // Pre-multiply by translation
        GLdouble currentMatrix[16] = {0};
        glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
        glLoadIdentity();
        glTranslated(0.0, 0.0, -stepBackAmount);
        glMultMatrixd(currentMatrix);
    //    glDisable(GL_STENCIL_TEST);

        if (frame_enabled)
        {
    //		glClear(GL_DEPTH_BUFFER_BIT);
            glCallList(frame_list);
        }

        if (old_enabled)
        {
            glCallList(old_list);
            glCallList(smooth_mesh_list);
        }
        else
        {

            if (mesh_enabled)
            {
                if (flat_mesh_enabled)
                {
        //			cout << "Calling flat_mesh_list" << endl;
                    glCallList(flat_mesh_list);
                }
                if (smooth_mesh_enabled)
                {
        //			cout << "Calling smooth_mesh_list" << endl;
                    glCallList(smooth_mesh_list);
                }
                else if (edged_mesh_enabled)
                {
                    glCallList(edged_mesh_list);
        //			cout << "Calling edged_mesh_list" << endl;
                }
                else if (culled_mesh_enabled)
                {
                    glCallList(culled_mesh_list);
        //			cout << "Calling culled_mesh_list" << endl;
                }
                else if (curvG_mesh_enabled)
                {
                    glCallList(curvG_mesh_list);
        //			cout << "Calling curvG_mesh_list" << endl;
                }
                else if (height_mesh_enabled)
                {
                    glCallList(height_mesh_list);
        //			cout << "Calling curvG_mesh_list" << endl;
                }
                else if (IP_mesh_enabled)
                {
                    glCallList(IP_mesh_list);
        //			cout << "Calling curvG_mesh_list" << endl;
                }
            }

            if (feature_lines_enabled)
            {
                glCallList(feature_lines_list);
            }

            if (ctrl_enabled)
            {
                if (edged_ctrl_enabled)
                {
        //			glClear(GL_DEPTH_BUFFER_BIT);
                }
        //		cout << "Calling ctrl_list" << endl;

    //            buildPoiBound();

                glCallList(ctrl_list);
                glCallList(poiBound_list);
            }

//            glClear(GL_DEPTH_BUFFER_BIT);
//            glCallList(poiSub_list);

            if (Lab_enabled)
            {
                glCallList(Lab_list);
            }
        }

        if (ctrl_enabled || feature_lines_enabled)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glCallList(poi_list);
            glCallList(poiSub_list);
        }

        glPopMatrix();
    }
}

void GLviewsubd::mousePressEvent(QMouseEvent *event)
{
	Point_3D 		poi;
	int 			x, y, mouseX, mouseY;
	unsigned int 	i;
	float			dst, minD;
	int				newInd;

	Mesh			*mesh;

	makeCurrent();
	minD = 1000000;


	if ((event->buttons() & Qt::LeftButton) && meshCurr.size() > 0 && indexMesh > -1)
	{
		if (probeOnCtrl)
		{
			mesh = meshCtrl[indexMesh];
		}
		else
		{
			mesh = meshCurr[indexMesh];
		}

		mouseX = mapFromGlobal(QCursor::pos()).x();
		mouseY = size().height() - mapFromGlobal(QCursor::pos()).y();

//		newInd = -1;

		for (i = 0 ; i < mesh->my_vertices.size() ; i++)
		{
			poi = worldToScreen(mesh->my_vertices[i].my_point);
			x = poi.getX();
            y = poi.getY();
            // HENRIK edit: convert to float
            dst = sqrt(float((mouseX - x) * (mouseX - x) + (mouseY - y) * (mouseY - y)));
			if (dst < minD)
			{
				minD = dst;
				newInd = i;
			}
		}

		if (probeOnCtrl)
		{
			verIndSub = -2;
			buildPoiSub();
			if (verInd == newInd)
			{
				verInd = -2;
			}
			else
			{
				verInd = newInd;
			}

			lastChangeX = 0;
			lastChangeY = 0;
			lastChangeZ = 0;

			emit poiChanged();
			emit indexChanged(verInd + 1); // shifted by 1 because of 'Select...'
		}
		else
		{
			verInd = -2;
			lastChangeX = 0;
			lastChangeY = 0;
			lastChangeZ = 0;
			emit poiChanged();
			emit indexChanged(verInd + 1); // shifted by 1 because of 'Select...'
			if (verIndSub == newInd)
			{
				verIndSub = -2;
			}
			else
			{
				verIndSub = newInd;
				mesh->my_vertices[verIndSub].coutV();
			}
			buildPoiSub();
			updateGL();
		}
	}
	else if (event->buttons() & Qt::MidButton)
	{
		emit middleClicked();
	}
	lastPos = event->pos();
}

void GLviewsubd::drawPoi()
{
	if (verInd > -1)
	{
		glDisable(GL_TEXTURE_1D);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_poi);
		glPointSize(15);
        glColor3fv(col_poi);
		glBegin(GL_POINTS);
        glVertex3fv(meshCtrl[indexMesh]->my_vertices[verInd].my_point.getCoords());
		glEnd();
		glPointSize(1);
	}
}

void GLviewsubd::drawPoiSub()
{
	if (verIndSub > -1)
	{
		glDisable(GL_TEXTURE_1D);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_poi);
		glPointSize(15);
        glColor3fv(col_poi);
		glBegin(GL_POINTS);
        glVertex3fv(meshCurr[indexMesh]->my_vertices[verIndSub].my_point.getCoords());
		glEnd();
		glPointSize(1);
	}
}

void GLviewsubd::drawPoiBound()
{
    Point_3D		poi1, poi2;
    unsigned int 	i, j, k;
	Mesh			*mesh;

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	for (j = 0 ; j < meshCtrl.size() ; j++)
	{
        for (k = 0 ; k < 2 ; k++)
        {
            if (k == 0)
            {
                mesh = meshCtrl[j];
            }
            else
            {
                mesh = meshCurr[j];
            }

//            if (mesh->my_NCdegree != 0 )
            {
                glDisable(GL_TEXTURE_1D);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_poi);
                if (k == 0)
                {
                    glColor3fv(col_poi);
                    glPointSize(6);
                }
                else
                {
                    glColor3fv(col_poi);
                    glPointSize(-1); // or set to 10 to make it visible
                }
                glBegin(GL_POINTS);
                for (i = 0 ; i < mesh->my_numV ; i++)
                {
//                    if (mesh->my_vertices[i].isOnBoundary ||
//                        mesh->my_vertices[i].my_valency != 4)
//                    if (mesh->my_vertices[i].isOnBoundary)
                    if (!mesh->my_vertices[i].isOnBoundary &&
                        mesh->my_vertices[i].my_valency <= 2) // picks up interior valency <=2 vertices
                    {
                        glNormal3fv(mesh->my_vertices[i].my_normalFlat);
                        glVertex3fv(mesh->my_vertices[i].my_point.getCoords());
                    }
                }
                glEnd();
                glPointSize(1);
            }
        }
	}
    glDisable(GL_POINT_SMOOTH);
}

void GLviewsubd::drawFrame()
{
    int     z;
//    Point_3D    p0, p1, p2, p3;

//    glDisable(GL_TEXTURE_1D);

//    if (indexMesh > -1)
//    {
//        p0 = meshCtrl[indexMesh]->my_centre;
//        p1 = p0;
//        p2 = p0;
//        p3 = p0;
//        p1.setX(p1.getX()  + 0.1 / meshCtrl[0]->my_scale);
//        p2.setY(p1.getY()  + 0.1 / meshCtrl[0]->my_scale);
//        p3.setZ(p1.getZ()  + 0.1 / meshCtrl[0]->my_scale);
//    }
//    else
//    {
//        p0 = Point_3D(0.5 * (double)imageWidth, 0.5 * (double)imageHeight, 0);
//        p1 = p0;
//        p2 = p1;
//        p3 = p2;

//        p1.setX(p1.getX() + 0.1 * (double)qMin(imageWidth, imageHeight));
//        p2.setY(p1.getY() + 0.1 * (double)qMin(imageWidth, imageHeight));
//        p3.setZ(p1.getZ() + 0.1 * (double)qMin(imageWidth, imageHeight));
//    }

//    // draw frame
//    glLineWidth(7);
//    glBegin(GL_LINES);
//        glColor3fv(col_red);
//        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_red);
//        glVertex3fv(p0.getCoords());
//        glVertex3fv(p1.getCoords());
//        glColor3fv(col_green);
//        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_green);
//        glVertex3fv(p0.getCoords());
//        glVertex3fv(p2.getCoords());
//        glColor3fv(col_blue);
//        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_blue);
//        glVertex3fv(p0.getCoords());
//        glVertex3fv(p3.getCoords());
//    glEnd();

    //draw rectangle with image texture
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textID[6]);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(10, 10);

    if (offScreen)
    {
        z = -200;
    }
    else
    {
        z = 0;
    }

    glLineWidth(4);
    glBegin(GL_QUADS);
//        glColor3fv(col_dead);
//        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_dead);

        glNormal3f(0, 0, 1);
        glTexCoord2i(0, 1);
        glVertex3f(0, 0, z);

        glNormal3f(0, 0, 1);
        glTexCoord2i(1, 1);
        glVertex3f(super * imageWidth, 0, z);

        glNormal3f(0, 0, 1);
        glTexCoord2i(1, 0);
        glVertex3f(super * imageWidth, super * imageHeight, z);

        glNormal3f(0, 0, 1);
        glTexCoord2i(0, 0);
        glVertex3f(0, super * imageHeight, z);
    glEnd();

//    //draw rectangle
//    glLineWidth(4);
//    glBegin(GL_LINE_LOOP);
//        glColor3fv(col_dead);
//        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_dead);
//        glVertex3f(0, 0, 0);
//        glVertex3f(imageWidth, 0 ,0);
//        glVertex3f(imageWidth, imageHeight, 0);
//        glVertex3f(0, imageHeight, 0);
//    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1);
}

void GLviewsubd::drawLab()
{
    unsigned int i, j, k, l, max, max2;
    double       R, G, B, L, a, b, Y, x, y;
    GLfloat      col[3];

    glDisable(GL_TEXTURE_1D);

    if (ctrl_enabled)
    {
        max = 30;
        if (flat_mesh_enabled)
        {
            glPointSize(10);
            for (i = 0 ; i <= max ; i++)
            {
                for (j = 0 ; j <= max ; j++)
                {
                    for (k = 0 ; k <= max ; k++)
                    {
                        R = (double)i / (double)max;
                        G = (double)j / (double)max;
                        B = (double)k / (double)max;
                        RGB2LAB(R, G, B, L, a, b);

                        col[0] = R;
                        col[1] = G;
                        col[2] = B;

                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                        glColor3fv(col);
                        glBegin(GL_POINTS);
        //                glVertex3f(a/100, b/100, L/100 - 0.5);
                        glVertex3f(a/100.0, L/100.0 - 0.5, b/100.0);
                        glEnd();
                    }
                }
            }
        }
        else if (smooth_mesh_enabled)
        {
            glPointSize(8);
            for (i = 0 ; i <= max ; i++)
            {
                for (j = 0 ; j <= max ; j++)
                {
                    for (k = 0 ; k <= max ; k++)
                    {
                        L = (double)i / (double)max * 500 - 200;
                        a = (double)j / (double)max * 255 - 128;
                        b = (double)k / (double)max * 255 - 128;
                        LAB2RGB(L, a, b, R, G, B);

                        col[0] = R;
                        col[1] = G;
                        col[2] = B;

                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                        glColor3fv(col);
                        glBegin(GL_POINTS);
        //                glVertex3f(a/100, b/100, L/100 - 0.5);
                        glVertex3f(a/100.0, L/100.0 - 0.5, b/100.0);
                        glEnd();
                    }
                }
            }
        }
        else if (edged_mesh_enabled)
        {
            glPointSize(6);
            max = 170;
            for (j = 0 ; j <= max ; j++)
            {
                for (k = 0 ; k <= max ; k++)
                {
                    L = 0;
                    a = (double)j / (double)max * 255 - 128;
                    b = (double)k / (double)max * 255 - 128;
                    LAB2RGB(L, a, b, R, G, B);

                    col[0] = R;
                    col[1] = G;
                    col[2] = B;

                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                    glColor3fv(col);
                    glBegin(GL_POINTS);
    //                glVertex3f(a/100, b/100, L/100 - 0.5);
                    glVertex3f(a/100.0, L/100.0 - 0.5, b/100.0);
                    glEnd();
                }
            }
            for (j = 0 ; j <= max ; j++)
            {
                for (k = 0 ; k <= max ; k++)
                {
                    L = 100;
                    a = (double)j / (double)max * 255 - 128;
                    b = (double)k / (double)max * 255 - 128;
                    LAB2RGB(L, a, b, R, G, B);

                    col[0] = R;
                    col[1] = G;
                    col[2] = B;

                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                    glColor3fv(col);
                    glBegin(GL_POINTS);
    //                glVertex3f(a/100, b/100, L/100 - 0.5);
                    glVertex3f(a/100.0, L/100.0 - 0.5, b/100.0);
                    glEnd();
                }
            }
        }
        else if (culled_mesh_enabled)
        {
            glPointSize(8);
            max = 30;
            max2 = 3;
            for (i = 0 ; i <= max ; i++)
            {
                for (j = 0 ; j <= max ; j++)
                {
                    for (k = 0 ; k <= max ; k++)
                    {
                        R = (double)i / (double)max;
                        G = (double)j / (double)max;
                        B = (double)k / (double)max;
                        RGB2LAB(R, G, B, L, a, b);

                        for (l = 0 ; l <= max2 ; l++)
                        {
                            L = (double)l * 100 - 100;
                            LAB2RGB(L, a, b, R, G, B);

                            col[0] = R;
                            col[1] = G;
                            col[2] = B;

                            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                            glColor3fv(col);
                            glBegin(GL_POINTS);
            //                glVertex3f(a/100, b/100, L/100 - 0.5);
                            glVertex3f(a/100, L/100 - 0.5, b/100);
                            glEnd();

                        }
                    }
                }
            }
        }
    }
    else
    {
        max = 30;
        if (flat_mesh_enabled)
        {
            glPointSize(10);
            for (i = 0 ; i <= max ; i++)
            {
                for (j = 0 ; j <= max ; j++)
                {
                    for (k = 0 ; k <= max ; k++)
                    {
                        R = (double)i / (double)max;
                        G = (double)j / (double)max;
                        B = (double)k / (double)max;
                        RGB2YXY(R, G, B, Y, x, y);

                        col[0] = R;
                        col[1] = G;
                        col[2] = B;

                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                        glColor3fv(col);
                        glBegin(GL_POINTS);
        //                glVertex3f(a/100, b/100, L/100 - 0.5);
                        glVertex3f(x, Y - 0.5, y);
                        glEnd();
                    }
                }
            }
        }
        else if (smooth_mesh_enabled)
        {
            glPointSize(8);
            for (i = 0 ; i <= max ; i++)
            {
                for (j = 0 ; j <= max ; j++)
                {
                    for (k = 0 ; k <= max ; k++)
                    {
                        Y = (double)i / (double)max;
                        x = (double)j / (double)max;
                        y = (double)k / (double)max;
                        YXY2RGB(Y, x, y, R, G, B);

                        col[0] = R;
                        col[1] = G;
                        col[2] = B;

                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                        glColor3fv(col);
                        glBegin(GL_POINTS);
        //                glVertex3f(a/100, b/100, L/100 - 0.5);
                        glVertex3f(x, Y - 0.5, y);
                        glEnd();
                    }
                }
            }
        }
        else if (edged_mesh_enabled)
        {
            glPointSize(6);
            max = 170;
            for (j = 0 ; j <= max ; j++)
            {
                for (k = 0 ; k <= max ; k++)
                {
                    Y = 0;
                    x = (double)j / (double)max;
                    y = (double)k / (double)max;
                    YXY2RGB(Y, x, y, R, G, B);

                    col[0] = R;
                    col[1] = G;
                    col[2] = B;

                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                    glColor3fv(col);
                    glBegin(GL_POINTS);
    //                glVertex3f(a/100, b/100, L/100 - 0.5);
                    glVertex3f(x, Y - 0.5, y);
                    glEnd();
                }
            }
            for (j = 0 ; j <= max ; j++)
            {
                for (k = 0 ; k <= max ; k++)
                {
                    Y = 1;
                    x = (double)j / (double)max;
                    y = (double)k / (double)max;
                    YXY2RGB(Y, x, y, R, G, B);

                    col[0] = R;
                    col[1] = G;
                    col[2] = B;

                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                    glColor3fv(col);
                    glBegin(GL_POINTS);
    //                glVertex3f(a/100, b/100, L/100 - 0.5);
                    glVertex3f(x, Y - 0.5, y);
                    glEnd();
                }
            }
        }
        else if (culled_mesh_enabled)
        {
            glPointSize(8);
            max = 30;
            max2 = 3;
            for (i = 0 ; i <= max ; i++)
            {
                for (j = 0 ; j <= max ; j++)
                {
                    for (k = 0 ; k <= max ; k++)
                    {
                        R = (double)i / (double)max;
                        G = (double)j / (double)max;
                        B = (double)k / (double)max;
                        RGB2YXY(R, G, B, Y, x, y);

                        for (l = 0 ; l <= max2 ; l++)
                        {
                            Y = (double)l - 1;
                            YXY2RGB(Y, x, y, R, G, B);

                            col[0] = R;
                            col[1] = G;
                            col[2] = B;

                            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
                            glColor3fv(col);
                            glBegin(GL_POINTS);
            //                glVertex3f(a/100, b/100, L/100 - 0.5);
                            glVertex3f(x, Y - 0.5, y);
                            glEnd();

                        }
                    }
                }
            }
        }
    }

    glPointSize(1);
}

void GLviewsubd::buildFlatMesh()
{
	unsigned int j;

	makeCurrent();
    if(flat_mesh_list)	glDeleteLists(flat_mesh_list, 1);
    flat_mesh_list = glGenLists (1);
    glNewList (flat_mesh_list, GL_COMPILE);
//		glEnable(GL_LIGHTING);
        if (indexMesh > -1 )
        {
            drawMesh(FLAT, meshCurr[indexMesh], indexMesh, 0);
        }
        else
        {
            for (j = 0 ; j < meshCurr.size() ; j++)
            {
                drawMesh(FLAT, meshCurr[j], j, 0);
            }
        }
	glEndList();
}

void GLviewsubd::buildSmoothMesh()
{
    unsigned int j;

    makeCurrent();
    if(smooth_mesh_list)	glDeleteLists(smooth_mesh_list, 1);
    smooth_mesh_list = glGenLists (1);
    glNewList (smooth_mesh_list, GL_COMPILE);
    glEnable(GL_LIGHTING);

    if (old_enabled)
    {
        for (j = 0 ; j < meshCurr.size() ; j++)
        {
            drawMesh(SMOOTH, meshCurr[j], j, 0);
        }
    }
    else
    {
        if (indexMesh > -1 )
        {
            drawMesh(SMOOTH, meshCurr[indexMesh], indexMesh, 0);
        }
        else
        {
            for (j = 0 ; j < meshCurr.size() ; j++)
            {
                drawMesh(SMOOTH, meshCurr[j], j, 0);
            }
        }
    }

    glEndList();
}

void GLviewsubd::buildEdgedMesh()
{
	unsigned int j;

	makeCurrent();
	if(edged_mesh_list)	glDeleteLists(edged_mesh_list, 1);
	edged_mesh_list = glGenLists (1);
	glNewList (edged_mesh_list, GL_COMPILE);
//        glDisable(GL_LIGHTING);
        glEnable(GL_LIGHTING);
//		for (j = 0 ; j < meshCurr.size() ; j++)
//		{
//            drawMesh(WIREFRAME, meshCurr[j], j, 0);
//		}
        if (indexMesh > -1 )
        {
            drawMesh(WIREFRAME, meshCurr[indexMesh], indexMesh, 0);
        }
        else
        {
            for (j = 0 ; j < meshCurr.size() ; j++)
            {
                drawMesh(WIREFRAME, meshCurr[j], j, 0);
            }
        }
	glEndList();
}

void GLviewsubd::buildCulledMesh()
{
	unsigned int j;

	makeCurrent();
	if(culled_mesh_list)	glDeleteLists(culled_mesh_list, 1);
	culled_mesh_list = glGenLists (1);
	glNewList (culled_mesh_list, GL_COMPILE);
		glDisable(GL_LIGHTING);
//		for (j = 0 ; j < meshCurr.size() ; j++)
//		{
//            drawMesh(SOLIDFRAME, meshCurr[j], j, 0);
//		}
        if (indexMesh > -1 )
        {
            drawMesh(SOLIDFRAME, meshCurr[indexMesh], indexMesh, 0);
        }
        else
        {
            for (j = 0 ; j < meshCurr.size() ; j++)
            {
                drawMesh(SOLIDFRAME, meshCurr[j], j, 0);
            }
        }
	glEndList();
}

void GLviewsubd::buildCurvMMesh()
{
//	unsigned int j;

//	makeCurrent();
//	if(curvM_mesh_list)	glDeleteLists(curvM_mesh_list, 1);
//	curvM_mesh_list = glGenLists (1);
//	glNewList (curvM_mesh_list, GL_COMPILE);
//		glEnable(GL_LIGHTING);
//		for (j = 0 ; j < meshCurr.size() ; j++)
//		{
//			drawMesh(MEAN, meshCurr[j], j, 0);
//		}
//	glEndList();
}

void GLviewsubd::buildCurvGMesh()
{
	unsigned int j;
    PointPrec	 minG, maxG;

    minG = 100000000000;
    maxG = -minG;

    for (j = 0 ; j < meshCtrl.size() ; j++)
    {
        meshCurr[j]->compCurv(); // TO DO: call just compCurvG();

        //sync max. curvatures
        if (meshCurr[j]->my_minG < minG) minG = meshCurr[j]->my_minG;
        if (meshCurr[j]->my_maxG > maxG) maxG = meshCurr[j]->my_maxG;

        meshCurr[j]->compCurvSmooth(smoothCoef);
    }
    //sync max. curvatures
    for (j = 0 ; j < meshCtrl.size() ; j++)
    {
        meshCurr[j]->my_minG = minG;
        meshCurr[j]->my_maxG = maxG;
    }

	makeCurrent();
	if(curvG_mesh_list)	glDeleteLists(curvG_mesh_list, 1);
	curvG_mesh_list = glGenLists (1);
	glNewList (curvG_mesh_list, GL_COMPILE);
		glEnable(GL_LIGHTING);
//		for (j = 0 ; j < meshCurr.size() ; j++)
//		{
//            drawMesh(GAUSSIAN, meshCurr[j], j, 0);
//		}
        if (indexMesh > -1 )
        {
            drawMesh(GAUSSIAN, meshCurr[indexMesh], indexMesh, 0);
        }
        else
        {
            for (j = 0 ; j < meshCurr.size() ; j++)
            {
                drawMesh(GAUSSIAN, meshCurr[j], j, 0);
            }
        }
	glEndList();
}

void GLviewsubd::buildHeightMesh()
{
    unsigned int j;

    makeCurrent();
    if(height_mesh_list)	glDeleteLists(height_mesh_list, 1);
    height_mesh_list = glGenLists (1);
    glNewList (height_mesh_list, GL_COMPILE);
        glEnable(GL_LIGHTING);
//        for (j = 0 ; j < meshCurr.size() ; j++)
//        {
//            drawMesh(HEIGHT, meshCurr[j], j, 0);
//        }
        if (indexMesh > -1 )
        {
            drawMesh(HEIGHT, meshCurr[indexMesh], indexMesh, 0);
        }
        else
        {
            for (j = 0 ; j < meshCurr.size() ; j++)
            {
                drawMesh(HEIGHT, meshCurr[j], j, 0);
            }
        }
    glEndList();
}

void GLviewsubd::buildFeatureLines()
{
    unsigned int j;

    makeCurrent();
    if(feature_lines_list)	glDeleteLists(feature_lines_list, 1);
    feature_lines_list = glGenLists (1);
    glNewList (feature_lines_list, GL_COMPILE);
        glDisable(GL_LIGHTING);
//        for (j = 0 ; j < meshCurr.size() ; j++)
//        {
//            drawFeatureLines(meshCurr[j]);
//        }
//        if (indexMesh > -1 )
//        {
//            drawFeatureLines(meshCurr[indexMesh]);
//        }
        if (indexMesh > -1 )
        {
            drawFeatureLines(meshCurr[indexMesh]);
        }
        else
        {
            for (j = 0 ; j < meshCurr.size() ; j++)
            {
                drawFeatureLines(meshCurr[j]);
            }
        }
    glEndList();
}

void GLviewsubd::buildIPMesh()
{
	unsigned int j;

	makeCurrent();
	if(IP_mesh_list)	glDeleteLists(IP_mesh_list, 1);
	IP_mesh_list = glGenLists (1);
	glNewList (IP_mesh_list, GL_COMPILE);
		glEnable(GL_LIGHTING);
//		for (j = 0 ; j < meshCurr.size() ; j++)
//		{
//            drawMesh(ISOPHOTES, meshCurr[j], j, 0);
//		}
        if (indexMesh > -1 )
        {
            drawMesh(ISOPHOTES, meshCurr[indexMesh], indexMesh, 0);
        }
        else
        {
            for (j = 0 ; j < meshCurr.size() ; j++)
            {
                drawMesh(ISOPHOTES, meshCurr[j], j, 0);
            }
        }
	glEndList();
}

void GLviewsubd::buildCtrl()
{
	unsigned int j;

	makeCurrent();
	if(ctrl_list)	glDeleteLists(ctrl_list, 1);
	ctrl_list = glGenLists (1);
	glNewList (ctrl_list, GL_COMPILE);
	if (ctrl_enabled)
	{
        glDisable(GL_LIGHTING);
        if (indexMesh > -1 )
        {
            drawMesh(WIREFRAME, meshCtrl[indexMesh], indexMesh, 1);
        }
        else
        {
            for (j = 0 ; j < meshCtrl.size() ; j++)
            {
                drawMesh(WIREFRAME, meshCtrl[j], indexMesh, 1);
            }
        }
	}
	glEndList();
}

void GLviewsubd::buildOld()
{
    unsigned int j;

    makeCurrent();
    if(old_list)	glDeleteLists(old_list, 1);
    old_list = glGenLists (1);
    glNewList (old_list, GL_COMPILE);
    if (old_enabled)
    {
        glDisable(GL_LIGHTING);
//        glEnable(GL_LIGHTING);
        for (j = 0 ; j < meshCurr.size() ; j++)
        {
            drawMesh(WIREFRAME, meshCtrl[j], j, 2); //for meshOld
        }
    }
    glEndList();
}

void GLviewsubd::buildFrame()
{
	makeCurrent();
	if(frame_list)	glDeleteLists(frame_list, 1);
	frame_list = glGenLists (1);
	glNewList (frame_list, GL_COMPILE);
//	if (frame_enabled)
//	{
		glDisable(GL_LIGHTING);
		drawFrame();
//	}
	glEndList();
}

void GLviewsubd::buildPoi()
{
	makeCurrent();
	if(poi_list)	glDeleteLists(poi_list, 1);
	poi_list = glGenLists (1);
	glNewList (poi_list, GL_COMPILE);
		glDisable(GL_LIGHTING);
		drawPoi();
	glEndList();
}

void GLviewsubd::buildPoiSub()
{
	makeCurrent();
	if(poiSub_list)	glDeleteLists(poiSub_list, 1);
	poiSub_list = glGenLists (1);
	glNewList (poiSub_list, GL_COMPILE);
		glDisable(GL_LIGHTING);
		drawPoiSub();
	glEndList();
}

void GLviewsubd::buildPoiBound()
{
    makeCurrent();
    if(poiBound_list)	glDeleteLists(poiBound_list, 1);
    poiBound_list = glGenLists (1);
    glNewList (poiBound_list, GL_COMPILE);
        glDisable(GL_LIGHTING);
        drawPoiBound();
    glEndList();
}

void GLviewsubd::buildLab()
{
    makeCurrent();
    if(Lab_list)	glDeleteLists(Lab_list, 1);
    Lab_list = glGenLists (1);
    glNewList (Lab_list, GL_COMPILE);
        glDisable(GL_LIGHTING);
        drawLab();
    glEndList();
}

void GLviewsubd::drawMesh(DrawMeshType type, Mesh *mesh, unsigned int index, unsigned int ctrlType)
{
    unsigned int 	i, j;
    PointPrec 		val1, val2, value, minz, maxz;
	MeshFacet		*facet;
    Mesh 			tri;
    float           eps, epsG, epsZ;
    GLfloat         heightClr[3];

//    GLfloat ambientParams[4] = {0.2, 0.2, 0.2, 1};
    GLfloat ambientParams[4] = {0.1, 0.1, 0.1, 1};
    GLfloat specComp[4] = {0.25, 0.25, 0.25, 1};

    epsZ = 0.001; // workaround for Z930 texture bug
    epsG = 0.001; // workaround for Z930 texture bug

    // compute common minz and maxz
    minz = 100000;
    maxz = -100000;
    if (meshCtrl.size() > 0)
    {
        for (i = 0 ; i < meshCtrl.size() ; i++)
        {
            if (minz > meshCtrl[i]->my_minz)
            {
                minz = meshCtrl[i]->my_minz;
            }
            if (maxz < meshCtrl[i]->my_maxz)
            {
                maxz = meshCtrl[i]->my_maxz;
            }
        }
    }

    if (type == WIREFRAME || type == FLAT || type == SOLIDFRAME)
	{
        glShadeModel(GL_FLAT);
	}
	else
	{
        glShadeModel(GL_SMOOTH);
	}

    if (type == WIREFRAME && (int)index == indexMesh)
	{
        if (ctrlType == 0)
        {
            glLineWidth(1);
            glColor3fv(col_edges);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_edges);
            glEnable(GL_LIGHT2);
            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHT1);
        }
        else if (ctrlType == 1)
        {
            glLineWidth(2);
            glColor3fv(col_ctrl);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_ctrl);
            glEnable(GL_LIGHT2);
            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHT1);
        }
        else
        {
            glLineWidth(2);
            glColor3fv(col_old);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_old);
            glEnable(GL_LIGHT2);
            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHT1);
        }
	}
	else
	{
		glLineWidth(1);
        glColor3fv(col_edges);
	}

	switch(type)
	{
        case SMOOTH:
//        case FLAT:
            glDisable(GL_TEXTURE_1D);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if (!old_enabled)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(2 + index, 1);

                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specComp);
                glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 90);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientParams);
//                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_metal);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, meshCtrl[index]->colFlat);
            }
            else
            {
                glEnable(GL_BLEND);
                glDisable(GL_STENCIL_TEST);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDepthMask(GL_FALSE);

                GLfloat params[4];
                params[0] = 0.3; params[1] = 0.3; params[2] = 0.4; params[3] = 1;
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, params);
                params[0] = 0; params[1] = 0; params[2] = 0; params[3] = 0.4;
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, params);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, params);
            }
			break;

        case FLAT:
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_1D);
            glColor4fv(meshCtrl[index]->colFlat);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, meshCtrl[index]->colFlat);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;

		case SOLIDFRAME:
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(2 + index, 1);
            glDisable(GL_TEXTURE_1D);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;

		case WIREFRAME:
            glEnable(GL_POLYGON_OFFSET_LINE);
            if (ctrlType == 1)
            {
                glPolygonOffset(1 + index, 1);
            }
            else
            {
                glPolygonOffset(2 + index, 1);
            }
			glDisable(GL_TEXTURE_1D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;

		case MEAN:
        case GAUSSIAN:
        case HEIGHT:
            if (type == HEIGHT && clr == 3)
            {
                glDisable(GL_POLYGON_OFFSET_FILL);
            }
            else
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(2 + index, 1);
            }
            if (type == HEIGHT && !clrVsTxtr)
            {
                glDisable(GL_TEXTURE_GEN_S);
                glEnable(GL_TEXTURE_1D);
                glBindTexture(GL_TEXTURE_1D, textID[clr]);
            }
            else
            {
                glDisable(GL_TEXTURE_1D);
                glDisable(GL_LIGHTING);
            }
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


//            glEnable(GL_BLEND);
//            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//            glEnable(GL_BLEND);
//            glDisable(GL_STENCIL_TEST);
//            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//            glDepthMask(GL_FALSE);

            break;

		case ISOPHOTES:
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(2 + index, 1);
            glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_1D);
            glBindTexture(GL_TEXTURE_1D, textID[5]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;

		default:
//			cout << "Mesh TYPE out of range" << endl;
            break;
	}

	for (i = 0 ; i < mesh->my_numF ; i++)
	{
		facet = &(mesh->my_facets[i]);

        glBegin(GL_POLYGON);
        for(j = 0 ; j < facet->my_valency ; j++)
        {
//            if (type == WIREFRAME || type == SOLIDFRAME || type == FLAT)
            if (type == WIREFRAME || type == SOLIDFRAME)
            {
                glNormal3fv(facet->my_normalFlat);
            }
            else if (type == FLAT)
            {
//                value = 1.0 / (double)meshCtrl.size() * (double)index;
//                glTexCoord1f(value);
            }
            else
            {
                glNormal3fv(facet->my_corners[j].my_vertex->my_normalSmooth);
                if (type == GAUSSIAN)
                {
                    val1 = (mesh->my_minG);
                    val2 = (mesh->my_maxG);
                    if (val1 < 0) val1 = val1 / pow(1.414, curvRatio2);
                    if (val2 > 0) val2 = val2 / pow(1.414, curvRatio1);
                    value = (facet->my_corners[j].my_vertex->my_curvGsmooth - val1) / (val2 - val1);

                    if (clr == 2)
                    {
                        eps = (val2 - val1) / 500.0;
                        if (facet->my_corners[j].my_vertex->my_curvGsmooth > eps)
                        {
                            glTexCoord1f(0.75);
                        }
                        else if (facet->my_corners[j].my_vertex->my_curvGsmooth < -eps)
                        {
                            glTexCoord1f(0.25);
                        }
                        else
                        {
                            glTexCoord1f(0.5);
                        }
                    }
                    else
                    {
                        if (value < 0) value = 0;
                        if (value > 1) value = 1;
                        value *= 1 - 2 * epsG;
                        value += epsG;
                        glTexCoord1f(value);
                    }
                }
                if (type == HEIGHT)
                {
                    if (clr == 2)
                    {
                        if (facet->my_corners[j].my_vertex->my_point.getZ() > 0)
                        {
                            glTexCoord1f(0.75);
                        }
                        else if (facet->my_corners[j].my_vertex->my_point.getZ() < 0)
                        {
                            glTexCoord1f(0.25);
                        }
                        else
                        {
                            glTexCoord1f(0.5);
                        }
                    }
                    else if (clr == 3) // special luminance height (-100 .. 100)
                    {
                        value = facet->my_corners[j].my_vertex->my_point.getZ();
                        value = 0.5 + value / 200.0;
                        if (!clrVsTxtr)
                        {
                            glTexCoord1f(value); // relies on clamping to (0,1)
                        }
                        else
                        {
                            heightClr[0] = value;
                            heightClr[1] = value;
                            heightClr[2] = value;
                            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, heightClr);
                            glColor3fv(heightClr);
                        }
                    }
                    else
                    {
                        value = (facet->my_corners[j].my_vertex->my_point.getZ() - minz) / (maxz - minz);
                        value *= 1 - 2 * epsZ;
                        value += epsZ;
                        glTexCoord1f(value);
                        // z - coord colouring !!! NEED TO INHERIT minz and maxz through subdivision!
                    }
                }
                else if (type == ISOPHOTES)
                {
//                      glTexCoord1f(1);
                }
            }
            if (offScreen)
            {
                glVertex3f(super * facet->my_corners[j].my_vertex->my_point.getX(),
                           super * facet->my_corners[j].my_vertex->my_point.getY(),
                           facet->my_corners[j].my_vertex->my_point.getZ());
            }
            else
            {
                glVertex3fv(facet->my_corners[j].my_vertex->my_point.getCoords());
            }
        }
        glEnd();
	}

    glDisable(GL_BLEND);

    if (type == SOLIDFRAME)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(3 + index, 1);
        glDisable(GL_TEXTURE_1D);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_back);
        glColor4fv(col_back);

        for (i = 0 ; i < mesh->my_numF ; i++)
        {
            facet = &(mesh->my_facets[i]);
            glBegin(GL_POLYGON);
            for(j = 0 ; j < facet->my_valency ; j++)
            {
                glNormal3fv(facet->my_normalFlat);
                glVertex3fv(facet->my_corners[j].my_vertex->my_point.getCoords());
            }
            glEnd();
        }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glDepthMask(GL_TRUE);

    glDisable(GL_LIGHT2);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

//    glEnable(GL_DEPTH_BUFFER);
//    glDisable(GL_CULL_FACE);
}

void GLviewsubd::drawFeatureLines(Mesh *mesh)
{
    unsigned int        i, k;
    MeshFacet           *face;

    glDisable(GL_TEXTURE_1D);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_feature);
    glColor3fv(col_feature);
    glLineWidth(2);
    glBegin(GL_LINES);
    for (i = 0 ; i < mesh->my_numF ; i++)
    {
        face = &mesh->my_facets[i];
        for (k = 0 ; k < face->my_valency ; k++)
        {
            if (face->my_corners[k].my_vertex->isFeature && face->my_corners[k].my_nextCorner->my_vertex->isFeature)
            {
                glVertex3fv(face->my_corners[k].my_vertex->my_point.getCoords());
                glVertex3fv(face->my_corners[k].my_nextCorner->my_vertex->my_point.getCoords());
            }
        }
    }
    glEnd();
    glLineWidth(1);
}

void GLviewsubd::regenSurfs()
{
    unsigned int            i, j, level;
    std::vector < Mesh* >   tmp;
//    bool                    alive;

	level = meshCurr[0]->my_level;
//    alive = meshCurr[0]->lifeIsOn; // ideally, I'd need to remember the state of all facets!

    for (i = 0 ; i < meshSubd.size() ; i++)
    {
        for (j = 1 ; j < meshSubd[i].size() ; j++)
        {
            delete meshSubd[i][j];
            meshSubd[i][j] = NULL;
        }
    }

	for (j = 0 ; j < meshCurr.size() ; j++)
	{
        meshSubd[j].resize(1);
		meshCurr[j] = meshCtrl[j];
	}

//cout << "RegenSurfs with level: " << level << endl;

	setSubdivLevel(level);

//    if (alive)
//    {
//        for (i = 0 ; i < meshSubd.size() ; i++)
//        {
//            meshCurr[i]->lifeInit();
//        }
//    }
}

void GLviewsubd::resetSurfs()
{
    unsigned int	i, j, level;
    std::vector < Mesh* > tmp;
//    bool                    alive;

    if (meshCurr.size() > 0)
    {
        level = meshCurr[0]->my_level;

        for (i = 0 ; i < meshSubd.size() ; i++)
        {
            for (j = 1 ; j < meshSubd[i].size() ; j++)
            {
                delete meshSubd[i][j];
                meshSubd[i][j] = NULL;
            }
        }

        for (j = 0 ; j < meshCurr.size() ; j++)
        {
            meshSubd[j].resize(1);
            meshSubd[j][0] = meshCtrl[j]; // necessary for invSub!
            meshCurr[j] = meshCtrl[j];
        }

        setSubdivLevel(level);
    }
}

void GLviewsubd::loadFile(std::istream &is)
{
    unsigned int i, j;
	std::vector< Mesh* > vec;

    for (i = 0 ; i < meshSubd.size() ; i++)
    {
        meshCurr[i] = NULL;
        for (j = 1 ; j < meshSubd[i].size() ; j++)
        {
            delete meshSubd[i][j]; // TO DO - should clear more memory
            meshSubd[i][j] = NULL;
        }
    }
    meshSubd.clear();

    for (i = 0 ; i < meshCtrl.size() ; i++)
    {
        delete meshOld[i];
        meshOld[i] = NULL;
    }

	if (clear)
	{
        for (i = 0 ; i < meshCtrl.size() ; i++)
        {
            delete meshCtrl[i];
            meshCtrl[i] = NULL;
        }
		meshCtrl.clear();
		meshCurr.clear();
        meshOld.clear();
//		setRotZero();
	}


    Mesh *tmp = new Mesh();
//	tmp->my_file = fileName;
	tmp->my_rand = 0;
	tmp->transform = transf;

    tmp->load(is, imageHeight);

	meshCtrl.push_back(tmp);
    meshCurr = meshCtrl;

    Mesh *tmp2 = new Mesh();
    tmp2->my_numF = 0;
    tmp2->my_numV = 0;
    meshOld.push_back(tmp2);

	indexMesh = meshCtrl.size() - 1;

    vec.clear();
	for (j = 0 ; j < meshCtrl.size() ; j++)
	{
        meshSubd.push_back(vec);
        meshSubd[j].push_back(meshCtrl[j]);
	}

	verInd = -1;
	verIndSub = -1;

	lastChangeX = 0;
	lastChangeY = 0;
	lastChangeZ = 0;

    if (transf)
    {
        tmp->transf();
        my_scale = tmp->my_scale;
        my_centre = tmp->my_centre;
        makeCurrent();
        glLoadIdentity();
        glScalef(0.7 * my_scale, 0.7 * my_scale, 0.7 * my_scale);
        glTranslatef(-my_centre.getX(), -my_centre.getY(), -my_centre.getZ());
        scale = 1;
    }

    if (!offScreen)
    {
        updateAll();
    }
}

void GLviewsubd::saveFile(const char *fileName, bool isPly)
{
	meshCtrl[indexMesh]->save(fileName, isPly);
}

void GLviewsubd::saveFileLim(const char *fileName, bool isPly)
{
	meshCurr[indexMesh]->save(fileName, isPly);
}

void GLviewsubd::setPoint(int index)
{
    cout << "setPoint called with " << index - 1 << endl;
										  // shifted by 1 because of 'Select...'

	lastChangeX = 0;
	lastChangeY = 0;
	lastChangeZ = 0;

	verInd = index - 1; // shifted by 1 because of 'Select...'

	buildPoi();
	updateGL();
}

void GLviewsubd::changeSmoothing(int change)
{
	unsigned int j;

	smoothCoef = change;
	for (j = 0 ; j < meshCtrl.size() ; j++)
	{
		meshCurr[j]->compCurvSmooth(smoothCoef);
		meshCurr[j]->computeNormalsSmooth(smoothCoef);
	}
    buildAll();
    updateGL();
}

void GLviewsubd::movePointX(int change)
{
    PointPrec chng;
    Mesh 	 *mesh;

    if (change != lastChangeX && meshCtrl.size() > 0)
    {
        if (probeOnCtrl)
        {
            mesh = meshCtrl[indexMesh];
            if (verInd > -1)
            {
                chng = (float)1.0 / 240.0 * (change - lastChangeX) / mesh->my_scale;
                mesh->my_vertices[verInd].my_point.setX(mesh->my_vertices[verInd].my_point.getX() + chng);
                regenSurfs();
            }
        }
        else
        {
            mesh = meshCurr[indexMesh];
            if (verIndSub > -1)
            {
                chng = (float)1.0 / 240.0 * (change - lastChangeX) / meshCtrl[indexMesh]->my_scale;
                mesh->my_vertices[verIndSub].my_point.setX(mesh->my_vertices[verIndSub].my_point.getX() + chng);
                updateAll();
            }
        }
        lastChangeX = change;
    }
}

void GLviewsubd::movePointY(int change)
{
    PointPrec chng;
    Mesh 	 *mesh;

    if (change != lastChangeY && meshCtrl.size() > 0)
    {
        if (probeOnCtrl)
        {
            mesh = meshCtrl[indexMesh];
            if (verInd > -1)
            {
                chng = (float)1.0 / 240.0 * (change - lastChangeY) / mesh->my_scale;
                mesh->my_vertices[verInd].my_point.setY(mesh->my_vertices[verInd].my_point.getY() + chng);
                regenSurfs();
            }
        }
        else
        {
            mesh = meshCurr[indexMesh];
            if (verIndSub > -1)
            {
                chng = (float)1.0 / 240.0 * (change - lastChangeY) / meshCtrl[indexMesh]->my_scale;
                mesh->my_vertices[verIndSub].my_point.setY(mesh->my_vertices[verIndSub].my_point.getY() + chng);
                updateAll();
            }
        }
        lastChangeY = change;
    }
}

void GLviewsubd::movePointZ(int change)
{
    PointPrec chng;
    Mesh 	 *mesh;

    if (change != lastChangeZ && meshCtrl.size() > 0)
    {
        if (probeOnCtrl)
        {
            mesh = meshCtrl[indexMesh];
            if (verInd > -1)
            {
                chng = (float)1.0 / 240.0 * (change - lastChangeZ) / mesh->my_scale;
                mesh->my_vertices[verInd].my_point.setZ(mesh->my_vertices[verInd].my_point.getZ() + chng);
                regenSurfs();
            }
        }
        else
        {
            mesh = meshCurr[indexMesh];
            if (verIndSub > -1)
            {
                chng = (float)1.0 / 240.0 * (change - lastChangeZ) / meshCtrl[indexMesh]->my_scale;
                mesh->my_vertices[verIndSub].my_point.setZ(mesh->my_vertices[verIndSub].my_point.getZ() + chng);
                updateAll();
            }
        }
        lastChangeZ = change;
    }
}

void GLviewsubd::changeStripeDensity(int newDensity)
{
	static const int stripeRes = 1024;
	static const double pi = 3.14159265358979323846;
    GLfloat stripes[stripeRes], realDensity;

    glBindTexture(GL_TEXTURE_1D, textID[5]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    realDensity = pow(1.4, newDensity);

	for (int i = 0; i < stripeRes; ++i)
	{
        stripes[i] = 0.5 + sin(2 * pi * i * realDensity / stripeRes);
		stripes[i] = (stripes[i] < 0 ? 0 : stripes[i]);
		stripes[i] = (stripes[i] > 1 ? 1 : stripes[i]);
	}

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE, stripeRes,
				 0, GL_LUMINANCE, GL_FLOAT, stripes);

cout << "CHSD called" << endl;

    stripeDensityLevel = newDensity;
    buildIPMesh();
    updateGL();
}

void GLviewsubd::changeCurvRatio1(int newRatio)
{
	if (newRatio != curvRatio1)
	{
		curvRatio1 = newRatio;
//		buildCurvMMesh();
		buildCurvGMesh();
		updateGL();
	}
}

void GLviewsubd::changeCurvRatio2(int newRatio)
{
	if (newRatio != curvRatio2)
	{
		curvRatio2 = newRatio;
//		buildCurvMMesh();
		buildCurvGMesh();
		updateGL();
	}
}

void GLviewsubd::updateAll()
{
	unsigned int j;

    if (!offScreen)
    {
        //compute normals
        for (j = 0 ; j < meshCtrl.size() ; j++)
        {
            //assert(meshCurr[j] != NULL);
            meshCurr[j]->computeNormalsFlat();
            meshCurr[j]->computeNormalsSmooth(smoothCoef);
        }
    }
    buildAll();
    updateGL();
}

void GLviewsubd::setProbeOnCtrl(const bool b)
{
	probeOnCtrl = b;

	verIndSub = -2;
	buildPoiSub();

	verInd = -2;
	emit poiChanged();
	emit indexChanged(verInd + 1); // shifted by 1 because of 'Select...'
}

void GLviewsubd::setCC(const bool b)
{
    subType = CC;
    if (b)
    {
        resetSurfs();
    }
}

void GLviewsubd::setCCB(const bool b)
{
    subType = CCB;
    if (b)
    {
        resetSurfs();
    }
}

void GLviewsubd::setICC(const bool b)
{
    subType = ICC;
    if (b)
    {
        resetSurfs();
    }
}

void GLviewsubd::setCF(const bool b)
{
    subType = CF;
    if (b)
    {
        resetSurfs();
    }
}

void GLviewsubd::setLAP(const bool b)
{
    subType = LAP;
    if (b)
    {
        resetSurfs();
    }
}

void GLviewsubd::setShowIPMesh(const bool b)
{
    IP_mesh_enabled = b;
    if (b)
    {
        changeStripeDensity(stripeDensityLevel); // TO DO: FIX THIS!!!
        changeStripeDensity(stripeDensityLevel); // why does it need to be called twice?
        buildIPMesh();
        updateGL();
    }
}

void GLviewsubd::lapSm1(void)
{
    unsigned int    i;

    for (i = 0 ; i < meshCtrl.size() ; i++)
    {
        meshCurr[i]->LaplacianSmooth(lapSmValue);
    }

    updateAll();
}

void GLviewsubd::lapSm10(void)
{
    unsigned int    i, j;

    for (j = 0 ; j < 10 ; j ++)
    {
        for (i = 0 ; i < meshCtrl.size() ; i++)
        {
            meshCurr[i]->LaplacianSmooth(lapSmValue);
        }
    }
    updateAll();
}

void GLviewsubd::lapSm100(void)
{
    unsigned int    i, j;

    for (j = 0 ; j < 100 ; j ++)
    {
        for (i = 0 ; i < meshCtrl.size() ; i++)
        {
            meshCurr[i]->LaplacianSmooth(lapSmValue);
        }
    }
    updateAll();
}

void GLviewsubd::changeLapSmValue(int newValue)
{
    lapSmValue = newValue;
}

void GLviewsubd::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
    {
        event->acceptProposedAction();
    }
}

void GLviewsubd::dropEvent(QDropEvent *event)
{
    QString         str;
    QByteArray 		bytes;
    const char 			*file_name;

    if (event->mimeData()->hasUrls())
    {
//        foreach (QUrl url, event->mimeData()->urls())
//        {
//            std::cout << url.toString().toUtf8().constData() << endl;
//        }

        str = event->mimeData()->urls().at(0).toString();

        str = str.remove(0,7);

        bytes = str.toAscii();
        file_name = bytes.data();

        emit openFile(file_name);
    }

    event->acceptProposedAction();
}

void GLviewsubd::buffer2img()
{
    offScreen = true;

//    emit subdivToLevel(4);

    updateGL();
}

void GLviewsubd::focusView(void)
{
    if (indexMesh > -1)
    {
        makeCurrent();
        glLoadIdentity();
        glScalef(0.7 * meshCtrl[indexMesh]->my_scale, 0.7 * meshCtrl[indexMesh]->my_scale, 0.7 * meshCtrl[indexMesh]->my_scale);
        glTranslatef(-meshCtrl[indexMesh]->my_centre.getX(), -meshCtrl[indexMesh]->my_centre.getY(), -meshCtrl[indexMesh]->my_centre.getZ());
        scale = 1;
    }
    else
    {
        setRotZero();
    }

    updateGL();
}

void GLviewsubd::setRotZero(void)
{
    makeCurrent();

    double max = qMax(imageWidth, imageHeight);
    double sc = 1.0 / max;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(0.9 * sc, 0.9 * sc, 0.9 * sc);
    glTranslatef(-(double)imageWidth / 2.0, -(double) imageHeight / 2.0, -0);
    scale = 1;
    updateGL();
}

//somewhere from the web
void GLviewsubd::lab2rgbVer2(double L, double a, double b, double &R, double &G, double &B)
{
    double X, Y, Z;

    // Lab -> normalized XYZ (X,Y,Z are all in 0...1)

    Y = L * (1.0/116.0) + 16.0/116.0;
    X = a * (1.0/500.0) + Y;
    Z = b * (-1.0/200.0) + Y;

    X = X > 6.0/29.0 ? X * X * X : X * (108.0/841.0) - 432.0/24389.0;
    Y = L > 8.0 ? Y * Y * Y : L * (27.0/24389.0);
    Z = Z > 6.0/29.0 ? Z * Z * Z : Z * (108.0/841.0) - 432.0/24389.0;

    // normalized XYZ -> linear sRGB (in 0...1)

    R = X * (1219569.0/395920.0)     + Y * (-608687.0/395920.0)    + Z * (-107481.0/197960.0);
    G = X * (-80960619.0/87888100.0) + Y * (82435961.0/43944050.0) + Z * (3976797.0/87888100.0);
    B = X * (93813.0/1774030.0)      + Y * (-180961.0/887015.0)    + Z * (107481.0/93370.0);

    // linear sRGB -> gamma-compressed sRGB (in 0...1)

    R = R > 0.0031308 ? powJiri(R, 1.0 / 2.4) * 1.055 - 0.055 : R * 12.92;
    G = G > 0.0031308 ? powJiri(G, 1.0 / 2.4) * 1.055 - 0.055 : G * 12.92;
    B = B > 0.0031308 ? powJiri(B, 1.0 / 2.4) * 1.055 - 0.055 : B * 12.92;
}

//somewhere from the web
template<class S,class T>
void GLviewsubd::lab2rgb(
    const S li,
    const S ai,
    const S bi,
    T& ro,
    T& go,
    T& bo)
{
    const S lThresh=0.008856*903.3;

    S y, fy;
    if (li <= lThresh)
    {
        y = li / 903.3;
        fy = 7.787 * y + 16.0 / 116.0;
    }
    else
    {
        fy = (li + 16.0) / 116.0;
        y = fy * fy * fy;
    }
    S fxz[] = {S(ai / 500.0 + fy), S(bi /-200.0 + fy)};
    S xzUnnorm[2];
    const S fThresh = 7.787 * 0.008856 + 16.0 / 116.0;

    for (int i = 0; i < 2; i++)
    {
        S f = fxz[i];
        if (f <= fThresh)
        {
            xzUnnorm[i] = (f - 16.0 / 116.0) / 7.787;
        }
        else
        {
            xzUnnorm[i] =f * f * f;
        }
    }

    S x = xzUnnorm[0] * 0.950456,
      z = xzUnnorm[1] * 1.088754;
    ro = 3.240479*x -1.53715*y - 0.498535*z;
    go = -0.969256*x +1.875991*y + 0.041556*z;
    bo = 0.055648*x -0.204043*y + 1.057311*z;
}

//reimplemented from http://www.easyrgb.com/index.php?X=MATH&H=01#text1
void GLviewsubd::RGB2XYZ(double R, double G, double B, double &X, double &Y, double &Z)
{
    double var_R,var_G,var_B;

    var_R = R; // / 255 )
    var_G = G; // / 255 )
    var_B = B; // / 255 )

    if ( var_R > 0.04045 )
    {
        var_R = powJiri( ( var_R + 0.055 ) / 1.055, 2.4);
    }
    else
    {
        var_R = var_R / 12.92;
    }

    if ( var_G > 0.04045 )
    {
        var_G = powJiri( ( var_G + 0.055 ) / 1.055, 2.4);
    }
    else
    {
        var_G = var_G / 12.92;
    }

    if ( var_B > 0.04045 )
    {
        var_B = powJiri( ( var_B + 0.055 ) / 1.055, 2.4);
    }
    else
    {
        var_B = var_B / 12.92;
    }

    var_R = var_R * 100.0;
    var_G = var_G * 100.0;
    var_B = var_B * 100.0;

    //Observer. = 2°, Illuminant = D65
    X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
    Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
    Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
}

void GLviewsubd::XYZ2LAB(double X, double Y, double Z, double &L, double &a ,double &b)
{
    double var_X,var_Y,var_Z;

    double ref_X = 95.047;
    double ref_Y = 100.0;
    double ref_Z = 100.0;

    var_X = X / ref_X;          //ref_X =  95.047   Observer= 2°, Illuminant= D65
    var_Y = Y / ref_Y;          //ref_Y = 100.000
    var_Z = Z / ref_Z;          //ref_Z = 108.883

    if ( var_X > 0.008856 )
    {
        var_X = powJiri(var_X, 1.0/3.0);
    }
    else
    {
        var_X = ( 7.787 * var_X ) + ( 16.0 / 116.0 );
    }

    if ( var_Y > 0.008856 )
    {
        var_Y = powJiri(var_Y, 1.0/3.0);
    }
    else
    {
        var_Y = ( 7.787 * var_Y ) + ( 16.0 / 116.0 );
    }

    if ( var_Z > 0.008856 )
    {
        var_Z = powJiri(var_Z, 1.0/3.0);
    }
    else
    {
        var_Z = ( 7.787 * var_Z ) + ( 16.0 / 116.0 );
    }

    L = ( 116.0 * var_Y ) - 16.0;
    a = 500.0 * ( var_X - var_Y );
    b = 200.0 * ( var_Y - var_Z );
}

void GLviewsubd::RGB2LAB(double R, double G,double B, double &L, double &a, double &b)
{
    double x,y,z;
    RGB2XYZ(R,G,B,x,y,z);
    XYZ2LAB(x,y,z,L,a,b);
}

//reimplemented from http://www.easyrgb.com/index.php?X=MATH&H=01#text1
void GLviewsubd::LAB2RGB(double L, double a, double b, double &R, double &G, double &B)
{
    double x,y,z;
    LAB2XYZ(L,a,b,x,y,z);
    XYZ2RGB(x,y,z,R,G,B);
}

void GLviewsubd::LAB2XYZ(double L, double a, double b, double &X, double &Y, double &Z)
{
    double var_X, var_Y, var_Z, x3, y3, z3;

    var_Y = ( L + 16.0 ) / 116.0;
    var_X = a / 500.0 + var_Y;
    var_Z = var_Y - b / 200.0;

    x3 = var_X * var_X * var_X;
    y3 = var_Y * var_Y * var_Y;
    z3 = var_Z * var_Z * var_Z;

    if ( y3 > 0.008856 )
    {
        var_Y = y3;
    }
    else
    {
        var_Y = ( var_Y - 16.0 / 116.0 ) / 7.787;
    }

    if ( x3 > 0.008856 )
    {
        var_X = x3;
    }
    else
    {
        var_X = ( var_X - 16.0 / 116.0 ) / 7.787;
    }

    if ( z3 > 0.008856 )
    {
        var_Z = z3;
    }
    else
    {
        var_Z = ( var_Z - 16.0 / 116.0 ) / 7.787;
    }

    double ref_X = 95.047;
    double ref_Y = 100.0;
    double ref_Z = 100.0;

    X = ref_X * var_X;     //ref_X =  95.047     Observer= 2°, Illuminant= D65
    Y = ref_Y * var_Y;     //ref_Y = 100.000
    Z = ref_Z * var_Z;     //ref_Z = 108.883
}

void GLviewsubd::XYZ2RGB(double X, double Y, double Z, double &R, double &G, double &B)
{
    double var_X,var_Y,var_Z,var_R,var_G,var_B;

    var_X = X / 100.0;        //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
    var_Y = Y / 100.0;        //Y from 0 to 100.000
    var_Z = Z / 100.0;        //Z from 0 to 108.883

    var_R = var_X *  3.2406 + var_Y * (-1.5372) + var_Z * (-0.4986);
    var_G = var_X * (-0.9689) + var_Y *  1.8758 + var_Z *  0.0415;
    var_B = var_X *  0.0557 + var_Y * (-0.2040) + var_Z *  1.0570;

    if ( var_R > 0.0031308 )
    {
        var_R = 1.055 * ( powJiri(var_R, 1.0 / 2.4) ) - 0.055;
    }
    else
    {
        var_R = 12.92 * var_R;
    }

    if ( var_G > 0.0031308 )
    {
        var_G = 1.055 * ( powJiri(var_G, 1.0 / 2.4) ) - 0.055;
    }
    else
    {
        var_G = 12.92 * var_G;
    }

    if ( var_B > 0.0031308 )
    {
        var_B = 1.055 * ( powJiri(var_B, 1.0 / 2.4) ) - 0.055;
    }
    else
    {
        var_B = 12.92 * var_B;
    }

    R = var_R; // * 255;
    G = var_G; // * 255;
    B = var_B; // * 255;
}

//reimplementation from Matlab
void GLviewsubd::matlabRGB2LAB(double R, double G,double B, double &L, double &a, double &b)
{
    double T = 0.008856,
           X, Y, Z, Y3, fX, fY, fZ;
    bool   XT, YT, ZT;

    X = R * 0.412453 + G * 0.357580 + B * 0.180423;
    Y = R * 0.212671 + G * 0.715160 + B * 0.072169;
    Z = R * 0.019334 + G * 0.119193 + B * 0.950227;

    X = X / 0.950456;
    Z = Z / 1.088754;

    XT = X > T;
    YT = Y > T;
    ZT = Z > T;

    fX = (int)XT * powJiri(X, 0.333333) + ((int)!XT) * (7.787 * X + 0.137931);

    // Compute L
    Y3 = powJiri(Y, 0.333333);
    fY = (int)YT * Y3 + ((int)!YT) * (7.787 * Y + 0.137931);
    L  = (int)YT * (116.0 * Y3 - 16.0) + ((int)!YT) * (903.3 * Y);

    fZ = (int)ZT * powJiri(Z, 0.333333) + ((int)!ZT) * (7.787 * Z + 0.137931);

    // Compute a and b
    a = 500.0 * (fX - fY);
    b = 200.0 * (fY - fZ);
}

void GLviewsubd::matlabLAB2RGB(double L, double a, double b, double &R, double &G, double &B)
{
    double T1 = 0.008856,
           T2 = 0.206893,
           fX, fY, fZ, X, Y, Z;
    bool XT, YT, ZT;

//    int tmp;
//    double tmp2;

    //Compute Y
    fY = powJiri((L + 16.0) / 116.0, 3.0);
    YT = fY > T1;
    fY = ((int)!YT) * (L / 903.3) + (int)YT * fY;
    Y = fY;

//    tmp = (int)YT;
//    tmp = (int)!YT;
//    tmp2 = powJiri(fY, 0.333333);

    // Alter fY slightly for further calculations
    fY = (int)YT * (powJiri(fY, 0.333333)) + ((int)!YT) * (7.787 * fY + 0.137931);

    // Compute X
    fX = a / 500.0 + fY;
    XT = fX > T2;
    X = ((int)XT * (powJiri(fX,3.0)) + ((int)!XT) * ((fX - 0.137931) / 7.787));

    // Compute Z
    fZ = fY - b / 200.0;
    ZT = fZ > T2;
    Z = ((int)ZT * (powJiri(fZ,3.0)) + ((int)!ZT) * ((fZ - 0.137931) / 7.787));

    X = X * 0.950456;
    Z = Z * 1.088754;

    R = X *  3.240479 + Y * (-1.537150) + Z * (-0.498535);
    G = X * (-0.969256) + Y *  1.875992 + Z *  0.041556;
    B = X *  0.055648 + Y * (-0.204043) + Z *  1.057311;

    if (blackOut)
    {
        if (R < 0 || R > 1 || G < 0 || G > 1 || B < 0 || B > 1)
        {
            R = 0;
            G = 0;
            B = 0;
        }
    }
    else
    {
        if (R > 1)
        {
            R = 1;
        }
        else if (R < 0)
        {
            R = 0;
        }
        if (G > 1)
        {
            G = 1;
        }
        else if (G < 0)
        {
            G = 0;
        }
        if (B > 1)
        {
            B = 1;
        }
        else if (B < 0)
        {
            B = 0;
        }
    }
}

double GLviewsubd::powJiri(double b, double e)
{
    if (b < 0)
    {
        return(-pow(-b, e));
    }
    else
    {
        return(pow(b,e));
    }
}

void GLviewsubd::RGB2YXY(double R, double G, double B, double &Y, double &x, double &y)
{
    double X, Z, s;

    X = R * 0.412453 + G * 0.357580 + B * 0.180423;
    Y = R * 0.212671 + G * 0.715160 + B * 0.072169;
    Z = R * 0.019334 + G * 0.119193 + B * 0.950227;

    s = X + Y + Z;

    if (s == 0)
    {
        x = 0;
        y = 0;
    }
    else
    {
        x = X / (X + Y + Z);
        y = Y / (X + Y + Z);
    }
}

void GLviewsubd::YXY2RGB(double Y, double x, double y, double &R, double &G, double &B)
{
    double X, Z;

    if (y == 0)
    {
        X = 0;
        Z = 0;
    }
    else
    {
        X = Y / y * x;
        Z = Y / y * (1 - x - y);
    }

    R = X *  3.240479 + Y * (-1.537150) + Z * (-0.498535);
    G = X * (-0.969256) + Y *  1.875992 + Z *  0.041556;
    B = X *  0.055648 + Y * (-0.204043) + Z *  1.057311;

    if (blackOut)
    {
        if (R < 0 || R > 1 || G < 0 || G > 1 || B < 0 || B > 1)
        {
            R = 0;
            G = 0;
            B = 0;
        }
    }
    else
    {
        if (R > 1)
        {
            R = 1;
        }
        else if (R < 0)
        {
            R = 0;
        }
        if (G > 1)
        {
            G = 1;
        }
        else if (G < 0)
        {
            G = 0;
        }
        if (B > 1)
        {
            B = 1;
        }
        else if (B < 0)
        {
            B = 0;
        }
    }
}
