#ifndef GLVIEWSUBD_H
#define GLVIEWSUBD_H

#include <iostream>
#include "3D/mesh.h"
#include "3D/point_3d.h"
#include "3D/glviewport.h"
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

enum ShadingType
{
    CVLAB, CVHLS, OWN, MATLAB, YXY, RGB
};

class GLviewsubd : public GLviewport
{
	Q_OBJECT

public:
    GLviewsubd(GLuint iW, GLuint iH, cv::Mat *timg, QWidget * parent = 0, QGLWidget * shareWidget = 0);
	~GLviewsubd();

    void   loadFile(std::istream &is);
	void   saveFile(const char *fileName, bool isPly);
	void   saveFileLim(const char *fileName, bool isPly);

    void buildFlatMesh();
    void buildSmoothMesh();
	void buildEdgedMesh();
	void buildCulledMesh();
    void buildHeightMesh();
    void buildFeatureLines();
	void buildIPMesh();
	void buildCtrl();
    void buildOld();
	void buildFrame(void);
	void buildPoi(void);
	void buildPoiSub(void);
    void buildPoiBound(void);
    void buildLab(void);
    void regenSurfs();
    void resetSurfs();

    void focusView      (void);

	std::vector< std::vector< Mesh* > >  meshSubd;
	std::vector< Mesh* >       		     meshCtrl;
	std::vector< Mesh* >				 meshCurr;
    std::vector< Mesh* >				 meshOld;
	Mesh				 				 *nextMesh;

	int verInd, verIndSub;

	int lastChangeX;
	int lastChangeY;
	int lastChangeZ;

	unsigned int smoothCoef;

	int indexMesh;

	bool clear;

    bool    writeImg, showImg;

    bool    clipping, blackOut;
    int     clipMin, clipMax;
    int     subdivTime;
    bool    flatImage, clrVsTxtr;

    ShadingType shade;

    bool offScreen, offMainWindow;
    cv::Mat  img, imgShaded, imgFill, imgFillShaded,
            *inputImg;

    int super;

    void lab2rgbVer2(double L, double a, double b, double &R, double &G, double &B);

    template<class S,class T>
    void lab2rgb(
        const S li,
        const S ai,
        const S bi,
        T& ro,
        T& go,
        T& bo);

    void RGB2XYZ(double R, double G, double B, double &X, double &Y, double &Z);
    void XYZ2LAB(double x, double y, double z, double &L, double &a ,double &b);
    void RGB2LAB(double R, double G, double B, double &L, double &a, double &b);
    void LAB2RGB(double L, double a, double b, double &R, double &G, double &B);
    void LAB2XYZ(double L, double a, double b, double &X, double &Y, double &Z);
    void XYZ2RGB(double X, double Y, double Z, double &R, double &G, double &B);

    void matlabRGB2LAB(double R, double G, double B, double &L, double &a, double &b);
    void matlabLAB2RGB(double L, double a, double b, double &R, double &G, double &B);

    void RGB2YXY(double R, double G, double B, double &Y, double &x, double &y);
    void YXY2RGB(double Y, double x, double y, double &R, double &G, double &B);

    double powJiri(double b, double e);

    std::vector<Point_3D> surfBlendColours;

public slots:
    void subdivide          (void);

    void setShowMesh		(const bool b)  { mesh_enabled = b; buildAll(); updateGL();}
    void setShowFlatMesh	(const bool b)  { flat_mesh_enabled = b; buildAll(); updateGL();}
    void setShowSmoothMesh	(const bool b)  { smooth_mesh_enabled = b; buildAll(); updateGL();}
    void setShowEdgedMesh	(const bool b)  { edged_mesh_enabled = b; buildAll(); updateGL();}
    void setShowCulledMesh	(const bool b)  { culled_mesh_enabled = b; buildAll(); updateGL();}
    void setShowHeightMesh	(const bool b)  { height_mesh_enabled = b; buildAll(); updateGL();}
    void setShowFeatureLines(const bool b)  { feature_lines_enabled = b; buildAll(); updateGL();}
    void setShowIPMesh		(const bool b);
    void setShowCtrl		(const bool b)  { ctrl_enabled = b; buildAll(); updateGL();}
    void setShowOld         (const bool b)  { old_enabled = b ; buildAll(); updateGL();}
    void setShowLab         (const bool b)  { Lab_enabled = b ; buildAll(); updateGL();}

	void setProbeOnCtrl(const bool b);

	void setTransf(const bool b) { transf = b; }

	void setClear(const bool b) { clear = b; }

    void setShowFrame		(const bool b)  { frame_enabled = b; buildAll(); updateGL();}
	void setSubdivLevel		(int newLevel);
	void setClr				(int newColor);
	void setIndMesh			(int newInd);
	void setPoint			(int index);
	void movePointX			(int change);
	void movePointY			(int change);
	void movePointZ			(int change);
	void buildAll			(void);
	void updateAll			(void);
    void changeStripeDensity(int newDensity);

    void setRotZero		(void);

    void buffer2img();

signals:
	void subdivLevelChanged(int newLevel);
	void rightClicked();
	void middleClicked();
	void callBar(int clr);
	void poiChanged();
	void indexChanged(int);
	void updateNeeded();
    void openFile       (const char *);
    void subdivToLevel(int level);

protected:
	void paintGL(void);
    void initializeGL(void);
	void mousePressEvent    (QMouseEvent *event);

    void drawMesh(DrawMeshType type, Mesh *mesh, unsigned int index, unsigned int ctrlType);
    void drawFeatureLines(Mesh *mesh);
	void drawPoi();
	void drawPoiSub();
    void drawPoiBound();
	void drawFrame();
    void drawLab();

private:
	bool mesh_enabled;
    bool flat_mesh_enabled;
    bool smooth_mesh_enabled;
	bool culled_mesh_enabled;
	bool edged_mesh_enabled;
    bool height_mesh_enabled;
    bool feature_lines_enabled;
	bool IP_mesh_enabled;
	bool ctrl_enabled;
    bool old_enabled;
	bool shaded_ctrl_enabled;
	bool edged_ctrl_enabled;
	bool culled_ctrl_enabled;
	bool frame_enabled;
    bool Lab_enabled;

	bool probeOnCtrl;
	bool transf;

    int stripeDensityLevel;
    int numberPaintCalls;

    unsigned int flat_mesh_list;
    unsigned int smooth_mesh_list;
	unsigned int edged_mesh_list;
	unsigned int culled_mesh_list;
    unsigned int height_mesh_list;
    unsigned int feature_lines_list;
	unsigned int IP_mesh_list;
	unsigned int frame_list;
	unsigned int poi_list;
	unsigned int poiSub_list;
    unsigned int poiBound_list;
	unsigned int ctrl_list;
    unsigned int old_list;
    unsigned int Lab_list;

    GLuint      imageHeight, imageWidth;
};

#endif
