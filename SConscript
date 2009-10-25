# Main scons file

import shutil
import sys

Import('env')

########################################################## [ Root environment ]
root_env = env.Clone()
root_env.Append(
  CPPDEFINES = [
    "CASE_SENSITIVE_FILESYSTEM",
    "_THREAD_SAFE"
  ],
)

if root_env['PLATFORM'] == 'darwin':
  root_env.Append(FRAMEWORKS=["OpenGL"])
else:
  root_env.Append(LIBS=["GL", "GLU"])
root_env.ParseConfig("sdl-config --libs")

#########################################################################

# All the PNG files taken from The Mana World that we want to compile in
guichan_resources = [
  "src/Platforms/gcn/button_disabled.png",
  "src/Platforms/gcn/buttonhi.png",
  "src/Platforms/gcn/button.png",
  "src/Platforms/gcn/buttonpress.png",
  "src/Platforms/gcn/deepbox.png",
  "src/Platforms/gcn/hscroll_left_default.png",
  "src/Platforms/gcn/hscroll_left_pressed.png",
  "src/Platforms/gcn/hscroll_right_default.png",
  "src/Platforms/gcn/hscroll_right_pressed.png",
  "src/Platforms/gcn/vscroll_down_default.png",
  "src/Platforms/gcn/vscroll_down_pressed.png",
  "src/Platforms/gcn/vscroll_grey.png",
  "src/Platforms/gcn/vscroll_up_default.png",
  "src/Platforms/gcn/vscroll_up_pressed.png"
]

# Build the resource compiler and then use it to make the inline header file.
root_env.Program('wxInclude', ['vendor/wxInclude.cpp'])
root_env.Command('$BUILD_DIR/resource_header.h',
                 ['$BUILD_DIR/wxInclude', guichan_resources],
                 ["$SOURCE --silent --wxnone --output-file=$TARGET " +
                  " ".join(guichan_resources)])

# We'll need to be able to see the autogenerated header:
root_env.Append(
  CPPPATH = ["$BUILD_DIR"]
)

#########################################################################

librlvm_files = [
  "src/Effects/BlindEffect.cpp",
  "src/Effects/Effect.cpp",
  "src/Effects/EffectFactory.cpp",
  "src/Effects/FadeEffect.cpp",
  "src/Effects/ScrollOnScrollOff.cpp",
  "src/Effects/WipeEffect.cpp",
  "src/Encodings/codepage.cpp",
  "src/Encodings/cp932.cpp",
  "src/Encodings/cp936.cpp",
  "src/Encodings/cp949.cpp",
  "src/Encodings/han2zen.cpp",
  "src/Encodings/western.cpp",
  "src/LongOperations/PauseLongOperation.cpp",
  "src/LongOperations/SelectLongOperation.cpp",
  "src/LongOperations/TextoutLongOperation.cpp",
  "src/LongOperations/WaitLongOperation.cpp",
  "src/LongOperations/ZoomLongOperation.cpp",
  "src/MachineBase/GeneralOperations.cpp",
  "src/MachineBase/LongOperation.cpp",
  "src/MachineBase/MappedRLModule.cpp",
  "src/MachineBase/Memory.cpp",
  "src/MachineBase/OpcodeLog.cpp",
  "src/MachineBase/RLMachine.cpp",
  "src/MachineBase/RLMachine_intmem.cpp",
  "src/MachineBase/RLModule.cpp",
  "src/MachineBase/RLOperation.cpp",
  "src/MachineBase/RealLiveDLL.cpp",
  "src/MachineBase/SaveGameHeader.cpp",
  "src/MachineBase/SerializationGlobal.cpp",
  "src/MachineBase/SerializationLocal.cpp",
  "src/MachineBase/StackFrame.cpp",
  "src/MachineBase/reference.cpp",
  "src/Modules/Module_Bgm.cpp",
  "src/Modules/Module_Bgr.cpp",
  "src/Modules/Module_DLL.cpp",
  "src/Modules/Module_Debug.cpp",
  "src/Modules/Module_EventLoop.cpp",
  "src/Modules/Module_Gan.cpp",
  "src/Modules/Module_Grp.cpp",
  "src/Modules/Module_Jmp.cpp",
  "src/Modules/Module_Koe.cpp",
  "src/Modules/Module_Mem.cpp",
  "src/Modules/Module_Mov.cpp",
  "src/Modules/Module_Msg.cpp",
  "src/Modules/Module_Obj.cpp",
  "src/Modules/Module_ObjCreation.cpp",
  "src/Modules/Module_ObjFgBg.cpp",
  "src/Modules/Module_ObjManagement.cpp",
  "src/Modules/Module_ObjPosDims.cpp",
  "src/Modules/Module_Os.cpp",
  "src/Modules/Module_Pcm.cpp",
  "src/Modules/Module_Refresh.cpp",
  "src/Modules/Module_Scr.cpp",
  "src/Modules/Module_Se.cpp",
  "src/Modules/Module_Sel.cpp",
  "src/Modules/Module_Shk.cpp",
  "src/Modules/Module_Shl.cpp",
  "src/Modules/Module_Str.cpp",
  "src/Modules/Module_Sys.cpp",
  "src/Modules/Module_Sys_Date.cpp",
  "src/Modules/Module_Sys_Frame.cpp",
  "src/Modules/Module_Sys_Name.cpp",
  "src/Modules/Module_Sys_Save.cpp",
  "src/Modules/Module_Sys_Syscom.cpp",
  "src/Modules/Module_Sys_Timer.cpp",
  "src/Modules/Module_Sys_Wait.cpp",
  "src/Modules/Module_Sys_index_series.cpp",
  "src/Modules/Modules.cpp",
  "src/Systems/Base/AnmGraphicsObjectData.cpp",
  "src/Systems/Base/CGMTable.cpp",
  "src/Systems/Base/Colour.cpp",
  "src/Systems/Base/EventListener.cpp",
  "src/Systems/Base/EventSystem.cpp",
  "src/Systems/Base/FrameCounter.cpp",
  "src/Systems/Base/GanGraphicsObjectData.cpp",
  "src/Systems/Base/GraphicsObject.cpp",
  "src/Systems/Base/GraphicsObjectData.cpp",
  "src/Systems/Base/GraphicsObjectOfFile.cpp",
  "src/Systems/Base/GraphicsStackFrame.cpp",
  "src/Systems/Base/GraphicsSystem.cpp",
  "src/Systems/Base/GraphicsTextObject.cpp",
  "src/Systems/Base/KOEPACVoiceArchive.cpp",
  "src/Systems/Base/MouseCursor.cpp",
  "src/Systems/Base/ObjectSettings.cpp",
  "src/Systems/Base/OVKVoiceArchive.cpp",
  "src/Systems/Base/OVKVoiceSample.cpp",
  "src/Systems/Base/ParentGraphicsObjectData.cpp",
  "src/Systems/Base/Platform.cpp",
  "src/Systems/Base/RLTimer.cpp",
  "src/Systems/Base/RlBabelDLL.cpp",
  "src/Systems/Base/Rect.cpp",
  "src/Systems/Base/SelectionElement.cpp",
  "src/Systems/Base/SoundSystem.cpp",
  "src/Systems/Base/Surface.cpp",
  "src/Systems/Base/System.cpp",
  "src/Systems/Base/SystemError.cpp",
  "src/Systems/Base/TextKeyCursor.cpp",
  "src/Systems/Base/TextPage.cpp",
  "src/Systems/Base/TextSystem.cpp",
  "src/Systems/Base/TextWaku.cpp",
  "src/Systems/Base/TextWakuNormal.cpp",
  "src/Systems/Base/TextWakuType4.cpp",
  "src/Systems/Base/TextWindow.cpp",
  "src/Systems/Base/TextWindowButton.cpp",
  "src/Systems/Base/VoiceArchive.cpp",
  "src/Systems/Base/VoiceCache.cpp",
  "src/Utilities/Exception.cpp",
  "src/Utilities/File.cpp",
  "src/Utilities/Graphics.cpp",
  "src/Utilities/StringUtilities.cpp",
  "src/Utilities/dateUtil.cpp",
  "src/Utilities/findFontFile.cpp",
  "src/libReallive/archive.cpp",
  "src/libReallive/bytecode.cpp",
  "src/libReallive/compression.cpp",
  "src/libReallive/expression.cpp",
  "src/libReallive/filemap.cpp",
  "src/libReallive/gameexe.cpp",
  "src/libReallive/intmemref.cpp",
  "src/libReallive/scenario.cpp",
  "vendor/xclannad/endian.cpp",
  "vendor/xclannad/file.cc",
  "vendor/xclannad/nwatowav.cc",
  "vendor/xclannad/wavfile.cc"
]

root_env.StaticLibrary('rlvm', librlvm_files)

libsystemsdl_files = [
  "src/Systems/SDL/SDLAudioLocker.cpp",
  "src/Systems/SDL/SDLEventSystem.cpp",
  "src/Systems/SDL/SDLGraphicsSystem.cpp",
  "src/Systems/SDL/SDLMusic.cpp",
  "src/Systems/SDL/SDLRenderToTextureSurface.cpp",
  "src/Systems/SDL/SDLSoundChunk.cpp",
  "src/Systems/SDL/SDLSoundSystem.cpp",
  "src/Systems/SDL/SDLSurface.cpp",
  "src/Systems/SDL/SDLSystem.cpp",
  "src/Systems/SDL/SDLTextSystem.cpp",
  "src/Systems/SDL/SDLTextWindow.cpp",
  "src/Systems/SDL/SDLUtils.cpp",
  "src/Systems/SDL/Texture.cpp",
  "vendor/pygame/alphablit.cc"
]

root_env.StaticLibrary('system_sdl', libsystemsdl_files)

guichan_platform = [
  "src/Platforms/gcn/GCNButton.cpp",
  "src/Platforms/gcn/GCNGraphics.cpp",
  "src/Platforms/gcn/GCNInfoWindow.cpp",
  "src/Platforms/gcn/GCNMenu.cpp",
  "src/Platforms/gcn/GCNPlatform.cpp",
  "src/Platforms/gcn/GCNSaveLoadWindow.cpp",
  "src/Platforms/gcn/GCNScrollArea.cpp",
  "src/Platforms/gcn/GCNWindow.cpp",
  "src/Platforms/gcn/SDLTrueTypeFont.cpp",
  "src/Platforms/gcn/gcnUtils.cpp"
]

root_env.StaticLibrary('guichan_platform', guichan_platform)

root_env.RlvmProgram('rlvm', ["src/rlvm.cpp"],
                     use_lib_set = ["SDL"],
                     rlvm_libs = ["guichan_platform", "system_sdl", "rlvm"])
root_env.Install('$OUTPUT_DIR', 'rlvm')

#########################################################################

test_env = root_env.Clone()
test_env.Append(CPPPATH = ["#/test"])

test_case_files = [
  "test/TestSystem/TestMachine.cpp",

  "test/testUtils.cpp",
  "test/gameexe_test.cpp",
  "test/rlmachine_test.cpp",
  "test/lazy_array_test.cpp",
  "test/graphics_object_test.cpp",
  "test/rloperation_test.cpp",
  "test/regressions_test.cpp",
  "test/text_system_test.cpp",
  "test/expression_test.cpp",
  "test/sound_system_test.cpp",
  "test/text_window_test.cpp",
  "test/effect_test.cpp",
  "test/rlbabel_test.cpp",

  # medium tests
  "test/medium_eventloop_test.cpp",
  "test/medium_msg_test.cpp",
  "test/medium_object_promotion.cpp",
  "test/medium_grp_test.cpp",

  # large tests
  "test/large_sys_test.cpp",
  "test/large_str_test.cpp",
  "test/large_mem_test.cpp",
  "test/large_jmp_test.cpp"
]

null_system_files = [
  "test/TestSystem/TestEventSystem.cpp",
  "test/TestSystem/TestGraphicsSystem.cpp",
  "test/TestSystem/TestSoundSystem.cpp",
  "test/TestSystem/TestSystem.cpp",
  "test/TestSystem/TestTextSystem.cpp",
  "test/TestSystem/TestTextWindow.cpp",
  "test/TestSystem/MockSurface.cpp",
  "test/TestSystem/MockTextWindow.cpp"
]

if env['BUILD_RLC_TESTS']:
  test_env.RlvmProgram('rlvmTests', ["test/rlvmTest.cpp", null_system_files,
                                     test_case_files],
                       use_lib_set = ["TEST"],
                       rlvm_libs = ["rlvm"])
  test_env.Install('$OUTPUT_DIR', 'rlvmTests')

#########################################################################

script_machine_files = [
  "test/ScriptMachine/ScriptMachine.cpp",
  "test/ScriptMachine/ScriptWorld.cpp",
  "test/ScriptMachine/luabind_EventSystem.cpp",
  "test/ScriptMachine/luabind_GraphicsObject.cpp",
  "test/ScriptMachine/luabind_GraphicsSystem.cpp",
  "test/ScriptMachine/luabind_Machine.cpp",
  "test/ScriptMachine/luabind_System.cpp",
  "test/ScriptMachine/luabind_utility.cpp"
]

if env['BUILD_LUA_TESTS'] == True:
  test_env.Append(CPPPATH = [ "/usr/include/lua5.1" ] )

  test_env.RlvmProgram("luaRlvm", ['test/luaRlvm.cpp', script_machine_files],
                       use_lib_set = ["SDL", "LUA"],
                       rlvm_libs = ["system_sdl", "rlvm"])
  test_env.Install('$OUTPUT_DIR', 'luaRlvm')
