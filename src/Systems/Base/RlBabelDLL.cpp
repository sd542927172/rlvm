// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// -----------------------------------------------------------------------
//
// Copyright (c) 2006 Peter Jolly.
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/RlBabelDLL.hpp"

#include "Encodings/codepage.h"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "MachineBase/Memory.hpp"
#include "MachineBase/RLMachine.hpp"
#include "libReallive/gameexe.h"
#include "libReallive/intmemref.h"
#include "Utilities/StringUtilities.hpp"

#include <algorithm>
#include <iostream>
using std::cerr;
using std::endl;

// -----------------------------------------------------------------------

namespace {

const int kCodeMapSize = 6;
const getcReturn codemap[] = {
  getcEndOfString,
  getcEndOfString,
  getcEndOfString,
  getcNewLine,
  getcSetIndent,
  getcClearIndent
};

inline bool token_delimiter(char val) {
  return val < 6 || (val > 7 && val <= 32) || val == '-';
}

// -----------------------------------------------------------------------

boost::shared_ptr<TextWindow> getWindow(RLMachine& machine, int id) {
  TextSystem& text_system = machine.system().text();
  if (id >= 0) {
    return text_system.textWindow(machine, id);
  } else {
    return text_system.currentWindow(machine);
  }
}

} // anonymous namespace

// -----------------------------------------------------------------------
// Gloss
// -----------------------------------------------------------------------
Gloss::Gloss(RLMachine& machine, const std::string& cp932_src,
             int x1, int y1, int x2, int y2)
    : text_(cp932_src) {
  boost::shared_ptr<TextWindow> window = getWindow(machine, -1);

  int line_height = window->lineHeight();
  while (y1 < y2) {
    // Special case for multi-line links.  Hopefully these will be rare...
    link_areas_.push_back(Rect::GRP(x1, y1, window->textWindowSize().width(),
                                    y1 + line_height));

    y1 += line_height;
    x1 = window->currentIndentation();
  }

  link_areas_.push_back(Rect::GRP(x1, y1, x2, y2 + line_height));
}

// -----------------------------------------------------------------------

bool Gloss::contains(const Point& point) {
  return std::find_if(link_areas_.begin(), link_areas_.end(),
                      bind(&Rect::contains, _1, point)) != link_areas_.end();
}

// -----------------------------------------------------------------------
// RlBabelDLL
// -----------------------------------------------------------------------
RlBabelDLL::RlBabelDLL()
    : add_is_italic(false), gloss_start_x_(0), gloss_start_y_(0)
{}

// -----------------------------------------------------------------------

int RlBabelDLL::callDLL(RLMachine& machine, int func, int arg1, int arg2,
                        int arg3, int arg4) {
  switch (func) {
    case dllInitialise:
      return initialize(arg1, arg2);
    case dllTextoutStart:
      if (arg1 == -1) {
        textoutClear();
        return 1;
      } else {
        textoutClear();
        return textoutAdd(machine, *getSvar(machine, arg1));
      }
    case dllTextoutAppend:
      return textoutAdd(machine, *getSvar(machine, arg1));
    case dllTextoutGetChar:
      return textoutGetChar(machine, getSvar(machine, arg1),
                            getIvar(machine, arg2));
    case dllTextoutNewScreen:
      return startNewScreen(machine, *getSvar(machine, arg1));
    case dllSetNameMod: {
      boost::shared_ptr<TextWindow> textWindow = getWindow(machine, arg1);
      int original_mod = textWindow->nameMod();
      textWindow->setNameMod(arg2);
      return original_mod;
    }
    case dllGetNameMod:
      return getWindow(machine, arg1)->nameMod();
    case dllGetTextWindow:
      return getWindow(machine, -1)->windowNumber();
    case dllSetWindowName:
      return setCurrentWindowName(machine, getSvar(machine, arg1));
    case endSetWindowName:
    case endGetCharWinNam: {
      // Should never happen since we don't do the insane trick of overwriting
      // instruction memory to set the name of set the window name.
      return getcError;
    }
    case dllClearGlosses:
      return clearGlosses();
    case dllNewGloss:
      return newGloss(machine);
    case dllAddGloss:
      return addGloss(machine, *getSvar(machine, arg1));
    case dllTestGlosses:
      return testGlosses(machine, arg1, arg2, getSvar(machine, arg3), arg4);
    case dllGetRCommandMod: {
      int window = getWindow(machine, arg1)->windowNumber();
      return machine.system().gameexe()("WINDOW")(window)("R_COMMAND_MOD");
    }
    case dllMessageBox:
      //      return rlMsgBox(arg1, arg2);
    default:
      return -1;
  }
}

// -----------------------------------------------------------------------

int RlBabelDLL::initialize(int dllno, int windname) {
  // rlBabel hangs onto the dll index and uses it for something in his
  // SetCurrentWindowName implementation.
  return 1;
}

// -----------------------------------------------------------------------

int RlBabelDLL::textoutAdd(RLMachine& machine, const std::string& str) {
  const char* string = str.c_str();

  while (*string) {
    if (string[0] == 0x81 && (string[1] == 0x93 || string[1] == 0x96)
        && string[2] == 0x82 && (string[3] >= 0x60 && string[3] <= 0x79)) {
      // Name reference: expand it.
      bool global = string[1] == 0x96;
      // Get index.
      int idx = string[3] - 0x60;
      string += 4;
      if (string[0] == 0x82 && (string[1] >= 0x60 && string[1] <= 0x79)) {
        idx = (idx + 1) * 26 + string[1] - 0x60;
        string += 2;
      }
      Memory& memory = machine.memory();
      const char* namestr = global ? memory.getName(idx).c_str() :
        memory.getLocalName(idx).c_str();

      // Copy to string.
      if (string[0] == 0x82 &&(string[1] >= 0x4f && string[1] <= 0x58)) {
        // Just one character
        int offset = string[1] - 0x4f;
        string += 2;
        // Find character
        while (offset-- && *namestr) {
          if (shiftjis_lead_byte(*namestr++))
            ++namestr;
        }
        // Copy character
        if (*namestr) {
          AppendChar(namestr);
        }
      }
      else {
        // The whole string
        while (*namestr) {
          AppendChar(namestr);
        }
      }
    }
    else if (string[0] == 0x08) {
      // Quotation mark
//      string[0] = '"';
      const char* quote_mark = "\"";
      ++string;
      AppendChar(quote_mark);
    }
    else if (string[0] == 0x09) {
      add_is_italic = true;
      ++string;
    }
    else if (string[0] == 0x0a) {
      add_is_italic = false;
      ++string;
    }
    else {
      // Normal char: copy 1 or 2 bytes as appropriate.
      AppendChar(string);
    }
  }

  return !*string; // 0 if the full string wasn't copied
}

// -----------------------------------------------------------------------

void RlBabelDLL::AppendChar(const char*& ch) {
//   if (add_is_italic) {
//     short uc = *ch++;
//     if (shiftjis_lead_byte(uc))
//       uc = (uc << 8) | *ch++;
//     uc = Italicise(uc);
//     if (uc > 0xff)
//       cp932_text_buffer += uc >> 8;
//     cp932_text_buffer += uc & 0xff;
//   } else {
    copyOneShiftJisCharacter(ch, cp932_text_buffer);
//  }
}

// -----------------------------------------------------------------------

void RlBabelDLL::textoutClear() {
  cp932_text_buffer.clear();
  text_index = 0;
  end_token_index = 0;
  add_is_italic = false;
}

// -----------------------------------------------------------------------

int RlBabelDLL::textoutLineBreak(RLMachine& machine,
                                 StringReferenceIterator buf) {
  boost::shared_ptr<TextWindow> window = getWindow(machine, -1);

  // If there's room on this page, break the line, otherwise break
  // the page.
  if (window->lineHeight() <
      window->textWindowSize().height() - window->insertionPointY())
    return getcNewLine;
  else {
//     const char* const cnam = (const char*) interpreter->getCurrentName();
//     if (cnam)
//       *buf = 
//     else
    *buf = "";
    return getcNewScreen;
  }
}

// -----------------------------------------------------------------------

int RlBabelDLL::textoutGetChar(RLMachine& machine,
                               StringReferenceIterator buffer,
                               IntReferenceIterator xmod) {
  int rv = getcPrintChar;

  // No data? Error!
  if (cp932_text_buffer.empty())
    return getcEndOfString;

  // If a new token is needed, find it and determine whether to add
  // a line break.  If a line break is needed, bail at some point
  // and return getcNewLine.
  if (text_index == end_token_index) {
    if (!endToken())
      return getcEndOfString;

    switch(endToken()) {
      // TODO: I'm ignoring most of the cases for now. Ouch!
      case 1: {
        // \{...} -> \x01...\x02
        // If this is a name setter, we read the name.
        //
        // Processing of names is complicated.  There are three
        // modes, each with their own considerations:
        //
        // 0 - the name is displayed in the window.  In this case,
        // we set it with SetCurrentWindowName, then disable name
        // display ourselves and render it manually.
        //
        // 1 - the name is displayed in a separate window.  We
        // have no direct control over how the name is laid out,
        // so we need to render it with tile bitmaps (as for
        // select() calls).
        //
        // 2 - name display is disabled.  In this case, we set it
        // with SetCurrentWindowName, and do nothing further.
        //
        // So, if #WINDOW.NAME_MOD is 0, we fill the current token
        // with it; either way, we pass it to SetCurrentWindowName
        // to process.  This function is then called again
        // automatically, and will either read the current token
        // containing the name, or start reading a new token.
        text_index = ++end_token_index;
        while (endToken() && endToken() != 2)
          ++end_token_index;
        if (!endToken()) {
//           MessageBox(NULL, "Error: mismatched name delimiters",
//                      "RLdev.Textout", MB_OK | MB_ICONERROR);
          // For now, just bail out on this.
          return getcEndOfString;
        }

        // Haeleth's rlBabel for RealLive does a big ugly hack right about here
        // because the name window code has something preventing him from
        // hooking the text out function so he made a whole bunch of name moji
        // tiles and rendered into those. This is silly and unneccessary since
        // I control the rlvm code...
        *buffer = cp932_text_buffer.substr(text_index, end_token_index - text_index);

        // If name display is not inline, skip the token to avoid
        // rendering it inline.
        if (getWindow(machine, -1)->nameMod() >= 1)
          text_index = end_token_index;

        // Set the window name.
        rv = setCurrentWindowName(machine, buffer);

        // By default, Haeleth's rlBabel will rewrite a big chunk of
        // instruction memory to writeout the equivalent of '\{name}' and a
        // CallDLL() call to signal that it's done and that rlBabel should put
        // the instruction memory back to how it was found, and then print out
        // the buffer. This is, of course, completely insane! But it was
        // probably the only we could get around RealLives lousy binary
        // interface. We can maintain compatibility by just printing the
        // current buffer out right here.
        return textoutGetChar(machine, buffer, xmod);
      }
      case 2: {
        Codepage& cp = Cp::instance(machine.getTextEncoding());
        // End of previously-handled name setter.
        // If this is the end of the string, do nothing.
        if (!endToken(1))
          return getcEndOfString;
        // Add indentation as appropriate.
        text_index = --end_token_index;
        if (endToken(2) == '"' || endToken(2) == '(' || endToken(2) == '\'') {
          endToken(0) = ' ';
          endToken(1) = endToken(2);
          endToken(2) = 4;
        } else if (endToken(2)==':' || endToken(2)=='.' || endToken(2)==',') {
          if (endToken(3) == ' ')
            text_index = ++end_token_index;
          endToken(0) = endToken(1);
          endToken(1) = ' ';
          endToken(2) = 4;
        } else if (cp.DbcsDelim(&(cp932_text_buffer[end_token_index + 2]))) {
          endToken(0) = ' ';
          endToken(1) = endToken(2);
          endToken(2) = endToken(3);
          endToken(3) = 4;
        } else {
          endToken(0) = ' ';
          endToken(1) = 4;
        }
        return textoutGetChar(machine, buffer, xmod);
      }
      case 3: {
        // Line break code (\n)
        text_index = ++end_token_index;
        return textoutLineBreak(machine, buffer);
      }
      case 4: {
        // Set indent code
        if (getWindow(machine, -1)->useIndentation()) {
          char index = endToken();
          if (index < kCodeMapSize)
            rv = codemap[index];
          else
            rv = getcError;
          text_index = ++end_token_index;
          return rv;
        }
        // If #WINDOW.INDENT_USE is 0, we ignore this code.
        text_index = ++end_token_index;
        return textoutGetChar(machine, buffer, xmod);
      }
      case 5: {
        char index = endToken();
        if (index < kCodeMapSize)
          rv = codemap[index];
        else
          rv = getcError;
        text_index = ++end_token_index;
        return rv;
      }
      // Tags 6 and 7 are exfont moji, which are handled as part
      // of regular tokens.
      case 8:
      case 9:
      case 10: {
        // Shouldn't happen (should be expanded away in TextoutAdd).
        // For now, ignore them.
        text_index = ++end_token_index;
        return textoutGetChar(machine, buffer, xmod);
      }
      case 0x1f: // Begin gloss marker.  Treat as text.
        ++end_token_index;
        // Fall through!
      default: {
        // We tokenise text and break lines appropriately.
        // TODO: Haeleth appears to be caching horizontal width. Should do this
        // once I have something working.
        //
        // ZeroMemory(widthcache, 256 * sizeof(LONG));

        // Scan ahead for next delimiter, including spaces before
        // and the first hyphen after the token.
        while (endToken() == ' ')
          ++end_token_index;
        while (!token_delimiter(endToken()))
          end_token_index += 1 + shiftjis_lead_byte(endToken())
                             + (endToken() == 6 || endToken() == 7 ? 2 : 0);
        if (endToken() == '-')
          ++end_token_index;
        // If the token will not fit on the current line, insert a
        // line break and strip leading spaces.
        if (lineBreakRequired(machine)) {
          while (curPos() == ' ')
            ++text_index;
          return textoutLineBreak(machine, buffer);
        }
        // Otherwise the token will begin displaying.
      }
    }
  }

  // If we got here, we have a token and no line break is needed.

  // Special case for begin-gloss:
  if (curPos() == 0x1f) {
    ++text_index;
    rv = getcBeginGloss;
  }

  // TODO: Missing special case for emoji...
  if (curPos() == 6 || curPos() == 7) {
    assert(0 && "Implement special case for emoji in textoutGetChar");
  }

  // Fill buf with the next character, and xmod with how far to
  // adjust its width.
  unsigned short full_char = 0;
  char first_byte = cp932_text_buffer[text_index++];
  std::string cp932_char_out;
  cp932_char_out += first_byte;
  if (shiftjis_lead_byte(first_byte)) {
    cp932_char_out += cp932_text_buffer[text_index++];
    full_char = (cp932_char_out[0] << 8) | cp932_char_out[1];
  } else {
    full_char = cp932_char_out[0];
  }

  // FIXME: what's wrong with nbsps?
  if (full_char == 0x89a0) {
    cp932_char_out.clear();
    cp932_char_out += 0x20;
    full_char = 0x20;
  }

  *buffer = cp932_char_out;
  *xmod = getCharWidth(machine, full_char, true);
  return rv;
}

// -----------------------------------------------------------------------

int RlBabelDLL::startNewScreen(RLMachine& machine, const std::string& cnam) {
  if (cnam.empty())
    return getcEndOfString;

  string buf = cnam;
  buf += "\x02\"";
  buf += cp932_text_buffer.substr(text_index);
  textoutClear();
  return textoutAdd(machine, buf);
}

// -----------------------------------------------------------------------

int RlBabelDLL::setCurrentWindowName(RLMachine& machine,
                                     StringReferenceIterator buffer) {
  // Haeleth's implementation of SetCurrentWindowName in rlBabel goes through
  // some monstrous hacks, including temporarily rewriting the bytecode at the
  // instruction pointer. I *think* I can get away with a simple:
  getWindow(machine, -1)->setNameWithoutDisplay(*buffer);
  return 1;
}

// -----------------------------------------------------------------------

int RlBabelDLL::clearGlosses() {
  glosses_.clear();
  return 1;
}

// -----------------------------------------------------------------------

int RlBabelDLL::newGloss(RLMachine& machine) {
  boost::shared_ptr<TextWindow> window = getWindow(machine, -1);
  gloss_start_x_ = window->insertionPointX();
  gloss_start_y_ = window->insertionPointY();
  return 1;
}

// -----------------------------------------------------------------------

int RlBabelDLL::addGloss(RLMachine& machine,
                         const std::string& cp932_gloss_text) {
  boost::shared_ptr<TextWindow> window = getWindow(machine, -1);
  glosses_.push_back(Gloss(machine,
                           cp932_gloss_text, gloss_start_x_, gloss_start_y_,
                           window->insertionPointX(),
                           window->insertionPointY()));
  return 1;
}

// -----------------------------------------------------------------------

int RlBabelDLL::testGlosses(RLMachine& machine,
                            int x, int y, StringReferenceIterator text,
                            int globalwaku) {
  // Does this handle all cases?
  boost::shared_ptr<TextWindow> window = getWindow(machine, -1);
  x -= window->textX1();
  y -= window->textY1();

  std::vector<Gloss>::const_iterator it =
      std::find_if(glosses_.begin(), glosses_.end(),
                   bind(&Gloss::contains, _1, Point(x, y)));
  if (it == glosses_.end())
    return 0;

  *text = it->text();
  return 1;
}

// -----------------------------------------------------------------------

int RlBabelDLL::getCharWidth(RLMachine& machine, unsigned short cp932_char,
                             bool as_xmod) {
  Codepage& cp = Cp::instance(machine.getTextEncoding());
  unsigned short native_char = cp.JisDecode(cp932_char);
  unsigned short unicode_codepoint = cp.Convert(native_char);
  boost::shared_ptr<TextWindow> window = getWindow(machine, -1);
  // TODO: Can I somehow modify this to try to do proper kerning?
  int width = window->charWidth(unicode_codepoint);
  return as_xmod ? window->insertionPointX() + width : width;
}

// -----------------------------------------------------------------------

bool RlBabelDLL::lineBreakRequired(RLMachine& machine) {
  boost::shared_ptr<TextWindow> window = getWindow(machine, -1);

  int width = 0;
  std::string::size_type ptr = text_index;
  while (ptr < end_token_index) {
    unsigned short cp932_char = consumeNextCharacter(ptr);
    if (text_index < end_token_index) {
      width += getCharWidth(machine, cp932_char, false);
    } else {
      width += window->fontSizeInPixels();
    }
  }

  int max_space = window->textWindowSize().width();
  int remaining_space = max_space - window->insertionPointX();

  // If the token will fit on the current line, no line break is required.
  if (width < remaining_space) {
    return false;
  }

  // If the token will not fit on the next line either, truncate it.
  max_space -= window->currentIndentation();
  if (width >= max_space) {
    ptr = text_index;
    unsigned short cp932_char = consumeNextCharacter(ptr);
    width = getCharWidth(machine, cp932_char, false);

    // If the first character will fit on the current line, a line break is not
    // required.
    if (width < remaining_space) {
      while (ptr < end_token_index) {
        cp932_char = consumeNextCharacter(ptr);
        int cw = getCharWidth(machine, cp932_char, false);
        if (width + cw >= remaining_space)
          break;
        ptr += 1 + (cp932_char > 0xff);
        width += cw;
      }

      end_token_index = ptr;
      return false;
    }

    // If nothing CAN fit, EVER, something is seriously wrong; we
    // bail out to avoid disaster.
    if (width >= max_space)
      return false;

    // If this is not the case, however, we truncate to fit on the
    // next line, and a break is required.
    while (ptr < end_token_index) {
      cp932_char = consumeNextCharacter(ptr);
      int cw = getCharWidth(machine, cp932_char, false);
      if (width + cw >= remaining_space)
        break;
      ptr += 1 + (cp932_char > 0xff);
      width += cw;
    }

    end_token_index = ptr;
    // Don't return, but move on to the regular line-break-required handling
    // below.
  }

  // The token will fit on the next line, but not the current line.
  return true;
}

// -----------------------------------------------------------------------

unsigned short RlBabelDLL::consumeNextCharacter(std::string::size_type& index) {
  char cp932sb = cp932_text_buffer[index++];
  unsigned short cp932 = cp932sb;
  if (shiftjis_lead_byte(cp932sb))
    cp932 = (cp932 << 8) | cp932_text_buffer[index++];

  return cp932;
}

// -----------------------------------------------------------------------

IntReferenceIterator RlBabelDLL::getIvar(RLMachine& machine, int addr) {
  int bank = (addr >> 16) % 26;
  int location = addr & 0xffff;
  return IntReferenceIterator(&(machine.memory()), bank, location);
}

// -----------------------------------------------------------------------

StringReferenceIterator RlBabelDLL::getSvar(RLMachine& machine, int addr) {
  Memory* m = &(machine.memory());
  int bank = addr >> 16;
  int locaiton = addr & 0xfff;

  switch (bank) {
    case libReallive::STRS_LOCATION:
    case libReallive::STRM_LOCATION:
      return StringReferenceIterator(m, bank, addr & 0xfff);
    case libReallive::STRK_LOCATION:
      // To be bug for bug compatible with the real rlBabel so people don't
      // start targetting rlvm.
      cerr << "strK[] support is not implemented: falling back on strS[0]"
           << endl;
      break;
    default: {
      cerr << "Invalid string address; falling back on strS[0]" << endl;
      break;
    }
  }

  // Error.
  return StringReferenceIterator(m, libReallive::STRS_LOCATION, 0);
}
