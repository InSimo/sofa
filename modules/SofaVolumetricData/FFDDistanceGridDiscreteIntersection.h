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
#ifndef SOFA_COMPONENT_COLLISION_FFDDISTANCEGRIDDISCRETEINTERSECTION_H
#define SOFA_COMPONENT_COLLISION_FFDDISTANCEGRIDDISCRETEINTERSECTION_H

#include <sofa/core/collision/Intersection.h>
#include <sofa/helper/FnDispatcher.h>
#include <SofaBaseCollision/SphereModel.h>
#include <SofaMeshCollision/PointModel.h>
#include <SofaMeshCollision/LineModel.h>
#include <SofaMeshCollision/TriangleModel.h>
#include <SofaBaseCollision/CubeModel.h>
#include <SofaUserInteraction/RayModel.h>
#include <SofaVolumetricData/DistanceGridCollisionModel.h>
#include <SofaBaseCollision/DiscreteIntersection.h>

namespace sofa
{

namespace component
{

namespace collision
{
class SOFA_VOLUMETRIC_DATA_API FFDDistanceGridDiscreteIntersection : public core::collision::BaseIntersector
{
    template <class Elem1, class Elem2>
    using OutputContainer = DiscreteIntersection::OutputContainer<Elem1, Elem2>;

public:
    FFDDistanceGridDiscreteIntersection(DiscreteIntersection* object);

    bool testIntersection(FFDDistanceGridCollisionElement&, RigidDistanceGridCollisionElement&);
    bool testIntersection(FFDDistanceGridCollisionElement&, FFDDistanceGridCollisionElement&);
    bool testIntersection(FFDDistanceGridCollisionElement&, Point&);
    template<class T> bool testIntersection(FFDDistanceGridCollisionElement&, TSphere<T>&);
    bool testIntersection(FFDDistanceGridCollisionElement&, Triangle&);
    bool testIntersection(Ray&, FFDDistanceGridCollisionElement&);

    int computeIntersection(FFDDistanceGridCollisionElement&, RigidDistanceGridCollisionElement&, OutputContainer<FFDDistanceGridCollisionElement, RigidDistanceGridCollisionElement>*);
    int computeIntersection(FFDDistanceGridCollisionElement&, FFDDistanceGridCollisionElement&, OutputContainer<FFDDistanceGridCollisionElement, FFDDistanceGridCollisionElement>*);
    int computeIntersection(FFDDistanceGridCollisionElement&, Point&, OutputContainer<FFDDistanceGridCollisionElement, Point>*);
    template<class T> int computeIntersection(FFDDistanceGridCollisionElement&, TSphere<T>&, OutputContainer<FFDDistanceGridCollisionElement, TSphere<T>>*);
    int computeIntersection(FFDDistanceGridCollisionElement&, Triangle&, OutputContainer<FFDDistanceGridCollisionElement, Triangle>*);
    int computeIntersection(Ray&, FFDDistanceGridCollisionElement&, OutputContainer<Ray, FFDDistanceGridCollisionElement>*);

protected:

    DiscreteIntersection* intersection;

};

} // namespace collision

} // namespace component

} // namespace sofa

#endif
