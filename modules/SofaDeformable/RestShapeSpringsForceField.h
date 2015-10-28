/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*                (c) 2006-2011 MGH, INRIA, USTL, UJF, CNRS                    *
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
#ifndef SOFA_COMPONENT_FORCEFIELD_RESTSHAPESPRINGFORCEFIELD_H
#define SOFA_COMPONENT_FORCEFIELD_RESTSHAPESPRINGFORCEFIELD_H

#include <sofa/core/behavior/ForceField.h>
#include <sofa/core/objectmodel/Data.h>
#include <sofa/helper/vector.h>

#ifdef SOFA_HAVE_EIGEN2
#include <SofaEigen2Solver/EigenSparseMatrix.h>
#endif

#include <sofa/SofaCommon.h>

namespace sofa
{
namespace core
{
namespace behavior
{
template< class T > class MechanicalState;

} // namespace behavior
} // namespace core
} // namespace sofa

namespace sofa
{

namespace component
{

namespace forcefield
{

/**
* @brief This class describes a simple elastic springs ForceField between DOFs positions and rest positions.
*
* Springs are applied to given degrees of freedom between their current positions and their rest shape positions.
* An external MechanicalState reference can also be passed to the ForceField as rest shape position.
*/
template<class DataTypes>
class RestShapeSpringsForceField : public core::behavior::ForceField<DataTypes>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(RestShapeSpringsForceField, DataTypes), SOFA_TEMPLATE(core::behavior::ForceField, DataTypes));

    typedef core::behavior::ForceField<DataTypes> Inherit;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::CPos CPos;
    typedef typename DataTypes::Deriv Deriv;
    typedef typename DataTypes::Real Real;
    typedef helper::vector< unsigned int > VecIndex;
    typedef helper::vector< Real >	 VecReal;

    typedef core::objectmodel::Data<VecCoord> DataVecCoord;
    typedef core::objectmodel::Data<VecDeriv> DataVecDeriv;

    Data< helper::vector< unsigned int > > points;
    Data< VecReal > stiffness;
    Data< VecReal > angularStiffness;
    Data< helper::vector< CPos > > pivotPoints;
    Data< std::string > external_rest_shape;
    Data< helper::vector< unsigned int > > external_points;
    Data< bool > recompute_indices;
    
    Data< bool > d_drawSpring;
    Data< Real > d_drawSpringLengthThreshold;
    Data< sofa::defaulttype::Vec4f > d_springColor;
    Data< sofa::defaulttype::Vec4f > d_springSphereColor;
    Data< Real > d_springSphereRadius;

    sofa::core::behavior::MechanicalState< DataTypes > *restMState;
#ifdef SOFA_HAVE_EIGEN2
    linearsolver::EigenBaseSparseMatrix<typename DataTypes::Real> matS;    
#endif

protected:
    RestShapeSpringsForceField();

    Data< bool > d_useRestMState; ///< An external MechanicalState is used as rest reference.
    Data< Real > d_springLengthThreshold; /// Global Threshold

    Data< Real > d_springLengthThresholdZaxis; /// Threshold only on Z axis

public:
    /// BaseObject initialization method.
    void bwdInit();

    virtual void reset();

    /// Add the forces.
    virtual void addForce(const core::MechanicalParams* mparams, DataVecDeriv& f, const DataVecCoord& x, const DataVecDeriv& v);

    virtual void addDForce(const core::MechanicalParams* mparams, DataVecDeriv& df, const DataVecDeriv& dx);

    virtual double getPotentialEnergy(const core::MechanicalParams* /*mparams*/ /* PARAMS FIRST */, const DataVecCoord&  /* x */) const
    {
        serr << "Get potentialEnergy not implemented" << sendl;
        return 0.0;
    }

    /// Brings ForceField contribution to the global system stiffness matrix.
    virtual void addKToMatrix(const core::MechanicalParams* mparams, const sofa::core::behavior::MultiMatrixAccessor* matrix );

    virtual void addSubKToMatrix(const core::MechanicalParams* mparams, const sofa::core::behavior::MultiMatrixAccessor* matrix, const helper::vector<unsigned> & addSubIndex );

    virtual void draw(const core::visual::VisualParams* vparams);


    const DataVecCoord* getExtPosition() const;
    const VecIndex& getIndices() const { return m_indices; }
    const VecIndex& getExtIndices() const { return (d_useRestMState.getValue() ? m_ext_indices : m_indices); }

protected :

    void recomputeIndices();

    bool checkOutOfBoundsIndices();
    bool checkOutOfBoundsIndices(const VecIndex &indices, const unsigned int dimension);

    bool breakElongatedSprings(const DataVecCoord &pos);

    bool releaseRestShapeFF(const DataVecCoord &pos );  

    VecIndex m_indices;
    VecIndex m_ext_indices;
    helper::vector<CPos> m_pivots;

#ifdef SOFA_HAVE_EIGEN2
    double lastUpdatedStep;
#endif
};

#if defined(SOFA_EXTERN_TEMPLATE) && !defined(SOFA_BUILD_DEFORMABLE)

#ifndef SOFA_FLOAT
extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<sofa::defaulttype::Vec3dTypes>;
//extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<Vec2dTypes>;
extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<sofa::defaulttype::Vec1dTypes>;
//extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<Vec6dTypes>;
extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<sofa::defaulttype::Rigid3dTypes>;
//extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<Rigid2dTypes>;
#endif
#ifndef SOFA_DOUBLE
extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<sofa::defaulttype::Vec3fTypes>;
//extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<Vec2fTypes>;
extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<sofa::defaulttype::Vec1fTypes>;
//extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<Vec6fTypes>;
extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<sofa::defaulttype::Rigid3fTypes>;
//extern template class SOFA_DEFORMABLE_API RestShapeSpringsForceField<Rigid2fTypes>;
#endif

#endif

} // namespace forcefield

} // namespace component

} // namespace sofa

#endif // SOFA_COMPONENT_FORCEFIELD_RESTSHAPESPRINGFORCEFIELD_H
