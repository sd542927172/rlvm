// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

/**
 * @file   EffectFactory.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:36:53 2006
 * 
 * @brief  Factory that creates all Effect s.
 */

#include "Modules/EffectFactory.hpp"
#include "Modules/FadeEffect.hpp"
#include "Modules/WipeEffect.hpp"

#include <iostream>
#include <sstream>

//#include "libReallive/defs.h"
#include "libReallive/gameexe.h"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"

using namespace std;
using namespace libReallive;

const int SEL_SIZE = 16;

// Helper functions
namespace {

/** 
 * Changes the coordinate types. All operations internally are done in
 * rec coordinates, (x, y, width, height). The GRP functions pass
 * parameters of the format (x1, y1, x2, y2).
 * 
 * @param x1 X coordinate. Not changed by this function
 * @param y1 Y coordinate. Not changed by this function
 * @param x2 X2. In place changed to width.
 * @param y2 Y2. In place changed to height.
 */
void grpToRecCoordinates(int x1, int y1, int& x2, int& y2)
{
  x2 = x2 - x1;
  y2 = y2 - y1;
}

}

// -----------------------------------------------------------------------

LongOperation* EffectFactory::buildFromSEL(RLMachine& machine, int selNum)
{
  Gameexe& gexe = machine.system().gameexe();
  int selParams[SEL_SIZE];

  for(int i = 0; i < SEL_SIZE; ++i)
    selParams[i] = gexe.getInt("SEL", selNum, i, 0);

  grpToRecCoordinates(selParams[0], selParams[1], 
                      selParams[2], selParams[3]);

  return build(machine, selParams[0], selParams[1], selParams[2], selParams[3],
               selParams[4], selParams[5], selParams[6], selParams[7], 
               selParams[8], selParams[9], selParams[10], selParams[11],
               selParams[12], selParams[13], selParams[14], selParams[15]);
}

// -----------------------------------------------------------------------

LongOperation* EffectFactory::buildFromSELR(RLMachine& machine, int selNum)
{
  Gameexe& gexe = machine.system().gameexe();
  int selParams[SEL_SIZE];

  for(int i = 0; i < SEL_SIZE; ++i)
    selParams[i] = gexe.getInt("SELR", selNum, i, 0);

  return build(machine, selParams[0], selParams[1], selParams[2], selParams[3],
               selParams[4], selParams[5], selParams[6], selParams[7], 
               selParams[8], selParams[9], selParams[10], selParams[11],
               selParams[12], selParams[13], selParams[14], selParams[15]);  
}

// -----------------------------------------------------------------------

LongOperation* EffectFactory::build(RLMachine& machine, 
  int x, int y, int width, int height, int dx, int dy, int time, int style,
  int direction, int interpolation, int xsize, int ysize, int a, int b,
  int opacity, int c)
{
  // There is a completely ridaculous number of transitions here! Damn
  // you, VisualArts, for making something so simple sounding so
  // confusing and hard to implement!
  switch(style)
  {
  case 10:
    return new WipeEffect(machine, x, y, width, height, dx, dy, time, 
                          direction, interpolation);
  case 0:
  case 50:
  default:
    cerr << "FadeEffect(" << x << "," << y << "," << width << "," 
         << height << "," << dx << "," << dy << "," << time << ")"
         << endl;
    return new FadeEffect(machine, x, y, width, height, dx, dy, time);
  }

  stringstream ss;
  ss << "Unsupported effect number in #SEL:" << style;
  throw Error(ss.str());
}