/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*            (c) 2006-2021 MGH, INRIA, USTL, UJF, CNRS, InSimo                *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_INTERACTIONFORCEFIELD_MESHSPRINGFORCEFIELD_H
#define SOFA_COMPONENT_INTERACTIONFORCEFIELD_MESHSPRINGFORCEFIELD_H

#include <SofaDeformable/StiffSpringForceField.h>
#include <set>

namespace sofa
{

namespace component
{

namespace interactionforcefield
{

template<class DataTypes>
class MeshSpringForceField : public StiffSpringForceField<DataTypes>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(MeshSpringForceField, DataTypes), SOFA_TEMPLATE(StiffSpringForceField, DataTypes));

    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::Deriv Deriv;
    typedef typename Coord::value_type Real;
    typedef core::objectmodel::Data<VecDeriv>    DataVecDeriv;
    typedef core::objectmodel::Data<VecCoord>    DataVecCoord;

protected:
    Data< Real >  linesStiffness;
    Data< Real >  linesDamping;
    Data< Real >  trianglesStiffness;
    Data< Real >  trianglesDamping;
    Data< Real >  quadsStiffness;
    Data< Real >  quadsDamping;
    Data< Real >  tetrahedraStiffness;
    Data< Real >  tetrahedraDamping;
    Data< Real >  cubesStiffness;
    Data< Real >  cubesDamping;
    Data< bool >  noCompression;
    Data< Real >  defaultTension;
    Data< Coord > d_scale3d;
    Data< Real >  prevDefaultTension;
    Data< bool  > d_drawEnabled; // Activation of draw
    Data< Real >  d_drawMinElongationRange;
    Data< Real >  d_drawMaxElongationRange;
    Data< Real >  d_drawSpringSize;
    Coord prevScale3d;
    helper::vector<Real> initialStiffnessVec;
    helper::vector<Real> initialDampingVec;

    /// optional range of local DOF indices. Any computation involving only indices outside of this range are discarded (useful for parallelization using mesh partitionning)
    Data< defaulttype::Vec<2,int> > localRange;

    void addSpring(std::set<std::pair<int,int> >& sset, int m1, int m2, Real stiffness, Real damping);


    MeshSpringForceField()
        : linesStiffness(initData(&linesStiffness,Real(0),"linesStiffness","Stiffness for the Lines",true))
        , linesDamping(initData(&linesDamping,Real(0),"linesDamping","Damping for the Lines",true))
        , trianglesStiffness(initData(&trianglesStiffness,Real(0),"trianglesStiffness","Stiffness for the Triangles",true))
        , trianglesDamping(initData(&trianglesDamping,Real(0),"trianglesDamping","Damping for the Triangles",true))
        , quadsStiffness(initData(&quadsStiffness,Real(0),"quadsStiffness","Stiffness for the Quads",true))
        , quadsDamping(initData(&quadsDamping,Real(0),"quadsDamping","Damping for the Quads",true))
        , tetrahedraStiffness(initData(&tetrahedraStiffness,Real(0),"tetrahedraStiffness","Stiffness for the Tetrahedra",true))
        , tetrahedraDamping(initData(&tetrahedraDamping,Real(0),"tetrahedraDamping","Damping for the Tetrahedra",true))
        , cubesStiffness(initData(&cubesStiffness,Real(0),"cubesStiffness","Stiffness for the Cubes",true))
        , cubesDamping(initData(&cubesDamping,Real(0),"cubesDamping","Damping for the Cubes",true))
        , noCompression( initData(&noCompression, false, "noCompression", "Only consider elongation", false))
        , defaultTension( initData(&defaultTension, Real(1.0),"defaultTension", "Percentage to apply to length of edges to artificially create tension",true))
        , d_scale3d(initData(&d_scale3d, "scale3d", "Scale of the rest length of the springs in n dimensions", true))
        , prevDefaultTension((Real)1.0)
        , d_drawEnabled(initData(&d_drawEnabled, false, "drawEnabled","Activation of draw"))
        , d_drawMinElongationRange(initData(&d_drawMinElongationRange, Real(8.), "drawMinElongationRange","Min range of elongation (red eongation - blue neutral - green compression)"))
        , d_drawMaxElongationRange(initData(&d_drawMaxElongationRange, Real(15.), "drawMaxElongationRange","Max range of elongation (red eongation - blue neutral - green compression)"))
        , d_drawSpringSize(initData(&d_drawSpringSize, Real(8.), "drawSpringSize","Size of drawed lines"))
        , localRange( initData(&localRange, defaulttype::Vec<2,int>(-1,-1), "localRange", "optional range of local DOF indices. Any computation involving only indices outside of this range are discarded (useful for parallelization using mesh partitionning)" ) )
    {
        this->ks.setDisplayed(false);
        this->kd.setDisplayed(false);
        this->f_listening.setValue(true);
        this->addAlias(&d_drawEnabled, "draw");
        this->addAlias(&linesStiffness,     "stiffness"); this->addAlias(&linesDamping,     "damping");
        this->addAlias(&trianglesStiffness, "stiffness"); this->addAlias(&trianglesDamping, "damping");
        this->addAlias(&quadsStiffness,     "stiffness"); this->addAlias(&quadsDamping,     "damping");
        this->addAlias(&tetrahedraStiffness,"stiffness"); this->addAlias(&tetrahedraDamping, "damping");
        this->addAlias(&cubesStiffness,     "stiffness"); this->addAlias(&cubesDamping,      "damping");
        //Name changes: keep compatibility with old version
        this->addAlias(&tetrahedraStiffness,"tetrasStiffness"); this->addAlias(&tetrahedraDamping, "tetrasDamping");

        Coord& scale3d = *d_scale3d.beginEdit();

        for (unsigned int i=0; i<scale3d.size(); ++i)
        {
            scale3d[i] = 1.0;
            prevScale3d[i] = 1.0;
        }
        d_scale3d.endEdit();
    }

    virtual ~MeshSpringForceField();
    
    virtual void handleEvent(core::objectmodel::Event* e);

public:
    virtual double getPotentialEnergy() const;

    Real getStiffness() const { return linesStiffness.getValue(); }
    Real getLinesStiffness() const { return linesStiffness.getValue(); }
    Real getTrianglesStiffness() const { return trianglesStiffness.getValue(); }
    Real getQuadsStiffness() const { return quadsStiffness.getValue(); }
    Real getTetrahedraStiffness() const { return tetrahedraStiffness.getValue(); }
    Real getCubesStiffness() const { return cubesStiffness.getValue(); }
    void setStiffness(Real val)
    {
        linesStiffness.setValue(val);
        trianglesStiffness.setValue(val);
        quadsStiffness.setValue(val);
        tetrahedraStiffness.setValue(val);
        cubesStiffness.setValue(val);
    }
    void setLinesStiffness(Real val)
    {
        linesStiffness.setValue(val);
    }
    void setTrianglesStiffness(Real val)
    {
        trianglesStiffness.setValue(val);
    }
    void setQuadsStiffness(Real val)
    {
        quadsStiffness.setValue(val);
    }
    void setTetrahedraStiffness(Real val)
    {
        tetrahedraStiffness.setValue(val);
    }
    void setCubesStiffness(Real val)
    {
        cubesStiffness.setValue(val);
    }

    Real getDamping() const { return linesDamping.getValue(); }
    Real getLinesDamping() const { return linesDamping.getValue(); }
    Real getTrianglesDamping() const { return trianglesDamping.getValue(); }
    Real getQuadsDamping() const { return quadsDamping.getValue(); }
    Real getTetrahedraDamping() const { return tetrahedraDamping.getValue(); }
    Real getCubesDamping() const { return cubesDamping.getValue(); }
    void setDamping(Real val)
    {
        linesDamping.setValue(val);
        trianglesDamping.setValue(val);
        quadsDamping.setValue(val);
        tetrahedraDamping.setValue(val);
        cubesDamping.setValue(val);
    }
    void setLinesDamping(Real val)
    {
        linesDamping.setValue(val);
    }
    void setTrianglesDamping(Real val)
    {
        trianglesDamping.setValue(val);
    }
    void setQuadsDamping(Real val)
    {
        quadsDamping.setValue(val);
    }
    void setTetrahedraDamping(Real val)
    {
        tetrahedraDamping.setValue(val);
    }
    void setCubesDamping(Real val)
    {
        cubesDamping.setValue(val);
    }

    virtual void init();
    virtual void reset();
    virtual void reinit();
    
    void draw(const core::visual::VisualParams* vparams);
};

#if defined(SOFA_EXTERN_TEMPLATE) && !defined(SOFA_BUILD_DEFORMABLE)
#ifndef SOFA_FLOAT
extern template class SOFA_DEFORMABLE_API MeshSpringForceField<defaulttype::Vec3dTypes>;
extern template class SOFA_DEFORMABLE_API MeshSpringForceField<defaulttype::Vec2dTypes>;
extern template class SOFA_DEFORMABLE_API MeshSpringForceField<defaulttype::Vec1dTypes>;
#endif
#ifndef SOFA_DOUBLE
extern template class SOFA_DEFORMABLE_API MeshSpringForceField<defaulttype::Vec3fTypes>;
extern template class SOFA_DEFORMABLE_API MeshSpringForceField<defaulttype::Vec2fTypes>;
extern template class SOFA_DEFORMABLE_API MeshSpringForceField<defaulttype::Vec1fTypes>;
#endif
#endif

} // namespace interactionforcefield

} // namespace component

} // namespace sofa

#endif  /* SOFA_COMPONENT_INTERACTIONFORCEFIELD_MESHSPRINGFORCEFIELD_H */
