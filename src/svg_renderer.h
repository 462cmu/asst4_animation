#ifndef CMU462_SVG_RENDERER_H
#define CMU462_SVG_RENDERER_H

#include <stdio.h>

#include "CMU462/CMU462.h"
#include "svg.h"
#include "viewport.h"
#include <iostream>
#include <stack>

namespace CMU462 {

class SVGRenderer {
 public:

  SVGRenderer()
  {
     transformation.push( Matrix3x3::identity() );
  }

  // Free used resources
  virtual ~SVGRenderer() { }

  virtual void resize( size_t w, size_t h ) = 0;
  
  // Clear to specified color
  virtual void clear( Color clearColor = Color::Black ) = 0;

  // Draw an svg file
  virtual void draw_svg( SVG& svg ) = 0;
  
  // Draws an SVG element
  virtual void draw_element( SVGElement* element ) = 0;

  // Set viewport
  inline void set_viewport( Viewport* viewport ) {
    this->viewport = viewport;
  }

  void loadIdentity( void )
  {
     if( transformation.size() == 0 )
     {
        transformation.push( Matrix3x3::identity() );
     }
     else
     {
        transformation.top() = Matrix3x3::identity();
     }
  }

  void pushTransformation( void )
  {
     transformation.push( transformation.top() );
  }

  void popTransformation( void )
  {
     transformation.pop();
  }

  void concatenateTransformation( const Matrix3x3& X )
  {
     transformation.top() = transformation.top() * X;
  }

  // returns the color of the pixel closest to the specified coordinates
  virtual Color readPixel( float x, float y ) const = 0;

 protected:

  // Viewport
  Viewport* viewport;
  
  // Projective space transformation stack
  std::stack<Matrix3x3> transformation;

  // Transform object coordinate to screen coordinate
  inline Vector2D transform( Vector2D p ) {

    // map point from 2D Euclidean plane to 3D projective space
    Vector3D u( p.x, p.y, 1.0 );

    // apply projective space transformation
    u = transformation.top() * u;

    // project back to 2D Euclidean plane
    return Vector2D(u.x / u.z, u.y / u.z);
  }

  // Transform a vector that represents a direction rather than
  // a point (i.e., ignore translation and only apply rotation and
  // scaling)
  inline Vector2D transformDirection( Vector2D p ) {

    // map point from 2D Euclidean plane to 3D projective space
    Vector3D u( p.x, p.y, 0.0 );

    // apply projective space transformation
    u = transformation.top() * u;

    // project back to 2D Euclidean plane
    return Vector2D( u.x, u.y );
  }

};

} // namespace CMU462

#endif // CMU462_SVG_RENDERER_H
