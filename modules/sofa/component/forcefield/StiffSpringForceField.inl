// Author: François Faure, INRIA-UJF, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
#ifndef SOFA_COMPONENT_FORCEFIELD_STIFFSPRINGFORCEFIELD_INL
#define SOFA_COMPONENT_FORCEFIELD_STIFFSPRINGFORCEFIELD_INL

#include <sofa/component/forcefield/StiffSpringForceField.h>
#include <sofa/component/forcefield/SpringForceField.inl>
#include <assert.h>

namespace sofa
{

namespace component
{

namespace forcefield
{

template<class DataTypes>
void StiffSpringForceField<DataTypes>::init()
{
    this->SpringForceField<DataTypes>::init();
}

template<class DataTypes>
void StiffSpringForceField<DataTypes>::addSpringForce( double& potentialEnergy, VecDeriv& f1, const VecCoord& p1, const VecDeriv& v1, VecDeriv& f2, const VecCoord& p2, const VecDeriv& v2, int i, const Spring& spring)
{
    int a = spring.m1;
    int b = spring.m2;
    Coord u = p2[b]-p1[a];
    Real d = u.norm();
    if( d>1.0e-4 )
    {
        Real inverseLength = 1.0f/d;
        u *= inverseLength;
        Real elongation = (Real)(d - spring.initpos);
        potentialEnergy += elongation * elongation * spring.ks / 2;
        /*                    cerr<<"StiffSpringForceField<DataTypes>::addSpringForce, p1 = "<<p1<<endl;
                            cerr<<"StiffSpringForceField<DataTypes>::addSpringForce, p2 = "<<p2<<endl;
                            cerr<<"StiffSpringForceField<DataTypes>::addSpringForce, new potential energy = "<<potentialEnergy<<endl;*/
        Deriv relativeVelocity = v2[b]-v1[a];
        Real elongationVelocity = dot(u,relativeVelocity);
        Real forceIntensity = (Real)(spring.ks*elongation+spring.kd*elongationVelocity);
        Deriv force = u*forceIntensity;
        f1[a]+=force;
        f2[b]-=force;


        Mat& m = this->dfdx[i];
        Real tgt = forceIntensity * inverseLength;
        for( int j=0; j<N; ++j )
        {
            for( int k=0; k<N; ++k )
            {
                m[j][k] = ((Real)spring.ks-tgt) * u[j] * u[k];
            }
            m[j][j] += tgt;
        }
    }
    else // null length, no force and no stiffness
    {
        Mat& m = this->dfdx[i];
        for( int j=0; j<N; ++j )
        {
            for( int k=0; k<N; ++k )
            {
                m[j][k] = 0;
            }
        }
    }
}

template<class DataTypes>
void StiffSpringForceField<DataTypes>::addSpringDForce(VecDeriv& f1, const VecDeriv& dx1, VecDeriv& f2, const VecDeriv& dx2, int i, const Spring& spring)
{
    const int a = spring.m1;
    const int b = spring.m2;
    const Coord d = dx2[b]-dx1[a];
    const Deriv dforce = this->dfdx[i]*d;
    f1[a]+=dforce;
    f2[b]-=dforce;
    //cerr<<"StiffSpringForceField<DataTypes>::addSpringDForce, a="<<a<<", b="<<b<<", dforce ="<<dforce<<endl;
}

template<class DataTypes>
void StiffSpringForceField<DataTypes>::addForce(VecDeriv& f1, VecDeriv& f2, const VecCoord& x1, const VecCoord& x2, const VecDeriv& v1, const VecDeriv& v2)
{
    const vector<Spring>& springs= this->springs.getValue();
    this->dfdx.resize(springs.size());
    f1.resize(x1.size());
    f2.resize(x2.size());
    m_potentialEnergy = 0;
    /*        cerr<<"StiffSpringForceField<DataTypes>::addForce()"<<endl;*/
    for (unsigned int i=0; i<springs.size(); i++)
    {
        /*            cerr<<"StiffSpringForceField<DataTypes>::addForce() between "<<springs[i].m1<<" and "<<springs[i].m2<<endl;*/
        this->addSpringForce(m_potentialEnergy,f1,x1,v1,f2,x2,v2, i, springs[i]);
    }
}

template<class DataTypes>
void StiffSpringForceField<DataTypes>::addDForce(VecDeriv& df1, VecDeriv& df2, const VecDeriv& dx1, const VecDeriv& dx2)
{
    df1.resize(dx1.size());
    df2.resize(dx2.size());
    //cerr<<"StiffSpringForceField<DataTypes>::addDForce, dx1 = "<<dx1<<endl;
    //cerr<<"StiffSpringForceField<DataTypes>::addDForce, df1 before = "<<f1<<endl;
    const vector<Spring>& springs = this->springs.getValue();
    for (unsigned int i=0; i<springs.size(); i++)
    {
        this->addSpringDForce(df1,dx1,df2,dx2, i, springs[i]);
    }
    //cerr<<"StiffSpringForceField<DataTypes>::addDForce, df1 = "<<f1<<endl;
    //cerr<<"StiffSpringForceField<DataTypes>::addDForce, df2 = "<<f2<<endl;
}

} // namespace forcefield

} // namespace component

} // namespace sofa

#endif

