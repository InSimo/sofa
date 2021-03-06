#include <gtest/gtest.h>
#include <SofaBaseTopology/PointSetTopologyContainer.h>
#include <sofa/simulation/tree/GNode.h>

using namespace sofa::component::topology;

namespace
{

TEST( PointSetTopology_test, checkPointSetTopologyIsEmptyConstructed )
{
    PointSetTopologyContainer::SPtr pointContainer = sofa::core::objectmodel::New< PointSetTopologyContainer >();
    EXPECT_EQ( 0, pointContainer->getNbPoints() );
    EXPECT_EQ( 0u, pointContainer->getPoints().size() );
}


TEST( PointSetTopology_test, checkPointSetTopologyInitialization )
{
    PointSetTopologyContainer::SPtr pointContainer = sofa::core::objectmodel::New< PointSetTopologyContainer >();
    sofa::defaulttype::Vec3Types::VecCoord initPos;
    initPos.resize( 50 );
    pointContainer->d_initPoints.setValue( initPos );

    pointContainer->init();

    const sofa::helper::vector< PointSetTopologyContainer::PointID >& points = pointContainer->getPoints();
    EXPECT_EQ( 50, pointContainer->getNbPoints() );
    EXPECT_EQ( 50u, points.size() );

    
    for(std::size_t i=0;i<50;++i)
    {
        EXPECT_EQ( PointSetTopologyContainer::PointID(i), points[i] );
    }
}

TEST( PointSetTopology_test, checkAddPoint )
{
    PointSetTopologyContainer::SPtr pointContainer = sofa::core::objectmodel::New< PointSetTopologyContainer >();
    pointContainer->addPoint();

    const sofa::helper::vector< PointSetTopologyContainer::PointID >& points = pointContainer->getPoints();

    EXPECT_EQ( 1, pointContainer->getNbPoints() );
    ASSERT_EQ( 1u, points.size() );
    EXPECT_EQ( PointSetTopologyContainer::PointID(0), points[0] );
}

TEST( PointSetTopology_test, checkAddPoints )
{
    PointSetTopologyContainer::SPtr pointContainer = sofa::core::objectmodel::New< PointSetTopologyContainer >();
    pointContainer->addPoints(10);

    const sofa::helper::vector< PointSetTopologyContainer::PointID >& points = pointContainer->getPoints();

    EXPECT_EQ( 10, pointContainer->getNbPoints() );
    ASSERT_EQ( 10u, points.size() );

    for(std::size_t i=0;i<10;++i)
    {
        EXPECT_EQ( PointSetTopologyContainer::PointID(i), points[i] );
    }

    pointContainer->addPoints(5);

    EXPECT_EQ( 15, pointContainer->getNbPoints() );
    ASSERT_EQ( 15u, points.size() );

    for(std::size_t i=10;i<15;++i)
    {
        EXPECT_EQ( PointSetTopologyContainer::PointID(i), points[i] );
    }
}

TEST( PointSetTopology_test, checkRemovePoint )
{
    PointSetTopologyContainer::SPtr pointContainer = sofa::core::objectmodel::New< PointSetTopologyContainer >();
    pointContainer->addPoint();
    pointContainer->removePoint();
    const sofa::helper::vector< PointSetTopologyContainer::PointID >& points = pointContainer->getPoints();

    EXPECT_EQ( 0, pointContainer->getNbPoints() );
    ASSERT_EQ( 0u, points.size() );
}

TEST( PointSetTopology_test, checkRemovePoints )
{
    PointSetTopologyContainer::SPtr pointContainer = sofa::core::objectmodel::New< PointSetTopologyContainer >();
    pointContainer->addPoints(10);
    pointContainer->removePoints(3);
    const sofa::helper::vector< PointSetTopologyContainer::PointID >& points = pointContainer->getPoints();

    EXPECT_EQ( 7, pointContainer->getNbPoints() );
    ASSERT_EQ( 7u, points.size() );

    for(std::size_t i=0;i<7;++i)
    {
        EXPECT_EQ( PointSetTopologyContainer::PointID(i), points[i] );
    }

    pointContainer->removePoints(3);

    EXPECT_EQ( 4, pointContainer->getNbPoints() );
    ASSERT_EQ( 4u, points.size() );

    for(std::size_t i=0;i<4;++i)
    {
        EXPECT_EQ( PointSetTopologyContainer::PointID(i), points[i] );
    }


}





}
