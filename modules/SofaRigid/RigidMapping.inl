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
#ifndef SOFA_COMPONENT_MAPPING_RIGIDMAPPING_INL
#define SOFA_COMPONENT_MAPPING_RIGIDMAPPING_INL

#include <SofaRigid/RigidMapping.h>

#include <sofa/core/BlocMatrixWriter.h>
#include <sofa/helper/decompose.h>

#include <sofa/core/visual/VisualParams.h>

#include <sofa/core/behavior/MechanicalState.h>
#include <sofa/core/Mapping.inl>
#include <sofa/core/State.h>

#include <sofa/defaulttype/VecTypes.h>
#include <sofa/defaulttype/RigidTypes.h>

#include <sofa/helper/io/MassSpringLoader.h>
#include <sofa/helper/io/SphereLoader.h>
#include <sofa/helper/io/Mesh.h>
#include <sofa/helper/gl/template.h>

#include <sofa/simulation/common/Simulation.h>

#include <string.h>
#include <iostream>
#include <cassert>


namespace sofa
{

namespace component
{

namespace mapping
{

extern void rigidMappingDummyFunction(); ///< Used for setting breakpoints, since gdb sometimes fails at breaking within template methods. Implemented in RigidMapping.C


template <class TIn, class TOut>
class RigidMapping<TIn, TOut>::Loader : public helper::io::MassSpringLoader,
        public helper::io::SphereLoader
{
public:

    RigidMapping<TIn, TOut>* dest;
    helper::WriteAccessor<Data<VecCoord> > points;

    Loader(RigidMapping<TIn, TOut>* dest) :
        dest(dest),
        points(dest->points)
    {
    }
    virtual void addMass(SReal px, SReal py, SReal pz, SReal, SReal, SReal,
                         SReal, SReal, bool, bool)
    {
        Coord c;
        Out::set(c, px, py, pz);
        points.push_back(c); //Coord((Real)px,(Real)py,(Real)pz));
    }
    virtual void addSphere(SReal px, SReal py, SReal pz, SReal)
    {
        Coord c;
        Out::set(c, px, py, pz);
        points.push_back(c); //Coord((Real)px,(Real)py,(Real)pz));
    }
};

template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::load(const char *filename)
{
    points.beginEdit()->resize(0);
    points.endEdit();

    if (strlen(filename) > 4
            && !strcmp(filename + strlen(filename) - 4, ".xs3"))
    {
        Loader loader(this);
        loader.helper::io::MassSpringLoader::load(filename);
    }
    else if (strlen(filename) > 4
             && !strcmp(filename + strlen(filename) - 4, ".sph"))
    {
        Loader loader(this);
        loader.helper::io::SphereLoader::load(filename);
    }
    else if (strlen(filename) > 0)
    {
        // Default to mesh loader
        helper::io::Mesh* mesh = helper::io::Mesh::Create(filename);
        if (mesh != NULL)
        {
            helper::WriteAccessor<Data<VecCoord> > points = this->points;

            points.resize(mesh->getVertices().size());
            for (unsigned int i = 0; i < mesh->getVertices().size(); i++)
            {
                Out::set(points[i],
                         mesh->getVertices()[i][0],
                         mesh->getVertices()[i][1],
                         mesh->getVertices()[i][2]);
            }
            delete mesh;
        }
    }
}


template <class TIn, class TOut>
RigidMapping<TIn, TOut>::RigidMapping()
    : Inherit()
    , points(initData(&points, "initialPoints", "Local Coordinates of the points"))
    , index(initData(&index, (unsigned)0, "index", "input DOF index"))
    , fileRigidMapping(initData(&fileRigidMapping, "fileRigidMapping", "Filename"))
    , useX0(initData(&useX0, false, "useX0", "Use x0 instead of local copy of initial positions (to support topo changes)"))
    , indexFromEnd(initData(&indexFromEnd, false, "indexFromEnd", "input DOF index starts from the end of input DOFs vector"))
    , pointsPerFrame(initData(&pointsPerFrame, "repartition", "number of dest dofs per entry dof"))
    , globalToLocalCoords(initData(&globalToLocalCoords, false, "globalToLocalCoords", "are the output DOFs initially expressed in global coordinates"))
    , updateJ(false)
    , d_useGeometricStiffness( initData ( &d_useGeometricStiffness,false, "useGeometricStiffness","to be use with a geometricstiffness forcefield" ) )
{
    //std::cout << "RigidMapping Creation\n";
    this->addAlias(&fileRigidMapping, "filename");


}

template <class TIn, class TOut>
int RigidMapping<TIn, TOut>::addPoint(const Coord& c)
{
    helper::WriteAccessor<Data<VecCoord> > points = this->points;
    int i = points.size();
    points.push_back(c);
    return i;
}

template <class TIn, class TOut>
int RigidMapping<TIn, TOut>::addPoint(const Coord& c, int indexFrom)
{
    helper::WriteAccessor<Data<VecCoord> > points = this->points;
    int i = points.size();
    points.push_back(c);
    if (!pointsPerFrame.getValue().empty())
    {
        pointsPerFrame.beginEdit()->push_back(indexFrom);
        pointsPerFrame.endEdit();
    }
    else if (!i)
    {
        index.setValue(indexFrom);
    }
    else if ((int) index.getValue() != indexFrom)
    {
        sofa::helper::vector<unsigned int>& rep = *pointsPerFrame.beginEdit();
        rep.clear();
        rep.reserve(i + 1);
        rep.insert(rep.end(), index.getValue(), i);
        rep.push_back(indexFrom);
        pointsPerFrame.endEdit();
    }
    return i;
}


template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::reinit()
{
    if (this->points.getValue().empty() && this->toModel != NULL && !useX0.getValue())
    {
        //        cerr<<"RigidMapping<TIn, TOut>::init(), from " << this->fromModel->getName() << " to " << this->toModel->getName() << endl;
        const VecCoord& xTo =this->toModel->read(core::ConstVecCoordId::position())->getValue();
        helper::WriteOnlyAccessor<Data<VecCoord> > points = this->points;
        points.resize(xTo.size());
        unsigned int i = 0;
        if (globalToLocalCoords.getValue())
        {
            //            cerr<<"globalToLocal is true, compute local coordinates"  << endl;
            const VecCoord& xTo =this->toModel->read(core::ConstVecCoordId::position())->getValue();
            points.resize(xTo.size());
            unsigned int i = 0, cpt = 0;
            const InVecCoord& xFrom =this->fromModel->read(core::ConstVecCoordId::position())->getValue();
            switch (pointsPerFrame.getValue().size())
            {
            case 0:
                for (i = 0; i < xTo.size(); i++)
                {
                    points[i] = xFrom[0].inverseRotate(xTo[i]- xFrom[0].getCenter());
                }
                break;
            case 1:
                for (i = 0; i < xFrom.size(); i++)
                {
                    for (unsigned int j = 0; j < pointsPerFrame.getValue()[0]; j++, cpt++)
                    {
                        points[cpt]
                                = xFrom[i].inverseRotate(xTo[cpt] - xFrom[i].getCenter());
                    }
                }
                break;
            default:
                for (i = 0; i < xFrom.size(); i++)
                {
                    for (unsigned int j = 0; j < pointsPerFrame.getValue()[i]; j++, cpt++)
                    {
                        points[cpt]
                                = xFrom[i].inverseRotate(xTo[cpt] - xFrom[i].getCenter());
                    }
                }
                break;

            }
        }
        else
        {
            for (i = 0; i < xTo.size(); i++)
            {
                points[i] = xTo[i];
            }
            //            cerr<<"globalToLocal is false, points in local coordinates : " << points << endl;
        }
    }
    else
    {
        //        cerr << "RigidMapping<TIn, TOut>::init(), points not empty or toModel is null or useX0" << endl;
    }
}

template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::init()
{
    if (!fileRigidMapping.getValue().empty())
        this->load(fileRigidMapping.getFullPath().c_str());


#ifdef SOFA_HAVE_EIGEN2
    eigenJacobians.resize( 1 );
    eigenJacobians[0] = &eigenJacobian;
#endif

    this->reinit();

    this->Inherit::init();
}

/*
template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::disable()
{
 if (!this->points.getValue().empty() && this->toModel!=NULL)
 {
  VecCoord& x =this->toModel->read(core::ConstVecCoordId::position())->getValue();
  x.resize(points.getValue().size());
  for (unsigned int i=0;i<points.getValue().size();i++)
   x[i] = points.getValue()[i];
 }
}
*/

template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::clear(int reserve)
{
    helper::WriteOnlyAccessor<Data<VecCoord> > points = this->points;
    points.clear();
    if (reserve)
        points.reserve(reserve);
}

template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::setRepartition(unsigned int value)
{
    helper::WriteOnlyAccessor< Data< vector<unsigned int> > > rep = this->pointsPerFrame;
    rep.clear();
    rep.push_back(value);
}

template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::setRepartition(sofa::helper::vector<
                                             unsigned int> values)
{
    helper::WriteOnlyAccessor< Data< vector<unsigned int> > > rep = this->pointsPerFrame;
    rep.clear();
    rep.reserve(values.size());
    //repartition.setValue(values);
    sofa::helper::vector<unsigned int>::iterator it = values.begin();
    while (it != values.end())
    {
        rep.push_back(*it);
        it++;
    }
}

template <class TIn, class TOut>
const typename RigidMapping<TIn, TOut>::VecCoord & RigidMapping<TIn, TOut>::getPoints()
{
    if (useX0.getValue())
    {
        const Data<VecCoord>* v = this->toModel.get()->read(core::VecCoordId::restPosition());
        if (v)
            return v->getValue();
        else
            serr
                    << "RigidMapping: ERROR useX0 can only be used in MechanicalMappings."
                    << sendl;
    }
    return points.getValue();
}

/// get the index of the rigid frame mapping the given output point
template <class TIn, class TOut>
unsigned int RigidMapping<TIn, TOut>::getRigidIndexFromOutIndex(unsigned int pout) const
{
    helper::ReadAccessor< Data< vector<unsigned int> > > rep = this->pointsPerFrame;
    if (rep.empty())
    {
        return index.getValue();
    }
    else if (rep.size() == 1) // N points per frame (with N the same for all frames)
    {
        return pout / rep[0];
    }
    else // each frame has different number of points
    {
        unsigned int pout0 = 0;
        for (unsigned int pin = 0; pin < (unsigned int)rep.size(); ++pin)
        {
            pout0 += rep[pin];
            if (pout < pout0)
            {
                return pin;
            }
        }
        return (unsigned int)-1;
    }
}

template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::apply(const core::MechanicalParams * /*mparams*/ /* PARAMS FIRST */, Data<VecCoord>& dOut, const Data<InVecCoord>& dIn)
{
    helper::WriteOnlyAccessor< Data<VecCoord> > out = dOut;
    helper::ReadAccessor< Data<InVecCoord> > in = dIn;
    const VecCoord& pts = this->getPoints();

    updateJ = true;
#ifdef SOFA_HAVE_EIGEN2
    eigenJacobian.resizeBlocks(out.size(),in.size());
#endif

    rotatedPoints.resize(pts.size());
    out.resize(pts.size());

    // NOTE: this case is maybe fixed in https://github.com/sofa-framework/sofa/commit/521c86d0f06d0af8395fcfd63adc23090b2380ce
    if (in.empty())
    {
        return;
    }

    unsigned repartitionCount = pointsPerFrame.getValue().size();

    if (repartitionCount > 1 && repartitionCount != in.size())
    {
        serr << "Error : mapping dofs repartition is not correct" << sendl;
        return;
    }

    unsigned inIdxBegin;
    unsigned inIdxEnd;

    if (repartitionCount == 0)
    {
        inIdxBegin = index.getValue();
        if (indexFromEnd.getValue())
        {
            inIdxBegin = in.size() - 1 - inIdxBegin;
        }
        inIdxEnd = inIdxBegin + 1;
    }
    else
    {
        inIdxBegin = 0;
        inIdxEnd = in.size();
    }

    unsigned outputPerInput;
    if (repartitionCount == 0)
    {
        outputPerInput = pts.size();
    }
    else
    {
        outputPerInput = pointsPerFrame.getValue()[0];
    }

    Coord translation;
    Mat rotation;

    for (unsigned inIdx = inIdxBegin, outIdx = 0; inIdx < inIdxEnd; ++inIdx)
    {
        if (repartitionCount > 1)
        {
            outputPerInput = pointsPerFrame.getValue()[inIdx];
        }

        translation = in[inIdx].getCenter();
        in[inIdx].writeRotationMatrix(rotation);

        for (unsigned iOutput = 0;
             iOutput < outputPerInput;
             ++iOutput, ++outIdx)
        {
            rotatedPoints[outIdx] = rotation * pts[outIdx];
            out[outIdx] = rotatedPoints[outIdx];
            out[outIdx] += translation;
        }
    }

    //    cerr<<"RigidMapping<TIn, TOut>::apply, " << this->getName() << endl;
    //    cerr<<"RigidMapping<TIn, TOut>::apply, in = " << dIn << endl;
    //    cerr<<"RigidMapping<TIn, TOut>::apply, points = " << pts << endl;
    //    cerr<<"RigidMapping<TIn, TOut>::apply, out = " << dOut << endl;
}

template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::applyJ(const core::MechanicalParams * /*mparams*/ /* PARAMS FIRST */, Data<VecDeriv>& dOut, const Data<InVecDeriv>& dIn)
{
    helper::WriteOnlyAccessor< Data<VecDeriv> > out = dOut;
    helper::ReadAccessor< Data<InVecDeriv> > in = dIn;

    const VecCoord& pts = this->getPoints();
    out.resize(pts.size());

    if (in.empty())
    {
        return;
    }

    unsigned repartitionCount = pointsPerFrame.getValue().size();

    if (repartitionCount > 1 && repartitionCount != in.size())
    {
        serr << "Error : mapping dofs repartition is not correct" << sendl;
        return;
    }

    unsigned inIdxBegin;
    unsigned inIdxEnd;

    if (repartitionCount == 0)
    {
        inIdxBegin = index.getValue();
        if (indexFromEnd.getValue())
        {
            inIdxBegin = in.size() - 1 - inIdxBegin;
        }
        inIdxEnd = inIdxBegin + 1;
    }
    else
    {
        inIdxBegin = 0;
        inIdxEnd = in.size();
    }

    unsigned outputPerInput;
    if (repartitionCount == 0)
    {
        outputPerInput = pts.size();
    }
    else
    {
        outputPerInput = pointsPerFrame.getValue()[0];
    }

    for (unsigned inIdx = inIdxBegin, outIdx = 0; inIdx < inIdxEnd; ++inIdx)
    {
        if (repartitionCount > 1)
        {
            outputPerInput = pointsPerFrame.getValue()[inIdx];
        }

        for (unsigned iOutput = 0;
             iOutput < outputPerInput;
             ++iOutput, ++outIdx)
        {
            out[outIdx] = velocityAtRotatedPoint( in[inIdx], rotatedPoints[outIdx] );
        }
    }
}

template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::applyJT(const core::MechanicalParams * /*mparams*/ /* PARAMS FIRST */, Data<InVecDeriv>& dOut, const Data<VecDeriv>& dIn)
{
    helper::WriteAccessor< Data<InVecDeriv> > out = dOut;
    helper::ReadAccessor< Data<VecDeriv> > in = dIn;

    const VecCoord& pts = this->getPoints();

    if (in.empty())
    {
        return;
    }

    unsigned repartitionCount = pointsPerFrame.getValue().size();

    if (repartitionCount > 1 && repartitionCount != out.size())
    {
        serr << "RigidMapping<TIn, TOut>::applyJT, Error : mapping dofs repartition is not correct" << sendl;
        return;
    }

    unsigned outIdxBegin;
    unsigned outIdxEnd;
    unsigned inputPerOutput;

    if (repartitionCount == 0)
    {
        outIdxBegin = index.getValue();
        if (indexFromEnd.getValue())
        {
            outIdxBegin = out.size() - 1 - outIdxBegin;
        }
        outIdxEnd = outIdxBegin + 1;
        inputPerOutput = pts.size();
    }
    else
    {
        outIdxBegin = 0;
        outIdxEnd = out.size();
        inputPerOutput = pointsPerFrame.getValue()[0];
    }

    for (unsigned outIdx = outIdxBegin, inIdx = 0; outIdx < outIdxEnd; ++outIdx)
    {
        if (repartitionCount > 1)
        {
            inputPerOutput = pointsPerFrame.getValue()[outIdx];
        }

        for (unsigned iInput = 0;
             iInput < inputPerOutput;
             ++iInput, ++inIdx)
        {
            getVCenter(out[outIdx]) += in[inIdx];
            getVOrientation(out[outIdx]) +=  (typename InDeriv::Rot)cross(rotatedPoints[inIdx], in[inIdx]);
            //                        cerr<<"RigidMapping<TIn, TOut>::applyJT, inIdx = "<< inIdx << endl;
            //                        cerr<<"RigidMapping<TIn, TOut>::applyJT, in[inIdx] = "<< in[inIdx] << endl;
            //                        cerr<<"RigidMapping<TIn, TOut>::applyJT, rotatedPoint[inIdx] = "<< rotatedPoints[inIdx] << endl;
            //                        cerr<<"RigidMapping<TIn, TOut>::applyJT, cross(rotatedPoints[inIdx], in[inIdx]) = "<< cross(rotatedPoints[inIdx], in[inIdx]) << endl;
            //                        cerr<<"RigidMapping<TIn, TOut>::applyJT, force(out[outIdx]) = "<< getVCenter(out[outIdx]) << endl;
            //                        cerr<<"RigidMapping<TIn, TOut>::applyJT, torque(out[outIdx]) = "<< getVOrientation(out[outIdx]) << endl;

        }
    }
}

//
//            /** Symmetric cross cross product.
//              Let [a×(.×c)] be the linear operator such that: a×(b×c) = [a×(.×c)]b, where × denotes the cross product.
//              This operator is not symmetric, and can mess up conjugate gradient solutions.
//              This method computes sym([a×(.×c)])b , where sym(M) = (M+M^T)/2
//              */
//            template<class Rp, class Rc>  // p for parent, c for child
//            Vec<3,Rp> symCrossCross( const Vec<3,Rc>& a,  const Vec<3,Rp>& b,  const Vec<3,Rc>& c  )
//            {
////                Rp m00 = a[1]*c[1]+a[2]*c[2], m01= -0.5*(a[1]*c[0]+a[0]*c[1]), m02 = -0.5*(a[2]*c[0]+a[0]*c[2]) ;
////                Rp                            m11=  a[0]*c[0]+a[2]*c[2],       m12 = -0.5*(a[2]*c[1]+a[1]*c[2]) ;
////                Rp                                                             m22=  a[0]*c[0]+a[1]*c[1];
//                Rp m00 = a[1]*c[1]+a[2]*c[2], m01= 0, m02 = 0 ;
//                Rp                            m11=  a[0]*c[0]+a[2]*c[2],       m12 = 0 ;
//                Rp                                                             m22=  a[0]*c[0]+a[1]*c[1];
//                return Vec<3,Rp>(
//                        m00*b[0] + m01*b[1] + m02*b[2],
//                        m01*b[0] + m11*b[1] + m12*b[2],
//                        m02*b[0] + m12*b[1] + m22*b[2]
//                        );
//            }
//
//            /** Symmetric cross cross product in 2D (see doc in 3D)
//              In 2D, this operator is a scalar so it is symmetric.
//              */
//            template<class Rp, class Rc> // p for parent, c for child
//            Rp symCrossCross( const Vec<2,Rc>& a,  const Rp& b,  const Vec<2,Rc>& c  )
//            {
//                return (a*c)*b;
//            }


template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::applyDJT(const core::MechanicalParams* mparams /* PARAMS FIRST */, core::MultiVecDerivId parentForceChangeId, core::ConstMultiVecDerivId )
{
    if( !d_useGeometricStiffness.getValue() )
        return;

    helper::ReadAccessor<Data<VecDeriv> > childForces (*mparams->readF(this->toModel));
    helper::WriteAccessor<Data<InVecDeriv> > parentForces (*parentForceChangeId[this->fromModel.get(mparams)].write());
    helper::ReadAccessor<Data<InVecDeriv> > parentDisplacements (*mparams->readDx(this->fromModel));
    //    cerr<<"RigidMapping<TIn, TOut>::applyDJT, parent displacements = "<< parentDisplacements << endl;
    //    cerr<<"RigidMapping<TIn, TOut>::applyDJT, parent forces = "<< parentForces << endl;

    InReal kfactor = (InReal)mparams->kFactor();
    //    cerr<<"RigidMapping<TIn, TOut>::applyDJT, kfactor = "<< kfactor << endl;

    const VecCoord& pts = this->getPoints();

    if (parentForces.empty())
    {
        return;
    }

    unsigned repartitionCount = pointsPerFrame.getValue().size();

    if (repartitionCount > 1 && repartitionCount != parentForces.size())
    {
        serr << "Error : mapping dofs repartition is not correct" << sendl;
        return;
    }


    unsigned parentIdxBegin;
    unsigned parentIdxEnd;
    unsigned inputPerOutput;

    if (repartitionCount == 0)
    {
        parentIdxBegin = index.getValue();
        if (indexFromEnd.getValue())
        {
            parentIdxBegin = parentForces.size() - 1 - parentIdxBegin;
        }
        parentIdxEnd = parentIdxBegin + 1;
        inputPerOutput = pts.size();
    }
    else
    {
        parentIdxBegin = 0;
        parentIdxEnd = parentForces.size();
        inputPerOutput = pointsPerFrame.getValue()[0];
    }

    for (unsigned parentIdx = parentIdxBegin, childIdx = 0; parentIdx < parentIdxEnd; ++parentIdx)
    {
        if (repartitionCount > 1)
        {
            inputPerOutput = pointsPerFrame.getValue()[parentIdx];
        }

        for (unsigned iInput = 0;
             iInput < inputPerOutput;
             ++iInput, ++childIdx)
        {
            typename TIn::AngularVector& parentTorque = getVOrientation(parentForces[parentIdx]);
            const typename TIn::AngularVector& parentRotation = getVOrientation(parentDisplacements[parentIdx]);
            parentTorque += (vecK[childIdx]*parentRotation*kfactor);
        }
    }
}


template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::updateK(const sofa::core::MechanicalParams *mparams, sofa::core::ConstMultiVecDerivId childForceId )
{
    if( !d_useGeometricStiffness.getValue() ) return;
    sofa::helper::ReadAccessor< sofa::Data<VecDeriv > > childForce( *childForceId[this->toModel.get(mparams)].read() );
    //Real kfactor = (Real)mparams->kFactor();
    vecK.clear();
    defaulttype::Mat<3,3,Real> K;
    for (unsigned childIndex=0; childIndex<points.getValue().size(); ++childIndex)
    {
        const Deriv& lambda = childForce[childIndex];
        K = defaulttype::crossProductMatrix<Real>( lambda ) * defaulttype::crossProductMatrix<Real>( rotatedPoints[childIndex]);
        // This corresponds to a non-symmetric matrix, due to the non-commutativity of the group of rotations.
        K.symmetrize();
        sofa::helper::Decompose<Real>::NSDProjection( K ); // negative, semi-definite projection
        vecK.push_back(K);

    }

}


template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::addGeometricStiffnessToMatrix(const sofa::core::MechanicalParams* mparams, const sofa::core::behavior::MultiMatrixAccessor* matrix)
{
    if( !d_useGeometricStiffness.getValue() ) return;

    sofa::core::BlocMatrixWriter< defaulttype::Mat<3, 3, Real> > writer;
    sofa::core::behavior::MultiMatrixAccessor::MatrixRef r = matrix->getMatrix(this->getMechFrom()[0]);
    writer.addGeometricStiffnessToMatrix(this, mparams, r);
}


template <class TIn, class TOut>
template<class MatrixWriter>
void RigidMapping<TIn, TOut>::addGeometricStiffnessToMatrixT(const sofa::core::MechanicalParams* mparams, MatrixWriter mwriter)
{
    const InReal kfactor = (InReal)mparams->kFactor();
    const VecCoord& pts = this->getPoints();
    unsigned repartitionCount = pointsPerFrame.getValue().size();

    std::size_t parentSize = this->fromModel->getSize();

    if (repartitionCount > 1 && repartitionCount != parentSize)
    {
        serr << "Error : mapping dofs repartition is not correct" << sendl;
        return;
    }

    unsigned parentIdxBegin;
    unsigned parentIdxEnd;
    unsigned inputPerOutput;

    if (repartitionCount == 0)
    {
        parentIdxBegin = index.getValue();
        if (indexFromEnd.getValue())
        {
            parentIdxBegin = parentSize - 1 - parentIdxBegin;
        }
        parentIdxEnd = parentIdxBegin + 1;
        inputPerOutput = pts.size();
    }
    else
    {
        parentIdxBegin = 0;
        parentIdxEnd = parentSize;
        inputPerOutput = pointsPerFrame.getValue()[0];
    }

    for (unsigned parentIdx = parentIdxBegin, childIdx = 0; parentIdx < parentIdxEnd; ++parentIdx)
    {
        if (repartitionCount > 1)
        {
            inputPerOutput = pointsPerFrame.getValue()[parentIdx];
        }

        for (unsigned iInput = 0;
             iInput < inputPerOutput;
             ++iInput, ++childIdx)
        {
            mwriter.addDiag(2*parentIdx + 1, vecK[childIdx]*kfactor);
        }
    }
}


// RigidMapping::applyJT( InMatrixDeriv& out, const OutMatrixDeriv& in ) //
// this function propagate the constraint through the rigid mapping :
// if one constraint along (vector n) with a value (v) is applied on the childModel (like collision model)
// then this constraint is transformed by (Jt.n) with value (v) for the rigid model
// There is a specificity of this propagateConstraint: we have to find the application point on the childModel
// in order to compute the right constaint on the rigidModel.
template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::applyJT(const core::ConstraintParams * /*cparams*/ /* PARAMS FIRST */, Data<InMatrixDeriv>& dOut, const Data<OutMatrixDeriv>& dIn)
{
    InMatrixDeriv& out = *dOut.beginEdit();
    const OutMatrixDeriv& in = dIn.getValue();

    if (this->f_printLog.getValue())
    {
        sout << "J on mapped DOFs == " << in << sendl;
        sout << "J on input  DOFs == " << out << sendl;
    }

    switch (pointsPerFrame.getValue().size())
    {
    case 0:
    {
        typename Out::MatrixDeriv::RowConstIterator rowItEnd = in.end();

        for (typename Out::MatrixDeriv::RowConstIterator rowIt = in.begin(); rowIt != rowItEnd; ++rowIt)
        {
            DPos v;
            DRot omega = DRot();

            typename Out::MatrixDeriv::ColConstIterator colItEnd = rowIt.end();

            for (typename Out::MatrixDeriv::ColConstIterator colIt = rowIt.begin(); colIt != colItEnd; ++colIt)
            {
                const Deriv f = colIt.val();
                v += f;
                omega += (DRot) cross(rotatedPoints[colIt.index()], f);
            }

            const InDeriv result(v, omega);
            typename InMatrixDeriv::RowIterator o = out.writeLine(rowIt.index());

            if (!indexFromEnd.getValue())
            {
                o.addCol(index.getValue(), result);
            }
            else
            {
                // Commented by PJ. Bug??
                // o.addCol(out.size() - 1 - index.getValue(), result);
                const unsigned int numDofs = this->getFromModel()->read(core::ConstVecCoordId::position())->getValue().size();
                o.addCol(numDofs - 1 - index.getValue(), result);
            }
        }

        break;
    }
    case 1:
    {
        const unsigned int numDofs = this->getFromModel()->read(core::ConstVecCoordId::position())->getValue().size();
        const unsigned int val = pointsPerFrame.getValue()[0];

        typename Out::MatrixDeriv::RowConstIterator rowItEnd = in.end();

        for (typename Out::MatrixDeriv::RowConstIterator rowIt = in.begin(); rowIt != rowItEnd; ++rowIt)
        {
            int cpt = 0;

            typename Out::MatrixDeriv::ColConstIterator colIt = rowIt.begin();
            typename Out::MatrixDeriv::ColConstIterator colItEnd = rowIt.end();

            for (unsigned int ito = 0; ito < numDofs; ito++)
            {
                DPos v;
                DRot omega = DRot();
                bool needToInsert = false;

                for (unsigned int r = 0; r < val && colIt != colItEnd; r++, cpt++)
                {
                    if (colIt.index() != cpt)
                        continue;

                    needToInsert = true;
                    const Deriv f = colIt.val();
                    v += f;
                    omega += (DRot) cross(rotatedPoints[cpt], f);

                    ++colIt;
                }

                if (needToInsert)
                {
                    const InDeriv result(v, omega);

                    typename InMatrixDeriv::RowIterator o = out.writeLine(rowIt.index());
                    o.addCol(ito, result);
                }
            }
        }

        break;
    }
    default:
    {
        const unsigned int numDofs = this->getFromModel()->read(core::ConstVecCoordId::position())->getValue().size();

        typename Out::MatrixDeriv::RowConstIterator rowItEnd = in.end();

        for (typename Out::MatrixDeriv::RowConstIterator rowIt = in.begin(); rowIt != rowItEnd; ++rowIt)
        {
            int cpt = 0;

            typename Out::MatrixDeriv::ColConstIterator colIt = rowIt.begin();
            typename Out::MatrixDeriv::ColConstIterator colItEnd = rowIt.end();

            for (unsigned int ito = 0; ito < numDofs; ito++)
            {
                DPos v;
                DRot omega = DRot();
                bool needToInsert = false;

                for (unsigned int r = 0; r < pointsPerFrame.getValue()[ito] && colIt
                     != colItEnd; r++, cpt++)
                {
                    if (colIt.index() != cpt)
                        continue;

                    needToInsert = true;
                    const Deriv f = colIt.val();
                    v += f;
                    omega += (DRot) cross(rotatedPoints[cpt], f);

                    ++colIt;
                }

                if (needToInsert)
                {
                    const InDeriv result(v, omega);

                    typename InMatrixDeriv::RowIterator o = out.writeLine(rowIt.index());
                    o.addCol(ito, result);
                }
            }
        }

        break;
    }
    }

    if (this->f_printLog.getValue())
    {
        sout << "new J on input  DOFs = " << out << sendl;
    }

    dOut.endEdit();
}


namespace impl {

template<class U, class Coord>
static void fill_block(Eigen::Matrix<U, 3, 6>& block, const Coord& v) {
	U x = v[0];
	U y = v[1];
	U z = v[2];
				
	// note: this is -hat(v)
	block.template rightCols<3>() <<
					
		0,   z,  -y,
		-z,  0,   x,
		y,  -x,   0;
}

template<class U, class Coord>
void fill_block(Eigen::Matrix<U, 2, 3>& block, const Coord& v) {
	U x = v[0];
	U y = v[1];
				
	// note: this is -hat(v)
	block.template rightCols<1>() <<
		-y,
		x;
}


}

#ifdef SOFA_HAVE_EIGEN2
template <class TIn, class TOut>
const helper::vector<sofa::defaulttype::BaseMatrix*>* RigidMapping<TIn, TOut>::getJs()
{
	const VecCoord& out =this->toModel->read(core::ConstVecCoordId::position())->getValue();
    const InVecCoord& in =this->fromModel->read(core::ConstVecCoordId::position())->getValue();
    const VecCoord& pts = this->getPoints();

	typename SparseMatrixEigen::CompressedMatrix& J = eigenJacobian.compressedMatrix;
	
	if( updateJ || J.size() == 0 ) {
		
		J.resize(out.size() * NOut, in.size() * NIn);
		J.setZero();

		// delicious copypasta... why do we have to deal with all this
		// crap *inside* the mapping in the first place? ideally, the
		// mapping should only have a (index, local_coords) list,
		// setup from the outside.
		unsigned repartitionCount = pointsPerFrame.getValue().size();

        if (repartitionCount > 1 && repartitionCount != in.size())
        {
            serr << "Error : mapping dofs repartition is not correct" << sendl;
            return 0;
        }

        unsigned inIdxBegin;
        unsigned inIdxEnd;

        if (repartitionCount == 0)
        {
            inIdxBegin = index.getValue();
            if (indexFromEnd.getValue())
            {
                inIdxBegin = in.size() - 1 - inIdxBegin;
            }
            inIdxEnd = inIdxBegin + 1;
        }
        else
        {
            inIdxBegin = 0;
            inIdxEnd = in.size();
        }

        unsigned outputPerInput;
        if (repartitionCount == 0)
        {
            outputPerInput = pts.size();
        }
        else
        {
            outputPerInput = pointsPerFrame.getValue()[0];
        }

		// matrix chunk
		typedef typename TOut::Real real;
		typedef Eigen::Matrix<real, NOut, NIn> block_type;
		block_type block;
		
		// translation part
		block.template leftCols<NOut>().setIdentity();
		
		// col indices are strictly increasing
		for (unsigned inIdx = inIdxBegin, outIdx = 0; inIdx < inIdxEnd; ++inIdx) {
            if (repartitionCount > 1) {
				
				// max: wtf ? we just set outputPerInput earlier 
                outputPerInput = pointsPerFrame.getValue()[inIdx];
            }

            for (unsigned iOutput = 0;
                 iOutput < outputPerInput; 
                 ++iOutput, ++outIdx) {
				
				const Coord& v = rotatedPoints[outIdx];

				impl::fill_block(block, v);

				// block is set, now insert it in sparse matrix
				for(unsigned i = 0; i < NOut; ++i){
					unsigned row = outIdx * NOut + i;
					
					J.startVec( row );
					
					// TODO optimize identity off-diagonal and
					// skew-symmetric diagonal
					for(unsigned j = 0; j < NIn; ++j) {
						unsigned col = inIdx * NIn + j;

						if( block(i, j) != 0 ) {
							J.insertBack(row, col) = block(i, j);
						}

					}
				}
            }
        }

		J.finalize();		
	}
												
    return &eigenJacobians;
}
#endif

template <class TIn, class TOut>
const sofa::defaulttype::BaseMatrix* RigidMapping<TIn, TOut>::getJ()
{
#ifdef SOFA_HAVE_EIGEN2
    getJs();
    return &eigenJacobian;
#else
    return nullptr;
#endif
}

template<class Real>
struct RigidMappingMatrixHelper<2, Real>
{
    template <class Matrix, class Vector>
    static void setMatrix(Matrix& mat,
                          const Vector& vec)
    {
        mat[0][0] = (Real) 1     ;    mat[1][0] = (Real) 0     ;
        mat[0][1] = (Real) 0     ;    mat[1][1] = (Real) 1     ;
        mat[0][2] = (Real)-vec[1];    mat[1][2] = (Real) vec[0];
    }
};

template<class Real>
struct RigidMappingMatrixHelper<3, Real>
{
    template <class Matrix, class Vector>
    static void setMatrix(Matrix& mat,
                          const Vector& vec)
    {
        // out = J in
        // J = [ I -OM^ ]
        mat[0][0] = (Real) 1     ;    mat[1][0] = (Real) 0     ;    mat[2][0] = (Real) 0     ;
        mat[0][1] = (Real) 0     ;    mat[1][1] = (Real) 1     ;    mat[2][1] = (Real) 0     ;
        mat[0][2] = (Real) 0     ;    mat[1][2] = (Real) 0     ;    mat[2][2] = (Real) 1     ;
        mat[0][3] = (Real) 0     ;    mat[1][3] = (Real)-vec[2];    mat[2][3] = (Real) vec[1];
        mat[0][4] = (Real) vec[2];    mat[1][4] = (Real) 0     ;    mat[2][4] = (Real)-vec[0];
        mat[0][5] = (Real)-vec[1];    mat[1][5] = (Real) vec[0];    mat[2][5] = (Real) 0     ;
    }
};


template <class TIn, class TOut>
void RigidMapping<TIn, TOut>::draw(const core::visual::VisualParams* vparams)
{
    if (!vparams->displayFlags().getShowMappings() || this->toModel==NULL )
        return;
    sofa::helper::vector<defaulttype::Vector3> points;
    defaulttype::Vector3 point;

    const VecCoord& x =this->toModel->read(core::ConstVecCoordId::position())->getValue();
    for (unsigned int i = 0; i < x.size(); i++)
    {
        point = OutDataTypes::getCPos(x[i]);
        points.push_back(point);
    }
    vparams->drawTool()->drawPoints(points, 7,
                                    defaulttype::Vec<4, float>(1, 1, 0,1));
}

} // namespace mapping

} // namespace component

} // namespace sofa

#endif
