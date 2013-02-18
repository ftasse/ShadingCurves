#ifndef GLVIEWSUBD_H
#define GLVIEWSUBD_H

#include <iostream>
#include "3D/mesh.h"
#include "3D/point_3d.h"
#include "3D/glviewport.h"

class GLviewsubd : public GLviewport
{
	Q_OBJECT

public:
	GLviewsubd(QWidget * parent = 0, QGLWidget * shareWidget = 0);
	~GLviewsubd();

    void   loadFile(const char *fileName);
    void   loadLine(const char *fileName);
	void   saveFile(const char *fileName, bool isPly);
	void   saveFileLim(const char *fileName, bool isPly);
//	void   loadMesh(int level);

    void buildFlatMesh();
    void buildSmoothMesh();
	void buildEdgedMesh();
	void buildCulledMesh();
	void buildCurvMMesh();
	void buildCurvGMesh();
    void buildHeightMesh();
	void buildCurvTGMesh();
    void buildFeatureLines();
	void buildIPMesh();
	void buildCtrl();
    void buildOld();
	void buildFrame(void);
	void buildPoi(void);
	void buildPoiSub(void);
    void buildPoiBound(void);
    void buildLine(void);
    void regenSurfs();
    void resetSurfs();

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

	std::vector< std::vector< Mesh* > >  meshSubd;
	std::vector< Mesh* >       		     meshCtrl;
	std::vector< Mesh* >				 meshCurr;
    std::vector< Mesh* >				 meshOld;
	Mesh				 				 *nextMesh;
    SplineC                              line;

	int verInd, verIndSub;

	int lastChangeX;
	int lastChangeY;
	int lastChangeZ;

	unsigned int smoothCoef;

	int indexMesh;

	bool clear;

	int curvRatio1, curvRatio2;

public slots:
    void subdivide          (void);
    void lapSm1             (void);
    void lapSm10            (void);
    void lapSm100           (void);
    void changeLapSmValue   (int);
//    void clear(void);

	void setShowMesh		(const bool b)  { mesh_enabled = b; updateGL(); }
    void setShowFlatMesh	(const bool b)  { flat_mesh_enabled = b; if (b) {buildFlatMesh(); updateGL();} }
    void setShowSmoothMesh	(const bool b)  { smooth_mesh_enabled = b; if (b) {buildSmoothMesh(); updateGL();} }
    void setShowEdgedMesh	(const bool b)  { edged_mesh_enabled = b; if (b) {buildEdgedMesh(); updateGL();} }
    void setShowCulledMesh	(const bool b)  { culled_mesh_enabled = b; if (b) {buildCulledMesh(); updateGL();} }
    void setShowCurvMMesh	(const bool b)  { curvM_mesh_enabled = b; if (b) {buildCurvMMesh(); updateGL();} }
    void setShowCurvGMesh	(const bool b)  { curvG_mesh_enabled = b; if (b) {buildCurvGMesh(); updateGL();} }
    void setShowHeightMesh	(const bool b)  { height_mesh_enabled = b; if (b) {buildHeightMesh(); updateGL();} }
    void setShowFeatureLines(const bool b)  { feature_lines_enabled = b; if (b) {buildFeatureLines();} updateGL(); }
    void setShowIPMesh		(const bool b);
	void setShowTriang		(const bool b)  { triang_enabled = b; updateAll(); }
	void setShowTriang2		(const bool b)  { triang2_enabled = b; updateAll(); }
    void setShowCtrl		(const bool b);
    void setShowOld         (const bool b);
    void setShowShadedCtrl	(const bool b)  { shaded_ctrl_enabled = b; if (b) {buildCtrl(); updateGL();} }
    void setShowEdgedCtrl	(const bool b)  { edged_ctrl_enabled = b; if (b) {buildCtrl(); updateGL();} }
    void setShowCulledCtrl	(const bool b)  { culled_ctrl_enabled = b; if (b) {buildCtrl(); updateGL();} }
    void setShowLine        (const bool b)  { line_enabled = b; if (b) {buildLine();} updateGL();}
    void setCC              (const bool b);
    void setCCB             (const bool b);
    void setICC             (const bool b);
    void setCF              (const bool b);
    void setLAP             (const bool b);

	void setProbeOnCtrl(const bool b);

	void setTransf(const bool b) { transf = b; }

	void setClear(const bool b) { clear = b; }

	void setShowFrame		(const bool b)  { frame_enabled = b; buildFrame(); updateGL(); }
//    void setSubdivLevel		(unsigned int newLevel);
	void setSubdivLevel		(int newLevel);
	void setClr				(int newColor);
	void setIndMesh			(int newInd);
	void setPoint			(int index);
	void movePointX			(int change);
	void movePointY			(int change);
	void movePointZ			(int change);
	void changeSmoothing	(int change);
//	void moveRayPoint(unsigned int i, unsigned int j, unsigned int k);
	void buildAll			(void);
	void updateAll			(void);
    void changeStripeDensity(int newDensity);
	void changeCurvRatio1	(int newRatio);
	void changeCurvRatio2	(int newRatio);

signals:
	void subdivLevelChanged(int newLevel);
	void rightClicked();
	void middleClicked();
	void callBar(int clr);
	void poiChanged();
	void indexChanged(int);
	void updateNeeded();
    void openFile       (const char *);

protected:
	void paintGL(void);
	void mousePressEvent    (QMouseEvent *event);

    void drawMesh(DrawMeshType type, Mesh *mesh, unsigned int index, unsigned int ctrlType);
    void drawFeatureLines(Mesh *mesh);
//	void drawCurv(bool drawMean);
	void drawPoi();
	void drawPoiSub();
    void drawPoiBound();
	void drawFrame();
    void drawLine();

private:
	bool mesh_enabled;
    bool flat_mesh_enabled;
    bool smooth_mesh_enabled;
	bool culled_mesh_enabled;
	bool edged_mesh_enabled;
	bool curvM_mesh_enabled;
	bool curvG_mesh_enabled;
	bool curvTG_mesh_enabled;
    bool height_mesh_enabled;
    bool feature_lines_enabled;
	bool IP_mesh_enabled;
//	bool smooth_enabled;
	bool triang_enabled;
	bool triang2_enabled;
	bool ctrl_enabled;
    bool old_enabled;
	bool shaded_ctrl_enabled;
	bool edged_ctrl_enabled;
	bool culled_ctrl_enabled;
	bool frame_enabled;
    bool line_enabled;

    enum SubdivType
    {
        CC, CCB, ICC, CF, LAP
    };

    SubdivType subType;

	bool probeOnCtrl;
	bool transf;

    int stripeDensityLevel;
    int lapSmValue;

    unsigned int flat_mesh_list;
    unsigned int smooth_mesh_list;
	unsigned int edged_mesh_list;
	unsigned int culled_mesh_list;
	unsigned int curvM_mesh_list;
	unsigned int curvG_mesh_list;
	unsigned int curvTG_mesh_list;
    unsigned int height_mesh_list;
    unsigned int feature_lines_list;
	unsigned int IP_mesh_list;
	unsigned int frame_list;
	unsigned int poi_list;
	unsigned int poiSub_list;
    unsigned int poiBound_list;
	unsigned int ctrl_list;
    unsigned int old_list;
    unsigned int line_list;
};

#endif
