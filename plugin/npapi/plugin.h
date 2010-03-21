// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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


#ifndef GNASH_PLUGIN_H
#define GNASH_PLUGIN_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifndef HAVE_FUNCTION
# ifndef HAVE_func
#  define dummystr(x) # x
#  define dummyestr(x) dummystr(x)
#  define __FUNCTION__ __FILE__":"dummyestr(__LINE__)
# else
#  define __FUNCTION__ __func__	
# endif
#endif

#ifndef HAVE_PRETTY_FUNCTION
	#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

/* Xlib/Xt stuff */
#include <X11/Xlib.h>
//#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <glib.h>
#include <string>
#include <map>
#include <vector>

#include "pluginbase.h"
#include "prlock.h"
#include "prcvar.h"
#include "prthread.h"

extern NPBool      plugInitialized;
extern PRLock      *playerMutex;
extern PRCondVar   *playerCond;

class nsPluginInstance : public nsPluginInstanceBase
{
public:
    nsPluginInstance(nsPluginCreateData* );
    virtual ~nsPluginInstance();

    // We are required to implement these three methods.
    NPBool init(NPWindow *aWindow);
    NPBool isInitialized() { return plugInitialized; }
    void shut();

    NPError GetValue(NPPVariable variable, void *value);
    NPError SetWindow(NPWindow *aWindow);

    NPError NewStream(NPMIMEType type, NPStream *stream, NPBool seekable,
                      uint16_t *stype);
    NPError DestroyStream(NPStream * stream, NPError reason);

    int32_t WriteReady(NPStream *stream);
    int32_t Write(NPStream *stream, int32_t offset, int32_t len, void *buffer);

private:
    void startProc();
    std::vector<std::string> getCmdLine(int hostfd, int controlfd);

    static bool handlePlayerRequestsWrapper(GIOChannel* iochan, GIOCondition cond, nsPluginInstance* plugin);

    bool handlePlayerRequests(GIOChannel* iochan, GIOCondition cond);

    /// Process a null-terminated request line
    //
    /// @param buf
    ///	  The single request.
    ///   Caller is responsible for memory management, but give us
    ///   permission to modify the string.
    ///
    /// @param len
    ///	  Lenght of buffer.
    ///
    /// @return true if the request was processed, false otherwise (bogus request..)
    ///
    bool processPlayerRequest(gchar* buf, gsize len);

    // EMBED or OBJECT attributes / parameters
    // @@ this should likely replace the _options element below
    std::map<std::string, std::string> _params;

    NPP                                _instance;
    Window                             _window;
    std::string                        _swf_url;
    std::string                        _swf_file;
    unsigned int                       _width;
    unsigned int                       _height;
    std::map<std::string, std::string> _options;
    int                                _streamfd;
    int                                _ichanWatchId;
    int                                _controlfd;
    pid_t                              _childpid;
    int                                _filefd;

    /// Name of the plugin instance element in the dom 
    std::string                        _name;

    const char* getCurrentPageURL() const;
};

// end of __PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// End: