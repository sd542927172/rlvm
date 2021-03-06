// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#include "Modules/Module_Bgr.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

#include "Effects/Effect.hpp"
#include "Effects/EffectFactory.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "MachineBase/RLOperation/Special_T.hpp"
#include "Modules/Module_Grp.hpp"
#include "Systems/Base/Colour.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/HIKRenderer.hpp"
#include "Systems/Base/HIKScript.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Utilities/Graphics.hpp"

using namespace std;
namespace fs = boost::filesystem;
using boost::iends_with;
using boost::shared_ptr;

// Working theory of how this module works: The haikei module is one backing
// surface and (optionally) a HIK script. Games like AIR and the Maiden Halo
// demo use just the surface with a combination of bgrMulti and
// bgrLoadHaikei. OTOH, ALMA and planetarian use HIK scripts and the whole
// point of HIK scripts is to manipulate the backing surface on a timer that's
// divorced from the main interpreter loop.

namespace {

struct bgrLoadHaikei_blank : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int sel) {
    GraphicsSystem& graphics = machine.system().graphics();
    graphics.setDefaultBgrName("");
    graphics.setHikRenderer(NULL);
    graphics.setGraphicsBackground(BACKGROUND_HIK);

    boost::shared_ptr<Surface> before = graphics.renderToSurface();
    graphics.getHaikei()->fill(RGBAColour::Clear());

    if (!machine.replaying_graphics_stack())
      graphics.clearAndPromoteObjects();

    boost::shared_ptr<Surface> after = graphics.renderToSurface();

    LongOperation* effect =
        EffectFactory::buildFromSEL(machine, after, before, sel);
    machine.pushLongOperation(effect);
  }
};

struct bgrLoadHaikei_main : RLOp_Void_2<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int sel) {
    System& system = machine.system();
    GraphicsSystem& graphics = system.graphics();
    graphics.setDefaultBgrName(filename);
    graphics.setGraphicsBackground(BACKGROUND_HIK);

    // bgrLoadHaikei clears the stack.
    graphics.clearStack();

    fs::path path = system.findFile(filename, HIK_FILETYPES);
    if (iends_with(path.string(), "hik")) {
      if (!machine.replaying_graphics_stack())
        graphics.clearAndPromoteObjects();

      graphics.setHikRenderer(new HIKRenderer(
          system,
          graphics.GetHIKScript(system, filename, path)));
    } else {
      boost::shared_ptr<Surface> before = graphics.renderToSurface();

      if (!path.empty()) {
        boost::shared_ptr<const Surface> source(
            graphics.getSurfaceNamedAndMarkViewed(machine, filename));
        boost::shared_ptr<Surface> haikei = graphics.getHaikei();
        source->blitToSurface(*haikei,
                              source->rect(),
                              source->rect(),
                              255, true);
      }

      // Promote the objects if we're in normal mode. If we're restoring the
      // graphics stack, we already have our layers promoted.
      if (!machine.replaying_graphics_stack())
        graphics.clearAndPromoteObjects();

      boost::shared_ptr<Surface> after = graphics.renderToSurface();

      LongOperation* effect =
          EffectFactory::buildFromSEL(machine, after, before, sel);
      machine.pushLongOperation(effect);
    }
  }
};

struct bgrLoadHaikei_wtf
    : RLOp_Void_4<StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int sel, int a, int b) {
    // cerr << "Filename: " << filename
    //      << "(a: " << a << ", b: " << b << ")" << endl;
    bgrLoadHaikei_main()(machine, filename, sel);
  }
};

struct bgrLoadHaikei_wtf2
    : RLOp_Void_6<StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
                  IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int sel, int a, int b,
                  int c, int d) {
    // cerr << "Filename: " << filename
    //      << "(a: " << a << ", b: " << b << ", c: " << c << ", d: " << d << ")"
    //      << endl;
    bgrLoadHaikei_main()(machine, filename, sel);
  }
};

// -----------------------------------------------------------------------

typedef Argc_T<
  Special_T<
    // 0:copy(strC 'filename')
    StrConstant_T,
    // 1:DUMMY. Unknown.
    Complex2_T<StrConstant_T, IntConstant_T>,
    // 2:copy(strC 'filename', '?')
    Complex2_T<StrConstant_T, IntConstant_T>,
    // 3:DUMMY. Unknown.
    Complex2_T<StrConstant_T, IntConstant_T>,
    // 4:copy(strC, '?', '?')
    Complex3_T<StrConstant_T, IntConstant_T, IntConstant_T>
    > > BgrMultiCommand;

struct bgrMulti_1 : public RLOp_Void_3<
  StrConstant_T, IntConstant_T, BgrMultiCommand> {
 public:
  void operator()(RLMachine& machine, string filename, int effectNum,
                  BgrMultiCommand::type commands) {
    GraphicsSystem& graphics = machine.system().graphics();

    // Get the state of the world before we do any processing.
    shared_ptr<Surface> before = graphics.renderToSurface();

    graphics.setGraphicsBackground(BACKGROUND_HIK);

    // May need to use current background.
    if (filename == "???")
      filename = graphics.defaultBgrName();

    // Load "filename" as the background.
    shared_ptr<const Surface> surface(
        graphics.getSurfaceNamedAndMarkViewed(machine, filename));
    surface->blitToSurface(*graphics.getHaikei(),
                           surface->rect(), surface->rect(),
                           255, true);

    // TODO(erg): Unsure about the alpha in these implementation.
    for (BgrMultiCommand::type::const_iterator it = commands.begin();
         it != commands.end(); it++) {
      switch (it->type) {
        case 0: {
          // 0:copy(strC 'filename')
          surface = graphics.getSurfaceNamedAndMarkViewed(machine, it->first);
          surface->blitToSurface(*graphics.getHaikei(),
                                 surface->rect(), surface->rect(),
                                 255, true);
          break;
        }
        case 2: {
          // 2:copy(strC 'filename', '?')
          Rect srcRect;
          Point dest;
          getSELPointAndRect(machine, it->third.get<1>(), srcRect, dest);

          surface = graphics.getSurfaceNamedAndMarkViewed(machine, it->third.get<0>());
          Rect destRect = Rect(dest, srcRect.size());
          surface->blitToSurface(*graphics.getHaikei(), srcRect, destRect,
                                 255, true);
          break;
        }
        default: {
          cerr << "Don't know what to do with a type " << it->type
               << " in bgrMulti_1"
               << endl;
          break;
        }
      }
    }

    // Promote the objects if we're in normal mode. If we're restoring the
    // graphics stack, we already have our layers promoted.
    if (!machine.replaying_graphics_stack())
      graphics.clearAndPromoteObjects();

    shared_ptr<Surface> after = graphics.renderToSurface();
    LongOperation* effect =
        EffectFactory::buildFromSEL(machine, after, before, effectNum);
    machine.pushLongOperation(effect);
  }
};

struct bgrNext : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    HIKRenderer* renderer = machine.system().graphics().getHikRenderer();
    if (renderer) {
      renderer->NextAnimationFrame();
    }
  }
};

struct bgrSetXOffset : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int offset) {
    HIKRenderer* renderer = machine.system().graphics().getHikRenderer();
    if (renderer) {
      renderer->set_x_offset(offset);
    }
  }
};

struct bgrSetYOffset : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int offset) {
    HIKRenderer* renderer = machine.system().graphics().getHikRenderer();
    if (renderer) {
      renderer->set_y_offset(offset);
    }
  }
};

struct bgrPreloadScript : public RLOp_Void_2<IntConstant_T, StrConstant_T> {
  void operator()(RLMachine& machine, int slot, string name) {
    System& system = machine.system();
    fs::path path = system.findFile(name, HIK_FILETYPES);
    if (iends_with(path.string(), "hik")) {
      system.graphics().PreloadHIKScript(system, slot, name, path);
    }
  }
};

}  // namespace

// -----------------------------------------------------------------------

BgrModule::BgrModule()
    : MappedRLModule(graphicsStackMappingFun, "Bgr", 1, 40) {
  addOpcode(10, 0, "bgrLoadHaikei", new bgrLoadHaikei_blank);
  addOpcode(10, 1, "bgrLoadHaikei", new bgrLoadHaikei_main);
  addOpcode(10, 2, "bgrLoadHaikei", new bgrLoadHaikei_wtf);
  addOpcode(10, 3, "bgrLoadHaikei", new bgrLoadHaikei_wtf2);

  addUnsupportedOpcode(100, 0, "bgrMulti");
  addOpcode(100, 1, "bgrMulti", new bgrMulti_1);

  addOpcode(1000, 0, "bgrNext", new bgrNext);

  addOpcode(1104, 0, "bgrSetXOffset", new bgrSetXOffset);
  addOpcode(1105, 0, "bgrSetYOffset", new bgrSetYOffset);

  addOpcode(2000, 0, "bgrPreloadScript", new bgrPreloadScript);
  addOpcode(2001, 0, "bgrClearPreloadedScript",
            callFunction(&GraphicsSystem::ClearPreloadedHIKScript));
  addOpcode(2002, 0, "bgrClearAllPreloadedScripts",
            callFunction(&GraphicsSystem::ClearAllPreloadedHIKScripts));
}
