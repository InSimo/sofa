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
#ifndef SOFA_COMPONENT_COLLISION_DISCRETEINTERSECTION_H
#define SOFA_COMPONENT_COLLISION_DISCRETEINTERSECTION_H

#include <sofa/core/collision/DetectionOutput.h>
#include <sofa/core/collision/Intersection.h>
#include <sofa/core/collision/IntersectorFactory.h>
#include <sofa/helper/FnDispatcher.h>
#include <SofaBaseCollision/SphereModel.h>
#include <SofaBaseCollision/CubeModel.h>
#include <SofaBaseCollision/CapsuleModel.h>
#include <SofaBaseCollision/CapsuleIntTool.h>
#include <SofaBaseCollision/OBBModel.h>
#include <SofaBaseCollision/OBBIntTool.h>
#include <SofaBaseCollision/BaseIntTool.h>
#include <SofaBaseCollision/RigidCapsuleModel.h>

namespace sofa
{

namespace component
{

namespace collision
{
class SOFA_BASE_COLLISION_API DiscreteIntersection : public core::collision::Intersection, public core::collision::BaseIntersector
{
public:
    SOFA_CLASS(DiscreteIntersection,sofa::core::collision::Intersection);
protected:
    DiscreteIntersection();
	virtual ~DiscreteIntersection() { }
	
public:

    template <class Elem1, class Elem2>
    using OutputContainer = sofa::core::collision::TDetectionOutputContainer<typename Elem1::Model, typename Elem2::Model>;
    sofa::Data<bool> d_useContinuous;

    /// Return the intersector class handling the given pair of collision models, or NULL if not supported.
    /// @param swapModel output value set to true if the collision models must be swapped before calling the intersector.
    virtual core::collision::ElementIntersector* findIntersector(core::CollisionModel* object1, core::CollisionModel* object2, bool& swapModels) override;

    core::collision::IntersectorMap intersectors;
    typedef core::collision::IntersectorFactory<DiscreteIntersection> IntersectorFactory;

    virtual bool useContinuous() const { return d_useContinuous.getValue(); }

    template <class Elem1,class Elem2>
    int computeIntersection(Elem1 & e1,Elem2 & e2,OutputContainer<Elem1,Elem2>* contacts)
    {
        return BaseIntTool::computeIntersection(e1,e2,e1.getProximity() + e2.getProximity() + getAlarmDistance(),e1.getProximity() + e2.getProximity() + getContactDistance(), contacts);
    }

    template <class Elem1,class Elem2>
    bool testIntersection(Elem1& e1,Elem2& e2){
        return BaseIntTool::testIntersection(e1,e2,this->getAlarmDistance());
    }
};

} // namespace collision

} // namespace component

namespace core
{
namespace collision
{
#if defined(SOFA_EXTERN_TEMPLATE) && !defined(SOFA_BUILD_BASE_COLLISION)
extern template class SOFA_BASE_COLLISION_API IntersectorFactory<component::collision::DiscreteIntersection>;
#endif
}
}

} // namespace sofa

#endif
