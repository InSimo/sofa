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
#ifndef SOFA_COMPONENT_MISC_EVALSURFACEDISTANCE_INL
#define SOFA_COMPONENT_MISC_EVALSURFACEDISTANCE_INL

#include "EvalSurfaceDistance.h"
#include <sofa/defaulttype/DataTypeInfo.h>
#include <sofa/simulation/common/Node.h>
#include <sofa/helper/gl/template.h>

#include <fstream>

namespace sofa
{

namespace component
{

namespace misc
{

template<class DataTypes>
EvalSurfaceDistance<DataTypes>::EvalSurfaceDistance()
    : maxDist( initData(&maxDist, (SReal)1.0, "maxDist", "alarm distance for proximity detection"))
    , pointsCM(NULL)
    , surfaceCM(NULL)
    , intersection(NULL)
    , detection(NULL)
{
}

template<class DataTypes>
EvalSurfaceDistance<DataTypes>::~EvalSurfaceDistance()
{
}

//-------------------------------- init ------------------------------------
template<class DataTypes>
void EvalSurfaceDistance<DataTypes>::init()
{
    Inherit::init();
    if (!this->mstate1 || !this->mstate2)
        return;
    sofa::core::objectmodel::BaseContext* c1 = this->mstate1->getContext();
    c1->get(pointsCM);
    if (pointsCM == NULL)
    {
        serr << "EvalSurfaceDistance ERROR: object1 PointModel not found."<<sendl;
        return;
    }
    sofa::core::objectmodel::BaseContext* c2 = this->mstate2->getContext();
    c2->get(surfaceCM);
    if (surfaceCM == NULL)
    {
        serr << "EvalSurfaceDistance ERROR: object2 TriangleModel not found."<<sendl;
        return;
    }

    intersection = sofa::core::objectmodel::New<sofa::component::collision::NewProximityIntersection>();
    this->addSlave(intersection);
    intersection->init();

    detection = sofa::core::objectmodel::New<sofa::component::collision::BruteForceDetection>();
    this->addSlave(detection);
    detection->init();
}

//-------------------------------- eval------------------------------------
template<class DataTypes>
SReal EvalSurfaceDistance<DataTypes>::eval()
{
    if (!this->mstate1 || !this->mstate2 || !surfaceCM || !pointsCM || !intersection || !detection) return 0.0;

    const VecCoord& x0 = this->mstate1->read(core::ConstVecCoordId::restPosition())->getValue();
    const VecCoord& x1 = this->mstate1->read(core::ConstVecCoordId::position())->getValue();

    surfaceCM->computeBoundingTree(6);
    pointsCM->computeBoundingTree(6);
    intersection->setAlarmDistance(maxDist.getValue());
    intersection->setContactDistance(0.0);
    detection->setInstance(this);
    detection->setIntersectionMethod(intersection.get());
    sofa::helper::vector<std::pair<sofa::core::CollisionModel*, sofa::core::CollisionModel*> > vectCMPair;
    vectCMPair.push_back(std::make_pair(surfaceCM->getFirst(), pointsCM->getFirst()));

    detection->beginNarrowPhase();
    sout << "narrow phase detection between " <<surfaceCM->getClassName()<< " and " << pointsCM->getClassName() << sendl;
    detection->addCollisionPairs(vectCMPair);
    detection->endNarrowPhase();

    /// gets the pairs Triangle-Line detected in a radius lower than maxDist
    const core::collision::NarrowPhaseDetection::DetectionOutputMap& detectionOutputs = detection->getDetectionOutputs();

    core::collision::NarrowPhaseDetection::DetectionOutputMap::const_iterator it = detectionOutputs.begin();
    core::collision::NarrowPhaseDetection::DetectionOutputMap::const_iterator itend = detectionOutputs.end();

    xproj = x1;
    sofa::helper::vector<Real> dmin(xproj.size());
    std::fill(dmin.begin(),dmin.end(),(Real)(2*maxDist.getValue()));

    while (it != itend)
    {
        const ContactContainer* contacts = dynamic_cast<const ContactContainer*>(it->second.first);
        if (contacts != NULL)
        {
            sout << contacts->size() << " contacts detected." << sendl;
            for (const core::collision::DetectionOutput& o : *contacts)
            {
                if (o.elem.first.getCollisionModel() == surfaceCM)
                {
                    if (o.elem.second.getCollisionModel() == pointsCM)
                    {
                        int pi = o.elem.second.getIndex();
                        if (o.value < dmin[pi])
                        {
                            dmin[pi] = (Real)(o.value);
                            xproj[pi] = o.point[0];
                        }
                    }
                }
                else if (o.elem.second.getCollisionModel() == surfaceCM)
                {
                    if (o.elem.first.getCollisionModel() == pointsCM)
                    {
                        int pi = o.elem.first.getIndex();
                        if (o.value < dmin[pi])
                        {
                            dmin[pi] = (Real)(o.value);
                            xproj[pi] = o.point[1];
                        }
                    }
                }
            }
        }
        ++it;
    }
    return this->doEval(x1, xproj, x0);
}

//-------------------------------- draw------------------------------------
template<class DataTypes>
void EvalSurfaceDistance<DataTypes>::draw(const core::visual::VisualParams* )
{
    if (!this->f_drawEnabled.getValue())
        return;
    if (!this->mstate1 || !this->mstate2 || xproj.empty()) return;
    const VecCoord& x1 = this->mstate1->read(core::ConstVecCoordId::position())->getValue();
    const VecCoord& x2 = xproj; //this->mstate2->read(core::ConstVecCoordId::position())->getValue();
    this->doDraw(x1, x2);
}

} // namespace misc

} // namespace component

} // namespace sofa

#endif
