// gnash-view.cpp: Gtk view widget for gnash
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

#include "gnash-view.h"
#include "gtk_canvas.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkbutton.h>

#include "log.h"

#include "VM.h"
#include "movie_definition.h"
#include "movie_root.h" // for Abstract callbacks
#include "sound_handler.h"
#include "MediaHandler.h"
#include "RunInfo.h" // for passing handlers and other data to the core.
#include "VirtualClock.h"
#include "SystemClock.h"
#include "smart_ptr.h"

#ifdef USE_FFMPEG
# include "MediaHandlerFfmpeg.h"
#elif defined(USE_GST)
# include "MediaHandlerGst.h"
#endif

struct _GnashView {
	GtkBin base_instance;

    GnashCanvas *canvas;

	std::auto_ptr<gnash::media::MediaHandler> media_handler;
    boost::shared_ptr<gnash::sound::sound_handler> sound_handler;

    /// Handlers (for sound etc) for a libcore run.
    //
    /// This must be kept alive for the entire lifetime of the movie_root
    /// (currently: of the Gui).
    std::auto_ptr<gnash::RunInfo> run_info;

    std::auto_ptr<gnash::movie_definition> movie_definition;
    std::auto_ptr<gnash::movie_root> stage;
    std::auto_ptr<gnash::SystemClock> system_clock;
    std::auto_ptr<gnash::InterruptableVirtualClock> virtual_clock;
    guint advance_timer;
};

G_DEFINE_TYPE(GnashView, gnash_view, GTK_TYPE_BIN)

static GObjectClass *parent_class = NULL;

static void gnash_view_class_init(GnashViewClass *gnash_view_class);
static void gnash_view_init(GnashView *view);
static void gnash_view_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void gnash_view_size_request (GtkWidget *widget, GtkRequisition *requisition);
static gboolean gnash_view_key_press_event(GtkWidget *widget, GdkEventKey *event);
static gnash::key::code gdk_to_gnash_key(guint key);
static int gdk_to_gnash_modifier(int state);
static gboolean gnash_view_advance_movie(GnashView *view);
static void gnash_view_display(GnashView *view);

GtkWidget *
gnash_view_new (void)
{
    return GTK_WIDGET(g_object_new (GNASH_TYPE_VIEW, NULL));
}

static void
gnash_view_class_init(GnashViewClass *gnash_view_class)
{
    GNASH_REPORT_FUNCTION;
    parent_class = (GObjectClass *) g_type_class_peek_parent(gnash_view_class);

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (gnash_view_class);
    widget_class->size_allocate = gnash_view_size_allocate;
    widget_class->size_request = gnash_view_size_request;
    widget_class->key_press_event = gnash_view_key_press_event;
}

static void
gnash_view_init(GnashView *view)
{
    GNASH_REPORT_FUNCTION;

    gnash::gnashInit();
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity();
    dbglogfile.setVerbosity();
    dbglogfile.setVerbosity();

    // Init media
#ifdef USE_FFMPEG
    view->media_handler.reset( new gnash::media::ffmpeg::MediaHandlerFfmpeg() );
    gnash::media::MediaHandler::set(view->media_handler);
#elif defined(USE_GST)
    view->media_handler.reset( new gnash::media::gst::MediaHandlerGst() );
    gnash::media::MediaHandler::set(view->media_handler);
#else
    log_error(_("No media support compiled in"));
#endif    

    // Init sound
#ifdef SOUND_SDL
    try {
        view->sound_handler.reset(gnash::sound::create_sound_handler_sdl(""));
    } catch (gnash::SoundException& ex) {
        gnash::log_error(_("Could not create sound handler: %s."
                           " Will continue w/out sound."), ex.what());
    }
#elif defined(SOUND_GST)
    view->sound_handler.reset(media::create_sound_handler_gst());
#else
    gnash::log_error(_("Sound requested but no sound support compiled in"));
#endif

    view->canvas = GNASH_CANVAS(gnash_canvas_new());
    gnash_canvas_setup(view->canvas, 0, NULL);
    gtk_container_add (GTK_CONTAINER (view), GTK_WIDGET(view->canvas));
    gtk_widget_show (GTK_WIDGET(view->canvas));
}

static void
gnash_view_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    GnashView *view = GNASH_VIEW(widget);
    widget->allocation = *allocation;
    gtk_widget_size_allocate (GTK_BIN(widget)->child, allocation);

    if( view->stage.get() != NULL) {
    	view->stage->set_display_viewport(0, 0, allocation->width, allocation->height);

        gnash::render_handler *renderer = gnash_canvas_get_renderer(view->canvas);
        float xscale = allocation->width / view->movie_definition->get_width_pixels();
        float yscale = allocation->height / view->movie_definition->get_height_pixels();
		renderer->set_scale(xscale, yscale);
    }
}

static void
gnash_view_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
    GnashView *view = GNASH_VIEW(widget);
    if( view->movie_definition.get() == NULL ) {
        requisition->width = 0;
        requisition->height = 0;
    } else {
        requisition->width = view->movie_definition->get_width_pixels();
        requisition->height = view->movie_definition->get_height_pixels();
    }
}

static gboolean
gnash_view_key_press_event(GtkWidget *widget, GdkEventKey *event)
{
    if (GNASH_VIEW(widget)->stage.get() == NULL)
        return FALSE;

    gnash::key::code c = gdk_to_gnash_key(event->keyval);
    //int mod = gdk_to_gnash_modifier(event->state);
    
    if (c != gnash::key::INVALID) {
        if( GNASH_VIEW(widget)->stage->notify_key_event(c, true) )
            gnash_view_display(GNASH_VIEW(widget));
        return TRUE;
    }
    
    return FALSE;
}

void
gnash_view_load_movie(GnashView *view, gchar *path)
{
    GNASH_REPORT_FUNCTION;

    // The RunInfo should be populated before parsing.
    view->run_info.reset(new gnash::RunInfo(path));
    view->run_info->setSoundHandler(view->sound_handler);

    std::auto_ptr<gnash::NamingPolicy> np(new gnash::IncrementalRename(gnash::URL(path)));
    boost::shared_ptr<gnash::StreamProvider> sp(new gnash::StreamProvider(np));
    view->run_info->setStreamProvider(sp);

    // Load the actual movie.
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();

    const std::string& str_path(path);
    size_t lastSlash = str_path.find_last_of('/');
    std::string dir = str_path.substr(0, lastSlash+1);
    rcfile.addLocalSandboxPath(dir);
    gnash::log_debug(_("%s appended to local sandboxes"), dir.c_str());

    rcfile.addLocalSandboxPath(str_path);
    gnash::log_debug(_("%s appended to local sandboxes"), path);

    view->movie_definition.reset(gnash::create_library_movie(gnash::URL(path),
            *view->run_info, path, false));

    // NOTE: it's important that _systemClock is constructed
    //       before and destroyed after _virtualClock !
    view->system_clock.reset(new gnash::SystemClock());
    view->virtual_clock.reset(new gnash::InterruptableVirtualClock(*view->system_clock));
    view->stage.reset(new gnash::movie_root(*view->movie_definition, *view->virtual_clock, *view->run_info));
    view->movie_definition->completeLoad();

    view->advance_timer = g_timeout_add_full(G_PRIORITY_LOW, 10,
            (GSourceFunc)gnash_view_advance_movie, view, NULL);

    gtk_widget_queue_resize (GTK_WIDGET(view));
}

void
gnash_view_start(GnashView *view)
{
    GNASH_REPORT_FUNCTION;

    std::auto_ptr<gnash::Movie> mr ( view->movie_definition->createMovie() );

    view->stage->setRootMovie( mr.release() ); // will construct the instance

    bool background = true; // ??
    view->stage->set_background_alpha(background ? 1.0f : 0.05f);

    // @todo since we registered the sound handler, shouldn't we know
    //       already what it is ?!
    gnash::sound::sound_handler* s = view->stage->runInfo().soundHandler();
    if ( s ) s->unpause();
    
    gnash::log_debug("Starting virtual clock");
    view->virtual_clock->resume();

    gnash_view_advance_movie(view);
}

static gboolean
gnash_view_advance_movie(GnashView *view)
{
    view->stage->advance();

    gnash_view_display(view);

    return TRUE;
}

static void
gnash_view_display(GnashView *view)
{
    gnash::InvalidatedRanges changed_ranges;
    changed_ranges.setWorld();

    gnash::render_handler *renderer = gnash_canvas_get_renderer(view->canvas);
    renderer->set_invalidated_regions(changed_ranges);
    gdk_window_invalidate_rect(GTK_WIDGET(view->canvas)->window, NULL, false);

    gnash_canvas_before_rendering(view->canvas);
	view->stage->display();

    gdk_window_process_updates(GTK_WIDGET(view->canvas)->window, false);
}

static gnash::key::code
gdk_to_gnash_key(guint key)
{
    gnash::key::code c(gnash::key::INVALID);

    // ascii 32-126 in one range:    
    if (key >= GDK_space && key <= GDK_asciitilde) {
        c = (gnash::key::code) ((key - GDK_space) + gnash::key::SPACE);
    }

    // Function keys:
    else if (key >= GDK_F1 && key <= GDK_F15)	{
        c = (gnash::key::code) ((key - GDK_F1) + gnash::key::F1);
    }

    // Keypad:
    else if (key >= GDK_KP_0 && key <= GDK_KP_9) {
        c = (gnash::key::code) ((key - GDK_KP_0) + gnash::key::KP_0);
    }

    // Extended ascii:
    else if (key >= GDK_nobreakspace && key <= GDK_ydiaeresis) {
        c = (gnash::key::code) ((key - GDK_nobreakspace) + 
                gnash::key::NOBREAKSPACE);
    }

    // non-character keys don't correlate, so use a look-up table.
    else {
        struct {
            guint             gdk;
            gnash::key::code  gs;
        } table[] = {
            { GDK_BackSpace, gnash::key::BACKSPACE },
            { GDK_Tab, gnash::key::TAB },
            { GDK_Clear, gnash::key::CLEAR },
            { GDK_Return, gnash::key::ENTER },
            
            { GDK_Shift_L, gnash::key::SHIFT },
            { GDK_Shift_R, gnash::key::SHIFT },
            { GDK_Control_L, gnash::key::CONTROL },
            { GDK_Control_R, gnash::key::CONTROL },
            { GDK_Alt_L, gnash::key::ALT },
            { GDK_Alt_R, gnash::key::ALT },
            { GDK_Caps_Lock, gnash::key::CAPSLOCK },
            
            { GDK_Escape, gnash::key::ESCAPE },
            
            { GDK_Page_Down, gnash::key::PGDN },
            { GDK_Page_Up, gnash::key::PGUP },
            { GDK_Home, gnash::key::HOME },
            { GDK_End, gnash::key::END },
            { GDK_Left, gnash::key::LEFT },
            { GDK_Up, gnash::key::UP },
            { GDK_Right, gnash::key::RIGHT },
            { GDK_Down, gnash::key::DOWN },
            { GDK_Insert, gnash::key::INSERT },
            { GDK_Delete, gnash::key::DELETEKEY },
            
            { GDK_Help, gnash::key::HELP },
            { GDK_Num_Lock, gnash::key::NUM_LOCK },

            { GDK_VoidSymbol, gnash::key::INVALID }
        };
        
        for (int i = 0; table[i].gdk != GDK_VoidSymbol; i++) {
            if (key == table[i].gdk) {
                c = table[i].gs;
                break;
            }
        }
    }
    
    return c;
}

static int
gdk_to_gnash_modifier(int state)
{
    int modifier = gnash::key::GNASH_MOD_NONE;

    if (state & GDK_SHIFT_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_SHIFT;
    }
    if (state & GDK_CONTROL_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_CONTROL;
    }
    if (state & GDK_MOD1_MASK) {
      modifier = modifier | gnash::key::GNASH_MOD_ALT;
    }

    return modifier;
}
