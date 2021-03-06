/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*            (c) 2006-2021 INRIA, USTL, UJF, CNRS, MGH, InSimo                *
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
*                              SOFA :: Framework                              *
*                                                                             *
* Authors: The SOFA Team (see Authors.txt)                                    *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_CORE_BEHAVIOR_PAIRINTERACTIONFORCEFIELD_H
#define SOFA_CORE_BEHAVIOR_PAIRINTERACTIONFORCEFIELD_H

#include <sofa/SofaFramework.h>
#include <sofa/core/MultiVecId.h>
#include <sofa/core/MechanicalParams.h>
#include <sofa/core/behavior/BaseInteractionForceField.h>
#include <sofa/core/behavior/MechanicalState.h>

#include <sofa/core/objectmodel/Data.h>
#include <sofa/defaulttype/Vec.h>

#include <sofa/defaulttype/Vec3Types.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/helper/OptionsGroup.h>
#include <sofa/core/visual/VisualParams.h>

namespace sofa
{

namespace core
{

namespace behavior
{

/**
 *  \brief Component computing forces between a pair of simulated body.
 *
 *  This class define the abstract API common to interaction force fields
 *  between a pair of bodies using a given type of DOFs.
 */
template<class TDataTypes>
class PairInteractionForceField : public BaseInteractionForceField
{
public:
    SOFA_ABSTRACT_CLASS_UNIQUE((PairInteractionForceField<TDataTypes>), ((BaseInteractionForceField)));

    typedef TDataTypes DataTypes;
    typedef typename DataTypes::Real             Real;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::Deriv Deriv;

    typedef core::objectmodel::Data<VecCoord>    DataVecCoord;
    typedef core::objectmodel::Data<VecDeriv>    DataVecDeriv;

protected:
    PairInteractionForceField(MechanicalState<DataTypes> *mm1 = NULL, MechanicalState<DataTypes> *mm2 = NULL);

    virtual ~PairInteractionForceField();
public:
    virtual void init();

    /// Retrieve the associated MechanicalState
    MechanicalState<DataTypes>* getMState1() { return mstate1.get(); }
    BaseMechanicalState* getMechModel1() { return mstate1.get(); }
    /// Retrieve the associated MechanicalState
    MechanicalState<DataTypes>* getMState2() { return mstate2.get(); }
    BaseMechanicalState* getMechModel2() { return mstate2.get(); }

    /// Set the Object1 path
    void setPathObject1(const std::string & path) { mstate1.setPath(path); }
    /// Set the Object2 path
    void setPathObject2(const std::string & path) { mstate2.setPath(path); }
    /// Retrieve the Object1 path
    std::string getPathObject1() const { return mstate1.getPath(); }
    /// Retrieve the Object2 path
    std::string getPathObject2() const { return mstate2.getPath(); }


    /// @name Vector operations
    /// @{

    /// Given the current position and velocity states, update the current force
    /// vector by computing and adding the forces associated with this
    /// ForceField.
    ///
    /// If the ForceField can be represented as a matrix, this method computes
    /// $ f += B v + K x $
    ///
    /// This method retrieves the force, x and v vector from the two MechanicalState
    /// and call the internal addForce(VecDeriv&,VecDeriv&,const VecCoord&,const VecCoord&,const VecDeriv&,const VecDeriv&)
    /// method implemented by the component.
    virtual void addForce(const MechanicalParams* mparams /* PARAMS FIRST */, MultiVecDerivId fId );

    /// Compute the force derivative given a small displacement from the
    /// position and velocity used in the previous call to addForce().
    ///
    /// The derivative should be directly derived from the computations
    /// done by addForce. Any forces neglected in addDForce will be integrated
    /// explicitly (i.e. using its value at the beginning of the timestep).
    ///
    /// If the ForceField can be represented as a matrix, this method computes
    /// $ df += kFactor K dx + bFactor B dx $
    ///
    /// This method retrieves the force and dx vector from the two MechanicalState
    /// and call the internal addDForce(VecDeriv&,VecDeriv&,const VecDeriv&,const VecDeriv&,double,double)
    /// method implemented by the component.
    virtual void addDForce(const MechanicalParams* mparams /* PARAMS FIRST */, MultiVecDerivId dfId );


    /// Get the potential energy associated to this ForceField.
    ///
    /// Used to extimate the total energy of the system by some
    /// post-stabilization techniques.
    ///
    /// This method retrieves the x vector from the MechanicalState and call
    /// the internal getPotentialEnergy(const VecCoord&,const VecCoord&) method implemented by
    /// the component.
    virtual double getPotentialEnergy(const MechanicalParams* mparams) const;

    /// Given the current position and velocity states, update the current force
    /// vector by computing and adding the forces associated with this
    /// ForceField.
    ///
    /// If the ForceField can be represented as a matrix, this method computes
    /// $ f += B v + K x $
    ///
    /// This method must be implemented by the component, and is usually called
    /// by the generic ForceField::addForce() method.

    virtual void addForce(const MechanicalParams* mparams /* PARAMS FIRST */, DataVecDeriv& f1, DataVecDeriv& f2, const DataVecCoord& x1, const DataVecCoord& x2, const DataVecDeriv& v1, const DataVecDeriv& v2 )=0;

    /// Compute the force derivative given a small displacement from the
    /// position and velocity used in the previous call to addForce().
    ///
    /// The derivative should be directly derived from the computations
    /// done by addForce. Any forces neglected in addDForce will be integrated
    /// explicitly (i.e. using its value at the beginning of the timestep).
    ///
    /// If the ForceField can be represented as a matrix, this method computes
    /// $ df += kFactor K dx + bFactor B dx $
    ///
    /// This method must be implemented by the component, and is usually called
    /// by the generic PairInteractionForceField::addDForce() method.
    ///
    /// To support old components that implement the deprecated addForce method
    /// without scalar coefficients, it defaults to using a temporaty vector to
    /// compute $ K dx $ and then manually scaling all values by kFactor.

    virtual void addDForce(const MechanicalParams* mparams /* PARAMS FIRST */, DataVecDeriv& df1, DataVecDeriv& df2, const DataVecDeriv& dx1, const DataVecDeriv& dx2)=0;

    /// Get the potential energy associated to this ForceField.
    ///
    /// Used to extimate the total energy of the system by some
    /// post-stabilization techniques.
    ///
    /// This method must be implemented by the component, and is usually called
    /// by the generic ForceField::getPotentialEnergy() method.

    virtual double getPotentialEnergy(const MechanicalParams* mparams /* PARAMS FIRST */, const DataVecCoord& x1, const DataVecCoord& x2) const=0;


    /// @}

    /// Pre-construction check method called by ObjectFactory.
    /// Check that DataTypes matches the MechanicalState.
    template<class T>
    static bool canCreate(T* obj, objectmodel::BaseContext* context, objectmodel::BaseObjectDescription* arg)
    {
        MechanicalState<DataTypes>* mstate1 = NULL;
        MechanicalState<DataTypes>* mstate2 = NULL;
        std::string object1 = arg->getAttribute("object1","@./");
        std::string object2 = arg->getAttribute("object2","@./");
        if (object1.empty()) object1 = "@./";
        if (object2.empty()) object2 = "@./";

        context->findLinkDest(mstate1, object1, NULL);
        context->findLinkDest(mstate2, object2, NULL);

        if (!mstate1 || !mstate2)
        {
            return false;
        }

        return BaseInteractionForceField::canCreate(obj, context, arg);
    }

    /// Construction method called by ObjectFactory.
    template<class T>
    static typename T::SPtr create(T* /*p0*/, core::objectmodel::BaseContext* context, core::objectmodel::BaseObjectDescription* arg)
    {
        typename T::SPtr obj = sofa::core::objectmodel::New<T>();

        if (context)
            context->addObject(obj);

        if (arg)
        {
            std::string object1 = arg->getAttribute("object1","");
            std::string object2 = arg->getAttribute("object2","");
            if (!object1.empty())
            {
                arg->setAttribute("object1", object1.c_str());
            }
            if (!object2.empty())
            {
                arg->setAttribute("object2", object2.c_str());
            }
            obj->parse(arg);
        }

        return obj;
    }

    virtual std::string getTemplateName() const
    {
        return templateName(this);
    }

    static std::string templateName(const PairInteractionForceField<DataTypes>* = NULL)
    {
        return DataTypes::Name();
    }

    template<class T>
    static std::string shortName(const T* ptr = NULL)
    {
        std::string name = Inherit1::shortName(ptr);
        sofa::helper::replaceAll(name, "InteractionForceField", "IFF");
        sofa::helper::replaceAll(name, "ForceField", "FF");
        return name;
    }


    bool storeStatsAddForces() const 
    { 
        return d_computeStatsOnAddForce.getValue() || d_drawStatsForcesObj1.getValue() || d_drawStatsForcesObj2.getValue();
    }

    Data< bool > d_computeStatsOnAddForce;

    Data< unsigned int > d_statsNumberOfActiveDofsObj1;

    Data< Real > d_statsMaxAddForceObj1;
    Data< Real > d_statsMeanAddForceObj1;
    Data< Real > d_statsMedianAddForceObj1;
    Data< Real > d_statsMinAddForceObj1;

    Data< unsigned int > d_statsNumberOfActiveDofsObj2;
    Data< Real > d_statsMaxAddForceObj2;
    Data< Real > d_statsMeanAddForceObj2;
    Data< Real > d_statsMedianAddForceObj2;
    Data< Real > d_statsMinAddForceObj2;

    Data < bool >  d_drawStatsForcesObj1;
    Data < sofa::helper::OptionsGroup >  d_drawStatsForcesColorObj1;
    Data< bool > d_drawStatsActiveDofsObj1;
    Data < sofa::helper::OptionsGroup >  d_drawStatsActiveDofsColorObj1;

    Data < bool >  d_drawStatsForcesObj2;
    Data < sofa::helper::OptionsGroup >  d_drawStatsForcesColorObj2;
    Data< bool > d_drawStatsActiveDofsObj2;
    Data < sofa::helper::OptionsGroup >  d_drawStatsActiveDofsColorObj2;

    Data< float > d_drawStatsArrowWidth;
    Data< float > d_drawStatsArrowScaleLength;
    Data< float > d_drawStatsSpheresRadius;

    // vectors storing increments from methods addForce() and addDForce()
    Data< VecDeriv > d_statsAddForcesObj1;
    Data< VecDeriv > d_statsAddForcesObj2;

    void draw(const core::visual::VisualParams* vparams);

protected:
    SingleLink<PairInteractionForceField<DataTypes>, MechanicalState<DataTypes>, BaseLink::FLAG_STOREPATH|BaseLink::FLAG_STRONGLINK> mstate1;
    SingleLink<PairInteractionForceField<DataTypes>, MechanicalState<DataTypes>, BaseLink::FLAG_STOREPATH|BaseLink::FLAG_STRONGLINK> mstate2;

    void computeStatsOnAddForce(const unsigned int objectID);
    virtual void getStatsActivePoints(helper::vector<core::topology::Topology::PointID>& pointsInTopology, sofa::core::topology::BaseMeshTopology* topology, MechanicalState<DataTypes>* mechState);

    sofa::core::topology::BaseMeshTopology* m_statsTopology1 = nullptr;
    sofa::core::topology::BaseMeshTopology* m_statsTopology2 = nullptr;

};

#if defined(SOFA_EXTERN_TEMPLATE) && !defined(SOFA_BUILD_CORE)
#ifndef SOFA_FLOAT
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Vec3dTypes>;
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Vec2dTypes>;
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Vec1dTypes>;
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Rigid3dTypes>;
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Rigid2dTypes>;
#endif

#ifndef SOFA_DOUBLE
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Vec3fTypes>;
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Vec2fTypes>;
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Vec1fTypes>;
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Rigid3fTypes>;
extern template class SOFA_CORE_API PairInteractionForceField<defaulttype::Rigid2fTypes>;
#endif
#endif

} // namespace behavior

} // namespace core

} // namespace sofa

#endif
