#ifndef CMU462_HARDWARE_RENDERER_H
#define CMU462_HARDWARE_RENDERER_H

#include <stdio.h>

#include "CMU462/CMU462.h"
#include "svg_renderer.h"
#include "GL/glew.h"



namespace CMU462 {

class HardwareRenderer : public SVGRenderer {
 public:

  HardwareRenderer()
  {
     transformation.push( Matrix3x3::identity() );
  }

  // Implements Renderer
  ~HardwareRenderer() { }

  // 2D drawing mode
  void begin2DDrawing();
  void leave2DDrawing();

  // Draw an svg input to render target
  void draw_svg( SVG& svg );
  
  // Draws an SVG element
  void draw_element( SVGElement* element );

  // resize context
  virtual void resize(size_t w, size_t h);

  // Clear render target
  virtual void clear( Color clearColor = Color::Black )
  {
     glClearColor( clearColor.r,
                   clearColor.g,
                   clearColor.b,
                   clearColor.a );

     glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  }

  // Set svg to screen transformation
  inline void set_canvas_to_screen( Matrix3x3 canvas_to_screen ) {
    this->canvas_to_screen = canvas_to_screen;
  }
  
  // returns the color of the pixel closest to the specified coordinates
  virtual Color readPixel( float x, float y ) const;
  
 private:

  // Draws a point
  void draw_point( Point& p );

  // Draw a line
  void draw_line( Line& line );

  // Draw a polyline
  void draw_polyline( Polyline& polyline );

  // Draw a rectangle
  void draw_rect ( Rect& rect );

  // Draw a polygon
  void draw_polygon( Polygon& polygon );

  // Draw a circle
  void draw_circle( Circle& circle );

  // Draw a ellipse
  void draw_ellipse( Ellipse& ellipse );

  // Draws a bitmap image
  void draw_image( Image& image );

  // Draw a group
  void draw_group( Group& group );

  // Rasterization //

  // rasterize a point
  void rasterize_point( float x, float y, Color color );

  // rasterize a line
  void rasterize_line( float x0, float y0,
                       float x1, float y1,
                       Color color,
                       double strokeWidth = 1. );

  // rasterize a triangle
  void rasterize_triangle( float x0, float y0,
                           float x1, float y1,
                           float x2, float y2,
                           Color color );

  // rasterize an image
  void rasterize_image( float x0, float y0,
                        float x1, float y1,
                        Texture& tex );

  // resolve samples to render target
  // void resolve( void );

  // GL context dimension
  size_t context_w; size_t context_h;

  // SVG coordinates to screen space coordinates
  Matrix3x3 canvas_to_screen;

}; // class HardwareRenderer

} // namespace CMU462

#endif // CMU462_HARDWARE_RENDERER_H
