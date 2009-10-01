// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


#ifndef GNASH_RECT_H
#define GNASH_RECT_H

#include "dsodefs.h"
#include "Range2d.h"
#include "Point2d.h"

#include <cassert> // for inlines
#include <iostream> // for output operator
#include <boost/cstdint.hpp>

// Forward decl
namespace gnash {
    class SWFMatrix;
    class SWFStream;
}

namespace gnash {

/// \brief
/// Rectangle class, see swf defined rectangle record.
///
class rect
{

public:
    /// Read a bit-packed rectangle from an SWF stream
    //
    /// Format of the bit-packed rectangle is:
    ///
    /// bits  | name  | description
    /// ------+-------+-------------------------
    ///   5   | nbits | number of bits used in subsequent values
    /// nbits | xmin  | minimum X value
    /// nbits | xmax  | maximum X value
    /// nbits | ymin  | minimum Y value
    /// nbits | ymax  | maximum Y value
    ///
    /// If max values are less then min values the SWF is malformed;
    /// in this case this method will raise an swf_error and set the
    /// rectangle to the NULL rectangle. See is_null().
    /// 
    ///
    void    read(SWFStream& in);

    static const boost::int32_t rectNull = 0x80000000;
    static const boost::int32_t rectMax = 0x7fffffff;
    
    /// Ouput operator
    friend std::ostream& operator<< (std::ostream& os, const rect& rect);

    /// Construct a NULL rectangle
    rect()
        :
       _xMin(rectNull),
       _yMin(rectNull),
       _xMax(rectNull),
       _yMax(rectNull)
    {}

    /// Construct a rectangle with given coordinates
    rect(int xmin, int ymin, int xmax, int ymax)
        :
        _xMin(xmin),
        _yMin(ymin),
        _xMax(xmax),
        _yMax(ymax)
    {
    }

    /// returns true if this is a NULL rectangle
    bool is_null() const
    {
        return (_xMin == rectNull && _xMax == rectNull);
    }

    /// set the rectangle to the NULL value
    void set_null()
    {
        _xMin = _yMin = _xMax = _yMax = rectNull;
    }

    /// TODO: deprecate this 'world' concept.
    bool is_world() const
    {
        return _xMin == (- rectMax >> 9) 
            && _yMin == (- rectMax >> 9) 
            && _xMax == (rectMax >> 9)
            && _yMax == (rectMax >> 9);
    }
	
    /// set the rectangle to the WORLD value
    void set_world()
    {
        _xMin = _yMin = - rectMax >> 9;
        _xMax = _yMax = rectMax >> 9;
    }

    /// Return width of this rectangle in TWIPS
    boost::int32_t width() const
    {
        return _xMax - _xMin;
    }

    /// Return height of this rectangle in TWIPS
    boost::int32_t height() const
    {
        return _yMax - _yMin;
    }

    /// Get the x coordinate of the left-up corner.
    boost::int32_t get_x_min() const
    {
        assert(!is_null());
        return _xMin;
    }

    /// Get the x coordinate of the right-down corner.
    boost::int32_t get_x_max() const
    {
        assert(!is_null());
        return _xMax;
    }

    /// Get the y coordinate of the left-up corner.
    boost::int32_t get_y_min() const
    {
        assert(!is_null());
        return _yMin;
    }

    /// Get the y coordinate of the right-down corner.
    boost::int32_t get_y_max() const
    {
        assert(!is_null());
        return _yMax;
    }
    
    /// Return true if the given point is inside this rect.
    bool point_test(boost::int32_t x, boost::int32_t y) const
    {
        if (is_null()) return false;
        
        if (x < _xMin || x > _xMax || y < _yMin || y > _yMax) {
            return false;
        } 
        return true;
    }

    /// Set ourself to bound the given point
    void set_to_point(boost::int32_t x, boost::int32_t y)
    {
        _xMin = _xMax = x;
        _yMin = _yMax = y;
    }

    
    void set_to_rect(boost::int32_t x1, boost::int32_t y1, boost::int32_t x2,
            boost::int32_t y2)
    {
        _xMin = x1;
        _yMin = y1;
        _xMax = x2;
        _yMax = y2;
    }
    
    /// Expand this rectangle to enclose the given point.
    void expand_to_point(boost::int32_t x, boost::int32_t y)
    {
        if (is_null()) {
            set_to_point(x, y);
        } else {
            expand_to(x, y);
        }
    }

    /// Set ourself to bound a rectangle that has been transformed by m.  
    /// This is an axial bound of an oriented (and/or
    /// sheared, scaled, etc) box.
    void enclose_transformed_rect(const SWFMatrix& m, const rect& r);
    
    /// Expand this rectangle to enclose the given circle.
    void expand_to_circle(boost::int32_t x, boost::int32_t y, boost::int32_t radius)
    {
        // I know it's easy to make code work for minus radius.
        // would do that untill I see the requirement for a SWF RECTANGLE.
        assert(radius >= 0); 
        if (is_null()) {
            _xMin = x - radius;
            _yMin = y - radius;
            _xMax = x + radius;
            _yMax = y + radius;
        } else {
            _xMin = std::min(_xMin, x - radius);
            _yMin = std::min(_yMin, y - radius);
            _xMax = std::max(_xMax, x + radius);
            _yMax = std::max(_yMax, y + radius);
        }
    }
      
    /// Same as enclose_transformed_rect but expanding the current rect instead
    /// of replacing it.
    DSOEXPORT void expand_to_transformed_rect(const SWFMatrix& m, const rect& r);
    
    /// Makes union of the given and the current rect
    DSOEXPORT void expand_to_rect(const rect& r);
    
    void set_lerp(const rect& a, const rect& b, float t);

    /// \brief
    /// Make sure that the given point falls in this rectangle, 
    /// modifying it's coordinates if needed.
    void clamp(point& p) const;

    /// Construct and return a Range2d object.
    // TODO: deprecate this.
    geometry::Range2d<float> getRange() const
    {
        if (is_null())
        {
           // Range2d has a differnt idea about what is a null rect.
           return geometry::Range2d<float>(geometry::nullRange); //null range
        }
        else if( is_world() ) 
        {
            return geometry::Range2d<float>(geometry::worldRange); //world range
        }
        else
        {
            return geometry::Range2d<float>(_xMin, _yMin, _xMax, _yMax);
        }
    }

    /// Return a string representation for this rectangle
    std::string toString() const;

private:
	
    // make ourself to enclose the given point.
    void expand_to(boost::int32_t x, boost::int32_t y)
    {
        _xMin = std::min(_xMin, x);
        _yMin = std::min(_yMin, y);
        _xMax = std::max(_xMax, x);
        _yMax = std::max(_yMax, y);
    }

private:

    boost::int32_t _xMin; // TWIPS
    boost::int32_t _yMin; // TWIPS
    boost::int32_t _xMax; // TWIPS
    boost::int32_t _yMax; // TWIPS
};


inline std::ostream&
operator<< (std::ostream& os, const rect& r)
{
    if( !r.is_null() ) {
        os << "RECT(" 
           << r.get_x_min() << "," 
           << r.get_y_min() << "," 
           << r.get_x_max() << "," 
           << r.get_y_max() << ")";
    }else {
        os << "NULL RECT!";
    }

    return os;
}

}   // namespace gnash

#endif // GNASH_RECT_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
