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
#ifndef SOFA_COMPONENT_COLLISION_RAYCONTACT_H
#define SOFA_COMPONENT_COLLISION_RAYCONTACT_H

#include <sofa/core/collision/Contact.h>
#include <sofa/SofaGeneral.h>
#include <sofa/helper/Factory.h>

namespace sofa
{

namespace component
{

namespace collision
{

class RayModel;

class SOFA_USER_INTERACTION_API BaseRayContact : public core::collision::Contact
{
public:
    SOFA_ABSTRACT_CLASS_EXTERNAL((BaseRayContact), ((core::collision::Contact)));
    typedef RayModel CollisionModel1;

protected:
    CollisionModel1* model1;
    sofa::helper::vector<core::collision::DetectionOutput> collisions;


    BaseRayContact(CollisionModel1* model1, core::collision::Intersection* instersectionMethod);

    ~BaseRayContact();
public:
    const sofa::helper::vector<core::collision::DetectionOutput>& getDetectionOutputs() const { return collisions; }

    void createResponse(core::objectmodel::BaseContext* /*group*/) override
    {
    }

    void removeResponse() override
    {
    }
    
    void resetResponse() override
    {
    }

};

template<class CM2>
class RayContact : public BaseRayContact
{
public:
    SOFA_CLASS_DEFAULT((RayContact), ((BaseRayContact)));

    typedef RayModel CollisionModel1;
    typedef CM2 CollisionModel2;
    typedef core::collision::Intersection Intersection;
    typedef core::collision::TDetectionOutputContainer<CollisionModel1, CollisionModel2> OutputContainer;
protected:
    CollisionModel2* model2;
    core::objectmodel::BaseContext* parent;
public:
    RayContact(CollisionModel1* model1, CollisionModel2* model2, Intersection* intersectionMethod)
        : BaseRayContact(model1, intersectionMethod), model2(model2)
    {
    }

    void setDetectionOutputs(core::collision::DetectionOutputContainer* outputs) override
    {
        OutputContainer* o = static_cast<OutputContainer*>(outputs);
        //collisions = outputs;
        collisions.resize(o->size());
        for (unsigned int i=0; i< o->size(); ++i)
        {
            const bool ok = o->getDetectionOutput(i, collisions[i]);
            assert(ok);
#ifdef NDEBUG
            SOFA_UNUSED(ok);
#endif
        }
    }

    std::pair<core::CollisionModel*,core::CollisionModel*> getCollisionModels() override { return std::make_pair(model1,model2); }
};

} // namespace collision

} // namespace component

} // namespace sofa

#endif
