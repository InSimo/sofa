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
#ifndef SOFA_COMPONENT_CONSTRAINTSET_CONSTRAINTSOLVERIMPL_H
#define SOFA_COMPONENT_CONSTRAINTSET_CONSTRAINTSOLVERIMPL_H

#include <sofa/SofaGeneral.h>
#include <sofa/core/behavior/ConstraintSolver.h>
#include <sofa/simulation/common/MechanicalVisitor.h>
#include <sofa/defaulttype/FullMatrix.h>

namespace sofa
{

namespace component
{

namespace constraintset
{


class SOFA_CONSTRAINT_API ConstraintProblem
{
public:
    sofa::defaulttype::LPtrFullMatrix<double> W;
    sofa::defaulttype::FullVector<double> dFree, f;
    sofa::defaulttype::FullVector<double> _d;
    std::vector<core::behavior::ConstraintResolution*> constraintsResolutions;

    ConstraintProblem();
    virtual ~ConstraintProblem();

    double tolerance;
    int maxIterations;

    virtual void clear(int nbConstraints);
    
    void freeConstraintResolutions();

    void setDimension(int nbConstraints);

    int getDimension() const { return dimension; }
          double**              getW()       { return W.lptr(); }
    const double* const *       getW() const { return W.lptr(); }
          double* getDfree()       { return dFree.ptr(); }
    const double* getDfree() const { return dFree.ptr(); }
    double* getF()                 { return f.ptr();     }
    const double* getF()     const { return f.ptr();     }

    /// Solve the contained constraint problem, using included temporary arrays
    virtual void solveTimed(double tolerance, int maxIt, double timeout) = 0;
    /// Solve the contained constraint problem, using given temporary arrays
    /// This version is safe to call from several threads concurrently
    /// Returns a pair containing the number of iterations and residual error
    virtual std::pair<int,double> solveTimed(double tolerance, int maxIt, double timeout, const double* localDFree, double* localD, double* localF) const = 0;

    unsigned int getProblemId() const;

    void incrementProblemId();

    const std::vector<core::behavior::ConstraintResolution*>& getConstraintResolutions() const { return constraintsResolutions; }

protected:
    int dimension;
    unsigned int problemId;
};


class SOFA_CONSTRAINT_API ConstraintSolverImpl : public sofa::core::behavior::ConstraintSolver
{
public:
    SOFA_CLASS(ConstraintSolverImpl, sofa::core::behavior::ConstraintSolver);

    virtual ConstraintProblem* getConstraintProblem() = 0;

    /// Do not use the following LCPs until the next call to this function.
    /// This is used to prevent concurent access to the LCP when using a LCPForceFeedback through an haptic thread.
    virtual void lockConstraintProblem(sofa::core::objectmodel::BaseObject* from, ConstraintProblem* p1, ConstraintProblem* p2=NULL) = 0;
};



/// Gets the vector of constraint violation values
class MechanicalGetConstraintViolationVisitor : public simulation::BaseMechanicalVisitor
{
public:

    MechanicalGetConstraintViolationVisitor(const core::ConstraintParams* params /* PARAMS FIRST */, sofa::defaulttype::BaseVector *v)
        : simulation::BaseMechanicalVisitor(params)
        , cparams(params)
        , m_v(v)
    {
#ifdef SOFA_DUMP_VISITOR_INFO
        setReadWriteVectors();
#endif
    }

    virtual Result fwdConstraintSet(simulation::Node* node, core::behavior::BaseConstraintSet* cSet)
    {
        if (core::behavior::BaseConstraintSet *c=core::behavior::BaseConstraintSet::DynamicCast(cSet))
        {
            const bool applyConstraint = (c->d_isConstitutiveConstraint.getValue(cparams) == cparams->isAssemblingConstitutiveConstraints() );
            if (!applyConstraint)
            {
                return RESULT_CONTINUE;
            }

            ctime_t t0 = begin(node, c);
            c->getConstraintViolation(cparams /* PARAMS FIRST */, m_v);
            end(node, c, t0);
        }
        return RESULT_CONTINUE;
    }

    /// This visitor must go through all mechanical mappings, even if isMechanical flag is disabled
    virtual bool stopAtMechanicalMapping(simulation::Node* /*node*/, core::BaseMapping* /*map*/)
    {
        return false; // !map->isMechanical();
    }

    /// Return a class name for this visitor
    /// Only used for debugging / profiling purposes
    virtual const char* getClassName() const { return "MechanicalGetConstraintViolationVisitor";}

#ifdef SOFA_DUMP_VISITOR_INFO
    void setReadWriteVectors()
    {
    }
#endif

private:
    /// Constraint parameters
    const sofa::core::ConstraintParams *cparams;

    /// Vector for constraint values
    sofa::defaulttype::BaseVector* m_v;
};

} // namespace constraintset

} // namespace component

} // namespace sofa

#endif
