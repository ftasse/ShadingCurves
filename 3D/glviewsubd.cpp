#include "../Views/glew/GL/glew.h"
//#include <QtGui>
//#include <QtOpenGL>
#include "glviewsubd.h"
#include <assert.h>
//#include <GL/glu.h>

using namespace std;

GLviewsubd::GLviewsubd(GLuint iW, GLuint iH, QWidget *parent, QGLWidget *shareWidget) : GLviewport(parent, shareWidget)
{
    setAcceptDrops(true);

//    meshCtrl.clear();
//	meshCurr.clear();
//	meshSubd.clear();
//    meshOld.clear();

	mesh_enabled = true;
    flat_mesh_enabled = true;
    smooth_mesh_enabled = false;
	edged_mesh_enabled = false;
	culled_mesh_enabled = false;
	curvM_mesh_enabled = false;
	curvG_mesh_enabled = false;
	curvTG_mesh_enabled = false;
    height_mesh_enabled = false;
    feature_lines_enabled = true;
    line_enabled = false;
	IP_mesh_enabled = false;
    triang_enabled = false;
	triang2_enabled = false;
	ctrl_enabled = true;
    old_enabled = false;
	shaded_ctrl_enabled = false;
	edged_ctrl_enabled = true;
	culled_ctrl_enabled = false;
	frame_enabled = false;
	probeOnCtrl = true;
    transf = true;
	clear = true;

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
}

GLviewsubd::~GLviewsubd()
{
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

    showMessageCF = false;
    showMessageCCint = false;

    if (meshCurr.size() > 0 && meshCurr[0]->my_level != newL)
	{
        oldLevel = meshCurr[0]->my_level;
cout << "setSubdivLevel with old level " << oldLevel << " new level " << newLevel << endl;

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
        cout << "subdivide End " << endl;
	}
	updateAll();

    if (showMessageCCint)
    {
//        cout << "Cannot use CCint subdivision on this model!" << endl;
        QMessageBox msgBox;
        msgBox.setText("Cannot use CCint subdivision on this model\n"
                        "(boundary)!\n"
                        "Use Catmull-Clark instead.");
        msgBox.exec();
    }

    if (showMessageCF)
    {
//        cout << "Cannot use Conformal subdivision on this model!" << endl;
        QMessageBox msgBox;
        msgBox.setText("Cannot use Conformal subdivision on this model\n"
                        "(more than one EV)!\n"
                        "Use Catmull-Clark instead.");
        msgBox.exec();
    }
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

//		buildPoi();
//		buildPoiSub();
//		buildCtrl();
//		updateGL();

        focusView();
	}
}

void GLviewsubd::buildAll(void)
{
////    if (mesh_enabled)
//    {
//        if (flat_mesh_enabled)
//        {
//            buildFlatMesh();
//        }
//        else if (smooth_mesh_enabled)
//        {
//            buildSmoothMesh();
//        }
//        else if (edged_mesh_enabled)
//        {
//            buildEdgedMesh();
//        }
//        else if (culled_mesh_enabled)
//        {
//            buildCulledMesh();
//        }
//        else if (curvG_mesh_enabled)
//        {
//            buildCurvGMesh();
//        }
//        else if (height_mesh_enabled)
//        {
//            buildHeightMesh();
//        }
//        else if (IP_mesh_enabled)
//        {
//            buildIPMesh();
//        }
//    }

//    if (feature_lines_enabled)
//    {
//        buildFeatureLines();
//    }

//    if (ctrl_enabled)
//    {
//        buildCtrl();
//        buildPoi();
//        buildPoiSub();
//        buildPoiBound();
//    }

//    if (old_enabled)
//    {
//        buildOld();
//    }

//    if (frame_enabled)
//    {
//        buildFrame();
//    }

//    if (line_enabled)
//    {
//        buildLine();
//    }
}

void GLviewsubd::initializeGL(void)
{
    static const int res = 1024;
    PointPrec		col[3];
    GLfloat			texture[5][res][3];

//    if (joinTheDarkSide)
//    {
//        glClearColor(0, 0, 0, 0);
//    }
//    else
//    {
//        glClearColor(1, 1, 1, 0);
//    }

    glClearColor(col_back[0], col_back[1], col_back[2], col_back[3]);

//    // FOG
//    fogColor[0] = 0.5;
//    fogColor[1] = 0.5;
//    fogColor[2] = 0.5;
//    fogColor[3] = 1.0;
//    glClearColor(0.f,0.f,0.f,1.0f);  // Clear To The Color Of The Fog
//    glFogi(GL_FOG_MODE, GL_EXP);        // Fog Mode: GL_EXP, GL_EXP2, GL_LINEAR
//    glFogfv(GL_FOG_COLOR, fogColor);    // Set Fog Color
//    glFogf(GL_FOG_DENSITY, 0.3f);      // How Dense Will The Fog Be
//    glHint(GL_FOG_HINT, GL_NICEST);  // Fog Hint Value
////    glFogf(GL_FOG_START, 0.0f);         // Fog Start Depth
////    glFogf(GL_FOG_END, 1.0f);           // Fog End Depth
//    glEnable(GL_FOG);                   // Enables GL_FOG
//    // END FOG



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

    swapBuffers();

    glGenTextures(6, textID);

    for (int j = 0; j < 5; ++j) // number of colour systems
    {
        for (int i = 0; i < res; ++i)
        {
            genColor(j, (float)i / res - 0.5, -0.5, 0.5, col);

            texture[j][i][0] = col[0];
            texture[j][i][1] = col[1];
            texture[j][i][2] = col[2];
        }

        glBindTexture(GL_TEXTURE_1D, textID[j]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glDisable(GL_TEXTURE_GEN_S);

        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, res,
                     0, GL_RGB, GL_FLOAT, texture[j]);
    }
}

void GLviewsubd::paintGL(void)
{

    if (offScreen)
    {
        cout << "PaintGL OFFscreen called: " << numberPaintCalls << endl;
        numberPaintCalls++;

        int origClr, origView[4];

        makeCurrent();

        //Setup for offscreen drawing if fbos are supported
        GLuint framebuffer, renderbuffer;
        GLenum status;

        glGenFramebuffersEXT(1, &framebuffer);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
        glGenRenderbuffersEXT(1, &renderbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, imageWidth, imageHeight);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                         GL_RENDERBUFFER_EXT, renderbuffer);
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
            qDebug("Could not draw offscreen");

        //Drawing
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        glGetIntegerv(GL_VIEWPORT, origView);
        glViewport(0, 0, imageWidth, imageHeight);

        glOrtho(0, imageWidth, 0, imageHeight, -1000.0, 1000.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glRenderMode(GL_RENDER);
    //    glLineWidth(1.0);
    //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //    glEnable(GL_BLEND);
    //    glEnable(GL_LINE_SMOOTH);

        //draw stuff here

        origClr = clr;
        clr = 3;
    //    drawMesh(HEIGHT, meshCurr[0], 0, 0);
    //    paintGL();
        buildHeightMesh();
        glCallList(height_mesh_list);

        clr = origClr;

        cv::Mat img;
        img.create(imageHeight, imageWidth, CV_8UC3);
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
        glReadPixels(0, 0, imageWidth, imageHeight, inputColourFormat, GL_UNSIGNED_BYTE, img.data);

        //Clean up offscreen drawing
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glViewport(0, 0, origView[2], origView[3]);
//        glOrtho(0, origView[2], 0, origView[3], -1000.0, 1000.0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glDeleteRenderbuffersEXT(1, &renderbuffer);
        glClearColor(col_back[0], col_back[1], col_back[2], col_back[3]);

        cv::cvtColor(img, img, CV_BGR2RGB);
        cv::flip(img, img, 0);
//        cv::cvtColor(img, img, CV_RGB2GRAY);
        cv::imwrite("3Dbuffer.png", img);
    //    cv::threshold( img, img, 254, 255,   CV_THRESH_BINARY);
    //    cv::imshow("3Dbuffer", img);
//        return img;
        offScreen = false;
        updateGL();
    }
    else
    {
//        cout << "PaintGL ONscreen called: " << numberPaintCalls << endl;
//        numberPaintCalls++;

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

        if (old_enabled)
        {
            buildOld();
            glCallList(old_list);
            buildSmoothMesh();
            glCallList(smooth_mesh_list);
        }
        else
        {
            if (mesh_enabled)
            {
                if (flat_mesh_enabled)
                {
        //			cout << "Calling flat_mesh_list" << endl;
                    buildFlatMesh();
                    glCallList(flat_mesh_list);
                }
                if (smooth_mesh_enabled)
                {
        //			cout << "Calling smooth_mesh_list" << endl;
                    buildSmoothMesh();
                    glCallList(smooth_mesh_list);
                }
                else if (edged_mesh_enabled)
                {
                    buildEdgedMesh();
                    glCallList(edged_mesh_list);
        //			cout << "Calling edged_mesh_list" << endl;
                }
                else if (culled_mesh_enabled)
                {
                    buildCulledMesh();
                    glCallList(culled_mesh_list);
        //			cout << "Calling culled_mesh_list" << endl;
                }
    //            else if (curvM_mesh_enabled)
    //            {
    //                glCallList(curvM_mesh_list);
    //    //			cout << "Calling curvM_mesh_list" << endl;
    //            }
                else if (curvG_mesh_enabled)
                {
                    buildCurvGMesh();
                    glCallList(curvG_mesh_list);
        //			cout << "Calling curvG_mesh_list" << endl;
                }
                else if (height_mesh_enabled)
                {
                    buildHeightMesh();
                    glCallList(height_mesh_list);
        //			cout << "Calling curvG_mesh_list" << endl;
                }
    //            else if (curvTG_mesh_enabled)
    //            {
    //                glCallList(curvTG_mesh_list);
    //    //			cout << "Calling curvTG_mesh_list" << endl;
    //            }
                else if (IP_mesh_enabled)
                {
                    buildIPMesh();
                    glCallList(IP_mesh_list);
        //			cout << "Calling curvG_mesh_list" << endl;
                }
            }

            if (feature_lines_enabled)
            {
                buildFeatureLines();
                glCallList(feature_lines_list);
            }

            if (ctrl_enabled)
            {
                if (edged_ctrl_enabled)
                {
        //			glClear(GL_DEPTH_BUFFER_BIT);
                }
        //		cout << "Calling ctrl_list" << endl;

                buildCtrl();
                buildPoiSub();
    //            buildPoiBound();

                glCallList(ctrl_list);
                glCallList(poiBound_list);
            }

            if (line_enabled)
            {
                buildLine();
                glCallList(line_list);
            }


            glClear(GL_DEPTH_BUFFER_BIT);
            glCallList(poiSub_list);
        }

        if (ctrl_enabled)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            buildPoi();
            glCallList(poi_list);
        }

        if (frame_enabled)
        {
    //		glClear(GL_DEPTH_BUFFER_BIT);
            buildFrame();
            glCallList(frame_list);
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
    Point_3D    p0, p1, p2, p3;

    glDisable(GL_TEXTURE_1D);

    if (indexMesh > -1)
    {
        p0 = meshCtrl[indexMesh]->my_centre;
        p1 = p0;
        p2 = p0;
        p3 = p0;
        p1.setX(p1.getX()  + 0.1 / meshCtrl[0]->my_scale);
        p2.setY(p1.getY()  + 0.1 / meshCtrl[0]->my_scale);
        p3.setZ(p1.getZ()  + 0.1 / meshCtrl[0]->my_scale);
    }
    else
    {
        p0 = Point_3D(0.5 * (double)imageWidth, 0.5 * (double)imageHeight, 0);
        p1 = p0;
        p2 = p1;
        p3 = p2;

        p1.setX(p1.getX() + 0.1 * (double)qMin(imageWidth, imageHeight));
        p2.setY(p1.getY() + 0.1 * (double)qMin(imageWidth, imageHeight));
        p3.setZ(p1.getZ() + 0.1 * (double)qMin(imageWidth, imageHeight));
    }

    // draw frame
    glBegin(GL_LINES);
    glLineWidth(7);
        glColor3fv(col_red);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_red);
        glVertex3fv(p0.getCoords());
        glVertex3fv(p1.getCoords());
        glColor3fv(col_green);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_green);
        glVertex3fv(p0.getCoords());
        glVertex3fv(p2.getCoords());
        glColor3fv(col_blue);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_blue);
        glVertex3fv(p0.getCoords());
        glVertex3fv(p3.getCoords());
    glEnd();

    //draw rectangle
    glBegin(GL_LINE_LOOP);
        glLineWidth(4);
        glColor3fv(col_dead);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_dead);
        glVertex3f(0, 0, 0);


    glLineWidth(1);
}

void GLviewsubd::buildFlatMesh()
{
	unsigned int j;

	makeCurrent();
    if(flat_mesh_list)	glDeleteLists(flat_mesh_list, 1);
    flat_mesh_list = glGenLists (1);
    glNewList (flat_mesh_list, GL_COMPILE);
		glEnable(GL_LIGHTING);
//		for (j = 0 ; j < meshCurr.size() ; j++)
//		{
//            drawMesh(FLAT, meshCurr[j], j, 0);
//		}
        if (indexMesh > -1 )
        {
            drawMesh(FLAT, meshCurr[indexMesh], indexMesh, 0);
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
        for (j = 0 ; j < meshCurr.size() ; j++)
        {
            drawMesh(HEIGHT, meshCurr[j], j, 0);
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
        if (indexMesh > -1 )
        {
            drawFeatureLines(meshCurr[indexMesh]);
        }
    glEndList();
}

void GLviewsubd::buildLine()
{
    unsigned int j;

    makeCurrent();
    if(line_list)	glDeleteLists(line_list, 1);
    line_list = glGenLists (1);
    glNewList (line_list, GL_COMPILE);
        glDisable(GL_LIGHTING);
        drawLine();
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
//		if (shaded_ctrl_enabled)
//		{
//			glEnable(GL_LIGHTING);
//			for (j = 0 ; j < meshCurr.size() ; j++)
//			{
//				drawMesh(SHADED, meshCtrl[j], j, 1);
//			}
//		}
//		else if (edged_ctrl_enabled)
//		{

//            for (j = 0 ; j < meshCtrl.size() ; j++)
//			{
//                drawMesh(WIREFRAME, meshCtrl[j], j, 1);
//			}

            glDisable(GL_LIGHTING);
            //glEnable(GL_LIGHTING);
            if (indexMesh > -1 )
            {
                drawMesh(WIREFRAME, meshCtrl[indexMesh], indexMesh, 1);
            }


//		}
//		else if (culled_ctrl_enabled)
//		{
//			glDisable(GL_LIGHTING);
//			for (j = 0 ; j < meshCurr.size() ; j++)
//			{
//				drawMesh(SOLIDFRAME, meshCtrl[j], j, false);
//			}
//		}
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
	if (frame_enabled)
	{
		glDisable(GL_LIGHTING);
		drawFrame();
	}
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

void GLviewsubd::drawMesh(DrawMeshType type, Mesh *mesh, unsigned int index, unsigned int ctrlType)
{
    unsigned int 	i, j;
    PointPrec 		val1, val2, value, minz, maxz;
	MeshFacet		*facet;
    Mesh 			tri;
    float           eps, epsG, epsZ;

    GLfloat ambientParams[4] = {0.2, 0.2, 0.2, 1};
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
        case FLAT:
            glDisable(GL_TEXTURE_1D);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if (!old_enabled)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(2 + index, 1);

                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specComp);
                glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 90);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientParams);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_metal);
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
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(2 + index, 1);
            glDisable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_1D);
            glBindTexture(GL_TEXTURE_1D, textID[clr]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
            if (type == WIREFRAME || type == SOLIDFRAME || type == FLAT)
            {
                glNormal3fv(facet->my_normalFlat);
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
            glVertex3fv(facet->my_corners[j].my_vertex->my_point.getCoords());
        }
        glEnd();
	}

    if (type == SOLIDFRAME)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(3 + index, 1);
        glDisable(GL_TEXTURE_1D);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_faces);
        glColor4fv(col_faces);

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

void GLviewsubd::drawLine()
{
    unsigned int        i;

    glDisable(GL_TEXTURE_1D);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col_ctrl);
    glColor3fv(col_ctrl);
    glLineWidth(5);
    glBegin(GL_LINE_STRIP);
    if (line.my_plg.size() > 0)
    {
        for (i = 0 ; i <= line.my_numv ; i++)
        {
            glVertex3fv(line.my_plg[i].getCoords());
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

void GLviewsubd::loadFile(const char *fileName)
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

    tmp->load(fileName, imageHeight);

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

    updateAll();
}

void GLviewsubd::loadLine(const char *fileName)
{
    line.readC(fileName);

    buildLine();
    updateGL();
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
		buildCurvMMesh();
		buildCurvGMesh();
		updateGL();
	}
}

void GLviewsubd::changeCurvRatio2(int newRatio)
{
	if (newRatio != curvRatio2)
	{
		curvRatio2 = newRatio;
		buildCurvMMesh();
		buildCurvGMesh();
		updateGL();
	}
}

void GLviewsubd::updateAll()
{
	unsigned int j;

    //compute normals
    for (j = 0 ; j < meshCtrl.size() ; j++)
    {
        //assert(meshCurr[j] != NULL);
        meshCurr[j]->computeNormalsFlat();
        meshCurr[j]->computeNormalsSmooth(smoothCoef);
    }
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
    updateGL();
}

void GLviewsubd::focusView(void)
{
    if (indexMesh > -1 && !old_enabled)
    {
        makeCurrent();
        glLoadIdentity();
        glScalef(0.7 * meshCtrl[indexMesh]->my_scale, 0.7 * meshCtrl[indexMesh]->my_scale, 0.7 * meshCtrl[indexMesh]->my_scale);
        glTranslatef(-meshCtrl[indexMesh]->my_centre.getX(), -meshCtrl[indexMesh]->my_centre.getY(), -meshCtrl[indexMesh]->my_centre.getZ());
        scale = 1;

        updateAll();
    }
}

void GLviewsubd::setRotZero(void)
{
    makeCurrent();

    double max = qMax(imageWidth, imageHeight);
    double sc = 1.0 / max;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(0.7 * sc, 0.7 * sc, 0.7 * sc);
    glTranslatef(-(double)imageWidth / 2.0, -(double) imageHeight / 2.0, -0);
    scale = 1;

    updateGL();
}