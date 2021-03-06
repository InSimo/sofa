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
#include <SofaBaseCollision/BruteForceDetection.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/core/ObjectFactory.h>

#include <sofa/helper/AdvancedTimer.h>

#include <stack>

//#include <sofa/helper/system/gl.h>
//#include <sofa/helper/system/glut.h>

namespace sofa
{

namespace component
{

namespace collision
{

using namespace sofa::defaulttype;
using namespace sofa::helper;
using namespace collision;

SOFA_CLASS_IMPL((BruteForceDetection));
SOFA_DECL_CLASS(BruteForce)

int BruteForceDetectionClass = core::RegisterObject("Collision detection using extensive pair-wise tests")
        .add< BruteForceDetection >()
        ;

using namespace core::objectmodel;

BruteForceDetection::BruteForceDetection()
    : bDrawEnabled(initData(&bDrawEnabled, false, "drawEnabled", "enable/disable display of results"))
    , box(initData(&box, "box", "if not empty, objects that do not intersect this bounding-box will be ignored"))
{
    addAlias(&bDrawEnabled, "draw");
}

BruteForceDetection::~BruteForceDetection()
{
}

void BruteForceDetection::init()
{
    reinit();
}

void BruteForceDetection::reinit()
{
    if (box.getValue()[0][0] >= box.getValue()[1][0])
    {
        boxModel.reset();
    }
    else
    {
        if (!boxModel) boxModel = sofa::core::objectmodel::New<CubeModel>();
        boxModel->resize(1);
        boxModel->setParentOf(0, box.getValue()[0], box.getValue()[1]);
    }
}

void BruteForceDetection::addCollisionModel(core::CollisionModel *cm)
{
    //sout<<"--------- add Collision Model : "<<cm->getLast()->getName()<<" -------"<<sendl;
    if (cm->empty())
        return;

    if (boxModel)
    {
        bool swapModels = false;
        core::collision::ElementIntersector* intersector = intersectionMethod->findIntersector(cm, boxModel.get(), swapModels);
        if (intersector)
        {
            core::CollisionModel* cm1 = (swapModels?boxModel.get():cm);
            core::CollisionModel* cm2 = (swapModels?cm:boxModel.get());

            // Here we assume a single root element is present in both models
            if (!intersector->canIntersect(cm1->begin(), cm2->begin()))
                return;
        }
    }

    if (cm->isSimulated() && cm->getLast()->canCollideWith(cm->getLast()))
    {
        // self collision
        //sout << "Test broad phase Self "<<cm->getLast()->getName()<<sendl;
        bool swapModels = false;
        core::collision::ElementIntersector* intersector = intersectionMethod->findIntersector(cm, cm, swapModels);
        if (intersector != NULL)
            if (intersector->canIntersect(cm->begin(), cm->begin()))
            {
                //sout << "Broad phase Self "<<cm->getLast()->getName()<<sendl;
                cmPairs.push_back(std::make_pair(cm, cm));
            }

    }
    for (sofa::helper::vector<core::CollisionModel*>::iterator it = collisionModels.begin(); it != collisionModels.end(); ++it)
    {
        core::CollisionModel* cm2 = *it;

        if (!cm->isSimulated() && !cm2->isSimulated())
        {
            continue;
        }

        if (!keepCollisionBetween(cm->getLast(), cm2->getLast()))
            continue;

        bool swapModels = false;
        core::collision::ElementIntersector* intersector = intersectionMethod->findIntersector(cm, cm2, swapModels);
        if (intersector == NULL)
            continue;

        core::CollisionModel* cm1 = (swapModels?cm2:cm);
        cm2 = (swapModels?cm:cm2);

        // // Here we assume multiple root elements are present in both models
        // bool collisionDetected = false;
        // core::CollisionElementIterator begin1 = cm->begin();
        // core::CollisionElementIterator end1 = cm->end();
        // core::CollisionElementIterator begin2 = cm2->begin();
        // core::CollisionElementIterator end2 = cm2->end();
        // for (core::CollisionElementIterator it1 = begin1; it1 != end1; ++it1)
        // {
        //     for (core::CollisionElementIterator it2 = begin2; it2 != end2; ++it2)
        //     {
        //         //if (!it1->canCollideWith(it2)) continue;
        //         if (intersector->canIntersect(it1, it2))
        //         {
        //             collisionDetected = true;
        //             break;
        //         }
        //     }
        //     if (collisionDetected) break;
        // }
        // if (collisionDetected)

        // Here we assume a single root element is present in both models
        if (intersector->canIntersect(cm1->begin(), cm2->begin()))
        {
            //sout << "Broad phase "<<cm1->getLast()->getName()<<" - "<<cm2->getLast()->getName()<<sendl;
            cmPairs.push_back(std::make_pair(cm1, cm2));
        }
    }
    collisionModels.push_back(cm);
}

bool BruteForceDetection::keepCollisionBetween(core::CollisionModel *cm1, core::CollisionModel *cm2)
{
    if (!cm1->canCollideWith(cm2) || !cm2->canCollideWith(cm1))
    {
        return false;
    }

    return true;
}

void BruteForceDetection::addCollisionPair(const std::pair<core::CollisionModel*, core::CollisionModel*>& cmPair)
{
    sofa::helper::AdvancedTimer::stepBegin("BruteForceDetection::addCollisionPair");

    core::CollisionModel *cm1 = cmPair.first; //->getNext();
    core::CollisionModel *cm2 = cmPair.second; //->getNext();

    //int size0 = elemPairs.size();
    //sout << "Narrow phase "<<cm1->getLast()->getName()<<" - "<<cm2->getLast()->getName()<<sendl;

    if (!cm1->isSimulated() && !cm2->isSimulated())
        return;

    if (cm1->empty() || cm2->empty())
        return;

    core::CollisionModel *finalcm1 = cm1->getLast();//get the finnest CollisionModel which is not a CubeModel
    core::CollisionModel *finalcm2 = cm2->getLast();
    //sout << "Final phase "<<gettypename(typeid(*finalcm1))<<" - "<<gettypename(typeid(*finalcm2))<<sendl;
    bool swapModels = false;
    core::collision::ElementIntersector* finalintersector = intersectionMethod->findIntersector(finalcm1, finalcm2, swapModels);//find the method for the finnest CollisionModels
    if (finalintersector == NULL)
        return;
    if (swapModels)
    {
        core::CollisionModel* tmp;
        tmp = cm1; cm1 = cm2; cm2 = tmp;
        tmp = finalcm1; finalcm1 = finalcm2; finalcm2 = tmp;
    }

    const bool self = (finalcm1->getContext() == finalcm2->getContext());
    //if (self)
    //    sout << "SELF: Final intersector " << finalintersector->name() << " for "<<finalcm1->getName()<<" - "<<finalcm2->getName()<<sendl;

    auto& outputs = this->getDetectionOutputs(finalcm1, finalcm2);

    finalintersector->beginIntersect(finalcm1, finalcm2, outputs);//creates outputs if null

    if (finalcm1 == cm1 || finalcm2 == cm2)
    {
        // The last model also contains the root element -> it does not only contains the final level of the tree
        finalcm1 = NULL;
        finalcm2 = NULL;
        finalintersector = NULL;
    }

    std::queue< TestPair > externalCells;

    std::pair<core::CollisionElementIterator,core::CollisionElementIterator> internalChildren1 = cm1->begin().getInternalChildren();
    std::pair<core::CollisionElementIterator,core::CollisionElementIterator> internalChildren2 = cm2->begin().getInternalChildren();
    std::pair<core::CollisionElementIterator,core::CollisionElementIterator> externalChildren1 = cm1->begin().getExternalChildren();
    std::pair<core::CollisionElementIterator,core::CollisionElementIterator> externalChildren2 = cm2->begin().getExternalChildren();
    if (internalChildren1.first != internalChildren1.second)
    {
        if (internalChildren2.first != internalChildren2.second)
            externalCells.emplace(internalChildren1,internalChildren2);
        if (externalChildren2.first != externalChildren2.second)
            externalCells.emplace(internalChildren1,externalChildren2);
    }
    if (externalChildren1.first != externalChildren1.second)
    {
        if (internalChildren2.first != internalChildren2.second)
            externalCells.emplace(externalChildren1,internalChildren2);
        if (externalChildren2.first != externalChildren2.second)
            externalCells.emplace(externalChildren1,externalChildren2);
    }
    //externalCells.push(std::make_pair(std::make_pair(cm1->begin(),cm1->end()),std::make_pair(cm2->begin(),cm2->end())));

    //core::collision::ElementIntersector* intersector = intersectionMethod->findIntersector(cm1, cm2);
    core::collision::ElementIntersector* intersector = NULL;
    cm1 = NULL; // force later init of intersector
    cm2 = NULL;
    
    computeNarrowPhase(cm1, cm2, intersector, finalcm1, finalcm2, finalintersector, intersectionMethod, self, externalCells, outputs.first);
    
    sofa::helper::AdvancedTimer::stepEnd("BruteForceDetection::addCollisionPair");
    //sout << "Narrow phase "<<cm1->getLast()->getName()<<"("<<gettypename(typeid(*cm1->getLast()))<<") - "<<cm2->getLast()->getName()<<"("<<gettypename(typeid(*cm2->getLast()))<<"): "<<elemPairs.size()-size0<<" contacts."<<sendl;
}

void BruteForceDetection::computeNarrowPhase(
        core::CollisionModel *cm1, core::CollisionModel *cm2, core::collision::ElementIntersector* intersector,
        core::CollisionModel *finalcm1, core::CollisionModel *finalcm2, core::collision::ElementIntersector* finalintersector,
        core::collision::Intersection* intersectionMethod,
        bool self,
        std::queue< TestPair >& externalCells,
        sofa::core::collision::DetectionOutputContainer* outputs
        )
{
    MirrorIntersector mirror;
    std::stack< TestPair > internalCells;
                
    while (!externalCells.empty())
    {
        TestPair root = externalCells.front();
        externalCells.pop();

        if (cm1 != root.first.first.getCollisionModel() || cm2 != root.second.first.getCollisionModel())//if the CollisionElements do not belong to cm1 and cm2, update cm1 and cm2
        {
            cm1 = root.first.first.getCollisionModel();
            cm2 = root.second.first.getCollisionModel();
            if (!cm1 || !cm2) continue;
            bool swapModels = false;
            intersector = intersectionMethod->findIntersector(cm1, cm2, swapModels);

            if (intersector == NULL)
            {
                std::cout << "BruteForceDetection: Error finding intersector " << intersectionMethod->getName() << " for " << cm1->getClassName() << " - " << cm2->getClassName() << std::endl;
            }
            //else std::cout << "BruteForceDetection: intersector " << intersector->name() << " for " << intersectionMethod->getName() << " for "<<gettypename(typeid(*cm1))<<" - "<<gettypename(typeid(*cm2))<<std:endl;
            if (swapModels)
            {
                mirror.intersector = intersector; intersector = &mirror;
            }
        }
        if (intersector == NULL)
            continue;
        
        internalCells.push(root);

        while (!internalCells.empty())
        {
            const TestPair& current = internalCells.top();

            core::CollisionElementIterator begin1 = current.first.first;
            core::CollisionElementIterator end1 = current.first.second;
            core::CollisionElementIterator begin2 = current.second.first;
            core::CollisionElementIterator end2 = current.second.second;

            internalCells.pop();
            
            if (begin1.getCollisionModel() == finalcm1 && begin2.getCollisionModel() == finalcm2)
            {
                // Final collision pairs
                for (core::CollisionElementIterator it1 = begin1; it1 != end1; ++it1)
                {
                    for (core::CollisionElementIterator it2 = begin2; it2 != end2; ++it2)
                    {
                        if (!self || it1.canCollideWith(it2))
                            intersector->intersect(it1,it2,outputs);
                    }
                }
            }
            else
            {
                for (core::CollisionElementIterator it1 = begin1; it1 != end1; ++it1)
                {
                    for (core::CollisionElementIterator it2 = begin2; it2 != end2; ++it2)
                    {
                        bool b = intersector->canIntersect(it1,it2);
                        if (b)
                        {
                            // Need to test recursively
                            // Note that an element cannot have both internal and external children
                            TestPair newInternalTests(it1.getInternalChildren(),it2.getInternalChildren());
                            
                            if (newInternalTests.first.first != newInternalTests.first.second)
                            {
                                if (newInternalTests.second.first != newInternalTests.second.second)
                                {
                                    internalCells.push(newInternalTests);
                                }
                                else
                                {
                                    newInternalTests.second.first = it2;
                                    newInternalTests.second.second = it2;
                                    ++newInternalTests.second.second;
                                    internalCells.push(newInternalTests);
                                }
                            }
                            else
                            {
                                if (newInternalTests.second.first != newInternalTests.second.second)
                                {
                                    newInternalTests.first.first = it1;
                                    newInternalTests.first.second = it1;
                                    ++newInternalTests.first.second;
                                    internalCells.push(newInternalTests);
                                }
                                else
                                {
                                    // end of both internal tree of elements.
                                    // need to test external children
                                    TestPair newExternalTests(it1.getExternalChildren(),it2.getExternalChildren());
                                    
                                    if (newExternalTests.first.first != newExternalTests.first.second)
                                    {
                                        if (newExternalTests.second.first != newExternalTests.second.second)
                                        {
                                            if (newExternalTests.first.first.getCollisionModel() == finalcm1 && newExternalTests.second.first.getCollisionModel() == finalcm2)
                                            {
                                                core::CollisionElementIterator begin1 = newExternalTests.first.first;
                                                core::CollisionElementIterator end1 = newExternalTests.first.second;
                                                core::CollisionElementIterator begin2 = newExternalTests.second.first;
                                                core::CollisionElementIterator end2 = newExternalTests.second.second;
                                                for (core::CollisionElementIterator it1 = begin1; it1 != end1; ++it1)
                                                {
                                                    for (core::CollisionElementIterator it2 = begin2; it2 != end2; ++it2)
                                                    {
                                                        // Final collision pair
                                                        if (!self || it1.canCollideWith(it2))
                                                            finalintersector->intersect(it1,it2,outputs);
                                                    }
                                                }
                                            }
                                            else
                                                externalCells.push(newExternalTests);
                                        }
                                        else
                                        {
                                            // only first element has external children
                                            // test them against the second element
                                            newExternalTests.second.first = it2;
                                            newExternalTests.second.second = it2;
                                            ++newExternalTests.second.second;
                                            externalCells.push(newExternalTests);
                                        }
                                    }
                                    else if (newExternalTests.second.first != newExternalTests.second.second)
                                    {
                                        // only second element has external children
                                        // test them against the first element
                                        newExternalTests.first.first = it1;
                                        newExternalTests.first.second = it1;
                                        ++newExternalTests.first.second;
                                        externalCells.push(newExternalTests);
                                    }
                                    else
                                    {
                                        // No child -> final collision pair
                                        if (!self || it1.canCollideWith(it2))
                                            intersector->intersect(it1,it2, outputs);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void BruteForceDetection::draw(const core::visual::VisualParams* vparams )
{
    core::collision::NarrowPhaseDetection::draw(vparams);
}

} // namespace collision

} // namespace component

} // namespace sofa

