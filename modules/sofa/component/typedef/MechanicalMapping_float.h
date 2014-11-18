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



// File automatically generated by "generateTypedef"


#ifndef SOFA_TYPEDEF_MechanicalMapping_float_H
#define SOFA_TYPEDEF_MechanicalMapping_float_H

//Default files containing the declaration of the vector type
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <sofa/defaulttype/Mat.h>

//Default files needed to create a Mechanical Mapping
#include <sofa/core/behavior/MechanicalState.h>
#include <sofa/core/behavior/MechanicalMapping.h>


#include <SofaRigid/ArticulatedSystemMapping.h>
#include <SofaBaseMechanics/BarycentricMapping.h>
#include <SofaMiscMapping/BeamLinearMapping.h>
#include <SofaMiscMapping/CenterOfMassMapping.h>
#include <SofaMiscMapping/CenterPointMechanicalMapping.h>
#include <SofaMiscMapping/CurveMapping.h>
#include <SofaMiscMapping/ExternalInterpolationMapping.h>
#include <SofaBaseMechanics/IdentityMapping.h>
#include <SofaRigid/LineSetSkinningMapping.h>
#include <SofaTopologyMapping/Mesh2PointMechanicalMapping.h>
#include <SofaRigid/RigidMapping.h>
#include <SofaRigid/RigidRigidMapping.h>
#include <SofaTopologyMapping/SimpleTesselatedTetraMechanicalMapping.h>
#include <SofaRigid/SkinningMapping.h>
#include <SofaBaseMechanics/SubsetMapping.h>
#include <SofaMiscMapping/TubularMapping.h>



//---------------------------------------------------------------------------------------------
//Typedef for ArticulatedSystemMapping
typedef sofa::component::mapping::ArticulatedSystemMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<1, float>, sofa::defaulttype::Vec<1, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> > > > ArticulatedSystemMechanicalMapping1f_to_Rigid3f;



//---------------------------------------------------------------------------------------------
//Typedef for BarycentricMapping
typedef sofa::component::mapping::BarycentricMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> > > > BarycentricMechanicalMapping3f_to_Rigid3f;
typedef sofa::component::mapping::BarycentricMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > BarycentricMechanicalMapping3f_to_3f;



//---------------------------------------------------------------------------------------------
//Typedef for BeamLinearMapping
typedef sofa::component::mapping::BeamLinearMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > BeamLinearMechanicalMappingRigid3f_to_3f;



//---------------------------------------------------------------------------------------------
//Typedef for CenterOfMassMapping
typedef sofa::component::mapping::CenterOfMassMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > CenterOfMassMechanicalMappingRigid3f_to_3f;
typedef sofa::component::mapping::CenterOfMassMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<2, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<2, float>, sofa::defaulttype::Vec<2, float>, float> > > > CenterOfMassMechanicalMappingRigid2f_to_2f;



//---------------------------------------------------------------------------------------------
//Typedef for CenterPointMechanicalMapping
typedef sofa::component::mapping::CenterPointMechanicalMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > CenterPointMechanicalMechanicalMapping3f_to_3f;



//---------------------------------------------------------------------------------------------
//Typedef for CurveMapping
typedef sofa::component::mapping::CurveMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> > > > CurveMechanicalMapping3f_to_Rigid3f;



//---------------------------------------------------------------------------------------------
//Typedef for ExternalInterpolationMapping
typedef sofa::component::mapping::ExternalInterpolationMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > ExternalInterpolationMechanicalMapping3f_to_3f;
typedef sofa::component::mapping::ExternalInterpolationMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<2, float>, sofa::defaulttype::Vec<2, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<2, float>, sofa::defaulttype::Vec<2, float>, float> > > > ExternalInterpolationMechanicalMapping2f_to_2f;
typedef sofa::component::mapping::ExternalInterpolationMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<1, float>, sofa::defaulttype::Vec<1, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<1, float>, sofa::defaulttype::Vec<1, float>, float> > > > ExternalInterpolationMechanicalMapping1f_to_1f;



//---------------------------------------------------------------------------------------------
//Typedef for IdentityMapping
typedef sofa::component::mapping::IdentityMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > IdentityMechanicalMapping3f_to_3f;
typedef sofa::component::mapping::IdentityMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<2, float>, sofa::defaulttype::Vec<2, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<2, float>, sofa::defaulttype::Vec<2, float>, float> > > > IdentityMechanicalMapping2f_to_2f;
typedef sofa::component::mapping::IdentityMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<1, float>, sofa::defaulttype::Vec<1, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<1, float>, sofa::defaulttype::Vec<1, float>, float> > > > IdentityMechanicalMapping1f_to_1f;
typedef sofa::component::mapping::IdentityMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<6, float>, sofa::defaulttype::Vec<6, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<6, float>, sofa::defaulttype::Vec<6, float>, float> > > > IdentityMechanicalMapping6f_to_6f;
typedef sofa::component::mapping::IdentityMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> > > > IdentityMechanicalMappingRigid3f_to_Rigid3f;
typedef sofa::component::mapping::IdentityMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<2, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<2, float> > > > IdentityMechanicalMappingRigid2f_to_Rigid2f;
typedef sofa::component::mapping::IdentityMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > IdentityMechanicalMappingRigid3f_to_3f;
typedef sofa::component::mapping::IdentityMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<2, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<2, float>, sofa::defaulttype::Vec<2, float>, float> > > > IdentityMechanicalMappingRigid2f_to_2f;



//---------------------------------------------------------------------------------------------
//Typedef for LineSetSkinningMapping
typedef sofa::component::mapping::LineSetSkinningMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > LineSetSkinningMechanicalMappingRigid3f_to_3f;



//---------------------------------------------------------------------------------------------
//Typedef for Mesh2PointMechanicalMapping
typedef sofa::component::mapping::Mesh2PointMechanicalMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > Mesh2PointMechanicalMechanicalMapping3f_to_3f;



//---------------------------------------------------------------------------------------------
//Typedef for RigidMapping
typedef sofa::component::mapping::RigidMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > RigidMechanicalMappingRigid3f_to_3f;
typedef sofa::component::mapping::RigidMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<2, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<2, float>, sofa::defaulttype::Vec<2, float>, float> > > > RigidMechanicalMappingRigid2f_to_2f;



//---------------------------------------------------------------------------------------------
//Typedef for RigidRigidMapping
typedef sofa::component::mapping::RigidRigidMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> > > > RigidRigidMechanicalMappingRigid3f_to_Rigid3f;



//---------------------------------------------------------------------------------------------
//Typedef for SimpleTesselatedTetraMechanicalMapping
typedef sofa::component::mapping::SimpleTesselatedTetraMechanicalMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > SimpleTesselatedTetraMechanicalMechanicalMapping3f_to_3f;



//---------------------------------------------------------------------------------------------
//Typedef for SkinningMapping
typedef sofa::component::mapping::SkinningMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > SkinningMechanicalMappingRigid3f_to_3f;



//---------------------------------------------------------------------------------------------
//Typedef for SubsetMapping
typedef sofa::component::mapping::SubsetMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > SubsetMechanicalMapping3f_to_3f;
typedef sofa::component::mapping::SubsetMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<1, float>, sofa::defaulttype::Vec<1, float>, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<1, float>, sofa::defaulttype::Vec<1, float>, float> > > > SubsetMechanicalMapping1f_to_1f;
typedef sofa::component::mapping::SubsetMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> > > > SubsetMechanicalMappingRigid3f_to_Rigid3f;



//---------------------------------------------------------------------------------------------
//Typedef for TubularMapping
typedef sofa::component::mapping::TubularMapping<sofa::core::behavior::MechanicalMapping<sofa::core::behavior::MechanicalState<sofa::defaulttype::StdRigidTypes<3, float> >, sofa::core::behavior::MechanicalState<sofa::defaulttype::StdVectorTypes<sofa::defaulttype::Vec<3, float>, sofa::defaulttype::Vec<3, float>, float> > > > TubularMechanicalMappingRigid3f_to_3f;





#ifdef SOFA_FLOAT
typedef ArticulatedSystemMechanicalMapping1f_to_Rigid3f ArticulatedSystemMechanicalMapping1_to_Rigid3;
typedef BarycentricMechanicalMapping3f_to_Rigid3f BarycentricMechanicalMapping3_to_Rigid3;
typedef BarycentricMechanicalMapping3f_to_3f BarycentricMechanicalMapping3_to_3;
typedef BeamLinearMechanicalMappingRigid3f_to_3f BeamLinearMechanicalMappingRigid3_to_3;
typedef CenterOfMassMechanicalMappingRigid3f_to_3f CenterOfMassMechanicalMappingRigid3_to_3;
typedef CenterOfMassMechanicalMappingRigid2f_to_2f CenterOfMassMechanicalMappingRigid2_to_2;
typedef CenterPointMechanicalMechanicalMapping3f_to_3f CenterPointMechanicalMechanicalMapping3_to_3;
typedef CurveMechanicalMapping3f_to_Rigid3f CurveMechanicalMapping3_to_Rigid3;
typedef ExternalInterpolationMechanicalMapping3f_to_3f ExternalInterpolationMechanicalMapping3_to_3;
typedef ExternalInterpolationMechanicalMapping2f_to_2f ExternalInterpolationMechanicalMapping2_to_2;
typedef ExternalInterpolationMechanicalMapping1f_to_1f ExternalInterpolationMechanicalMapping1_to_1;
typedef IdentityMechanicalMapping3f_to_3f IdentityMechanicalMapping3_to_3;
typedef IdentityMechanicalMapping2f_to_2f IdentityMechanicalMapping2_to_2;
typedef IdentityMechanicalMapping1f_to_1f IdentityMechanicalMapping1_to_1;
typedef IdentityMechanicalMapping6f_to_6f IdentityMechanicalMapping6_to_6;
typedef IdentityMechanicalMappingRigid3f_to_Rigid3f IdentityMechanicalMappingRigid3_to_Rigid3;
typedef IdentityMechanicalMappingRigid2f_to_Rigid2f IdentityMechanicalMappingRigid2_to_Rigid2;
typedef IdentityMechanicalMappingRigid3f_to_3f IdentityMechanicalMappingRigid3_to_3;
typedef IdentityMechanicalMappingRigid2f_to_2f IdentityMechanicalMappingRigid2_to_2;
typedef LineSetSkinningMechanicalMappingRigid3f_to_3f LineSetSkinningMechanicalMappingRigid3_to_3;
typedef Mesh2PointMechanicalMechanicalMapping3f_to_3f Mesh2PointMechanicalMechanicalMapping3_to_3;
typedef RigidMechanicalMappingRigid3f_to_3f RigidMechanicalMappingRigid3_to_3;
typedef RigidMechanicalMappingRigid2f_to_2f RigidMechanicalMappingRigid2_to_2;
typedef RigidRigidMechanicalMappingRigid3f_to_Rigid3f RigidRigidMechanicalMappingRigid3_to_Rigid3;
typedef SimpleTesselatedTetraMechanicalMechanicalMapping3f_to_3f SimpleTesselatedTetraMechanicalMechanicalMapping3_to_3;
typedef SkinningMechanicalMappingRigid3f_to_3f SkinningMechanicalMappingRigid3_to_3;
typedef SubsetMechanicalMapping3f_to_3f SubsetMechanicalMapping3_to_3;
typedef SubsetMechanicalMapping1f_to_1f SubsetMechanicalMapping1_to_1;
typedef SubsetMechanicalMappingRigid3f_to_Rigid3f SubsetMechanicalMappingRigid3_to_Rigid3;
typedef TubularMechanicalMappingRigid3f_to_3f TubularMechanicalMappingRigid3_to_3;
#endif

#endif
