/* Copyright 2001 Kjetil S. Matheussen

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */


#include "Python.h"

#include "../common/nsmtracker.h"
#include "../common/vector_proc.h"
#include "../common/undo.h"
#include "../common/undo_blocks_proc.h"
#include "../common/undo_tracks_proc.h"

#include "api_common_proc.h"



void setMaxUndos(int windownum){
  struct Tracker_Windows *window=getWindowFromNum(windownum);if(window==NULL) return;
  SetMaxUndos(window);
}

const_char *getUndoHistory(void){
  const char *ret = "\n";
  vector_t history = Undo_get_history();
  VECTOR_FOR_EACH(const char *line, &history){
    ret = talloc_format("%s%s\n", ret, line);
  }END_VECTOR_FOR_EACH;

  return ret;
}

void redo(void){
  Redo();
}

void undo(void){
  Undo();
}

void resetUndo(void){
  ResetUndo();
}

void startIgnoringUndo(void){
  Undo_start_ignoring_undo_operations();
}

void stopIgnoringUndo(void){
  Undo_stop_ignoring_undo_operations();
}

void cancelLastUndo(void){
  UNDO_CANCEL_LAST_UNDO();
}

void addUndoBlock(void){
  struct Tracker_Windows *window=getWindowFromNum(-1);if(window==NULL) return;
  ADD_UNDO(Block_CurrPos(window));
}

void addUndoTrack(void){
  struct Tracker_Windows *window=getWindowFromNum(-1);if(window==NULL) return;
  ADD_UNDO(Track_CurrPos(window->wblock->l.num, window->wblock->wtrack->l.num));
}

void openUndo(void){
  UNDO_OPEN_REC();
}

void closeUndo(void){
  UNDO_CLOSE();
}
