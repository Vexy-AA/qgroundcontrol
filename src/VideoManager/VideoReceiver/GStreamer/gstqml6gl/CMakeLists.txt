find_package(GStreamer REQUIRED COMPONENTS Allocators Audio Codecparsers Controller Mpegts Net Pbutils Riff Rtp Rtsp Sdp Tag OPTIONAL_COMPONENTS Va Photography X11 EGL Wayland)

# Use Latest Revisions for each minor version: 1.16.3, 1.18.6, 1.20.7, 1.22.12, 1.24.7
string(REPLACE "." ";" GST_VERSION_LIST ${GStreamer_VERSION})
list(GET GST_VERSION_LIST 0 GST_VERSION_MAJOR)
list(GET GST_VERSION_LIST 1 GST_VERSION_MINOR)
list(GET GST_VERSION_LIST 2 GST_VERSION_PATCH)
cmake_print_variables(GST_VERSION_MAJOR GST_VERSION_MINOR GST_VERSION_PATCH)

if(GST_VERSION_MINOR EQUAL 16)
    set(GST_VERSION_PATCH 3)
elseif(GST_VERSION_MINOR EQUAL 18)
    set(GST_VERSION_PATCH 6)
elseif(GST_VERSION_MINOR EQUAL 20)
    set(GST_VERSION_PATCH 7)
elseif(GST_VERSION_MINOR EQUAL 22)
    set(GST_VERSION_PATCH 12)
elseif(GST_VERSION_MINOR EQUAL 24)
    set(GST_VERSION_PATCH 7)
endif()

set(GST_PLUGINS_VERSION ${GST_VERSION_MAJOR}.${GST_VERSION_MINOR}.${GST_VERSION_PATCH})
cmake_print_variables(GST_PLUGINS_VERSION)

if(GStreamer_VERSION VERSION_GREATER_EQUAL 1.22)
    FetchContent_Declare(gstreamer_good_plugins
        # https://gitlab.freedesktop.org/gstreamer/gstreamer/-/archive/${GST_PLUGINS_VERSION}/gstreamer-${GST_PLUGINS_VERSION}.zip?path=subprojects/gst-plugins-good/ext/qt6
        URL https://gstreamer.freedesktop.org/src/gst-plugins-good/gst-plugins-good-${GST_PLUGINS_VERSION}.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(gstreamer_good_plugins)
    set(QGC_GST_QT6_PLUGIN_PATH "${gstreamer_good_plugins_SOURCE_DIR}/ext/qt6" CACHE PATH "Location of downloaded GST Qt6 plugin sources")
else()
    set(QGC_GST_QT6_PLUGIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/qt6" CACHE PATH "Location of custom local GST plugin")
endif()
cmake_print_variables(QGC_GST_QT6_PLUGIN_PATH)

################################################################################

file(READ ${QGC_GST_QT6_PLUGIN_PATH}/qt6glitem.h FILE_CONTENTS)
string(FIND "${FILE_CONTENTS}" "#include <QtQuick/QQuickWindow>" GST_FIX_INCLUDES)
if(GST_FIX_INCLUDES EQUAL -1)
    string(REPLACE "#include <QtGui/QOpenGLFunctions>" "#include <QtGui/QOpenGLFunctions>\n#include <QtQuick/QQuickWindow>" FILE_CONTENTS "${FILE_CONTENTS}")
    file(WRITE ${QGC_GST_QT6_PLUGIN_PATH}/qt6glitem.h "${FILE_CONTENTS}")
endif()

################################################################################

file(GLOB gstqml6gl_SRCS
    ${QGC_GST_QT6_PLUGIN_PATH}/*.cc
    ${QGC_GST_QT6_PLUGIN_PATH}/*.h
)
qt_add_library(gstqml6gl STATIC ${gstqml6gl_SRCS})

target_include_directories(gstqml6gl PUBLIC ${QGC_GST_QT6_PLUGIN_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

target_link_libraries(gstqml6gl
    PRIVATE
        Qt6::GuiPrivate
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        GStreamer::GStreamer
        GStreamer::Allocators
        GStreamer::Audio
        GStreamer::Codecparsers
        GStreamer::Controller
        GStreamer::Mpegts
        GStreamer::Net
        GStreamer::Pbutils
        GStreamer::Riff
        GStreamer::Rtp
        GStreamer::Rtsp
        GStreamer::Sdp
        GStreamer::Tag
)

# Photography not found on ubuntu 20.04?
if(GStreamer_Photography_FOUND)
    target_link_libraries(gstqml6gl PUBLIC GStreamer::Photography)
endif()

if(GStreamer_Va_FOUND)
    target_link_libraries(gstqml6gl PUBLIC GStreamer::Va)
endif()

if(GStreamer_X11_FOUND)
    # X11, XCB, X11_XCB, GLX
    target_link_libraries(gstqml6gl PUBLIC GStreamer::X11)
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_X11)
endif()

if(GStreamer_EGL_FOUND)
    # EGL
    target_link_libraries(gstqml6gl PUBLIC GStreamer::EGL)
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_EGLFS)
endif()

if(GStreamer_Wayland_FOUND)
    # find_package(Wayland COMPONENTS Client Cursor Egl)
    # find_package(WaylandProtocols)
    # find_package(WaylandScanner)
    find_package(Qt6 COMPONENTS WaylandClient)
    if(Qt6WaylandClient_FOUND)
        target_link_libraries(gstqml6gl PUBLIC GStreamer::Wayland Qt6::WaylandClient)
        target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_WAYLAND)
    endif()
endif()

if(ANDROID)
    # GLESv2, egl
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_ANDROID)
elseif(WIN32)
    # GDI32, egl, GLESv2
    find_package(OpenGL)
    if(OpenGL_FOUND)
        target_link_libraries(gstqml6gl PUBLIC OpenGL::GL)
        target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_WIN32)
    endif()
elseif(MACOS)
    # appleframeworks
    # Foundation, QuartzCore, CoreFoundation
    # Cocoa
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_MAC)
elseif(IOS)
    # appleframeworks
    # Foundation, CoreFoundation, CoreGraphics, QuartzCore, UIkit
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_IOS)
endif()

target_compile_definitions(gstqml6gl
    PRIVATE
        HAVE_QT_QPA_HEADER
        QT_QPA_HEADER=<QtGui/qpa/qplatformnativeinterface.h>
)

if(EXISTS "${QGC_GST_QT6_PLUGIN_PATH}/resources.qrc")
    find_package(Qt6 REQUIRED COMPONENTS ShaderTools)

    set(SHADERS
        ${QGC_GST_QT6_PLUGIN_PATH}/vertex.vert
        ${QGC_GST_QT6_PLUGIN_PATH}/YUV_TRIPLANAR.frag
        ${QGC_GST_QT6_PLUGIN_PATH}/RGBA.frag
    )
    set(OUTPUTS
        vertex.vert.qsb
        YUV_TRIPLANAR.frag.qsb
        RGBA.frag.qsb
    )
    if(EXISTS "${QGC_GST_QT6_PLUGIN_PATH}/YUV_BIPLANAR.frag")
        LIST(APPEND SHADERS ${QGC_GST_QT6_PLUGIN_PATH}/YUV_BIPLANAR.frag)
        LIST(APPEND OUTPUTS YUV_BIPLANAR.frag.qsb)
    endif()
    if(EXISTS "${QGC_GST_QT6_PLUGIN_PATH}/RGBA_gles.frag")
        LIST(APPEND SHADERS ${QGC_GST_QT6_PLUGIN_PATH}/RGBA.frag@glsl,100es,${QGC_GST_QT6_PLUGIN_PATH}/RGBA_gles.frag)
        LIST(APPEND OUTPUTS RGBA_gles.frag.qsb)
        # file(COPY_FILE RGBA_gles.frag RGBA_gles.frag.qsb)
        # LIST(APPEND OUTPUTS RGBA_gles.frag.qsb.external)
    endif()
    qt6_add_shaders(gstqml6gl "gstqml6gl_shaders"
        PREFIX "/org/freedesktop/gstreamer/qml6"
        GLSL "100 es,120,330"
        OUTPUT_TARGETS gstqml6gl_shaders
        FILES ${SHADERS}
        OUTPUTS ${OUTPUTS}
        BATCHABLE
    )
endif()

if(UNIX)
    target_compile_options(gstqml6gl
        PRIVATE
            -Wno-unused-parameter
            -Wno-implicit-fallthrough
            -Wno-unused-private-field
    )
endif()

# gstva, gstasf
set(GST_TARGET_PLUGINS
    gstcoreelements
    gstisomp4
    gstlibav
    gstmatroska
    gstmpegtsdemux
    gstopengl
    gstplayback
    gstrtp
    gstrtpmanager
    gstrtsp
    gstsdpelem
    gsttcp
    gstudp
    gstvideoparsersbad
    gstx264
)
if(ANDROID)
    list(APPEND GST_TARGET_PLUGINS gstandroidmedia)
elseif(IOS)
    list(APPEND GST_TARGET_PLUGINS gstapplemedia)
endif()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GST_PLUGINS IMPORTED_TARGET ${GST_TARGET_PLUGINS})
    if(GST_PLUGINS_FOUND)
        target_link_libraries(gstqml6gl PUBLIC PkgConfig::GST_PLUGINS)
        if(WIN32)
            # install(FILES ${GST_PLUGINS_LIBRARIES} DESTINATION ${CMAKE_INSTALL_BINDIR})
        elseif(LINUX)
            install(FILES ${GST_PLUGINS_LIBRARIES} DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif()
    endif()

    pkg_check_modules(GSTREAMER_PLUGINS_BASE IMPORTED_TARGET gstreamer-plugins-base-1.0)
    if(GSTREAMER_PLUGINS_BASE_FOUND)
        target_link_libraries(gstqml6gl PUBLIC PkgConfig::GSTREAMER_PLUGINS_BASE)
    endif()
endif()

if(NOT GST_PLUGINS_FOUND)
    foreach(plugin IN LISTS GST_TARGET_PLUGINS)
        find_library(GStreamer_${plugin}_LIBRARY
            NAMES ${plugin}
            PATHS
                ${GSTREAMER_PREFIX}/lib
                ${GSTREAMER_PREFIX}/lib/gstreamer-1.0
                ${GSTREAMER_PREFIX}/lib/x86_64-linux-gnu
                ${GSTREAMER_PREFIX}/lib/x86_64-linux-gnu/gstreamer-1.0
                ${GSTREAMER_PREFIX}/Versions/Current/lib
                ${GSTREAMER_PREFIX}/Versions/Current/lib/gstreamer-1.0
        )
        if(GStreamer_${plugin}_LIBRARY)
            cmake_print_variables(GStreamer_${plugin}_LIBRARY)
            target_link_libraries(gstqml6gl PUBLIC ${GStreamer_${plugin}_LIBRARY})
            if(WIN32)
                # install(FILES ${GStreamer_${plugin}_LIBRARY} DESTINATION ${CMAKE_INSTALL_BINDIR})
            elseif(LINUX)
                install(FILES ${GStreamer_${plugin}_LIBRARY} DESTINATION ${CMAKE_INSTALL_LIBDIR})
            endif()
        endif()
    endforeach()

    if(ANDROID)
        target_include_directories(gstqml6gl
            PUBLIC
                ${GSTREAMER_PREFIX}/include/gstreamer-1.0
                ${GSTREAMER_PREFIX}/include/glib-2.0
                ${GSTREAMER_PREFIX}/lib/glib-2.0/include
                ${GSTREAMER_PREFIX}/lib/graphene-1.0/include
                ${GSTREAMER_PREFIX}/lib/gstreamer-1.0/include
                ${GSTREAMER_PREFIX}/include
        )

        find_package(FFMPEG REQUIRED COMPONENTS avcodec avfilter avformat avutil swresample)
        if(FFMPEG_FOUND)
            target_link_libraries(gstqml6gl PUBLIC ${FFMPEG_LIBRARIES})
            target_include_directories(gstqml6gl PUBLIC ${FFMPEG_INCLUDE_DIRS})
        endif()

        find_package(BZip2)
        if(BZIP2_FOUND)
            target_link_libraries(gstqml6gl PUBLIC BZip2::BZip2)
        endif()

        find_library(graphene_LIBRARY graphene-1.0)
        if(graphene_LIBRARY)
            target_link_libraries(gstqml6gl PUBLIC ${graphene_LIBRARY})
        endif()

        find_library(x264_LIBRARY x264)
        if(x264_LIBRARY)
            target_link_libraries(gstqml6gl PUBLIC ${x264_LIBRARY})
        endif()

        find_package(JPEG)
        if(JPEG_FOUND)
            target_link_libraries(gstqml6gl PUBLIC JPEG::JPEG)
        endif()

        find_package(PNG)
        if(PNG_FOUND)
            target_link_libraries(gstqml6gl PUBLIC PNG::PNG)
        endif()

    endif()
endif()

if(ANDROID)
    target_link_options(gstqml6gl PUBLIC "-Wl,-Bsymbolic")
endif()

if(QGC_GST_STATIC_BUILD)
    target_compile_definitions(gstqml6gl PUBLIC QGC_GST_STATIC_BUILD)
endif()

if(LINUX)
    install(DIRECTORY ${GSTREAMER_PREFIX}/lib/x86_64-linux-gnu/gstreamer1.0 DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(DIRECTORY ${GSTREAMER_PREFIX}/lib/x86_64-linux-gnu/gio DESTINATION ${CMAKE_INSTALL_LIBDIR})
elseif(WIN32)
    cmake_path(CONVERT "${GSTREAMER_PREFIX}/libexec/gstreamer-1.0/*.exe" TO_CMAKE_PATH_LIST GST_HELPER_BINS_PATH)
    file(GLOB GST_HELPER_BINS ${GST_HELPER_BINS_PATH})
    install(FILES ${GST_HELPER_BINS} DESTINATION ${CMAKE_INSTALL_BINDIR})

    cmake_path(CONVERT "${GSTREAMER_PREFIX}/lib/gio" TO_CMAKE_PATH_LIST GST_GIO_PATH)
    install(DIRECTORY ${GST_GIO_PATH} DESTINATION ${CMAKE_INSTALL_BINDIR})

    # cmake_path(CONVERT "${GSTREAMER_PREFIX}/share/gstreamer/gst-env" TO_CMAKE_PATH_LIST GST_SHARE_ENV_PATH)
    # TODO: update GSTREAMER_ROOT & install
    # install(CODE GST_SHARE_ENV_PATH)
endif()

if(WIN32)
    cmake_path(CONVERT "${GSTREAMER_PREFIX}/bin/*.dll" TO_CMAKE_PATH_LIST GST_WIN_BINS_PATH)
    file(GLOB GST_WIN_BINS ${GST_WIN_BINS_PATH})
    install(FILES ${GST_WIN_BINS} DESTINATION ${CMAKE_INSTALL_BINDIR})

    cmake_path(CONVERT "${GSTREAMER_PREFIX}/lib/*.dll" TO_CMAKE_PATH_LIST GST_WIN_LIBS_PATH)
    file(GLOB GST_WIN_LIBS ${GST_WIN_LIBS_PATH})
    install(FILES ${GST_WIN_LIBS} DESTINATION ${CMAKE_INSTALL_BINDIR})

    cmake_path(CONVERT "${GSTREAMER_PREFIX}/lib/gstreamer-1.0/*.dll" TO_CMAKE_PATH_LIST GST_WIN_PLUGINS_PATH)
    file(GLOB GST_WIN_PLUGINS ${GST_WIN_PLUGINS_PATH})
    install(FILES ${GST_WIN_PLUGINS} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
