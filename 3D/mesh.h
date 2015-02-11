#ifndef MESH_H
#define MESH_H

#include <vector>
#include <list>
#include <string>
#include "3D/point_3d.h"

// I ASSUME CLOCK-WISE ORIENTATION OF VERTICES WITHIN FACETS!

class MeshFacet;

class MeshVertex
{
public:
	MeshVertex() { ; }
   ~MeshVertex() { ; }

    Point_3D 		my_point,
                    my_Vpoi;

	unsigned int 	my_index,
					my_valency;

    bool			isOnBoundary, treatAsCornerVertex,
                    isFeature,
                    isStable;

	std::vector <unsigned int> my_faceIndices;
	std::vector <MeshFacet*>   my_facets;

    std::vector<Point_3D>      my_fakeD; // fake Ds for Int. C-C
    std::vector<unsigned int>  my_n;     // incident facet valencies - 3

	PointPrec		my_curvM,
					my_curvG,
					my_curvMsmooth,
					my_curvGsmooth;

	float			my_normalFlat[3],
					my_normalSmooth[3];

	void coutV(void);
};

class MeshCorner
{
public:
    MeshCorner() { ; }
   ~MeshCorner() { ; }

    unsigned int	 my_index;
    unsigned int	 my_vIndex;
    unsigned int	 my_nIndex;
    MeshVertex 		*my_vertex;
    MeshCorner		*my_nextCorner, *my_pairedCorner;
    MeshFacet		*my_facet;
    MeshFacet		*my_nextFacet;
    unsigned int     my_nextFshift; // index shift in next face to get the same-1 vertex

    Point_3D		 my_Epoi,
                     my_edgeContr; // for Int. C-C

    unsigned int	my_newEindex; //index of the new edge-vertex

    unsigned int    my_posi, my_posj, my_posa, my_posb;
};

class MeshFacet
{
public:
    MeshFacet() { drawMe = true; }
   ~MeshFacet() { ; }

	unsigned int 	my_index,
                    my_valency;

    std::vector <unsigned int> my_vertIndices;
	std::vector <MeshCorner>   my_corners;

    std::vector<Point_3D>      my_faceContr; // facet contributions from vertices

	float			my_normalFlat[3],
					my_normalSmooth[3];

    Point_3D		my_Fpoi;

    bool			drawMe;

	void coutF(void);
};

class Mesh
{
public:
    Mesh() {}
   ~Mesh() { DeleteData(); }
	void DeleteData	(void);
    void load		(std::istream &is, unsigned int iH);
	void save		(const char *fileName, bool isPly);
	void transf		(void);
	void build		(void);

	void computeNormalsFlat(void);
	void computeNormalsSmooth(unsigned int rings);

	void calculateNormal(float *norm, float p0[3],
						 float p1[3], float p2[3]);

    void getEdgeConnectedV(MeshVertex *vert, std::vector< MeshVertex* > *Vs);

    void CatmullClark(Mesh *mesh);

    void times(const char *which);

    MeshCorner* getNextCorner(MeshCorner *corner);
    MeshCorner* getSameCorner(MeshCorner *corner);
    MeshCorner* getPrevCorner(MeshCorner *corner);
    MeshCorner* findCorner(MeshVertex *vertex, MeshFacet *facet);

	unsigned int my_numV;
	unsigned int my_numF;
	unsigned int my_numE;

    float colFlat[4];
    Point_3D colBlend;
    bool isGhost;

	int my_degree;

	unsigned int	my_level;
	std::string		my_s,
					my_save;

	unsigned int	my_rand;

	const char 		*my_file;

	PointPrec		my_scale;
	Point_3D		my_centre;

	Point_3D		my_EV,
					my_newEV;

    PointPrec		my_minz,
                    my_maxz;

	bool			rever,
                    transform;

    bool            quadMesh, hasBoundary;

	std::vector< std::vector< std::vector < unsigned int > > >	map;

	std::vector <MeshVertex>	my_vertices;
	std::vector <MeshFacet>  	my_facets;

	std::vector <MeshCorner*>	my_boundaryCorners;
};

#endif
