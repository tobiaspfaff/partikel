// Color conversion

#include "render/colors.hpp"

Vec3 rgb2hsv(const Vec3& in)
{
    Vec3 out;
    float min, max, delta;

    min = in.x < in.y ? in.x : in.y;
    min = min  < in.z ? min  : in.z;

    max = in.x > in.y ? in.x : in.y;
    max = max  > in.z ? max  : in.z;

    out.z = max; // V
    delta = max - min;
    if (delta < 0.00001)
    {
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.y = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
            // s = 0, v is undefined
        out.y = 0.0;
        out.x = 0.0;                            // its now undefined
        return out;
    }
    if( in.x >= max )                           // > is bogus, just keeps compilor happy
        out.x = ( in.y - in.z ) / delta;        // between yellow & magenta
    else
    if( in.y >= max )
        out.x = 2.0 + ( in.z - in.x ) / delta;  // between cyan & yellow
    else
        out.x = 4.0 + ( in.x - in.y ) / delta;  // between magenta & cyan

    out.x *= 1.0/6.0;                              // degrees

    if( out.x < 0.0 )
        out.x += 1.0;

    return out;
}

Vec3 hsv2rgb(const Vec3& in)
{
    float hh, p, q, t, ff;
    int i;
    Vec3 out;

    if(in.y <= 0.0) {       // < is bogus, just shuts up warnings
        out.x = in.z;
        out.y = in.z;
        out.z = in.z;
        return out;
    }
    hh = in.x * 360;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (int)hh;
    ff = hh - i;
    p = in.z * (1.0 - in.y);
    q = in.z * (1.0 - (in.y * ff));
    t = in.z * (1.0 - (in.y * (1.0 - ff)));

    switch(i) {
    case 0:
        out.x = in.z;
        out.y = t;
        out.z = p;
        break;
    case 1:
        out.x = q;
        out.y = in.z;
        out.z = p;
        break;
    case 2:
        out.x = p;
        out.y = in.z;
        out.z = t;
        break;

    case 3:
        out.x = p;
        out.y = q;
        out.z = in.z;
        break;
    case 4:
        out.x = t;
        out.y = p;
        out.z = in.z;
        break;
    case 5:
    default:
        out.x = in.z;
        out.y = p;
        out.z = q;
        break;
    }
    return out;     
}