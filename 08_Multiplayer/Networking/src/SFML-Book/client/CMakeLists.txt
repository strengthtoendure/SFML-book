set(INCROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/${CMAKE_PROJECT_NAME}/client)
set(SRCROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(INCROOT_COMMON ${INCROOT}/../common)
set(SRCROOT_COMMON ${SRCROOT}/../common)

set(LIBS ${LIBS} utils-json)

#FILES
set(FILES_HEADER
    ${INCROOT}/Client.hpp
    ${INCROOT}/Component.hpp
    ${INCROOT}/Entity.hpp
    ${INCROOT}/Game.hpp
    ${INCROOT}/Gui.hpp
    ${INCROOT}/Level.hpp
    ${INCROOT}/System.hpp
    ${INCROOT}/Team.hpp

    ${INCROOT_COMMON}/std_hash.hpp
    ${INCROOT_COMMON}/Packet.hpp    
    ${INCROOT_COMMON}/FuncIds.hpp
    ${INCROOT_COMMON}/Connection.hpp
    ${INCROOT_COMMON}/Configuration.hpp
)

set(FILES_SRC
    ${SRCROOT}/Client.cpp
    ${SRCROOT}/Component.cpp
    ${SRCROOT}/Entity.cpp
    ${SRCROOT}/Game.cpp
    ${SRCROOT}/Gui.cpp
    ${SRCROOT}/Level.cpp
    ${SRCROOT}/System.cpp
    ${SRCROOT}/Team.cpp

    ${SRCROOT_COMMON}/Packet.cpp
    ${SRCROOT_COMMON}/Connection.cpp
    ${SRCROOT_COMMON}/Configuration.cpp

    ${SRCROOT}/main.cpp
)


add_executable(${CHAPTER_NAME}-client ${FILES_HEADER} ${FILES_SRC})
target_link_libraries(${CHAPTER_NAME}-client ${LIBS})
