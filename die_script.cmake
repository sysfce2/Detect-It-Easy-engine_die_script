include_directories(${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/../XDEX/xdex.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XPDF/xpdf.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XArchive/xarchives.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../Formats/xformats.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../Formats/scanitem.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XCapstone/xcapstone.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XOptions/xoptions.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XExtractor/xextractor.cmake)

set(DIE_SCRIPT_SOURCES
    ${XDEX_SOURCES}
    ${XPDF_SOURCES}
    ${XARCHIVES_SOURCES}
    ${XFORMATS_SOURCES}
    ${XCAPSTONE_SOURCES}
    ${SCANITEM_SOURCES}
    ${XOPTIONS_SOURCES}
    ${XEXTRACTOR_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/archive_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/archive_script.h
    ${CMAKE_CURRENT_LIST_DIR}/binary_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/binary_script.h
    ${CMAKE_CURRENT_LIST_DIR}/com_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/com_script.h
    ${CMAKE_CURRENT_LIST_DIR}/die_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/die_script.h
    ${CMAKE_CURRENT_LIST_DIR}/die_scriptengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/die_scriptengine.h
    ${CMAKE_CURRENT_LIST_DIR}/elf_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/elf_script.h
    ${CMAKE_CURRENT_LIST_DIR}/global_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/global_script.h
    ${CMAKE_CURRENT_LIST_DIR}/le_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/le_script.h
    ${CMAKE_CURRENT_LIST_DIR}/lx_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lx_script.h
    ${CMAKE_CURRENT_LIST_DIR}/mach_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mach_script.h
    ${CMAKE_CURRENT_LIST_DIR}/msdos_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/msdos_script.h
    ${CMAKE_CURRENT_LIST_DIR}/ne_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ne_script.h
    ${CMAKE_CURRENT_LIST_DIR}/pe_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pe_script.h
    ${CMAKE_CURRENT_LIST_DIR}/xscriptengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xscriptengine.h
    ${CMAKE_CURRENT_LIST_DIR}/zip_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/zip_script.h
    ${CMAKE_CURRENT_LIST_DIR}/jar_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/jar_script.h
    ${CMAKE_CURRENT_LIST_DIR}/apk_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/apk_script.h
    ${CMAKE_CURRENT_LIST_DIR}/ipa_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ipa_script.h
    ${CMAKE_CURRENT_LIST_DIR}/dex_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dex_script.h
    ${CMAKE_CURRENT_LIST_DIR}/npm_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/npm_script.h
    ${CMAKE_CURRENT_LIST_DIR}/util_script.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util_script.h
)
