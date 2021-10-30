// Copyright (C) 2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#ifndef _DESKTOP_WINDOW_H_
#define _DESKTOP_WINDOW_H_

#include <functional>
#include "win-system/WinClipboard.h"
#include "gui/DibFrameBuffer.h"
#include "region/Rect.h"
#include "region/Dimension.h"
#include "ScaleManager.h"
#include "client-config-lib/ConnectionConfig.h"
#include "gui/PaintWindow.h"
#include "gui/ScrollBar.h"
#include "gui/drawing/SolidBrush.h"
#include "gui/drawing/Graphics.h"
#include "rfb/RfbKeySym.h"
#include "viewer-core/RemoteViewerCore.h"

class DesktopWindow : public PaintWindow,
                      protected RfbKeySymListener
{
public:
  DesktopWindow(LogWriter *logWriter, ConnectionConfig *conConf,
    std::function<bool(unsigned char mouseKeys, unsigned short wheelSpeed, POINT position)> onMouseCb
  );
  virtual ~DesktopWindow();

  void setClipboardData(const StringStorage *strText);
  void updateFramebuffer(const FrameBuffer *framebuffer,
                         const Rect *dstRect);
  // this function must be called if size of image was changed
  // or the number of bits per pixel
  void setNewFramebuffer(const FrameBuffer *framebuffer);

  // set scale of image, can -1 = Auto, in percent
  void setScale(int scale);
  void setScaleCenterToMouseCursor(int scale);
  // it returns the image width and height considering scale
  Rect getViewerGeometry();
  // it returns the image width and height.
  Rect getFrameBufferGeometry();
  // it return size of server frame buffer and pixelsize.
  void getServerGeometry(Rect *rect, int *pixelsize);

  void setConnected();
  void setViewerCore(RemoteViewerCore *viewerCore);

  // This function set state key "Ctrl", but not send data to server.
  void setCtrlState(const bool ctrlState);
  // This function set state key "Alt", but not send data to server.
  void setAltState(const bool altState);
  // This function return true, if key "Control" is pressed.
  bool getCtrlState() const;
  // This function return true, if key "Alt" is pressed.
  bool getAltState() const;

  // this function sends to remote viewer core
  // key what is pressed or unpressed
  void sendKey(WCHAR key, bool pressed);
  // this function sends to remote viewer core the combination
  // Ctrl + Alt + Del
  void sendCtrlAltDel();

  // Set function for m_winKeyIgnore.
  void setWinKeyIgnore(bool winKeyIgnore) { m_rfbKeySym->setWinKeyIgnore(winKeyIgnore); }

  Point getMousePos() { return m_previousMousePos; }

  void startDragging(POINT mousePos);
  void stopDragging(POINT mousePos);
  bool isDragging();
  void doDragging(POINT mousePos);

protected:
  //
  // Overrides RfbKeySymListener::onRfbKeySymEvent().
  //
  void onRfbKeySymEvent(unsigned int rfbKeySym, bool down);

  //
  // Inherited from BaseWindow.
  //
  bool onMessage(UINT message, WPARAM wParam, LPARAM lParam);
  void onPaint(DeviceContext *dc, PAINTSTRUCT *paintStruct);
  bool onCreate(LPCREATESTRUCT pcs);
  bool onDrawClipboard();
  bool onEraseBackground(HDC hdc);
  bool onDeadChar(WPARAM wParam, LPARAM lParam);
  bool onHScroll(WPARAM wParam, LPARAM lParam);
  bool onVScroll(WPARAM wParam, LPARAM lParam);
  bool onKey(WPARAM wParam, LPARAM lParam);
  bool onChar(WPARAM wParam, LPARAM lParam);
  bool onMouse(unsigned char mouseKeys, unsigned short wheelSpeed, POINT position);
  bool onSize(WPARAM wParam, LPARAM lParam);
  bool onDestroy();

  // if clamp==false the coordinate returned will not be clamped to the viewer area
  // (so it could become negative if input coordinate is above or on the left of the
  // top left corner of the remote screen). This is used for mouse wheel zoom centering
  // around mouse cursor pos (clamping might change the zooming center during the wheel zooming)
  POINTS getViewerCoord(long xPos, long yPos, bool clamp=true);

  void calculateWndSize(bool isChanged);
  void applyScrollbarChanges(bool isChanged, bool isVert, bool isHorz, int wndWidth, int wndHeight);

  // This function check pointer to viewer core and send event.
  // If into viewer core throwing exception Exception, then it catching
  // in this function and logged.
  void sendKeyboardEvent(bool downFlag, UINT32 key);
  void sendPointerEvent(UINT8 buttonMask, const Point *position);
  void sendCutTextEvent(const StringStorage *cutText);

  LogWriter *m_logWriter;

  // This variable is true after call CoreEventsAdapter::onConnected().
  bool m_isConnected;

  // keyboard support
  std::auto_ptr<RfbKeySym> m_rfbKeySym;

  // This variable contained previously state of mouse-button and position of cursor.
  unsigned char m_previousMouseState;
  Point m_previousMousePos;

  // our mouse cursor position in remote screen pixels, relative to top left corner of remote screen,
  // not clamped into the remote screen rectangle (will be negative if on the left or above the remote screen),
  // used as zoom center point
  Point m_previousMousePosNoClamp;

  // scroll bars: vertical and horizontal
  ScrollBar m_sbar;

  // This flag is true if size of window is changed (scroll must be updated).
  bool m_winResize;
  bool m_showVert;
  bool m_showHorz;

  // for scaling
  ScaleManager m_scManager;
  Rect m_clientArea;
  int m_fbWidth;
  int m_fbHeight;
  SolidBrush m_brush;

  // frame buffer
  LocalMutex m_bufferLock;
  DibFrameBuffer m_framebuffer;
  // This variable save server dimension.
  // Dimension of m_framebuffer can be large m_serverDimension.
  Dimension m_serverDimension;

  // clipboard
  WinClipboard m_clipboard;
  StringStorage m_strClipboard;

  bool m_ctrlDown;
  bool m_altDown;

  RemoteViewerCore *m_viewerCore;
  ConnectionConfig *m_conConf;
  bool m_isBackgroundDirty;

  bool m_isDragging;  // are we currently panning the view with the mouse?
  Point m_dragStartViewStartPos; // scaleMgr.StartPos when started dragging
  Point m_dragStartMousePos; // mouse desktop screen pos (local, not remote) when started dragging


private:
  std::function<bool(unsigned char mouseKeys, unsigned short wheelSpeed, POINT position)> m_onMouseCb;

private:
  void doDraw(DeviceContext *dc);
  void scrollProcessing(int fbWidth, int fbHeight);
  void drawBackground(DeviceContext *dc, const RECT *rcMain, const RECT *rcImage);
  void drawImage(const RECT *src, const RECT *dst);
  void repaint(const Rect *repaintRect);
  void calcClientArea();
};

#endif
