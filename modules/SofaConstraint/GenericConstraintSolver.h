/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*                (c) 2006-2011 MGH, INRIA, USTL, UJF, CNRS                    *
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
#ifndef SOFA_COMPONENT_CONSTRAINTSET_GENERICCONSTRAINTSOLVER_H
#define SOFA_COMPONENT_CONSTRAINTSET_GENERICCONSTRAINTSOLVER_H

#include <sofa/SofaGeneral.h>

#include <SofaConstraint/ConstraintSolverImpl.h>
#include <sofa/core/behavior/BaseConstraint.h>
#include <sofa/core/behavior/ConstraintSolver.h>
#include <sofa/core/behavior/BaseConstraintCorrection.h>

#include <sofa/simulation/common/Node.h>
#include <sofa/simulation/common/MechanicalVisitor.h>

#include <SofaBaseLinearSolver/FullMatrix.h>
#include <SofaBaseLinearSolver/SparseMatrix.h>

#include <sofa/helper/map.h>

namespace sofa
{

namespace component
{

namespace constraintset
{

class GenericConstraintSolver;

class SOFA_CONSTRAINT_API GenericConstraintProblem : public ConstraintProblem
{
public:
    sofa::component::linearsolver::FullVector<double> _d;
	std::vector<core::behavior::ConstraintResolution*> constraintsResolutions;
	bool scaleTolerance, allVerified, unbuilt;
	double sor;
	double sceneTime;
    double currentError;
    int currentIterations;

	// For unbuilt version :
    sofa::component::linearsolver::SparseMatrix<double> Wdiag;
    std::list<unsigned int> constraints_sequence;
	bool change_sequence;

	typedef std::vector< core::behavior::BaseConstraintCorrection* > ConstraintCorrections;
	typedef std::vector< core::behavior::BaseConstraintCorrection* >::iterator ConstraintCorrectionIterator;

	std::vector< ConstraintCorrections > cclist_elems;
	

	GenericConstraintProblem() : scaleTolerance(true), allVerified(false), sor(1.0)
        , sceneTime(0.0), currentError(0.0), currentIterations(0)
		, change_sequence(false) {}
	~GenericConstraintProblem() { freeConstraintResolutions(); }

	void clear(int nbConstraints);
	void freeConstraintResolutions();
	void solveTimed(double tol, int maxIt, double timeout) override;
    std::pair<int,double> solveTimed(double tolerance, int maxIt, double timeout, const double* localDFree, double* localD, double* localF) const override;

	void gaussSeidel(GenericConstraintSolver* solver);
	std::pair<int,double> gaussSeidel(GenericConstraintSolver* solver, double tol, int maxIt, double timeout, const double* localDFree, double* localD, double* localF) const;
	void unbuiltGaussSeidel(GenericConstraintSolver* solver = NULL, double timeout=0);

    int getNumConstraints();
    int getNumConstraintGroups();
};

class SOFA_CONSTRAINT_API GenericConstraintSolver : public ConstraintSolverImpl
{
	typedef std::vector<core::behavior::BaseConstraintCorrection*> list_cc;
	typedef std::vector<list_cc> VecListcc;
	typedef sofa::core::MultiVecId MultiVecId;

public:
	SOFA_CLASS(GenericConstraintSolver, ConstraintSolverImpl);
protected:
	GenericConstraintSolver();
	virtual ~GenericConstraintSolver();
public:
	void init();

	bool prepareStates(const core::ConstraintParams * /*cParams*/, MultiVecId res1, MultiVecId res2=MultiVecId::null());
	bool buildSystem(const core::ConstraintParams * /*cParams*/, MultiVecId res1, MultiVecId res2=MultiVecId::null());
    void rebuildSystem(double massFactor, double forceFactor);
    bool solveSystem(const core::ConstraintParams * /*cParams*/, MultiVecId res1, MultiVecId res2=MultiVecId::null());
	bool applyCorrection(const core::ConstraintParams * /*cParams*/, MultiVecId res1, MultiVecId res2=MultiVecId::null());
    void computeResidual(const core::ExecParams* /*params*/);

	Data<bool> displayTime;
	Data<int> maxIt;
	Data<double> tolerance, sor;
	Data<bool> scaleTolerance, allVerified, schemeCorrection;
	Data<bool> unbuilt;
	Data<bool> computeGraphs;
	Data<std::map < std::string, sofa::helper::vector<double> > > graphErrors, graphConstraints, graphForces, graphViolations;

	Data<int> currentNumConstraints;
	Data<int> currentNumConstraintGroups;
	Data<int> currentIterations;
	Data<double> currentError;
    Data<bool> reverseAccumulateOrder;

	ConstraintProblem* getConstraintProblem();
	void lockConstraintProblem(sofa::core::objectmodel::BaseObject* from, ConstraintProblem* p1, ConstraintProblem* p2=0);

protected:
    enum { CP_BUFFER_SIZE = 10 };
    sofa::helper::fixed_array<GenericConstraintProblem,CP_BUFFER_SIZE> m_cpBuffer;
    sofa::helper::fixed_array<bool,CP_BUFFER_SIZE> m_cpIsLocked;
	GenericConstraintProblem *current_cp, *last_cp;
	std::vector<core::behavior::BaseConstraintCorrection*> constraintCorrections;

    void clearConstraintProblemLocks();

	simulation::Node *context;

    sofa::helper::system::thread::CTime timer;
    sofa::helper::system::thread::CTime timerTotal;

	double time;
	double timeTotal;
	double timeScale;
};


class SOFA_CONSTRAINT_API MechanicalGetConstraintResolutionVisitor : public simulation::BaseMechanicalVisitor
{
public:
    MechanicalGetConstraintResolutionVisitor(const core::ConstraintParams* params, std::vector<core::behavior::ConstraintResolution*>& res)
    : simulation::BaseMechanicalVisitor(params)
	, cparams(params)
	, _res(res)
	, _offset(0)
	{
#ifdef SOFA_DUMP_VISITOR_INFO
      setReadWriteVectors();
#endif
    }

    virtual Result fwdConstraintSet(simulation::Node* node, core::behavior::BaseConstraintSet* cSet)
    {
      if (core::behavior::BaseConstraint *c=core::behavior::BaseConstraint::DynamicCast(cSet))
      {
        ctime_t t0 = begin(node, c);
        c->getConstraintResolution(cparams, _res, _offset);
        end(node, c, t0);
      }
      return RESULT_CONTINUE;
    }

    /// Return a class name for this visitor
    /// Only used for debugging / profiling purposes
    virtual const char* getClassName() const
    {
        return "MechanicalGetConstraintResolutionVisitor";
    }

    virtual bool isThreadSafe() const
    {
        return false;
    }

    // This visitor must go through all mechanical mappings, even if isMechanical flag is disabled
    virtual bool stopAtMechanicalMapping(simulation::Node* /*node*/, core::BaseMapping* /*map*/)
    {
        return false; // !map->isMechanical();
    }

#ifdef SOFA_DUMP_VISITOR_INFO
    void setReadWriteVectors() { }
#endif
private:
	/// Constraint parameters
	const sofa::core::ConstraintParams *cparams;

    std::vector<core::behavior::ConstraintResolution*>& _res;
    unsigned int _offset;
};

} // namespace constraintset

} // namespace component

} // namespace sofa

#endif
