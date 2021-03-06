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
#ifndef SOFA_COMPONENT_COLLISION_BRUTEFORCEDETECTION_H
#define SOFA_COMPONENT_COLLISION_BRUTEFORCEDETECTION_H

#include <sofa/SofaBase.h>
#include <sofa/core/collision/BroadPhaseDetection.h>
#include <sofa/core/collision/NarrowPhaseDetection.h>
#include <SofaBaseCollision/CubeModel.h>

#include <queue>


namespace sofa
{

namespace component
{

namespace collision
{


class SOFA_BASE_COLLISION_API MirrorIntersector : public core::collision::ElementIntersector
{
public:
    core::collision::ElementIntersector* intersector;

    /// Test if 2 elements can collide. Note that this can be conservative (i.e. return true even when no collision is present)
    virtual bool canIntersect(core::CollisionElementIterator elem1, core::CollisionElementIterator elem2)
    {
        return intersector->canIntersect(elem2, elem1);
    }

    /// Begin intersection tests between two collision models. Return the number of contacts written in the contacts vector.
    /// If the given contacts vector is NULL, then this method should allocate it.
    virtual int beginIntersect(core::CollisionModel* model1, core::CollisionModel* model2, std::pair<core::collision::DetectionOutputContainer*, core::collision::DetectionOutputContainer*>& contacts)
    {
        return intersector->beginIntersect(model2, model1, contacts);
    }

    /// Compute the intersection between 2 elements. Return the number of contacts written in the contacts vector.
    virtual int intersect(core::CollisionElementIterator elem1, core::CollisionElementIterator elem2, core::collision::DetectionOutputContainer* contacts)
    {
        return intersector->intersect(elem2, elem1, contacts);
    }

    /// End intersection tests between two collision models. Return the number of contacts written in the contacts vector.
    virtual int endIntersect(core::CollisionModel* model1, core::CollisionModel* model2, core::collision::DetectionOutputContainer* contacts)
    {
        return intersector->endIntersect(model2, model1, contacts);
    }

    virtual std::string name() const
    {
        return intersector->name() + std::string("<SWAP>");
    }

};


class SOFA_BASE_COLLISION_API BruteForceDetection :
    public core::collision::BroadPhaseDetection,
    public core::collision::NarrowPhaseDetection
{
public:
    SOFA_CLASS_EXTERNAL((BruteForceDetection), ((core::collision::BroadPhaseDetection), (core::collision::NarrowPhaseDetection)));
    SOFA_CLASS_VIRTUAL_PARENTS((core::collision::Detection));

private:
    bool _is_initialized;
    sofa::helper::vector<core::CollisionModel*> collisionModels;
    Data<bool> bDrawEnabled;

    Data< helper::fixed_array<sofa::defaulttype::Vector3,2> > box;

    CubeModel::SPtr boxModel;


protected:
    BruteForceDetection();

    ~BruteForceDetection();

    virtual bool keepCollisionBetween(core::CollisionModel *cm1, core::CollisionModel *cm2);

public:
    void setDraw(bool val) { bDrawEnabled.setValue(val); }

    void init();
    void reinit();

    void addCollisionModel (core::CollisionModel *cm);
    void addCollisionPair (const std::pair<core::CollisionModel*, core::CollisionModel*>& cmPair);

    virtual void beginBroadPhase()
    {
        core::collision::BroadPhaseDetection::beginBroadPhase();
        collisionModels.clear();
    }

    void draw(const core::visual::VisualParams*  vparams);

    inline virtual bool needsDeepBoundingTree()const {return true;}
    
    typedef std::pair< std::pair<core::CollisionElementIterator,core::CollisionElementIterator>, std::pair<core::CollisionElementIterator,core::CollisionElementIterator> > TestPair;
    
    static void computeNarrowPhase(
            core::CollisionModel* cm1, core::CollisionModel* cm2, core::collision::ElementIntersector* intersector,
            core::CollisionModel* finalcm1, core::CollisionModel* finalcm2, core::collision::ElementIntersector* finalintersector,
            core::collision::Intersection* intersectionMethod,
            bool self,
            std::queue< TestPair >& externalCells,
            sofa::core::collision::DetectionOutputContainer* outputs
            );
};

} // namespace collision

} // namespace component

} // namespace sofa

#endif
