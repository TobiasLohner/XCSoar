/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_FORM_CONTROL_HPP
#define XCSOAR_FORM_CONTROL_HPP

#include "Screen/Color.hpp"
#include "Screen/Brush.hpp"
#include "Screen/ContainerWindow.hpp"

#include <tchar.h>

class ContainerControl;

/**
 * The WindowControl class is the base class for every other control
 * including the forms/windows itself, using the ContainerControl.
 */
class WindowControl : public ContainerWindow {
public:
  typedef void (*OnHelpCallback_t)(WindowControl *Sender);

public:
  /**
   * Constructor of the WindowControl class
   * @param Owner
   * @param Parent
   * @param X x-Coordinate of the Control
   * @param Y y-Coordinate of the Control
   * @param Width Width of the Control
   * @param Height Height of the Control
   * @param Visible Visibility of the Control
   */
  WindowControl(ContainerControl *Owner, ContainerWindow *Parent,
                int X, int Y, int Width, int Height,
                const WindowStyle style);

  /** Destructor */
  virtual ~WindowControl(void);

  /**
   * The on_setfocus event is called when the control gets focused
   * (derived from Window)
   */
  virtual bool on_setfocus();
  /**
   * The on_killfocus event is called when the control loses the focus
   * (derived from Window)
   */
  virtual bool on_killfocus();

  virtual bool on_unhandled_key(unsigned key_code);

  /**
   * The on_key_down event is called when a key is pressed while the
   * control is focused
   * (derived from Window)
   */
  virtual bool on_key_down(unsigned key_code);

  /**
   * The on_key_up event is called when a key is released while the
   * control is focused
   * (derived from Window)
   */
  virtual bool on_key_up(unsigned key_code);

  /**
   * Opens up a help dialog if a help text exists, or otherwise calls the
   * function defined by mOnHelpCallback
   * @return
   */
  virtual int OnHelp();

  /**
   * Sets the function that should be called when the help button is pressed
   * @param Function Pointer to the function to be called
   */
  void SetOnHelpCallback(OnHelpCallback_t Function) {
    mOnHelpCallback = Function;
  }

  /**
   * Returns the Control's font
   * @return The Control's font
   */
  const Font *GetFont(void) { return mhFont; }
  /**
   * Sets the Control's font
   * @param font The new font
   * @return The old font
   */
  virtual const Font *SetFont(const Font &font);
  /**
   * Sets the Control's font
   * @param font The new font
   * @return The old font
   */
  const Font *SetFont(const Font *font) { return SetFont(*font); }

  /**
   * Sets the Control's foreground color
   * @param Value The new foreground color
   * @return The old foreground color
   */
  virtual Color SetForeColor(Color Value);
  /**
   * Return the Control's foreground color
   * @return The Control's foreground color
   */
  Color GetForeColor(void) { return mColorFore; }

  /**
   * Sets the Control's background color
   * @param Value The new background color
   * @return The old background color
   */
  virtual Color SetBackColor(Color Value);
  /**
   * Return the Control's background color
   * @return The Control's background color
   */
  Color GetBackColor(void) { return mColorBack; }

  /**
   * Returns the brush for painting the background
   * @return The brush for painting the background
   */
  Brush &GetBackBrush(void) {
    return mhBrushBk;
  }

  /**
   * Returns the Caption/Text of the Control
   * @return The Caption/Text of the Control
   */
  TCHAR *GetCaption(void) { return mCaption; }
  /**
   * Sets the Caption/Text of the Control
   * @param Value The new Caption/Text of the Control
   */
  virtual void SetCaption(const TCHAR *Value);

  /**
   * Sets the Helptext of the Control
   * @param Value The new Helptext of the Control
   */
  void SetHelpText(const TCHAR *Value);

protected:
  /** Caption/Text of the Control */
  TCHAR mCaption[254];

  /** Paints the "Selector" */
  void PaintSelector(Canvas &canvas, const RECT rc);

private:
  /** Parent ContainerControl */
  ContainerControl *mOwner;

  /** Background color */
  Color mColorBack;
  /** Foreground color */
  Color mColorFore;

  /** Brush for painting the background */
  Brush mhBrushBk;

  /** Font of the Control */
  const Font *mhFont;

  /** Helptext of the Control */
  TCHAR *mHelpText;

  /**
   * The callback-function that should be called when the help button is
   * pressed while the control has focus
   * @see SetOnHelpCallback()
   */
  OnHelpCallback_t mOnHelpCallback;
};

#endif
