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

#include <SofaConstraint/ConstraintSolverImpl.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/helper/AdvancedTimer.h>

namespace sofa
{

namespace component
{

namespace constraintset
{

ConstraintProblem::ConstraintProblem()
    : tolerance(0.00001), maxIterations(1000),
      dimension(0), problemId(0)
{
}

ConstraintProblem::~ConstraintProblem()
{
    freeConstraintResolutions();
}

void ConstraintProblem::freeConstraintResolutions()
{
    for (unsigned int i = 0; i<constraintsResolutions.size(); i++)
    {
        if (constraintsResolutions[i] != nullptr)
        {
            delete constraintsResolutions[i];
            constraintsResolutions[i] = nullptr;
        }
    }
}

void ConstraintProblem::setDimension(int nbConstraints)
{
    dimension = nbConstraints;
}

void ConstraintProblem::incrementProblemId()
{
    static unsigned int counter = 0;
    problemId = ++counter;
}

void ConstraintProblem::clear(int nbConstraints)
{
    sofa::helper::AdvancedTimer::stepBegin("ConstraintProblem-Resize");
    {
        setDimension(nbConstraints);
        W.resize(nbConstraints, nbConstraints);
        dFree.resize(nbConstraints);
        f.resize(nbConstraints);
        _d.resize(nbConstraints);
    }
    sofa::helper::AdvancedTimer::stepEnd("ConstraintProblem-Resize");

    sofa::helper::AdvancedTimer::stepBegin("ConstraintProblem-FreeCR");
    {
        freeConstraintResolutions();
        constraintsResolutions.resize(nbConstraints, nullptr);
        incrementProblemId();
    }
    sofa::helper::AdvancedTimer::stepEnd("ConstraintProblem-FreeCR");
}

unsigned int ConstraintProblem::getProblemId() const
{
    return problemId;
}

} // namespace constraintset

} // namespace component

} // namespace sofa
