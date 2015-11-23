#ifndef CMU462_SVG_H
#define CMU462_SVG_H

#include <map>
#include <vector>

#include "CMU462/CMU462.h" // Standard 462 Vectors, etc.

#include "CMU462/color.h"
#include "CMU462/vector2D.h"
#include "CMU462/matrix3x3.h"
#include "CMU462/tinyxml2.h"

#include "texture.h"
#include "png.h"

#include "CMU462/base64.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>


using namespace tinyxml2;

namespace CMU462 {

typedef enum e_SVGElementType {
  NONE = 0,
  POINT,
  LINE,
  POLYLINE,
  POLYGON,
  RECT,
  ELLIPSE,
  CIRCLE,
  IMAGE,
  GROUP
} SVGElementType;

struct Style {
  Color strokeColor;
  Color fillColor;
  float strokeWidth;
  float miterLimit;
};

struct SVGElement {

  SVGElement( SVGElementType _type )
    : type( _type ), transform( Matrix3x3::identity() ) { }

  virtual ~SVGElement() { }

  // since SVGElement is an abstract base class,
  // we need some way to make a copy without
  // actually resolving the derived type
  virtual SVGElement* copy( void ) const = 0;

  // Returns the mass and moment of inertia around the given point,
  // assuming a uniform mass density between 0 and 1, determined by
  // the alpha value of the shape's (fill or line) color.  For
  // strokes, the mass density will be modulated by the stroke
  // thickness.  One can of course obtain the moment of inertia for
  // any other (uniform) density by simply multiplying this value by
  // a constant.
  virtual double mass( void ) const = 0;
  virtual double momentOfInertia( const Vector2D& center ) const = 0;
  virtual Vector2D centroid( void ) const = 0;

  // primitive type
  SVGElementType type;

  // styling
  Style style;

  // transformation list
  Matrix3x3 transform;

};

// Base class for Groups and SVG's that can contain SVG elements.
struct SVG_Container
{
  std::vector<SVGElement*> elements;
};

struct Group : SVGElement, SVG_Container {

  Group() : SVGElement  ( GROUP ) { }

  ~Group();

  virtual SVGElement* copy( void ) const
  {
     return new Group( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

struct Point : SVGElement {

  Point() : SVGElement ( POINT ) { }
  Vector2D position;

  virtual SVGElement* copy( void ) const
  {
     return new Point( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

struct Line : SVGElement {

  Line() : SVGElement ( LINE ) { }
  Vector2D from;
  Vector2D to;

  virtual SVGElement* copy( void ) const
  {
     return new Line( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

struct Polyline : SVGElement {

  Polyline() : SVGElement  ( POLYLINE ) { }
  std::vector<Vector2D> points;

  virtual SVGElement* copy( void ) const
  {
     return new Polyline( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

struct Polygon : SVGElement {

  Polygon() : SVGElement  ( POLYGON ) { }
  std::vector<Vector2D> points;

  virtual SVGElement* copy( void ) const
  {
     return new Polygon( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

struct Rect : SVGElement {

  Rect() : SVGElement ( RECT ) { }
  Vector2D position;
  Vector2D dimension;

  virtual SVGElement* copy( void ) const
  {
     return new Rect( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

struct Ellipse : SVGElement {

  Ellipse() : SVGElement  ( ELLIPSE ) { }
  Vector2D center;
  Vector2D radius;

  virtual SVGElement* copy( void ) const
  {
     return new Ellipse( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

struct Circle : SVGElement {

  Circle() : SVGElement  ( CIRCLE ) { }
  Vector2D center;
  double radius;

  virtual SVGElement* copy( void ) const
  {
     return new Circle( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

struct Image : SVGElement {

  Image() : SVGElement  ( IMAGE ) { }
  Vector2D position;
  Vector2D dimension;
  Texture tex;

  virtual SVGElement* copy( void ) const
  {
     return new Image( *this );
  }

  virtual double mass( void ) const;
  virtual double momentOfInertia( const Vector2D& center ) const;
  virtual Vector2D centroid( void ) const;
};

// AN SVG is just a glorified group.
 struct SVG : SVG_Container {

  ~SVG();
  float width, height;

};

class SVGParser {
 public:

  static int load( const char* filename, SVG* svg );
  static int save( const char* filename, const SVG* svg );

 private:

  // parse an SVG Container, including the original file.
  static void parseSVG_Container ( XMLElement* xml, SVG_Container* svg );

  // parse shared properties of svg elements
  static void parseElement   ( XMLElement* xml, SVGElement* element );

  // parse type specific properties
  static void parsePoint     ( XMLElement* xml, Point*    point       );
  static void parseLine      ( XMLElement* xml, Line*     line        );
  static void parsePolyline  ( XMLElement* xml, Polyline* polyline    );
  static void parseRect      ( XMLElement* xml, Rect*     rect        );
  static void parsePolygon   ( XMLElement* xml, Polygon*  polygon     );
  static void parseEllipse   ( XMLElement* xml, Ellipse*  ellipse     );
  static void parseCircle    ( XMLElement* xml, Circle*   circle      );
  static void parseImage     ( XMLElement* xml, Image*    image       );

  // parse a path-encoded shape
  static void parsePath ( XMLElement* xml, SVG_Container* svg );

  // Note: Groups are parsed using parseSVG_Container.

}; // class SVGParser

} // namespace CMU462

#endif // CMU462_SVG_H
