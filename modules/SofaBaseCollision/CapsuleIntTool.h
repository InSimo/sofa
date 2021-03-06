#ifndef CAPSULEINTTOOL_H
#define CAPSULEINTTOOL_H
#include <sofa/core/collision/Intersection.h>
#include <sofa/helper/FnDispatcher.h>
#include <SofaBaseCollision/CapsuleModel.h>
#include <SofaBaseCollision/RigidCapsuleModel.h>
#include <SofaBaseCollision/SphereModel.h>
#include <SofaBaseCollision/OBBModel.h>
#include <SofaBaseCollision/IntrCapsuleOBB.h>
#include <cmath>

namespace sofa
{
namespace component
{
namespace collision
{
class SOFA_BASE_COLLISION_API CapsuleIntTool{
public:
    template <class Elem1, class Elem2>
    using OutputContainer = sofa::core::collision::TDetectionOutputContainer<typename Elem1::Model, typename Elem2::Model>;

    template <class DataTypes1,class DataTypes2>
    static int computeIntersection(TCapsule<DataTypes1>&, TCapsule<DataTypes2>&,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<DataTypes1>, TCapsule<DataTypes2>>* contacts);

    template <class DataTypes1,class DataTypes2>
    static int computeIntersection(TCapsule<DataTypes1>&, TSphere<DataTypes2>&,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<DataTypes1>, TSphere<DataTypes2>>* contacts);

    template <class DataTypes>
    static int computeIntersection(TCapsule<DataTypes>&, OBB&,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<DataTypes>, OBB>* contacts);

    template <class DataTypes1,class DataTypes2>
    static bool shareSameVertex(const TCapsule<DataTypes1>& c1, const TCapsule<DataTypes2>& c2);

    static bool shareSameVertex(const Capsule & c1, const Capsule & c2);

};

template <class DataTypes1,class DataTypes2>
bool CapsuleIntTool::shareSameVertex(const TCapsule<DataTypes1>&, const TCapsule<DataTypes2>&){
    return false;
}

template <class DataTypes1,class DataTypes2>
int CapsuleIntTool::computeIntersection(TCapsule<DataTypes1> & cap, TSphere<DataTypes2> & sph,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<DataTypes1>, TSphere<DataTypes2>>* contacts){
    using namespace sofa::defaulttype;
    Vector3 sph_center = sph.center();
    Vector3 cap_p1 = cap.point1();
    Vector3 cap_p2 = cap.point2();
    SReal cap_rad = cap.radius();
    SReal sph_rad = sph.r();

    Vector3 AB = cap_p2 - cap_p1;
    Vector3 AC = sph_center - cap_p1;

    SReal theory_contactDist = (SReal) cap_rad + sph_rad + contactDist;
    SReal contact_exists = (SReal) cap_rad + sph_rad + alarmDist;
    SReal alpha = (SReal) (AB * AC)/AB.norm2();//projection of the sphere center on the capsule segment
                                        //alpha is the coefficient such as the projected point P = cap_p1 + alpha * AB
    if(alpha < 0.000001){//S is the sphere center, here is the case :
                         //        S
                         //           A--------------B
        Vector3 PQ = sph_center - cap_p1;

        if(PQ.norm2() > contact_exists * contact_exists)
            return 0;

        sofa::core::collision::DetectionOutput& detection = contacts->addDetectionOutput();

        detection.elem = std::pair<core::CollisionElementIterator, core::CollisionElementIterator>(cap, sph);
        //detection.id = (cap.getCollisionModel()->getSize() > sph.getCollisionModel()->getSize()) ? cap.getIndex() : sph.getIndex();
        detection.id = cap.getIndex();

        detection.normal = PQ;
        detection.value = detection.normal.norm();
        detection.normal /= detection.value;
        detection.point[0] = cap_p1 + cap_rad * detection.normal;
        detection.point[1] = sph.getContactPointByNormal( detection.normal );
        detection.value -= theory_contactDist;

        return 1;
    }
    else if(alpha > 0.999999){//the case :
                              //                         S
                              //      A-------------B
        Vector3 PQ = sph_center - cap_p2;

        if(PQ.norm2() > contact_exists * contact_exists)
            return 0;

        sofa::core::collision::DetectionOutput& detection = contacts->addDetectionOutput();

        detection.elem = std::pair<core::CollisionElementIterator, core::CollisionElementIterator>(cap, sph);
        //detection.id = (cap.getCollisionModel()->getSize() > sph.getCollisionModel()->getSize()) ? cap.getIndex() : sph.getIndex();
        detection.id = cap.getIndex();

        detection.normal = PQ;
        detection.value = detection.normal.norm();
        detection.normal /= detection.value;
        detection.point[0] = cap_p2 + cap_rad * detection.normal;
        detection.point[1] = sph.getContactPointByNormal( detection.normal );
        detection.value -= theory_contactDist;

        return 1;
    }
    else{//the case :
         //              S
         //      A-------------B
        Vector3 P = cap_p1 + alpha * AB;
        Vector3 PQ = sph_center - P;

        if(PQ.norm2() > contact_exists * contact_exists)
            return 0;

        sofa::core::collision::DetectionOutput& detection = contacts->addDetectionOutput();

        detection.elem = std::pair<core::CollisionElementIterator, core::CollisionElementIterator>(cap, sph);
        //detection.id = (cap.getCollisionModel()->getSize() > sph.getCollisionModel()->getSize()) ? cap.getIndex() : sph.getIndex();
        detection.id = cap.getIndex();

        detection.normal = PQ;
        detection.value = detection.normal.norm();
        detection.normal /= detection.value;
        detection.point[0] = P + cap_rad * detection.normal;
        detection.point[1] = sph.getContactPointByNormal( detection.normal );
        detection.value -= theory_contactDist;

        return 1;
    }
}


#if defined(SOFA_EXTERN_TEMPLATE) && !defined(SOFA_BUILD_BASE_COLLISION)
extern template SOFA_BASE_COLLISION_API int CapsuleIntTool::computeIntersection(TCapsule<sofa::defaulttype::Vec3Types>&, TCapsule<sofa::defaulttype::Vec3Types>&,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<sofa::defaulttype::Vec3Types>, TCapsule<sofa::defaulttype::Vec3Types>>* contacts);
extern template SOFA_BASE_COLLISION_API int CapsuleIntTool::computeIntersection(TCapsule<sofa::defaulttype::Vec3Types>&, TCapsule<sofa::defaulttype::RigidTypes>&,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<sofa::defaulttype::Vec3Types>, TCapsule<sofa::defaulttype::RigidTypes>>* contacts);
extern template SOFA_BASE_COLLISION_API int CapsuleIntTool::computeIntersection(TCapsule<sofa::defaulttype::RigidTypes>&, TCapsule<sofa::defaulttype::RigidTypes>&,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<sofa::defaulttype::RigidTypes>, TCapsule<sofa::defaulttype::RigidTypes>>* contacts);
extern template SOFA_BASE_COLLISION_API int CapsuleIntTool::computeIntersection(TCapsule<sofa::defaulttype::RigidTypes> & cap, OBB& obb,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<sofa::defaulttype::RigidTypes>, OBB>* contacts);
extern template SOFA_BASE_COLLISION_API int CapsuleIntTool::computeIntersection(TCapsule<sofa::defaulttype::Vec3Types> & cap, OBB& obb,SReal alarmDist,SReal contactDist,OutputContainer<TCapsule<sofa::defaulttype::Vec3Types>, OBB>* contacts);
#endif

}
}
}
#endif // CAPSULEINTTOOL_H
