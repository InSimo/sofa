include(${SOFA_CMAKE_DIR}/preProject.cmake)

set(SOURCE_FILES

    houseOfCards.cpp

    )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

AddSourceDependencies(eigen)
AddLinkerDependencies(SofaSimulationTree SofaComponentMain SofaGuiMain SceneCreator)

include(${SOFA_CMAKE_DIR}/postProject.cmake)

