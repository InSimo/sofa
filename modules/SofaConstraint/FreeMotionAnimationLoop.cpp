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
#include <SofaConstraint/FreeMotionAnimationLoop.h>
#include <sofa/core/visual/VisualParams.h>

#include <SofaConstraint/LCPConstraintSolver.h>

#include <sofa/core/ObjectFactory.h>
#include <sofa/core/VecId.h>

#include <sofa/helper/AdvancedTimer.h>

#include <sofa/simulation/common/BehaviorUpdatePositionVisitor.h>
#include <sofa/simulation/common/MechanicalOperations.h>
#include <sofa/simulation/common/SolveVisitor.h>
#include <sofa/simulation/common/VectorOperations.h>
#include <sofa/simulation/common/AnimateBeginEvent.h>
#include <sofa/simulation/common/AnimateEndEvent.h>
#include <sofa/simulation/common/PropagateEventVisitor.h>
#include <sofa/simulation/common/BehaviorUpdatePositionVisitor.h>
#include <sofa/simulation/common/UpdateContextVisitor.h>
#include <sofa/simulation/common/UpdateMappingVisitor.h>
#include <sofa/simulation/common/UpdateMappingEndEvent.h>
#include <sofa/simulation/common/UpdateBoundingBoxVisitor.h>
#include <sofa/simulation/common/MechanicalVisitor.h>


namespace sofa
{

namespace component
{

namespace animationloop
{

using namespace core::behavior;
using namespace sofa::simulation;

FreeMotionAnimationLoop::FreeMotionAnimationLoop(simulation::Node* gnode)
    : Inherit(gnode)
    , displayTime(initData(&displayTime,false,"displayTime","Dump the computation times"))
    , m_solveVelocityConstraintFirst(initData(&m_solveVelocityConstraintFirst , false, "solveVelocityConstraintFirst", "solve separately velocity constraint violations before position constraint violations"))
    , d_postStabilize(initData(&d_postStabilize,false,"postStabilize","If true perform a post stabilization step."))
    , constraintSolver(NULL)
    , defaultSolver(NULL)
{
}

FreeMotionAnimationLoop::~FreeMotionAnimationLoop()
{
    if (defaultSolver != NULL)
        defaultSolver.reset();
}

void FreeMotionAnimationLoop::parse ( sofa::core::objectmodel::BaseObjectDescription* arg )
{
    this->simulation::CollisionAnimationLoop::parse(arg);

    defaultSolver = sofa::core::objectmodel::New<constraintset::LCPConstraintSolver>();
    defaultSolver->parse(arg);
}


void FreeMotionAnimationLoop::init()
{

    {
    simulation::common::VectorOperations vop(core::ExecParams::defaultInstance(), this->getContext());
    MultiVecDeriv dx(&vop, core::VecDerivId::dx() ); dx.realloc( &vop, true, true );
    MultiVecDeriv df(&vop, core::VecDerivId::dforce() ); df.realloc( &vop, true, true );
    }




    getContext()->get(constraintSolver, core::objectmodel::BaseContext::SearchDown);
    if (constraintSolver == NULL && defaultSolver != NULL)
    {
        serr << "No ConstraintSolver found, using default LCPConstraintSolver" << sendl;
        this->getContext()->addObject(defaultSolver);
        constraintSolver = defaultSolver.get();
        defaultSolver = NULL;
    }
    else
    {
        defaultSolver.reset();
    }
}


void FreeMotionAnimationLoop::step(const sofa::core::ExecParams* params /* PARAMS FIRST */, double dt)
{
    if (dt == 0)
        dt = this->gnode->getDt();

    sofa::helper::AdvancedTimer::begin("Animate");

    sofa::helper::AdvancedTimer::stepBegin("AnimationStep");

    double startTime = this->gnode->getTime();

    simulation::common::VectorOperations vop(params, this->getContext());
    simulation::common::MechanicalOperations mop(params, this->getContext());

    MultiVecCoord pos(&vop, core::VecCoordId::position() );
    MultiVecDeriv vel(&vop, core::VecDerivId::velocity() );
    MultiVecDeriv dx(&vop, constraintSolver->getDx());

    MultiVecCoord freePos(&vop, core::VecCoordId::freePosition() );
    MultiVecDeriv freeVel(&vop, core::VecDerivId::freeVelocity() );

    core::ConstraintParams cparams(*params);
    cparams.setX(freePos);
    cparams.setV(freeVel);
    cparams.setDx(constraintSolver->getDx());
    cparams.setLambda(constraintSolver->getLambda());
    cparams.setOrder(m_solveVelocityConstraintFirst.getValue() ? core::ConstraintParams::VEL : core::ConstraintParams::POS_AND_VEL);

    {
        MultiVecDeriv dx(&vop, cparams.dx() ); dx.realloc( &vop, true, true );
        MultiVecDeriv lambda(&vop, cparams.lambda() ); lambda.realloc( &vop, true, true );
    }

    // This solver will work in freePosition and freeVelocity vectors.
    // We need to initialize them if it's not already done.
    sofa::helper::AdvancedTimer::stepBegin("MechanicalVInitVisitor");
    simulation::MechanicalVInitVisitor< core::V_COORD >(params, core::VecCoordId::freePosition(), core::ConstVecCoordId::position(), true).execute(this->gnode);
    simulation::MechanicalVInitVisitor< core::V_DERIV >(params, core::VecDerivId::freeVelocity(), core::ConstVecDerivId::velocity(), true).execute(this->gnode);

    sofa::helper::AdvancedTimer::stepEnd("MechanicalVInitVisitor");


#ifdef SOFA_DUMP_VISITOR_INFO
    simulation::Visitor::printNode("Step");
#endif

    {
        sofa::helper::AdvancedTimer::stepBegin("AnimateBeginEvent");
        AnimateBeginEvent ev ( dt );
        PropagateEventVisitor act ( params, &ev );
        this->gnode->execute ( act );
        sofa::helper::AdvancedTimer::stepEnd("AnimateBeginEvent");
    }

    BehaviorUpdatePositionVisitor beh(params , dt);

    using helper::system::thread::CTime;
    using sofa::helper::AdvancedTimer;

    double time = 0.0;
    //double timeTotal = 0.0;
    double timeScale = 1000.0 / (double)CTime::getTicksPerSec();

    if (displayTime.getValue())
    {
        time = (double) CTime::getTime();
        //timeTotal = (double) CTime::getTime();
    }

    // Update the BehaviorModels
    // Required to allow the RayPickInteractor interaction
    if (f_printLog.getValue())
        serr << "updatePos called" << sendl;

    AdvancedTimer::stepBegin("UpdatePosition");
    this->gnode->execute(&beh);
    AdvancedTimer::stepEnd("UpdatePosition");

    if (f_printLog.getValue())
        serr << "updatePos performed - beginVisitor called" << sendl;

    simulation::MechanicalBeginIntegrationVisitor beginVisitor(params, dt);
    this->gnode->execute(&beginVisitor);

    if (f_printLog.getValue())
        serr << "beginVisitor performed - SolveVisitor for freeMotion is called" << sendl;

    // Mapping geometric stiffness coming from previous lambda.
    {
        simulation::MechanicalVOpVisitor lambdaMultInvDt(params, cparams.lambda(), sofa::core::ConstMultiVecId::null(), cparams.lambda(), 1.0 / dt);
        lambdaMultInvDt.setMapped(true);
        this->getContext()->executeVisitor(&lambdaMultInvDt);
        simulation::MechanicalComputeGeometricStiffness geometricStiffnessVisitor(&mop.mparams, cparams.lambda());
        this->getContext()->executeVisitor(&geometricStiffnessVisitor);
    }


    // Free Motion
    AdvancedTimer::stepBegin("FreeMotion");
    simulation::SolveVisitor freeMotion(params, dt, true);
    this->gnode->execute(&freeMotion);
    AdvancedTimer::stepEnd("FreeMotion");

    {
        mop.projectResponse(freeVel);
        mop.propagateDx(freeVel, true);

        if (cparams.constOrder() == core::ConstraintParams::POS ||
            cparams.constOrder() == core::ConstraintParams::POS_AND_VEL)
        {
            // xfree = x + vfree*dt
            simulation::MechanicalVOpVisitor freePosEqPosPlusFreeVelDt(params, freePos, pos, freeVel, dt);
            freePosEqPosPlusFreeVelDt.setMapped(true);
            this->getContext()->executeVisitor(&freePosEqPosPlusFreeVelDt);
        }
    }

    if (f_printLog.getValue())
        serr << " SolveVisitor for freeMotion performed" << sendl;

    if (displayTime.getValue())
    {
        sout << " >>>>> Begin display FreeMotionAnimationLoop time" << sendl;
        sout <<" Free Motion " << ((double)CTime::getTime() - time) * timeScale << " ms" << sendl;

        time = (double)CTime::getTime();
    }

    // Collision detection and response creation
    AdvancedTimer::stepBegin("Collision");
    computeCollision(params);
    AdvancedTimer::stepEnd  ("Collision");

    if (displayTime.getValue())
    {
        sout << " computeCollision " << ((double) CTime::getTime() - time) * timeScale << " ms" << sendl;
        time = (double)CTime::getTime();
    }

    // Solve constraints
    if (constraintSolver)
    {
        AdvancedTimer::stepBegin("ConstraintSolver");

        if (cparams.constOrder() == core::ConstraintParams::VEL )
        {
            constraintSolver->solveConstraint(&cparams, vel);
            // x_t+1 = x_t + ( vfree + dv ) * dt
            pos.eq(pos, vel, dt);
        }
        else
        {
            constraintSolver->solveConstraint(&cparams, pos, vel);
        }

        AdvancedTimer::stepEnd("ConstraintSolver");

    }

    if ( displayTime.getValue() )
    {
        sout << " contactCorrections " << ((double)CTime::getTime() - time) * timeScale << " ms" <<sendl;
        sout << "<<<<<< End display FreeMotionAnimationLoop time." << sendl;
    }

    simulation::MechanicalEndIntegrationVisitor endVisitor(params /* PARAMS FIRST */, dt);
    this->gnode->execute(&endVisitor);

    mop.projectPositionAndVelocity(pos, vel);
    mop.propagateX(pos);
    mop.propagateDx(vel, true);

    this->gnode->setTime ( startTime + dt );
    this->gnode->execute<UpdateSimulationContextVisitor>(params);  // propagate time

    {
        AnimateEndEvent ev ( dt );
        PropagateEventVisitor act ( params, &ev );
        this->gnode->execute ( act );
    }

    if (d_postStabilize.getValue())
    {
        cparams.setX(pos);
        cparams.setV(vel);
        constraintSolver->postStabilize(&cparams, pos, vel);
        // linearize everything again against the post stabilized positions
        mop.propagateX(pos);
    }

    sofa::helper::AdvancedTimer::stepBegin("UpdateMapping");
    //Visual Information update: Ray Pick add a MechanicalMapping used as VisualMapping
    this->gnode->execute<UpdateMappingVisitor>(params);
    //	sofa::helper::AdvancedTimer::step("UpdateMappingEndEvent");
    {
        UpdateMappingEndEvent ev(dt);
        PropagateEventVisitor act(params, &ev);
        this->gnode->execute(act);
    }
    sofa::helper::AdvancedTimer::stepEnd("UpdateMapping");

#ifndef SOFA_NO_UPDATE_BBOX
    sofa::helper::AdvancedTimer::stepBegin("UpdateBBox");
    this->gnode->execute<UpdateBoundingBoxVisitor>(params);
    sofa::helper::AdvancedTimer::stepEnd("UpdateBBox");
#endif
#ifdef SOFA_DUMP_VISITOR_INFO
    simulation::Visitor::printCloseNode("Step");
#endif

    sofa::helper::AdvancedTimer::stepEnd("AnimationStep");
    sofa::helper::AdvancedTimer::end("Animate");
}


SOFA_DECL_CLASS(FreeMotionAnimationLoop)

int FreeMotionAnimationLoopClass = core::RegisterObject("Constraint solver")
        .add< FreeMotionAnimationLoop >()
        .addAlias("FreeMotionMasterSolver")
        ;

} // namespace animationloop

} // namespace component

} // namespace sofa
