// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "libReallive/reallive.h"
#include "libReallive/gameexe.h"
#include "MachineBase/RLMachine.hpp"
#include "ScriptMachine/ScriptMachine.hpp"
#include "ScriptMachine/ScriptWorld.hpp"
#include "MachineBase/Serialization.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/SDL/SDLSystem.hpp"
#include "Modules/Modules.hpp"
#include "Utilities.h"
#include "Utilities/findFontFile.h"

#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>

// We include this here because SDL is retarded and works by #define
// main(inat argc, char* agrv[]). Loosers.
#include <SDL/SDL.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

void printVersionInformation()
{
  cout
    << "rlvm (version 0.3)" << endl
    << "Copyright (C) 2006-2008 Elliot Glaysher, Haeleth, Jagarl, et all."
    << endl << endl
    << "This program is free software: you can redistribute it and/or modify"
    << endl
    << "it under the terms of the GNU General Public License as published by"
    << endl
    << "the Free Software Foundation, either version 3 of the License, or"
    << endl
    << "(at your option) any later version."
    << endl << endl
    << "This program is distributed in the hope that it will be useful,"
    << endl
    << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
    << endl
    << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
    << endl
    << "GNU General Public License for more details."
    << endl << endl
    << "You should have received a copy of the GNU General Public License"
    << endl
    << "along with this program.  If not, see <http://www.gnu.org/licenses/>."
    << endl << endl;
}

// -----------------------------------------------------------------------

void printUsage(const string& name, po::options_description& opts)
{
  cout << "Usage: " << name << " [options] <lua script to run>" << endl;
  cout << opts << endl;
}

// -----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  srand(time(NULL));

  // Set global state: allow spaces in game paths
  fs::path::default_name_check(fs::native);

  // -----------------------------------------------------------------------
  // Parse command line options

  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
    ("help", "Produce help message")
    ("version", "Display version and license information")
    ("font", po::value<string>(), "Specifies TrueType font to use.")
    ("undefined-opcodes", "Display a message on undefined opcodes")
    ("count-undefined",
     "On exit, present a summary table about how many times each undefined opcode was called")
    ("save-on-decision", po::value<int>(), "Automatically save the game on decision points to the specified save game slot. Useful while debugging crashes far into a game.")
    ("save-on-decision-counting-from", po::value<int>(), "Like --save-on-decision, but will increment the save number every time.")
    ;

  // Declare the final option to be game-root
  po::options_description hidden("Hidden");
  hidden.add_options()
    ("script-location", po::value<string>(), "Location of the lua script");

  po::positional_options_description p;
  p.add("script-location", -1);

  // Use these on the command line
  po::options_description commandLineOpts;
  commandLineOpts.add(opts).add(hidden);

  po::variables_map vm;
  po::store(po::basic_command_line_parser<char>(argc, argv).
            options(commandLineOpts).positional(p).run(),
            vm);
  po::notify(vm);

  // -----------------------------------------------------------------------
  // Process command line options
  fs::path scriptLocation, gamerootPath, gameexePath, seenPath;

  if(vm.count("help"))
  {
    printUsage(argv[0], opts);
    return 0;
  }

  if(vm.count("version"))
  {
    printVersionInformation();
    return 0;
  }

  if(vm.count("script-location"))
  {
    scriptLocation = vm["script-location"].as<string>();

    if(!fs::exists(scriptLocation))
    {
      cerr << "ERROR: File '" << gamerootPath << "' does not exist." << endl;
      return -1;
    }
  } else {
    printUsage(argv[0], opts);
    return -1;
  }

  try {
    // Run the incoming lua file and do some basic error checking on what it
    // wants us to do.
    ScriptWorld world(scriptLocation.file_string());

    if (world.regname() == "") {
      cerr << "Script did not specify a regname. Please add a World::setRegname() statement." << endl;
      return -2;
    }

    if (world.gameRoot() == "") {
      cerr << "Script did not specify a game root. Please add a World::setGameRoot() statement." << endl;
      return -2;
    }

    gamerootPath = world.gameRoot();
    if (!fs::exists(gamerootPath)) {
      cerr << "Specified gameroot \"" << gamerootPath << "\" does not exist." << endl;
      return -2;
    }

    gameexePath = correctPathCase(gamerootPath / "Gameexe.ini");
    seenPath = correctPathCase(gamerootPath / "Seen.txt");

    Gameexe gameexe(gameexePath);
    gameexe("__GAMEPATH") = gamerootPath.file_string();
    gameexe("MEMORY") = 1;

    SDLSystem sdlSystem(gameexe);
    libReallive::Archive arc(seenPath.file_string(), gameexe("REGNAME"));

    ScriptMachine rlmachine(sdlSystem, arc);
    addAllModules(rlmachine);
    world.initializeMachine(rlmachine);

    // Make sure we go as fast as possible:
    sdlSystem.setForceFastForward();

    if(vm.count("undefined-opcodes"))
      rlmachine.setPrintUndefinedOpcodes(true);

    if(vm.count("count-undefined"))
      rlmachine.recordUndefinedOpcodeCounts();

    if(vm.count("save-on-decision")) {
      int decision_num = vm["save-on-decision"].as<int>();
      rlmachine.saveOnDecisions(decision_num);
    }

    if (vm.count("save-on-decision-counting-from")) {
      int start_from = vm["save-on-decision-counting-from"].as<int>();
      rlmachine.saveOnDecisions(start_from);
      rlmachine.incrementOnSave();
    }

    Serialization::loadGlobalMemory(rlmachine);
    rlmachine.setHaltOnException(false);

    while(!rlmachine.halted()) {
      // Give SDL a chance to respond to events, redraw the screen,
      // etc.
      sdlSystem.run(rlmachine);

      // Run the rlmachine through another instruction
      rlmachine.executeNextInstruction();
    }

    Serialization::saveGlobalMemory(rlmachine);
  }
  catch (rlvm::Exception& e) {
    cerr << "Fatal RLVM error: " << e.what() << endl;
    return 1;
  }
  catch (libReallive::Error& e) {
    cerr << "Fatal libReallive error: " << e.what() << endl;
    return 1;
  }
  catch(SystemError& e) {
    cerr << "Fatal local system error: " << e.what() << endl;
    return 1;
  }
  catch(std::exception& e) {
    cout << "Uncaught exception: " << e.what() << endl;
    return 1;
  }
  catch(const char* e) {
    cout << "Uncaught exception: " << e << endl;
    return 1;
  }

  return 0;
}