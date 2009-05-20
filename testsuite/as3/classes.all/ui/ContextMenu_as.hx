// ContextMenu_as3.hx:  ActionScript 3 "ContextMenu" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090515 by "rob". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.ui.ContextMenu;
import flash.display.MovieClip;
#else
import flash.ContextMenu;
import flash.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as3 suffix, as that's the same name as the file.
class ContextMenu_as3 {
    static function main() {
        var x1:ContextMenu = new ContextMenu();

        // Make sure we actually get a valid class        
        if (x1 != null) {
            DejaGnu.pass("ContextMenu class exists");
        } else {
            DejaGnu.fail("ContextMenu class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.builtInItems == builtInItems) {
	    DejaGnu.pass("ContextMenu.builtInItems property exists");
	} else {
	    DejaGnu.fail("ContextMenu.builtInItems property doesn't exist");
	}
	if (x1.customItems == 0) {
	    DejaGnu.pass("ContextMenu.customItems property exists");
	} else {
	    DejaGnu.fail("ContextMenu.customItems property doesn't exist");
	}
	if (x1.items == 0) {
	    DejaGnu.pass("ContextMenu.items property exists");
	} else {
	    DejaGnu.fail("ContextMenu.items property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.ContextMenu == ContextMenu) {
	    DejaGnu.pass("ContextMenu::ContextMenu() method exists");
	} else {
	    DejaGnu.fail("ContextMenu::ContextMenu() method doesn't exist");
	}
	if (x1.display == null) {
	    DejaGnu.pass("ContextMenu::display() method exists");
	} else {
	    DejaGnu.fail("ContextMenu::display() method doesn't exist");
	}
	if (x1.hideBuiltInItems == null) {
	    DejaGnu.pass("ContextMenu::hideBuiltInItems() method exists");
	} else {
	    DejaGnu.fail("ContextMenu::hideBuiltInItems() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

