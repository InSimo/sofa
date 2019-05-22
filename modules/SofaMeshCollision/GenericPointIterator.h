/*******************************************************************************
*          Private SOFA components, (c) 2019 InSimo                            *
* CONFIDENTIAL SOURCE CODE. This file is the property of InSimo and should not *
* be redistributed. Commercial use is prohibited without a specific license.   *
*******************************************************************************/

#ifndef SOFA_COMPONENT_COLLISION_GENERICPOINTITERATOR_H
#define SOFA_COMPONENT_COLLISION_GENERICPOINTITERATOR_H

#include <sofa/core/CollisionModel.h>
#include <SofaMeshCollision/GenericTriangleModel.h>
#include <sofa/core/VecId.h>

namespace sofa
{
namespace component
{
namespace collision
{

template<class PointCollisionModel>
class GenericPointIterator : public sofa::core::TCollisionElementIterator<PointCollisionModel >
{
public:
    typedef typename PointCollisionModel::DataTypes DataTypes;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::Deriv Deriv;
    typedef PointCollisionModel ParentModel;

    GenericPointIterator(ParentModel* model, int index);
    GenericPointIterator() {}

    explicit GenericPointIterator(const sofa::core::CollisionElementIterator& i);

    const Coord& p() const;
    const Coord& pFree() const;
    const Deriv& v() const;
    Deriv n() const;

    /// Return true if the element stores a free position vector
    bool hasFreePosition() const;

    bool testLMD(const sofa::defaulttype::Vector3 &, double &, double &);

    bool activated(sofa::core::CollisionModel *cm = 0) const;

};

template<class DataTypes>
inline GenericPointIterator<DataTypes>::GenericPointIterator(ParentModel* model, int index)
    : sofa::core::TCollisionElementIterator<ParentModel>(model, index)
{

}

template<class DataTypes>
inline GenericPointIterator<DataTypes>::GenericPointIterator(const sofa::core::CollisionElementIterator& i)
    : sofa::core::TCollisionElementIterator<ParentModel>(static_cast<ParentModel*>(i.getCollisionModel()), i.getIndex())
{

}

template<class PointCollisionModel>
inline const typename GenericPointIterator<PointCollisionModel>::DataTypes::Coord& GenericPointIterator<PointCollisionModel>::p() const { return this->model->mstate->read(sofa::core::ConstVecCoordId::position())->getValue()[this->index]; }

template<class PointCollisionModel>
inline const typename GenericPointIterator<PointCollisionModel>::DataTypes::Coord& GenericPointIterator<PointCollisionModel>::pFree() const
{
    if (hasFreePosition())
        return this->model->mstate->read(sofa::core::ConstVecCoordId::freePosition())->getValue()[this->index];
    else
        return p();
}

template<class PointCollisionModel>
inline const typename GenericPointIterator<PointCollisionModel>::DataTypes::Deriv& GenericPointIterator<PointCollisionModel>::v() const { return this->model->mstate->read(sofa::core::ConstVecDerivId::velocity())->getValue()[this->index]; }

template<class PointCollisionModel>
inline typename GenericPointIterator<PointCollisionModel>::DataTypes::Deriv GenericPointIterator<PointCollisionModel>::n() const { return ((unsigned)this->index<this->model->normals.size()) ? this->model->normals[this->index] : Deriv(); }

template<class DataTypes>
inline bool GenericPointIterator<DataTypes>::hasFreePosition() const { return this->model->mstate->read(sofa::core::ConstVecCoordId::freePosition())->isSet(); }

template<class DataTypes>
inline bool GenericPointIterator<DataTypes>::activated(sofa::core::CollisionModel *cm) const
{
    return this->model->myActiver->activePoint(this->index, cm);
}

using namespace sofa::defaulttype;
#ifndef SOFA_FLOAT
template class SOFA_MESH_COLLISION_API GenericTriangleModel<Vec3dTypes>;
template class SOFA_MESH_COLLISION_API GenericTriangleModel<Rigid3dTypes>;
#endif
#ifndef SOFA_DOUBLE
template class SOFA_MESH_COLLISION_API GenericTriangleModel<Vec3fTypes>;
template class SOFA_MESH_COLLISION_API GenericTriangleModel<Rigid3fTypes>;
#endif

} // namespace collision

} // namespace component

} // namespace sofa

#endif
