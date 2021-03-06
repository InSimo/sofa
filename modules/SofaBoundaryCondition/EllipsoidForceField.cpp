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
#define SOFA_COMPONENT_FORCEFIELD_ELLIPSOIDFORCEFIELD_CPP

#include <SofaBoundaryCondition/EllipsoidForceField.inl>
#include <sofa/defaulttype/Vec3Types.h>
#include <sofa/core/ObjectFactory.h>

namespace sofa
{

namespace component
{

namespace forcefield
{

using namespace sofa::defaulttype;

SOFA_DECL_CLASS(EllipsoidForceField)

int EllipsoidForceFieldClass = core::RegisterObject("Repulsion applied by an ellipsoid toward the exterior or the interior")


// IMPORTANT: 1d and 2d templates are commented because they seemed to be unused in our simulations
// and do not function at the moment with quaternions

#ifndef SOFA_FLOAT
        .add< EllipsoidForceField<Vec3dTypes> >()
//        .add< EllipsoidForceField<Vec2dTypes> >()
//        .add< EllipsoidForceField<Vec1dTypes> >()
        .add< EllipsoidForceField<Rigid3dTypes> >()
//        .add< EllipsoidForceField<Rigid2dTypes> >()
#endif
#ifndef SOFA_DOUBLE
        .add< EllipsoidForceField<Vec3fTypes> >()
//        .add< EllipsoidForceField<Vec2fTypes> >()
//        .add< EllipsoidForceField<Vec1fTypes> >()
        .add< EllipsoidForceField<Rigid3fTypes> >()
//        .add< EllipsoidForceField<Rigid2fTypes> >()
#endif
        ;
#ifndef SOFA_FLOAT
template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Vec3dTypes>;
//template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Vec2dTypes>;
//template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Vec1dTypes>;
template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Rigid3dTypes>;
//template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Rigid2dTypes>;
#endif
#ifndef SOFA_DOUBLE
template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Vec3fTypes>;
//template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Vec2fTypes>;
//template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Vec1fTypes>;
template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Rigid3fTypes>;
//template class SOFA_BOUNDARY_CONDITION_API EllipsoidForceField<Rigid2fTypes>;
#endif

} // namespace forcefield

} // namespace component

} // namespace sofa
