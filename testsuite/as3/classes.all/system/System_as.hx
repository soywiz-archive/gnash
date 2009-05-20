// System_as3.hx:  ActionScript 3 "System" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090514 by "rob". Remove this
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
import flash.system.System;
import flash.display.MovieClip;
#else
import flash.System;
import flash.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as3 suffix, as that's the same name as the file.
class System_as {
    static function main() {
	if (Type.typeof(System) == ValueType.TObject) {
	    DejaGnu.pass("System class exists");
	} else {
	    DejaGnu.fail("System class doesn't exist");
	}

// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
// 	if (System.ime == ime) {
// 	    DejaGnu.pass("System.ime property exists");
// 	} else {
// 	    DejaGnu.fail("System.ime property doesn't exist");
// 	}
#if flash9
	if (System.totalMemory == 0) {
	    DejaGnu.pass("System.totalMemory property exists");
	} else {
	    DejaGnu.fail("System.totalMemory property doesn't exist");
	}
#end
	
#if flash6
	if (System.exactSettings == false) {
	    DejaGnu.pass("System.exactSettings property exists");
	} else {
	    DejaGnu.fail("System.exactSettings property doesn't exist");
	}
	if (System.useCodepage == false) {
	    DejaGnu.pass("System.useCodepage property exists");
	} else {
	    DejaGnu.fail("System.useCodepage property doesn't exist");
	}
#end
#if flash9
	if (System.useCodePage == false) {
	    DejaGnu.pass("System.useCodePage property exists");
	} else {
	    DejaGnu.fail("System.useCodePage property doesn't exist");
	}
#end
	
// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
	if (System.exit == null) {
	    DejaGnu.pass("System::exit() method exists");
	} else {
	    DejaGnu.fail("System::exit() method doesn't exist");
	}
	if (System.gc == null) {
	    DejaGnu.pass("System::gc() method exists");
	} else {
	    DejaGnu.fail("System::gc() method doesn't exist");
	}
	if (System.pause == null) {
	    DejaGnu.pass("System::pause() method exists");
	} else {
	    DejaGnu.fail("System::pause() method doesn't exist");
	}
	if (System.resume == null) {
	    DejaGnu.pass("System::resume() method exists");
	} else {
	    DejaGnu.fail("System::resume() method doesn't exist");
	}
#end

#if flash6
	if (System.setClipboard == null) {
	    DejaGnu.pass("System::setClipboard() method exists");
	} else {
	    DejaGnu.fail("System::setClipboard() method doesn't exist");
	}

	if (System.showSettings == null) {
	    DejaGnu.pass("System::showSettings() method exists");
	} else {
	    DejaGnu.fail("System::showSettings() method doesn't exist");
	}
#end
        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

