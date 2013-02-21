#include "../Curve/Surface.h"
#include "../Curve/BSplineGroup.h"

int createUniformKnotVectors(QVector<float>& knots, int spec_degree, int n)
{
    //if number of control points is less than spec_degree+1, return "n-1"
    //else return spec_degree

    knots.clear();
    int degree;
    if (n == 0)
        return 0;

    if (n > spec_degree)
        degree = spec_degree;
    else
         degree = n-1;

    float last=0.0;
    for (unsigned int i = 0; i < degree; i++) knots.push_back(0.0);
    for (int i = 0; i < n - degree + 1; i++) {
        knots.push_back(i);
        last=i;
    }
    for (unsigned int i = 0; i < degree; i++)
        knots.push_back(last);
    return degree;
}

Surface::Surface(QPoint degree):
    m_spec_degree(degree), idx(-1), connected_spline_id(-1)
{
}

void Surface::updateKnotVectors()
{
    if (controlPoints().size() > 0)
    {
        m_degree.setX( createUniformKnotVectors(m_knotVectors_u, m_spec_degree.x(), controlPoints().size()) );
        m_degree.setY( createUniformKnotVectors(m_knotVectors_v, m_spec_degree.y(), controlPoints()[0].size()) );
    } else
    {
        m_knotVectors_u.clear();
        m_knotVectors_v.clear();
        m_degree = QPoint(0, 0);
    }
}

ControlPoint& Surface::pointAt(QPoint pos)
{
    int cpt_idx = connected_cpts[pos.x()][pos.y()];
    return m_splineGroup->controlPoint(cpt_idx);
}

bool Surface::writeOFF(std::ostream &ofs)
{
    ofs << "OFF" << std::endl;
    if (controlPoints().size() == 0)
    {
        ofs << 0 << " " << 0 << " " << 0 << std::endl;
        qWarning("Warning: This surface is empty!");
        return false;
    }

    std::vector<int> vertex_indices;
    std::map<std::pair<int, int>, int > vertex_indices_uv;

    for (int k=0; k<controlPoints().size(); ++k)
    {
        for (int l=0; l<controlPoints()[k].size(); ++l)
        {
            std::pair<int, int> key(k,l);
            int cpt_id = connected_cpts[k][l];
            std::vector<int>::iterator it = std::find(vertex_indices.begin(), vertex_indices.end(), cpt_id);
            if (it == vertex_indices.end())
            {
                vertex_indices.push_back(cpt_id);
                vertex_indices_uv[key] = vertex_indices.size()-1;
            } else
                vertex_indices_uv[key] = std::distance(vertex_indices.begin(), it);
        }
    }

    BSpline& spline = m_splineGroup->spline(connected_spline_id);

    int face_count = (controlPoints().size()-1)*(controlPoints()[0].size() - 1);
    int edge_count = 0;

    if (spline.is_closed())
    {
        edge_count = (controlPoints().size()-1)*((controlPoints()[0].size()-1)*2) +
                     (controlPoints()[0].size()-1);
    } else
    {
        edge_count = (controlPoints().size()-1)*((controlPoints()[0].size() - 1) + (controlPoints()[0].size())) +
                     (controlPoints()[0].size() - 1);
    }


    ofs << vertex_indices.size() << " " << face_count << " " << edge_count << std::endl;

    //Write vertices
    for (int i=0; i<vertex_indices.size(); ++i)
    {
        int cpt_id = vertex_indices[i];
        ControlPoint& cpt = m_splineGroup->controlPoint(cpt_id);

        ofs << cpt.x() << " " << cpt.y() << " " << cpt.z() << std::endl;
    }

    //Write faces
    for (int k=1; k<controlPoints().size(); ++k)
    {
        for (int l=0; l<controlPoints()[0].size()-1; ++l)
        {
            std::vector<int> indices;

            indices.push_back(vertex_indices_uv[std::pair<int, int>(0,l)]);
            if (vertex_indices_uv[std::pair<int, int>(0,l+1)] != indices.back())
                indices.push_back(vertex_indices_uv[std::pair<int, int>(0,l+1)]);

            if (vertex_indices_uv[std::pair<int, int>(k,l+1)] != indices.back())
                indices.push_back(vertex_indices_uv[std::pair<int, int>(k,l+1)]);

            if (vertex_indices_uv[std::pair<int, int>(k,l)] != indices.back())
                indices.push_back(vertex_indices_uv[std::pair<int, int>(k,l)]);

            ofs << indices.size() << " ";
            for (int m=0; m<indices.size(); ++m)
                ofs << indices[m] << " ";
            ofs << std::endl;
        }
    }
}
