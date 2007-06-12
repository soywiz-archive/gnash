// text.cpp:  Implementation of ActionScript text tags, for Gnash.
// 
//   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/* $Id: text.cpp,v 1.30 2007/06/13 00:17:45 strk Exp $ */

// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2003

#include "utf8.h"
#include "utility.h"
#include "impl.h"
#include "shape_character_def.h"
#include "stream.h"
#include "log.h"
#include "font.h"
#include "fontlib.h"
#include "render.h"
#include "textformat.h"
#include "text.h"
#include "movie_definition.h"

// Define the following macro to get debugging messages
// for text rendering
#define GNASH_DEBUG_TEXT_RENDERING 1

namespace gnash {

	void text_style::resolve_font(movie_definition* root_def) const
	{
		if (m_font == NULL)
		{
			assert(m_font_id >= 0);

			m_font = root_def->get_font(m_font_id);
			if (m_font == NULL)
			{
				IF_VERBOSE_MALFORMED_SWF(
	log_error(_("text style references unknown font (id = %d)"),
		m_font_id);
				);
			}
		}
	}

	void text_glyph_record::read(stream* in, int glyph_count,
			int glyph_bits, int advance_bits)
	{
		m_glyphs.resize(glyph_count);
		for (int i = 0; i < glyph_count; i++)
		{
			m_glyphs[i].m_glyph_index = in->read_uint(glyph_bits);
			m_glyphs[i].m_glyph_advance = (float) in->read_sint(advance_bits);
		}
	}

	// Render the given glyph records.
	void	display_glyph_records(
		const matrix& this_mat,
		character* inst,
		const std::vector<text_glyph_record>& records,
		// root_def was used to resove fonts, now done at parse time
		movie_definition* /*root_def*/)
	{
//		GNASH_REPORT_FUNCTION;
		
		static std::vector<fill_style>	s_dummy_style;	// used to pass a color on to shape_character::display()
		static std::vector<line_style>	s_dummy_line_style;
		s_dummy_style.resize(1);

		matrix	mat = inst->get_world_matrix();
		mat.concatenate(this_mat);

		cxform	cx = inst->get_world_cxform();
		float	pixel_scale = inst->get_pixel_scale();

//		display_info	sub_di = di;
//		sub_di.m_matrix.concatenate(mat);

//		matrix	base_matrix = sub_di.m_matrix;
		matrix	base_matrix = mat;
		float	base_matrix_max_scale = base_matrix.get_max_scale();

		float	scale = 1.0f;
		float	x = 0.0f;
		float	y = 0.0f;

		for (unsigned int i = 0; i < records.size(); i++)
		{
			// Draw the characters within the current record; i.e. consecutive
			// chars that share a particular style.
			const text_glyph_record&	rec = records[i];

			//rec.m_style.resolve_font(root_def);

			const font*	fnt = rec.m_style.m_font;
			if (fnt == NULL)
			{
				continue;
			}

			scale = rec.m_style.m_text_height / 1024.0f;	// the EM square is 1024 x 1024
			float	text_screen_height = base_matrix_max_scale
				* scale
				* 1024.0f
				/ 20.0f
				* pixel_scale;

			int	nominal_glyph_height = fnt->get_texture_glyph_nominal_size();
			float	max_glyph_height = fontlib::get_texture_glyph_max_height(fnt);
#ifdef GNASH_ALWAYS_USE_TEXTURES_FOR_TEXT_WHEN_POSSIBLE
			bool	use_glyph_textures = gnash::render::allow_glyph_textures();
#else
			bool	use_glyph_textures =
				(text_screen_height <= max_glyph_height * 1.0f) &&
        (gnash::render::allow_glyph_textures());    
#endif

			if (rec.m_style.m_has_x_offset)
			{
				x = rec.m_style.m_x_offset;
			}
			if (rec.m_style.m_has_y_offset)
			{
				y = rec.m_style.m_y_offset;
			}

			s_dummy_style[0].set_color(rec.m_style.m_color);

			rgba	transformed_color = cx.transform(rec.m_style.m_color);

			for (unsigned int j = 0; j < rec.m_glyphs.size(); j++)
			{
				int	index = rec.m_glyphs[j].m_glyph_index;
					
				mat = base_matrix;
				mat.concatenate_translation(x, y);
				mat.concatenate_scale(scale);

				if (index == -1)
				{
#ifdef GNASH_DEBUG_TEXT_RENDERING
log_error(_("invalid glyph, render as an empty box"));
#endif
					render::set_matrix(mat);

					// The EM square is 1024x1024, but usually isn't filled up.
					// We'll use about half the width, and around 3/4 the height.
					// Values adjusted by eye.
					// The Y baseline is at 0; negative Y is up.
					static const int16_t	s_empty_char_box[5 * 2] =
					{
						 32,   32,
						480,   32,
						480, -656,
						 32, -656,
						 32,   32
					};
					render::draw_line_strip(s_empty_char_box, 5, transformed_color);  
				}
				else
				{
					const texture_glyph&	tg = fnt->get_texture_glyph(index);
					shape_character_def*	glyph = fnt->get_glyph(index);

					if (tg.is_renderable()
					    && (use_glyph_textures || glyph == NULL))
					{
#ifdef GNASH_DEBUG_TEXT_RENDERING
log_msg(_("render textured glyph (fontlib::draw_glyph)"));
#endif
						fontlib::draw_glyph(mat, tg, transformed_color, nominal_glyph_height);
					}
					else
					{

						// Draw the character using the filled outline.
						if (glyph)
						{
#ifdef GNASH_DEBUG_TEXT_RENDERING
log_msg(_("render shape glyph using filled outline (render::draw_glyph)"));
#endif

							gnash::render::draw_glyph(glyph, mat, transformed_color, pixel_scale);
							
						}
					}
				}
				x += rec.m_glyphs[j].m_glyph_advance;
			}
		}
	}

}	// end namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
