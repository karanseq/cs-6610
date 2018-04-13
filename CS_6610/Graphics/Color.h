/*
    This struct represents color in the RGBA format.
*/

#ifndef ENGINE_COLOR_H_
#define ENGINE_COLOR_H_

// Struct Declaration
//===================

namespace engine {
namespace graphics {

struct Color
{
    // Data
    //=====

    float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

    // Interface
    //==========

    // Addition
    //---------

    Color operator +(const Color& i_rhs) const;
    Color& operator +=(const Color& i_rhs);

    // Subtraction
    //------------

    Color operator -(const Color& i_rhs) const;
    Color& operator -=(const Color& i_rhs);

    // Multiplication
    //---------------

    Color operator *(const Color& i_rhs) const;
    Color& operator *=(const Color& i_rhs);

    Color operator *(const float i_scalar) const;
    Color& operator *=(const float i_scalar);

    // Division
    //---------

    Color operator /(const Color& i_rhs) const;
    Color& operator /=(const Color& i_rhs);

    Color operator /(const float i_scalar) const;
    Color& operator /=(const float i_scalar);

    // Comparison
    //-----------

    bool operator ==(const Color& i_rhs) const;
    bool operator !=(const Color& i_rhs) const;

    // Static Data
    //============

    static const Color BLACK;
    static const Color BLUE;
    static const Color CYAN;
    static const Color EMERALD;
    static const Color GRAY;
    static const Color GREEN;
    static const Color MAGENTA;
    static const Color ORANGE;
    static const Color PURPLE;
    static const Color RED;
    static const Color SILVER;
    static const Color TURQUOISE;
    static const Color WHITE;
    static const Color YELLOW;

    // Initialization / Shut Down
    //---------------------------

    Color() = default;
    Color(const float i_r, const float i_g, const float i_b, const float i_a);

}; // struct Color

} // namespaace graphics
} // namespace engine

#endif // ENGINE_COLOR_H_