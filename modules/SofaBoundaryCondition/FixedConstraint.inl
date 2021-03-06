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
#ifndef SOFA_COMPONENT_PROJECTIVECONSTRAINTSET_FIXEDCONSTRAINT_INL
#define SOFA_COMPONENT_PROJECTIVECONSTRAINTSET_FIXEDCONSTRAINT_INL

#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/core/behavior/ProjectiveConstraintSet.inl>
#include <SofaBoundaryCondition/FixedConstraint.h>
#include <SofaBaseLinearSolver/SparseMatrix.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/common/Simulation.h>
#include <sofa/helper/gl/template.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <iostream>
#include <SofaBaseTopology/TopologySubsetData.inl>
#include <sofa/helper/gl/BasicShapes.h>




namespace sofa
{

namespace component
{

namespace projectiveconstraintset
{

// Define TestNewPointFunction
template< class DataTypes>
bool FixedConstraint<DataTypes>::FCPointHandler::applyTestCreateFunction(unsigned int, const sofa::helper::vector<unsigned int> &, const sofa::helper::vector<double> &)
{
    if (fc)
    {
        return false;
    }
    else
    {
        return false;
    }
}

// Define RemovalFunction
template< class DataTypes>
void FixedConstraint<DataTypes>::FCPointHandler::applyDestroyFunction(unsigned int pointIndex, value_type &)
{
    if (fc)
    {
        fc->removeConstraint((unsigned int) pointIndex);
    }
}

template <class DataTypes>
FixedConstraint<DataTypes>::FixedConstraint()
    : core::behavior::ProjectiveConstraintSet<DataTypes>(NULL)
    , f_indices( initData(&f_indices,"indices","Indices of the fixed points") )
    , f_fixAll( initData(&f_fixAll,false,"fixAll","filter all the DOF to implement a fixed object") )
    , f_drawSize( initData(&f_drawSize,0.0,"drawSize","0 -> point based rendering, >0 -> radius of spheres") )
    , d_projectVelocity(initData(&d_projectVelocity,false,"projectVelocity","if true, project velocity") )
    , d_handleTopologyChange(initData(&d_handleTopologyChange, true, "handleTopologyChange", "Enable support of topological changes for point indices (disable if another component takes care of this)"))
    , data(new FixedConstraintInternalData<DataTypes>())
{
    // default to indice 0
    f_indices.beginEdit()->push_back(0);
    f_indices.endEdit();

    pointHandler = new FCPointHandler(this, &f_indices);
}


template <class DataTypes>
FixedConstraint<DataTypes>::~FixedConstraint()
{
    if (pointHandler)
        delete pointHandler;

    delete data;
}

template <class DataTypes>
void FixedConstraint<DataTypes>::clearConstraints()
{
    f_indices.beginEdit()->clear();
    f_indices.endEdit();
}

template <class DataTypes>
void FixedConstraint<DataTypes>::addConstraint(unsigned int index)
{
    f_indices.beginEdit()->push_back(index);
    f_indices.endEdit();
}

template <class DataTypes>
void FixedConstraint<DataTypes>::removeConstraint(unsigned int index)
{
    removeValue(*f_indices.beginEdit(),index);
    f_indices.endEdit();
}

// -- Constraint interface


template <class DataTypes>
void FixedConstraint<DataTypes>::init()
{
    this->core::behavior::ProjectiveConstraintSet<DataTypes>::init();

    topology = this->getContext()->getMeshTopology();

    //  if (!topology)
    //    serr << "Can not find the topology." << sendl;

    // Initialize functions and parameters
    if (topology && d_handleTopologyChange.getValue())
    {
        f_indices.createTopologicalEngine(topology, pointHandler);
        f_indices.registerTopologicalData();
    }

    const SetIndexArray & indices = f_indices.getValue();

    unsigned int maxIndex=this->mstate->getSize();
    for (unsigned int i=0; i<indices.size(); ++i)
    {
        const unsigned int index=indices[i];
        if (index >= maxIndex)
        {
            serr << "Index " << index << " not valid!" << sendl;
            removeConstraint(index);
        }
    }

    reinit();

//  cerr<<"FixedConstraint<DataTypes>::init(), getJ = " << *getJ(0) << endl;

}

template <class DataTypes>
void  FixedConstraint<DataTypes>::reinit()
{
}

template <class DataTypes>
void FixedConstraint<DataTypes>::projectMatrix( sofa::defaulttype::BaseMatrix* M, unsigned offset )
{
    unsigned blockSize = DataTypes::deriv_total_size;

    if( f_fixAll.getValue()==true )
    {
        unsigned size = this->mstate->getSize();
        for( unsigned i=0; i<size; i++ )
        {
            M->clearRowsCols( offset + i * blockSize, offset + (i+1) * (blockSize) );
        }
    }
    else
    {
        // clears the rows and columns associated with fixed particles
        for(SetIndexArray::const_iterator it= f_indices.getValue().begin(), iend=f_indices.getValue().end(); it!=iend; it++ )
        {
            M->clearRowsCols( offset + (*it) * blockSize, offset + (*it+1) * (blockSize) );
        }
    }
}


template <class DataTypes>
void FixedConstraint<DataTypes>::projectResponse(const core::MechanicalParams* mparams /* PARAMS FIRST */, DataVecDeriv& resData)
{
//    cerr<<"FixedConstraint<DataTypes>::projectResponse is called "<<endl;
//    assert(false);
    helper::WriteAccessor<DataVecDeriv> res ( mparams, resData );
    const SetIndexArray & indices = f_indices.getValue(mparams);
    //serr<<"FixedConstraint<DataTypes>::projectResponse, dx.size()="<<res.size()<<sendl;
    if( f_fixAll.getValue(mparams) )
    {
        // fix everything
        typename VecDeriv::iterator it;
        for( it = res.begin(); it != res.end(); ++it )
        {
            *it = Deriv();
        }
    }
    else
    {
        for (SetIndexArray::const_iterator it = indices.begin();
                it != indices.end();
                ++it)
        {
            res[*it] = Deriv();
        }
    }
    //cerr<<"FixedConstraint<DataTypes>::projectResponse is called  res = "<<endl<<res<<endl;
}

template <class DataTypes>
void FixedConstraint<DataTypes>::projectJacobianMatrix(const core::MechanicalParams* mparams /* PARAMS FIRST */, DataMatrixDeriv& cData)
{
    helper::WriteAccessor<DataMatrixDeriv> c ( mparams, cData );

    if( f_fixAll.getValue(mparams) )
    {
        c->clear();
    }
    else
    {
        const SetIndexArray & indices = f_indices.getValue(mparams);
        for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
        {
            c->clearColBloc(*it);
        }
    }
}

// projectVelocity applies the same changes on velocity vector as projectResponse on position vector :
// Each fixed point received a null velocity vector.
// When a new fixed point is added while its velocity vector is already null, projectVelocity is not usefull.
// But when a new fixed point is added while its velocity vector is not null, it's necessary to fix it to null. If not, the fixed point is going to drift.
template <class DataTypes>
void FixedConstraint<DataTypes>::projectVelocity(const core::MechanicalParams* mparams , DataVecDeriv& vData)
{
    if (d_projectVelocity.getValue())
    {
        helper::WriteAccessor<DataVecDeriv> res ( mparams, vData );
        const SetIndexArray & indices = f_indices.getValue();
        //serr<<"FixedConstraint<DataTypes>::projectVelocity, res.size()="<<res.size()<<sendl;
        if( f_fixAll.getValue()==true )    // fix everyting
        {
            for( unsigned i=0; i<res.size(); i++ )
                res[i] = Deriv();
        }
        else
        {
            for (SetIndexArray::const_iterator it = indices.begin();
                    it != indices.end();
                    ++it)
            {
                res[*it] = Deriv();
            }
        }
    }

}

template <class DataTypes>
void FixedConstraint<DataTypes>::projectPosition(const core::MechanicalParams* /*mparams*/ /* PARAMS FIRST */, DataVecCoord& /*xData*/)
{

}

// Matrix Integration interface
template <class DataTypes>
void FixedConstraint<DataTypes>::applyConstraint(const core::MechanicalParams* mparams /* PARAMS FIRST */, const sofa::core::behavior::MultiMatrixAccessor* matrix)
{
    core::behavior::MultiMatrixAccessor::MatrixRef r = matrix->getMatrix(this->mstate.get(mparams));
    if(r)
    {
        //sout << "applyConstraint in Matrix with offset = " << offset << sendl;
        //cerr<<"FixedConstraint<DataTypes>::applyConstraint(defaulttype::BaseMatrix *mat, unsigned int offset) is called "<<endl;
        const unsigned int N = Deriv::size();
        const SetIndexArray & indices = f_indices.getValue();

        //TODO take f_fixAll into account


        // Reset Fixed Row and Col
        for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
        {
            for (unsigned int c=0; c<N; ++c)
                r.matrix->clearRowCol(r.offset + N * (*it) + c);
        }
        // Set Fixed Vertex (separated from clearRowCol to avoid multiple recompression of the matrix if matrix->set
        // actually creates a new bloc which can be the case if no other action was built on this dof)
        for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
        {
            for (unsigned int c=0; c<N; ++c)
                r.matrix->set(r.offset + N * (*it) + c, r.offset + N * (*it) + c, 1.0);
        }
    }
}

template <class DataTypes>
void FixedConstraint<DataTypes>::applyConstraint(const core::MechanicalParams* mparams /* PARAMS FIRST */, defaulttype::BaseVector* vect, const sofa::core::behavior::MultiMatrixAccessor* matrix)
{
    int o = matrix->getGlobalOffset(this->mstate.get(mparams));
    if (o >= 0)
    {
        unsigned int offset = (unsigned int)o;

        //cerr<<"FixedConstraint<DataTypes>::applyConstraint(defaulttype::BaseVector *vect, unsigned int offset) is called "<<endl;
        //sout << "applyConstraint in Vector with offset = " << offset << sendl;
        const unsigned int N = Deriv::size();

        //TODO take f_fixAll into account


        const SetIndexArray & indices = f_indices.getValue();
        for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
        {
            for (unsigned int c=0; c<N; ++c)
                vect->clear(offset + N * (*it) + c);
        }
    }
}




template <class DataTypes>
void FixedConstraint<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
#ifndef SOFA_NO_OPENGL

    if (!vparams->displayFlags().getShowBehaviorModels()) return;
    if (!this->isActive()) return;
    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::position())->getValue();
    //serr<<"FixedConstraint<DataTypes>::draw(), x.size() = "<<x.size()<<sendl;




    const SetIndexArray & indices = f_indices.getValue();

    if( f_drawSize.getValue() == 0) // old classical drawing by points
    {
        sofa::helper::vector< sofa::defaulttype::Vector3 > points;
        sofa::defaulttype::Vector3 point;
        //serr<<"FixedConstraint<DataTypes>::draw(), indices = "<<indices<<sendl;
        if( f_fixAll.getValue() )
            for (unsigned i=0; i<x.size(); i++ )
            {
                point = DataTypes::getCPos(x[i]);
                points.push_back(point);
            }
        else
            for (SetIndexArray::const_iterator it = indices.begin();
                    it != indices.end();
                    ++it)
        {
                point = DataTypes::getCPos(x[*it]);
                points.push_back(point);
            }
        vparams->drawTool()->drawPoints(points, 10, sofa::defaulttype::Vec<4,float>(1,0.5,0.5,1));
    }
    else // new drawing by spheres
    {
        glEnable( GL_LIGHTING );

        sofa::helper::vector< sofa::defaulttype::Vector3 > points;
        sofa::defaulttype::Vector3 point;
        if( f_fixAll.getValue()==true )
            for (unsigned i=0; i<x.size(); i++ )
            {
                point = DataTypes::getCPos(x[i]);
                points.push_back(point);
            }
        else
            for (SetIndexArray::const_iterator it = indices.begin();
                    it != indices.end();
                    ++it)
        {
                point = DataTypes::getCPos(x[*it]);
                points.push_back(point);
            }
        vparams->drawTool()->drawSpheres(points, (float)f_drawSize.getValue(), sofa::defaulttype::Vec<4,float>(1.0f,0.35f,0.35f,1.0f));
    }

#endif /* SOFA_NO_OPENGL */
}

} // namespace constraint

} // namespace component

} // namespace sofa

#endif


