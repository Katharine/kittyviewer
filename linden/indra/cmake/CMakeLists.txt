# -*- cmake -*-

include(00-Common)

project(cmake)

set(cmake_SOURCE_FILES
    CMakeLists.txt
    run_build_test.py

    00-Common.cmake
    APR.cmake
    Audio.cmake
    BerkeleyDB.cmake
    Boost.cmake
    BuildVersion.cmake
    CARes.cmake
    CMakeCopyIfDifferent.cmake
    Copy3rdPartyLibs.cmake
    CopyBackToSource.cmake
    CSharpMacros.cmake
    CURL.cmake
    DBusGlib.cmake
    DeploySharedLibs.cmake
    DirectX.cmake
    DragDrop.cmake
    ELFIO.cmake
    ExamplePlugin.cmake
    EXPAT.cmake
    Externals.cmake
    FindAPR.cmake
    FindBerkeleyDB.cmake
    FindCARes.cmake
    FindELFIO.cmake
    FindGoogleBreakpad.cmake
    FindGooglePerfTools.cmake
    FindJsonCpp.cmake
    FindLLQtWebkit.cmake
    FindMono.cmake
    FindMT.cmake
    FindMySQL.cmake
    FindNDOF.cmake
    FindOpenJPEG.cmake
    FindSCP.cmake
    FindSVN.cmake
    FindTut.cmake
    FindXmlRpcEpi.cmake
    FindZLIB.cmake
    FMOD.cmake
    FreeType.cmake
    GetPrerequisites_2_8.cmake
    Glui.cmake
    Glut.cmake
    GoogleBreakpad.cmake
    GoogleMock.cmake
    GooglePerfTools.cmake
    GStreamer010Plugin.cmake
    JPEG.cmake
    JsonCpp.cmake
    Linking.cmake
    LLAddBuildTest.cmake
    LLAudio.cmake
    LLCharacter.cmake
    LLCommon.cmake
    LLCrashLogger.cmake
    LLDatabase.cmake
    LLImage.cmake
    LLImageJ2COJ.cmake
    LLInventory.cmake
    LLKDU.cmake
    LLLogin.cmake
    LLMath.cmake
    LLMessage.cmake
    LLPlugin.cmake
    LLPrimitive.cmake
    LLRender.cmake
    LLScene.cmake
    LLSharedLibs.cmake
    LLTestCommand.cmake
    LLUI.cmake
    LLVFS.cmake
    LLWindow.cmake
    LLXML.cmake
    LLXUIXML.cmake
    LScript.cmake
    MediaPluginBase.cmake
    MonoDeps.cmake
    MonoEmbed.cmake
    MySQL.cmake
    NDOF.cmake
    OPENAL.cmake
    OpenGL.cmake
    OpenJPEG.cmake
    OpenSSL.cmake
    PluginAPI.cmake
    PNG.cmake
    Prebuilt.cmake
    Pth.cmake
    PulseAudio.cmake
    Python.cmake
    QuickTimePlugin.cmake
    TemplateCheck.cmake
    Tut.cmake
    UI.cmake
    UnixInstall.cmake
    Variables.cmake
    ViewerMiscLibs.cmake
    WebKitLibPlugin.cmake
    XmlRpcEpi.cmake
    ZLIB.cmake
    )

source_group("Shared Rules" FILES ${cmake_SOURCE_FILES})

set(master_SOURCE_FILES
    ../CMakeLists.txt
    ../develop.py
    )

if (SERVER)
  list(APPEND master_SOURCE_FILES ../Server.cmake)
endif (SERVER)

source_group("Master Rules" FILES ${master_SOURCE_FILES})

set_source_files_properties(${cmake_SOURCE_FILES} ${master_SOURCE_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

add_library(cmake
            cmake_dummy.cpp
            ${cmake_SOURCE_FILES}
            ${master_SOURCE_FILES}
            )
