#include <fstream>
#include "mesh.h"
#include <set>
#include <cmath>
#include <complex>
#include <stdlib.h>
#include <assert.h>
#include <algorithm> // for reverse();
#include <string>
#include <omp.h>
//#include <stdio.h>

#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>


using namespace std;

void MeshVertex::coutV(void)
{
	unsigned int i;

    cout << "-----------------------------" << endl;
	cout << "Index: " << my_index << ", Val: " << my_valency << ": ";
	for (i = 0 ; i < my_valency ; i++)
	{
		cout << my_faceIndices[i] << " ";
	}
    cout << endl;
    cout << "x, y, z: " << my_point;
    cout << "CurvMfl: " << my_curvM << ", CurvGfl: " << my_curvG << endl;
    cout << "CurvMsm: " << my_curvMsmooth << ", CurvGsm: " << my_curvGsmooth << endl;
}

void MeshFacet::coutF(void)
{
	unsigned int i;

	cout << my_index << " " << my_valency << " ";
	for (i = 0 ; i < my_valency ; i++)
	{
		cout << my_vertIndices[i] << " ";
	}
    cout << endl;
}

void Mesh::DeleteData()
{
    unsigned int    i;

    map.clear();
    my_vertices.clear();

    // Is it necessary to delete corners of facets?
    for (i = 0 ; i < my_facets.size() ; i++)
    {
        my_facets[i].my_corners.clear();
    }

    my_facets.clear();
//    my_subdFacets.clear();
    my_rays.clear();
    my_insRays.clear();
    my_insMaxRays.clear();

    my_boundaryCorners.clear();

//    my_EVlist.clear();


//    cout << "Mesh data deleted!" << endl;
}

void Mesh::load(const char *fileName)
{
	unsigned int	i, j, vn, n;
	PointPrec		x, y, z;
    std::string		file_type, tmp;
	MeshVertex 		vertex;
	MeshFacet 		facet;
	ifstream 		file (fileName);
	MeshCorner		corner;

//	unsigned int	m, max;

	std::stringstream 	sstm, sstmsave;

	if(!file.is_open())
	{
        cout<< "Failed to find the file:" << fileName << endl;
	}
	else
	{
		my_level = 0;

		file >> file_type;
//		DeleteData();

        if (file_type == "OFF" || file_type == "off" || file_type == "Off")
        {
            file >> my_numV >> my_numF >> my_numE;

            if (my_numE != 0)
            {
                rever = true;
            }
            else
            {
                rever = false;
    cout << "Reverse needed..." << endl;
            }
        }
        else if (file_type == "PLY" || file_type == "ply" || file_type == "Ply")
        {
            do
            {
                file >> tmp;
            }
            while (tmp != "format");
            file >> tmp;
            if (tmp != "ascii")
            {
                cout << "Cannot read binary files!!!" << endl;
                assert(false);
            }

            do
            {
                file >> tmp;
            }
            while (tmp != "vertex");

            file >> my_numV;

            do
            {
                file >> tmp;
            }
            while (tmp != "face");

            file >> my_numF;

            do
            {
                file >> tmp;
            }
            while (tmp != "end_header");
        }
        else
        {
            cout << "Unsupported file type!" << endl;
            assert(false);
        }

		// read vertex coordinates
		for (i = 0 ; i < my_numV ; i++)
		{
			file >> x >> y >> z;
            getline(file, tmp);
			vertex.my_point.setX(x);
			vertex.my_point.setY(y);
            vertex.my_point.setZ(z);
            if (z != 0)
            {
                vertex.isStable = true;
            }
            else
            {
                vertex.isStable = false;
            }
			vertex.my_index = i;
            vertex.isFeature = true;
			my_vertices.push_back(vertex);
			my_vertices[i].my_faceIndices.clear();
		}

		// read facets
        quadMesh = true;
		for (i = 0 ; i < my_numF ; i++)
		{
			facet.my_index = i;
			facet.my_vertIndices.clear();
			file >> n;
			facet.my_valency = n;
            if (n != 4)
            {
                quadMesh = false;
            }
			for (j = 0 ; j < n ; j++)
			{
				file >> vn;
				facet.my_vertIndices.push_back(vn);
				my_vertices[vn].my_faceIndices.push_back(i);
			}
			my_facets.push_back(facet);
		}

//        for (i = 0 ; i < my_numV ; i++)
//        {
//            if (my_vertices[i].my_faceIndices.size() < 3)
//            {
//                cout << "A low-valent vertex present!!!" << endl;
//            }
//        }

//        //reverse orientation of faces (Tom's code uses reversed orient. compared to mine)
//        if (rever)
//        {
//            for (i = 0 ; i < my_facets.size() ; i ++)
//            {
//                fac = &(my_facets[i]);
//                reverse(fac->my_vertIndices.begin(), fac->my_vertIndices.end());
//            }
//        }

		my_s = fileName;
		my_save = fileName;

//        transf();
        build();

		cout << "Mesh loaded! V: " << my_numV << " F: " << my_numF
             << " 2E: " << my_numE << endl;
	} //end of else

	file.close();
}

void Mesh::save(const char *fileName, bool isPly)
{
    unsigned int	i, j;
    std::string		file_type;
    MeshVertex 		vertex;
    MeshFacet 		facet;
    ofstream 		file (fileName);

    if(!file.is_open())
    {
        cout<< "Failed to find the file:" << fileName << endl;
    }
    else
    {
        if (isPly)
        {
            file << "ply" << endl;
            file << "format ascii 1.0" << endl;
            file << "element vertex " << my_numV << endl;
            file << "property float32 x" << endl;
            file << "property float32 y" << endl;
            file << "property float32 z" << endl;
            file << "element face " << my_numF << endl;
            file << "property list uint8 int32 vertex_index" << endl;
            file << "end_header" << endl;
        }
        else
        {
            file << "OFF" << endl;
//	        file << my_numV << " " << my_numF << " " << my_numE << "\n" << endl;
            file << my_numV << " " << my_numF << " " << 0 << "" << endl;
        }
        for (i = 0 ; i < my_numV ; i++)
        {
            file << my_vertices[i].my_point;
        }

        if (!isPly) file << endl;

        // write facets
        for (i = 0 ; i < my_numF ; i++)
        {
            file << my_facets[i].my_valency;

            for (j = 0 ; j < my_facets[i].my_vertIndices.size() ; j++)
            {
                file << " " << my_facets[i].my_vertIndices[j];
            }
            file << endl;
        }

        cout << "Mesh saved" << endl;
    }
    file.close();
}

void Mesh::transf(void)
{
	unsigned int 		i;
	Point_3D 			cen_point, poi;
	PointPrec 			x, y, z, minx, miny, minz, maxx, maxy, maxz;
	PointPrec 			ratio, threshold, rng, scl;

	ratio = 1;
	threshold = 1.0e10;

	minx = threshold;
	maxx = -threshold;
	miny = threshold;
	maxy = -threshold;
	minz = threshold;
	maxz = -threshold;

	for(i = 0 ; i < my_numV ; i++)
	{
		poi = my_vertices[i].my_point;
		x = poi.getX();
		y = poi.getY();
		z = poi.getZ();
		if (x < minx) minx = x;
		if (x > maxx) maxx = x;
		if (y < miny) miny = y;
		if (y > maxy) maxy = y;
		if (z < minz) minz = z;
		if (z > maxz) maxz = z;
	}

	cen_point.setX((maxx + minx) / 2.0);
	cen_point.setY((maxy + miny) / 2.0);
	cen_point.setZ((maxz + minz) / 2.0);

	rng = maxx - minx;
	if (maxy - miny > rng) rng = maxy - miny;
	if (maxz - minz > rng) rng = maxz - minz;
	scl = ratio / rng;

	my_centre = cen_point;
	my_scale = scl;

    my_minz = minz;
    my_maxz = maxz;


	cout << "cen: " << my_centre;
    cout << "scale: " << my_scale << endl;
}

void Mesh::build(void)
{
    unsigned int 	i, j, k, l, ind1, ind2, shift;
    MeshCorner		corner, *crn, *crnn;
    MeshFacet		*facet, *adjFacet, *nextFacet;
    bool			search;
    MeshVertex		*vert;

    cout << "Building - Start" << endl;

    times("");

    my_numE = 0;

    hasBoundary = false;

    // fill vertex->facet pointers
//    my_EVlist.clear();

    #pragma omp parallel for default(none) private(i,j,vert,facet)
    for (i = 0 ; i < my_numV ; i++)
    {
        vert = &my_vertices[i];
        vert->isOnBoundary = false; // init all to false
        vert->my_valency = vert->my_faceIndices.size();
        if (vert->my_valency == 1) // fix for vertices of valency 1
        {
            vert->isOnBoundary = true;
        }
        vert->my_facets.clear();
        for (j = 0 ; j < vert->my_valency ; j++)
        {
//            vert->my_facets.push_back(&my_facets[vert->my_faceIndices[j]]);
            facet = &my_facets[vert->my_faceIndices[j]];
            vert->my_facets.push_back(facet);
        }
//        if (vert->my_valency > 2 && vert->my_valency != 4)
//        {
//            my_EVlist.push_back(vert);
//        }
    }

    // find corners

    #pragma omp parallel for default(none) private(i,j,facet,corner)
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        facet->my_corners.clear();
        my_numE += facet->my_valency;
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            corner.my_vIndex = facet->my_vertIndices[j];
            corner.my_vertex = &(my_vertices[corner.my_vIndex]);
            corner.my_facet = facet;
            corner.my_nIndex = facet->my_vertIndices[(j + 1) % facet->my_valency];
            facet->my_corners.push_back(corner);
        }
    }

    // find next corners

    #pragma omp parallel for default(none) private(i,j,facet)
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            facet->my_corners[j].my_nextCorner = &(facet->my_corners[(j + 1) % facet->my_valency]);
        }
    }

    // find next facets

    #pragma omp parallel for default(none) private(i,j,facet,crn,ind1,ind2,k,search,nextFacet,adjFacet,crnn,l,shift)
    for (i = 0 ; i < my_numF ; i++)
    {
//cout << "Face " << i << " has corners: " << endl;
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            ind1 = crn->my_vIndex;
            ind2 = crn->my_nIndex;

            // the following search returns only one face, thus may give wrong
            // answer if two-sided faces are present!!!
            k = 0;
            search = true;
            nextFacet = NULL;
            while (search && k < crn->my_vertex->my_valency)
            {
                adjFacet = crn->my_vertex->my_facets[k];
                crnn = &adjFacet->my_corners[0];
                l = 0;
                while (search && l < adjFacet->my_valency)
                {
//cout << "l " << l << " k " << k << endl;
//cout << crnn->my_vIndex << " " << crnn->my_nIndex << endl;
                    if (crnn->my_vIndex == ind2 && crnn->my_nIndex == ind1)
                    {
                        search = false;
                        nextFacet = adjFacet;
                        shift = l;
                    }
                    crnn = crnn->my_nextCorner;
                    l++;
                }
                k++;
            }
            if (nextFacet == NULL)
            {
//				cout << "Next face not found - boundary vertex!" << endl;
//				cout << crn->my_vertex->my_index << endl;
                crn->my_vertex->isOnBoundary = true;
                hasBoundary = true;
                crn->my_nextFacet = NULL;
                crn->my_nextFshift = 0;
                crn->my_pairedCorner = NULL;
//cout << crn->my_vIndex << " -> NULL" << endl;
            }
            else
            {
//				cout << "Next face found!" << endl;
                crn->my_nextFacet = nextFacet;
                crn->my_nextFshift = shift;
                crn->my_pairedCorner = &(nextFacet->my_corners[shift]);
//cout << crn->my_vIndex << " -> " << crn->my_nextFacet->my_index << " with shift " << shift << endl;
            }
        }
//cout << endl;
    }
//cout << "-------------------------" << endl;

    times("Building ");

    cout << "Building - End" << endl;
}

// returns the previous corner, but in the 'next' facet
MeshCorner* Mesh::getPrevCorner(MeshCorner *corner)
{
    MeshCorner *next;

    if (corner->my_nextFacet == NULL)
    {
        return NULL;
    }
    else
    {
        next = &corner->my_nextFacet->my_corners[corner->my_nextFshift];
        return next;
    }
}

// returns the same corner, but in the 'next' facet
MeshCorner* Mesh::getSameCorner(MeshCorner *corner)
{
    MeshCorner *next;

    if (corner->my_nextFacet == NULL)
    {
        return NULL;
    }
    else
    {
        next = &corner->my_nextFacet->my_corners[corner->my_nextFshift];
        next = next->my_nextCorner;
        return next;
    }
}

// returns the next corner, but in the 'next' facet
MeshCorner* Mesh::getNextCorner(MeshCorner *corner)
{
    MeshCorner *next;

    if (corner->my_nextFacet == NULL)
    {
        return NULL;
    }
    else
    {
        next = &corner->my_nextFacet->my_corners[corner->my_nextFshift];
        next = next->my_nextCorner->my_nextCorner;
        return next;
    }
}

void Mesh::compCurv(void)
{
	unsigned int 		i, j, ind, ind1, ind2, k, k1, k2, val1, val2;
	MeshVertex			*vertex;
	MeshFacet			*facet1, *facet2, *facet;
	Point_3D			poi11, poi12, poi13, poi21, poi22, poi23, vec;
	PointPrec			side11, side12, side13, m1, angle1,
						side21, side22, side23, m2, angle2,
                        sum, sumM, sumG, vor, minM, maxM, minG, maxG, mm, ang, angle;

	PointPrec PI = 4 * atan(1);

//cout << "Computing Mean curvature... " << endl;

	for (i = 0 ; i < my_numV ; i++)
	{
		sum = 0;
		vec = Point_3D(0,0,0);
		vor = 0;
//		mn = 0;
		vertex = &(my_vertices[i]);
		if (!(vertex->isOnBoundary))
		{
			ind = vertex->my_index;
			for (j = 0 ; j < vertex->my_valency ; j++)
			{
				facet1 = vertex->my_facets[j];
				facet2 = vertex->my_facets[(j + 1) % vertex->my_valency];

				val1 = facet1->my_valency;
				k1 = 0;
				ind1 = facet1->my_vertIndices[k1];
				while (ind1 != ind)
				{
					k1++;
					ind1 = facet1->my_vertIndices[k1];
				}
				val2 = facet2->my_valency;
				k2 = 0;

				ind2 = facet2->my_vertIndices[k2];
				while (ind2 != ind)
				{
					k2++;
					ind2 = facet2->my_vertIndices[k2];
				}

				// three consequtive points
				poi13 = my_vertices[facet1->my_vertIndices[(k1 + val1 - 1) % val1]].my_point;
				poi12 = vertex->my_point;
//				poi12 = my_vertices[facet1->my_vertIndices[k1]].my_point;
				poi11 = my_vertices[facet1->my_vertIndices[(k1 + 1) % val1]].my_point;

				poi23 = my_vertices[facet2->my_vertIndices[(k2 + val2 - 1) % val2]].my_point;
				poi22 = vertex->my_point;
//				poi22 = my_vertices[facet2->my_vertIndices[k2]].my_point;
				poi21 = my_vertices[facet2->my_vertIndices[(k2 + 1) % val2]].my_point;

				side11 = poi11.dist(poi12);
				side12 = poi12.dist(poi13);
				side13 = poi11.dist(poi13);

				side21 = poi21.dist(poi22);
				side22 = poi22.dist(poi23);
				side23 = poi21.dist(poi23);

//cout << side11 - side22 << endl;

				m1 = ((side11 * side11 - side12 * side12 - side13 * side13) / (-2.0 * side12 * side13));

//if (m1 < 0) cout << "m1 is negative" << endl;

				angle1 = acos(m1);

				m2 = ((side22 * side22 - side21 * side21 - side23 * side23) / (-2.0 * side21 * side23));
				angle2 = acos(m2);


				mm = ((side12 * side12 - side11 * side11 - side13 * side13) / (-2.0 * side11 * side13));
				ang = acos(mm);
//cout << mm << " " << ang << endl;

				angle = (PI - angle1 - ang) * (180 / PI);


				sum += angle;

//cout << sum << endl;

				vec = (1.0 / tan(angle1) + 1.0 / tan(angle2)) * poi12 + vec;
				vec = (- 1.0 / tan(angle1) - 1.0 / tan(angle2)) * poi11 + vec;

//if (tan(angle1) < 0) cout << "tan(angle1) is negative" << endl;

//				mn = (1.0 / tan(angle1) + 1.0 / tan(angle2)) * side11;

//cout << (1.0 / tan(angle1) + 1.0 / tan(angle2)) << endl;

//cout << angle1 * 180 / PI << " " << ang * 180 / PI << endl;

				if (angle1 * 180 / PI >= 90 || ang * 180 / PI >= 90 || angle >= 90)
				{
					if 	(angle >= 90)
					{
						vor += 1.0 / 2.0 * side12 * side13 * sin(angle1) / 2;
					}
					else
					{
						vor += 1.0 / 2.0 * side12 * side13 * sin(angle1) / 4;
					}
				}
				else
				{
					vor += 1.0 / 8.0 * (side12 * side12 / tan(ang) + side11 * side11 / tan(angle1));
				}
			}
//            vertex->my_curvM = vec.dist(Point_3D(0,0,0));// / 4.0 / vor;
//            vertex->my_curvM = mn / 2.0 / vor;
//            vertex->my_curvM = mn / vor;

			vertex->my_curvM = vor;

			// correction for boundary corners and edges

//			vor = 1.0;
//			if (vertex->my_valency == 1)
//			{
//				vertex->my_curvG = (90 - sum) / vor;
//			}
//			else if (vertex->my_valency == 2)
//			{
//				vertex->my_curvG = (180 - sum) / vor;
//			}
//			else
//			{
//				vertex->my_curvG = (360 - sum) / vor;
//			}

//            vertex->my_curvG = vor;
			vertex->my_curvG = (360 - sum) / vor;
		}
		else
		{
//cout << "Boundary " << i << endl;
			vertex->my_curvM = 0; // or other value for boundary points?
			vertex->my_curvG = 0;

//			for (j = 0 ; j < vertex->my_faceIndices.size() ; j++)
//			{
//				cout << vertex->my_faceIndices[j] << " ";
//			}
//			cout << endl;

		}

//cout << "G: " << vertex->my_curvG << endl;
//cout << "v: " << 10000 * vor << endl;

	}

	minM = 100000000000;
	maxM = -minM;
	minG = 100000000000;
	maxG = -minG;

	// average vertex curvatures and assign to facets
	for (i = 0 ; i < my_numF ; i++)
	{
		sumM = 0;
		sumG = 0;
		k = 0;
		facet = &(my_facets[i]);
		for (j = 0 ; j < facet->my_valency ; j++)
		{
			if (!my_vertices[facet->my_vertIndices[j]].isOnBoundary)
			{
				sumM += my_vertices[facet->my_vertIndices[j]].my_curvM;
				sumG += my_vertices[facet->my_vertIndices[j]].my_curvG;
				k++;
			}
		}
		if (k == 0 ) k = 1;
		sumM = sumM / k;
		sumG = sumG / k;
		if (sumM > maxM)
		{
			maxM = sumM;
		}
		if (sumM < minM)
		{
			minM = sumM;
		}
		if (sumG > maxG)
		{
			maxG = sumG;
		}
		if (sumG < minG)
		{
			minG = sumG;
		}
		facet->my_curvM = sumM;
		facet->my_curvG = sumG;
	}
	my_minM = minM;
	my_maxM = maxM;
	my_minG = minG;
	my_maxG = maxG;

cout << "curvM: " << minM << " -- " << maxM << " curvG: " << minG << " -- " << maxG << endl;

//cout << "DONE Computing curvature... " << endl;
}

void Mesh::compCurvSmooth(unsigned int rings)
{
	unsigned int 	i, j, k, count;
	MeshFacet		*facet;
	MeshVertex		*vertex, *ver;
	PointPrec		m, g;

	std::vector< std::vector <unsigned int> > ring_facets;
	std::vector< std::vector <unsigned int> > ring_vertices;

	// smooth curvature for vertices
	for (i = 0 ; i < my_numV ; i++)
	{
		vertex = &(my_vertices[i]);

		if (rings == 0)
		{
			vertex->my_curvMsmooth = vertex->my_curvM;
			vertex->my_curvGsmooth = vertex->my_curvG;
		}
		else
		{
			m = 0;
			g = 0;
			count = 0;
			getRingsV(vertex, rings, &ring_facets, &ring_vertices);
			for (j = 0 ; j < ring_vertices.size() ; j++)
			{
				for (k = 0 ; k < ring_vertices[j].size() ; k++)
				{
					ver = &(my_vertices[ring_vertices[j][k]]);
					m += ver->my_curvM;
					g += ver->my_curvG;
					count++;
				}
			}
			vertex->my_curvMsmooth = m / count;
			vertex->my_curvGsmooth = g / count;
		}
	}

	// smooth curvature for facets
	for (i = 0 ; i < my_numF ; i++)
	{
		facet = &(my_facets[i]);
		m = 0;
		g = 0;
		count = facet->my_valency;
		for (j = 0 ; j < count ; j++)
		{
			vertex = &(my_vertices[facet->my_vertIndices[j]]);
			m += vertex->my_curvMsmooth;
			g += vertex->my_curvGsmooth;
		}
		facet->my_curvMsmooth = m / count;
		facet->my_curvGsmooth = g / count;
	}
}

void Mesh::compCurvG(void)
{
    unsigned int 		i, j, ind, ind2, k, val;
    MeshVertex			*vertex;
    MeshFacet			*facet;
    Point_3D			poi1, poi2, poi3;
    PointPrec			side1, side2, side3, m, angle, sum, minG, maxG;

    PointPrec PI = 4 * atan(1);

//cout << "Computing Gaussian curvature... " << endl;

    for (i = 0 ; i < my_numV ; i++)
    {
        sum = 0;
        vertex = &(my_vertices[i]);
        ind = vertex->my_index;
        for (j = 0 ; j < vertex->my_valency ; j++)
        {
//cout << "val: " << vertex->my_valency << endl;
            facet = vertex->my_facets[j];
            val = facet->my_valency;
            k = 0;
            ind2 = facet->my_vertIndices[k];
            while (ind2 != ind)
            {
                k++;
                ind2 = facet->my_vertIndices[k];
            }
            // three consequtive points
            poi1 = my_vertices[facet->my_vertIndices[(k + val - 1) % val]].my_point;
            poi2 = vertex->my_point;
            poi3 = my_vertices[facet->my_vertIndices[(k + 1) % val]].my_point;

            side1 = poi1.dist(poi2);
            side2 = poi2.dist(poi3);
            side3 = poi1.dist(poi3);

            m = ((side3 * side3 - side2 * side2 - side1 * side1) / (-2.0 * side1 * side2));
            angle = acos(m);
            angle = angle * (180 / PI);

//cout << "Angle: " << angle << endl;

            sum += angle;
        }
        // correction for boundary corners and edges
        if (vertex->my_valency == 1)
        {
            vertex->my_curvG = 90 - sum;
        }
        else if (vertex->my_valency == 2)
        {
            vertex->my_curvG =180 - sum;
        }
        else
        {
            vertex->my_curvG = 360 - sum;
        }

//cout << vertex->my_valency << " curvG: " << vertex->my_curvG << endl;
    }

    minG = 100000;
    maxG = -minG;

    // average vertex curvatures and assign to facets
    for (i = 0 ; i < my_numF ; i++)
    {
        sum = 0;
        k = 0;
        facet = &(my_facets[i]);
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            if (!my_vertices[facet->my_vertIndices[j]].isOnBoundary)
            {
                sum += my_vertices[facet->my_vertIndices[j]].my_curvG;
                k++;
            }
        }
        sum = sum / k;
        if (sum > maxG)
        {
            maxG = sum;
        }
        if (sum < minG)
        {
            minG = sum;
        }
        facet->my_curvG = sum;
    }
    my_minG = minG;
    my_maxG = maxG;

//cout << "maxG: " << maxG << " minG: " << minG << endl;

//cout << "DONE Computing Gaussian curvature... " << endl;
}


void Mesh::computeNormalsFlat(void)
{
	unsigned int 	i, j;
	MeshFacet		*facet;
	Point_3D		p0, p1, p2;
	GLfloat			normal[3], n[3], q0[3], q1[3], q2[3];

	for (i = 0 ; i < my_numF ; i++)
	{
		facet = &(my_facets[i]);
		normal[0] = 0;
		normal[1] = 0;
		normal[2] = 0;
		for (j = 0 ; j < facet->my_valency ; j++) // I should simplify this computation
		{
			p0 = facet->my_corners[j].my_vertex->my_point;
			p1 = facet->my_corners[(j + 1) % facet->my_valency].my_vertex->my_point;
			p2 = facet->my_corners[(j + 2) % facet->my_valency].my_vertex->my_point;
			q0[0] = p0.getX();
			q0[1] = p0.getY();
			q0[2] = p0.getZ();
			q1[0] = p1.getX();
			q1[1] = p1.getY();
			q1[2] = p1.getZ();
			q2[0] = p2.getX();
			q2[1] = p2.getY();
			q2[2] = p2.getZ();
			calculateNormal(n, q0, q1, q2);
			normal[0] += n[0];
			normal[1] += n[1];
			normal[2] += n[2];
		}
		facet->my_normalFlat[0] = normal[0];
		facet->my_normalFlat[1] = normal[1];
		facet->my_normalFlat[2] = normal[2];
	}

//cout << "ComputeNormalsFlat called" << endl;

}

void Mesh::computeNormalsSmooth(unsigned int rings)
{
    unsigned int 	i, j;//, k;
    MeshFacet		*facet;//, *fc;
	MeshVertex		*vertex;
    GLfloat			//normal[3],
                    normal2[3], nextNormal[3], normA, normB, dot, dotThreshold;

    dotThreshold = 0.9;

//	std::vector< std::vector <unsigned int> > ring_facets;
//	std::vector< std::vector <unsigned int> > ring_vertices;

//	// smooth normals for facets
//	for (i = 0 ; i < my_numF ; i++)
//	{
//		facet = &(my_facets[i]);

//		if (rings == 0)
//		{
//			facet->my_normalSmooth[0] = facet->my_normalFlat[0];
//			facet->my_normalSmooth[1] = facet->my_normalFlat[1];
//			facet->my_normalSmooth[2] = facet->my_normalFlat[2];
//		}
//		else
//		{
//			normal[0] = 0;
//			normal[1] = 0;
//			normal[2] = 0;
//            getRingsF(facet, rings, &ring_facets, &ring_vertices);
//			for (j = 0 ; j < ring_facets.size() ; j++)
//			{
//				for (k = 0 ; k < ring_facets[j].size() ; k++)
//				{
//					fc = &(my_facets[ring_facets[j][k]]);
//					normal[0] += fc->my_normalFlat[0];
//					normal[1] += fc->my_normalFlat[1];
//					normal[2] += fc->my_normalFlat[2];
//				}
//			}
////			// change contribution of the original facet
////			normal[0] += 0 * facet->my_normalFlat[0];
////			normal[1] += 0 * facet->my_normalFlat[1];
////			normal[2] += 0 * facet->my_normalFlat[2];

//			facet->my_normalSmooth[0] = normal[0];
//			facet->my_normalSmooth[1] = normal[1];
//			facet->my_normalSmooth[2] = normal[2];

////cout << "Flat: " << facet->my_normalFlat[0] << " " << facet->my_normalFlat[1] << " " << facet->my_normalFlat[2] << endl;
////cout << "Smth: " << facet->my_normalSmooth[0] << " " << facet->my_normalSmooth[1] << " " << facet->my_normalSmooth[2] << endl;

//		}
//	}

	// smooth normals for vertices
	for (i = 0 ; i < my_numV ; i++)
	{
		vertex = &(my_vertices[i]);
//		normal[0] = 0;
//		normal[1] = 0;
//		normal[2] = 0;
		normal2[0] = 0;
		normal2[1] = 0;
		normal2[2] = 0;
		for (j = 0 ; j < vertex->my_valency ; j++)
		{
			facet = vertex->my_facets[j];
//			normal[0] += facet->my_normalSmooth[0];
//			normal[1] += facet->my_normalSmooth[1];
//			normal[2] += facet->my_normalSmooth[2];

            nextNormal[0] = facet->my_normalFlat[0];
            nextNormal[1] = facet->my_normalFlat[1];
            nextNormal[2] = facet->my_normalFlat[2];

//            // check for sharp creases; do not average normals across them
//            normA = sqrt(nextNormal[0] * nextNormal[0] + nextNormal[1] * nextNormal[1] + nextNormal[2] * nextNormal[2]);
//            normB = sqrt(normal2[0] * normal2[0] + normal2[1] * normal2[1] + normal2[2] * normal2[2]);

//            if ((normA == 0) || (normB == 0))
//            {
                normal2[0] += facet->my_normalFlat[0];
                normal2[1] += facet->my_normalFlat[1];
                normal2[2] += facet->my_normalFlat[2];
//            }
//            else
//            {
//                dot = (nextNormal[0] * normal2[0] + normal2[1] * normal2[1] + nextNormal[2] * normal2[2]) / normA / normB;
//                if (dot > dotThreshold)
//                {
//                    normal2[0] += facet->my_normalFlat[0];
//                    normal2[1] += facet->my_normalFlat[1];
//                    normal2[2] += facet->my_normalFlat[2];
//                }

//            }
		}
//		vertex->my_normalSmooth[0] = normal[0];
//		vertex->my_normalSmooth[1] = normal[1];
//		vertex->my_normalSmooth[2] = normal[2];
//		vertex->my_normalFlat[0] = normal2[0];
//		vertex->my_normalFlat[1] = normal2[1];
//		vertex->my_normalFlat[2] = normal2[2];

        vertex->my_normalSmooth[0] = normal2[0];
        vertex->my_normalSmooth[1] = normal2[1];
        vertex->my_normalSmooth[2] = normal2[2];
	}
}

void Mesh::calculateNormal(GLfloat *norm, GLfloat p0[3], GLfloat p1[3], GLfloat p2[3])
{
	unsigned int i;
	GLfloat v1[3], v2[3];

	for(i = 0 ; i < 3 ; i++)
	{
		v1[i] = p1[i] - p0[i];
		v2[i] = p2[i] - p0[i];
	}

	norm[0] = v1[1]*v2[2] - v1[2]*v2[1];
	norm[1] = v1[2]*v2[0] - v1[0]*v2[2];
	norm[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

void Mesh::getRingsF(MeshFacet *fac, unsigned int r,
					std::vector< std::vector <unsigned int> > *ring_facets,
					std::vector< std::vector <unsigned int> > *ring_vertices)
{
	MeshVertex							*vertex;
	MeshFacet							*facet;
	unsigned int 						i, j;
	std::vector<unsigned int>			facets;
	std::vector<unsigned int>			vertices;
	std::set<unsigned int>				set_facets;
	std::set<unsigned int>				set_vertices;
	std::set<unsigned int>::iterator	it;
	std::pair<std::set<unsigned int>::iterator, bool> ret;

	ring_facets->clear();
	ring_vertices->clear();
	facets.clear();
	vertices.clear();
	set_facets.clear();
	set_vertices.clear();

	if (r > 0)
	{
		set_facets.insert(fac->my_index);
		facets.push_back(fac->my_index);
		ring_facets->push_back(facets);
		for (i = 0 ; i < r ; i++)
		{
			vertices.clear();
			facets.clear();
			//insert vertices
			for (it = set_facets.begin() ; it != set_facets.end() ; it++)
			{
				facet = &(my_facets[*it]);
				for (j = 0 ; j < facet->my_valency ; j++)
				{
					ret = set_vertices.insert(facet->my_vertIndices[j]);
					if (ret.second)
					{
						vertices.push_back(facet->my_vertIndices[j]);
					}
				}
			}
			ring_vertices->push_back(vertices);
			// insert facets
			for (it = set_vertices.begin() ; it != set_vertices.end() ; it++)
			{
				vertex = &(my_vertices[*it]);
				for (j = 0 ; j < vertex->my_valency ; j++)
				{
					ret = set_facets.insert(vertex->my_faceIndices[j]);
					if (ret.second)
					{
						facets.push_back(vertex->my_faceIndices[j]);
					}
				}
			}
			ring_facets->push_back(facets);
		}
	}
}

void Mesh::getRingsV(MeshVertex *vert, unsigned int r,
					std::vector< std::vector <unsigned int> > *ring_facets,
					std::vector< std::vector <unsigned int> > *ring_vertices)
{
	MeshVertex							*vertex;
	MeshFacet							*facet;
	unsigned int 						i, j;
	std::vector<unsigned int>			facets;
	std::vector<unsigned int>			vertices;
	std::set<unsigned int>				set_facets;
	std::set<unsigned int>				set_vertices;
	std::set<unsigned int>::iterator	it;
	std::pair<std::set<unsigned int>::iterator, bool> ret;

	ring_facets->clear();
	ring_vertices->clear();
	facets.clear();
	vertices.clear();
	set_facets.clear();
	set_vertices.clear();

	if (r > 0)
	{
		set_vertices.insert(vert->my_index);
		vertices.push_back(vert->my_index);
		ring_vertices->push_back(vertices);
		for (i = 0 ; i < r ; i++)
		{
			vertices.clear();
			facets.clear();
			// insert facets
			for (it = set_vertices.begin() ; it != set_vertices.end() ; it++)
			{
				vertex = &(my_vertices[*it]);
				for (j = 0 ; j < vertex->my_valency ; j++)
				{
					ret = set_facets.insert(vertex->my_faceIndices[j]);
					if (ret.second)
					{
						facets.push_back(vertex->my_faceIndices[j]);
					}
				}
			}
			ring_facets->push_back(facets);
			//insert vertices
			for (it = set_facets.begin() ; it != set_facets.end() ; it++)
			{
				facet = &(my_facets[*it]);
				for (j = 0 ; j < facet->my_valency ; j++)
				{
					ret = set_vertices.insert(facet->my_vertIndices[j]);
					if (ret.second)
					{
						vertices.push_back(facet->my_vertIndices[j]);
					}
				}
			}
			ring_vertices->push_back(vertices);
		}
	}
}

//Catmull-Clark subdivision
void Mesh::CatmullClark(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt, num, numCorners;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert, *prev0, *prev1, *next0, *next1;
    MeshCorner					*cencrn, *prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg;
//    PointPrec                   x, y, z;
    vector<MeshCorner*>         vecCrn, vecAllCrn;

    cout << "Catmull-Clark start" << endl;

    // compute new face-points
    times("");

    #pragma omp parallel for default(none) private(i,j,facet,Fpoi)
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();

        for (j = 0 ; j < facet->my_valency ; j++)
        {
            Fpoi += facet->my_corners[j].my_vertex->my_point;
        }
        Fpoi *= (1.0 / facet->my_valency);
        facet->my_Fpoi = Fpoi;
    }
    times("Face average loop");

    // compute new edge-points - each only once!
    #pragma omp parallel for default(none) private(i,j,facet,crn,Epoi)
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                Epoi = Point_3D();
                Epoi += crn->my_vertex->my_point;
                Epoi += crn->my_nextCorner->my_vertex->my_point;
                if (crn->my_nextFacet == NULL)
                {
                    Epoi *= 0.5;
                }
                else
                {
                    Epoi += facet->my_Fpoi;
                    Epoi += facet->my_corners[j].my_nextFacet->my_Fpoi;
                    Epoi *= 0.25;
                }
                crn->my_Epoi = Epoi;
            }
        }
    }
    times("Edge average loop");

    // compute new vertex-points

    #pragma omp parallel for default(none) private(i,j,vertex,Vpoi,crn,next0,next1,prev0,prev1,cencrn,Favrg,Eavrg)
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];
        Vpoi = Point_3D();

        if (vertex->isOnBoundary)
        {
            if (vertex->my_valency == 1)
            {
                Vpoi = vertex->my_point;
            }
            else if (vertex->my_valency == 2)
            {
                Vpoi = 6 * vertex->my_point;
                crn = findCorner(vertex, vertex->my_facets[0]);
                next0 = crn->my_nextCorner->my_vertex;
                for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
                {
                    crn = crn->my_nextCorner;
                }
                prev0 = crn->my_vertex;

                crn = findCorner(vertex, vertex->my_facets[1]);
                next1 = crn->my_nextCorner->my_vertex;
                for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
                {
                    crn = crn->my_nextCorner;
                }
                prev1 = crn->my_vertex;

                if (next0->my_index == prev1->my_index)
                {
                    Vpoi += prev0->my_point + next1->my_point;
                }
                else
                {
                    Vpoi += prev1->my_point + next0->my_point;
                }
                Vpoi *= 0.125;
            }
            else
            {
//                cout << "Something is wrong at the boundary!" << endl;
//                assert(false); // something is wrong at the boundary!

                // instead of crashing, interpolate the point
                Vpoi = vertex->my_point;
            }
        }
        else
        {
            Favrg = Point_3D();
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Favrg += vertex->my_facets[j]->my_Fpoi;
            }
            Favrg *= (1.0 / vertex->my_valency);

            Eavrg = Point_3D();

            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Eavrg += cencrn->my_nextCorner->my_vertex->my_point;
                cencrn = getSameCorner(cencrn);
            }
            Eavrg *= (1.0 / vertex->my_valency);
            Eavrg += vertex->my_point;
            Eavrg *= 0.5;

            if (vertex->my_valency < 3)
            {
                Vpoi = Point_3D(0,0,0);
            }
            else
            {
                Vpoi = vertex->my_point * (vertex->my_valency - 3);
            }
            Vpoi = Vpoi + Eavrg + Eavrg;
            Vpoi += Favrg;
            if (vertex->my_valency < 3)
            {
                Vpoi *= (1.0 / 3.0);
            }
            else
            {
                Vpoi *= (1.0 / vertex->my_valency);
            }
        }
        vertex->my_Vpoi = Vpoi;
    }
    times("Vertex average loop");

    // copy over new positions of V-vertices
    smesh->my_vertices.clear();
    smesh->my_vertices.resize(my_numV);

    #pragma omp parallel for default(none) shared(smesh),private(i,vert)
    for (i = 0 ; i < my_numV ; i++)
    {
        vert.my_point = my_vertices[i].my_Vpoi;
        vert.my_index = i;
        vert.my_faceIndices.clear();
        vert.isFeature = true;
        smesh->my_vertices[i] = vert;
    }

    // add F-vertices
    smesh->my_vertices.resize(my_numV + my_numF);

    #pragma omp parallel for default(none) shared(smesh) private(i,vert)
    for (i = 0 ; i < my_numF ; i++)
    {
        vert.my_point = my_facets[i].my_Fpoi;
        vert.my_index = my_numV + i;
        vert.isFeature = false;
        smesh->my_vertices[i + my_numV] = vert;
    }
    times("Copy over V- and F-vertices");

    //index E-vertices and add them to vertex list
    //but first collect corners so that new E-vertices can be added in parallel
    vecCrn.clear();
    vecAllCrn.clear();
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            vecAllCrn.push_back(crn);
            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vecCrn.push_back(crn);
            }
        }
    }

    cnt = smesh->my_vertices.size();
    num = vecCrn.size();
    smesh->my_vertices.resize(cnt + num);

    #pragma omp parallel for default(none) shared(smesh,cnt,num,vecCrn), private(i,crn,vert)
    for (i = cnt ; i < cnt + num ; i++)
    {
        crn = vecCrn[i - cnt];
        vert.my_point = crn->my_Epoi;
        vert.my_index = i;
        crn->my_newEindex = i;
        if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
        {
            vert.isFeature = true;
        }
        else
        {
            vert.isFeature = false;
        }
        smesh->my_vertices[i] = vert;
    }
    times("Add E-vertices");

    //create facets
    smesh->my_facets.clear();
    smesh->quadMesh = true;
    numCorners = vecAllCrn.size();
    smesh->my_facets.resize(numCorners);

//    #pragma omp parallel for default(none) shared(smesh,numCorners,vecAllCrn), private(i,k,crn,prevcrn,facet,face)
    // THIS DOES NOT WORK IN PARALLEL
    for (i = 0 ; i < numCorners ; i++)
    {
        crn = vecAllCrn[i];
        facet = crn->my_facet;

        face.my_index = i;
        face.my_vertIndices.clear();
        face.my_valency = 4;

        //add four vertices
        face.my_vertIndices.push_back(crn->my_vertex->my_index);
        smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(i);

        if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(crn->my_newEindex);
            smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        face.my_vertIndices.push_back(facet->my_index + my_numV);
        smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(i);

        prevcrn = crn;
        for (k = 0 ; k < facet->my_valency - 1 ; k++)
        {
            prevcrn = prevcrn->my_nextCorner;
        }

        if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(prevcrn->my_newEindex);
            smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        smesh->my_facets[i] = face;
    }
    // old serial code
//    for (i = 0 ; i < my_numF ; i++)
//    {
//        facet = &my_facets[i];
//        for (j = 0 ; j < facet->my_valency ; j++)
//        {
//            crn = &(facet->my_corners[j]);

//            face.my_index = cnt;
//            face.my_vertIndices.clear();
//            face.my_valency = 4;

//            //add four vertices
//            face.my_vertIndices.push_back(crn->my_vertex->my_index);
//            smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(cnt);

//            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
//            {
//                face.my_vertIndices.push_back(crn->my_newEindex);
//                smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(cnt);
//            }
//            else
//            {
//                face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
//                smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
//            }

//            face.my_vertIndices.push_back(facet->my_index + my_numV);
//            smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(cnt);

//            prevcrn = crn;
//            for (k = 0 ; k < facet->my_valency - 1 ; k++)
//            {
//                prevcrn = prevcrn->my_nextCorner;
//            }

//            if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
//            {
//                face.my_vertIndices.push_back(prevcrn->my_newEindex);
//                smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(cnt);
//            }
//            else
//            {
//                face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
//                smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
//            }

//            smesh->my_facets.push_back(face);
//            cnt++;
//        }
//    }
    times("Create facets");

    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level + 1;

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;

    cout << "Catmull-Clark end" << endl;

    smesh->build();
}

void Mesh::LaplacianSmooth(int lapSmvalue)
{
    unsigned int 				i, j, k;
    MeshVertex					*vertex;
    MeshCorner					*cencrn;
    Point_3D					Vpoi;
    double                      lam;

//    cout << "Laplacian smoothing start" << endl;

    lam = (double)lapSmvalue / 10.0;

    // compute new vertex positions

    #pragma omp parallel for default(none), private (i,j,k,vertex,Vpoi,cencrn) shared(lam)
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];
        Vpoi = Point_3D();

        //if (vertex->isOnBoundary)
        if (vertex->isStable)
        {
            Vpoi = vertex->my_point;
        }
        else if (vertex->isOnBoundary && vertex->my_valency == 1)
        {
            Vpoi = vertex->my_point;
        }
        else if (vertex->isOnBoundary && vertex->my_valency == 2)
        {
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                cencrn = findCorner(vertex, vertex->my_facets[j]);
                Vpoi += cencrn->my_nextCorner->my_vertex->my_point;
                for (k = 0 ; k < cencrn->my_facet->my_valency - 1 ; k++)
                {
                     cencrn = cencrn->my_nextCorner;
                }
                Vpoi += cencrn->my_vertex->my_point;
            }
            Vpoi *= (lam / 4); // average of edge connected vertices (Bs); the inner one is counted twice
            Vpoi += (1.0 - lam) * vertex->my_point;
        }
        else if (vertex->isOnBoundary && vertex->my_valency > 2)
        {
//            assert(false);
//            cout << "This should not happen for Henrik's images!";
        }
        else
        {
            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Vpoi += cencrn->my_nextCorner->my_vertex->my_point;
                cencrn = getSameCorner(cencrn);
            }
            Vpoi *= (lam / vertex->my_valency); // average of edge connected vertices (Bs)
            Vpoi += (1.0 - lam) * vertex->my_point;
        }
        vertex->my_Vpoi = Vpoi;
    }

    // copy over new (z) positions
    #pragma omp parallel for default(none), private(i)
    for (i = 0 ; i < my_numV ; i++)
    {
//        my_vertices[i].my_point = my_vertices[i].my_Vpoi;
        my_vertices[i].my_point.setZ(my_vertices[i].my_Vpoi.getZ());
    }

//    cout << "Laplacian Smooth end" << endl;
}

MeshCorner* Mesh::findCorner(MeshVertex *vertex, MeshFacet *facet)
{
    MeshCorner *corner;

    corner = &(facet->my_corners[0]);
    while (corner->my_vIndex != vertex->my_index)
    {
        corner = corner->my_nextCorner;
    }
    return corner;
}

void Mesh::times (const char *which) {
/* If which is not empty, print the times since the previous call. */
    static double last_wall = 0.0, last_cpu = 0.0;
    double wall, cpu;
    struct timeval tv;
    clock_t stamp;

    wall = last_wall;
    cpu = last_cpu;
    if (gettimeofday(&tv,NULL) != 0 ||
            (stamp = clock()) == (clock_t)-1)
        cout << "Time failed" << endl;
    last_wall = tv.tv_sec+1.0e-6*tv.tv_usec;
    last_cpu = stamp/(double)CLOCKS_PER_SEC;
    if (strlen(which) > 0) {
        wall = last_wall-wall;
        cpu = last_cpu-cpu;
        cout << which << " time = " << wall << "s , CPU = " << cpu << "s" << endl;
//        printf("%s time = %.2f seconds, CPU = %.2f seconds\n",which,wall,cpu);
    }
}
