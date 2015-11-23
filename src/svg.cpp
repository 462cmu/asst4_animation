#include "svg.h"

using namespace std;

namespace CMU462 {

Group::~Group() {
  for (size_t i = 0; i < elements.size(); i++) {
    delete elements[i];
  }
  elements.clear();
}

SVG::~SVG() {
  for (size_t i = 0; i < elements.size(); i++) {
    delete elements[i];
  }
  elements.clear();
}

// Parser //

int SVGParser::load(const char *filename, SVG *svg) {
  ifstream in(filename);
  if (!in.is_open()) {
    return -1;
  }
  in.close();

  XMLDocument doc;
  doc.LoadFile(filename);
  if (doc.Error()) {
    doc.PrintError();
    exit(1);
  }

  XMLElement *root = doc.FirstChildElement("svg");
  if (!root) {
    cerr << "\nError: not an SVG file!";
    exit(1);
  }

  root->QueryFloatAttribute("width", &svg->width);
  root->QueryFloatAttribute("height", &svg->height);

  parseSVG_Container(root, svg);

  return 0;
}

void SVGParser::parseSVG_Container(XMLElement *xml, SVG_Container *svg) {
  /* NOTE (sky):
   * SVG uses a "painters model" when drawing elements. Elements
   * that appear later in the document are drawn after (on top of)
   * elements that appear earlier in the document. The parser loads
   * elements in the same order and the renderer should respect this
   * order when drawing elements.
   */

  XMLElement *elem = xml->FirstChildElement();
  while (elem) {
    string elementType(elem->Value());

    // cout << "\nElement Type = " << elementType;

    if (elementType == "line") {
      Line *line = new Line();
      parseElement(elem, line);
      parseLine(elem, line);
      svg->elements.push_back(line);

    } else if (elementType == "polyline") {
      Polyline *polyline = new Polyline();
      parseElement(elem, polyline);
      parsePolyline(elem, polyline);
      svg->elements.push_back(polyline);

    } else if (elementType == "rect") {
      float w = elem->FloatAttribute("width");
      float h = elem->FloatAttribute("height");

      // treat zero-size rectangles as points
      if (w == 0 && h == 0) {
        Point *point = new Point();
        parseElement(elem, point);
        parsePoint(elem, point);
        svg->elements.push_back(point);
      } else {
        Rect *rect = new Rect();
        parseElement(elem, rect);
        parseRect(elem, rect);
        svg->elements.push_back(rect);
      }

    } else if (elementType == "polygon") {
      Polygon *polygon = new Polygon();
      parseElement(elem, polygon);
      parsePolygon(elem, polygon);
      svg->elements.push_back(polygon);

    } else if (elementType == "circle") {
      Circle *circle = new Circle();
      parseElement(elem, circle);
      parseCircle(elem, circle);
      svg->elements.push_back(circle);

    } else if (elementType == "ellipse") {
      Ellipse *ellipse = new Ellipse();
      parseElement(elem, ellipse);
      parseEllipse(elem, ellipse);
      svg->elements.push_back(ellipse);

    } else if (elementType == "image") {
      Image *image = new Image();
      parseElement(elem, image);
      parseImage(elem, image);
      svg->elements.push_back(image);

    } else if (elementType == "path") {

      parsePath(elem, svg);

    } else if (elementType == "g") {

      // ignore a group that contains only a single group
      XMLElement *gg;
      do {
        gg = elem->FirstChildElement();
        string type(gg->Value());
        if ((type == "g") && (gg->NextSiblingElement() == NULL)) {
          elem = gg;
        } else
          break;
      } while (gg);

      Group *group = new Group();
      parseElement(elem, group);
      parseSVG_Container(elem, group); // Recursion.
      svg->elements.push_back(group);

    } else {
      // unknown element type --- include default handler here if desired
      cout << "\nWARNING: Type: " << elementType << " is not handled!";
    }

    elem = elem->NextSiblingElement();
  }
}

void SVGParser::parsePath(XMLElement *xml, SVG_Container *svg) {

  /** NOTE (Sky):
   *
   * A path can be used to specify a variety of shapes. In our SVG renderer
   * implementation we are limited to lines, polygons and polylines, thus
   * only the supported types are parsed.
   *
   * A path consists of the following commands. Note that the commands are
   * case sensitive, a command is capitalized to indicate that it takes
   * absolute coordinates in its arguments:
   * m/M : moveto, starts a subpath at the given coordinate. If a moveto is
   *       followed by multiple pairs of coordinates, the subsequent pairs
   *       are implicit treated as arguments to lineto commands. If a
   *       relative moveto (m) appears as the first element of the path,
   *       then it is treated as a pair of absolute coordinates. In this
   *       case, subsequent pairs of coordinates are treated as relative
   *       even though the initial moveto is interpreted as an absolute
   *       moveto.
   * l/L : lineto, draws a line from the current point to the argument
   *       coordinate which becomes the new current point. Subsequent
   *       argument coordinates are used to construct polylines.
   * h/H : horizontal lineto (srsly?!), draws a line from the current point
   *       to the argument coordinate which becomes the new current point.
   * v/V : vertical lineto (srsly?!), draws a line from the current point
   *       to the argument coordinate which becomes the new current point.
   * z/Z : close the current subpath by drawing a straight line from the
   *       current point to current subpath's initial point.
   * Other commands are available for Bézier curves, arcs, etc... but we
   * won't be using those.
   */

  // make sure there's data
  const char *d = xml->Attribute("d");
  if (!d) {
    cout << "\nError: No data in path element, path ignored";
    xml = xml->NextSiblingElement();
    return;
  }

  string dstr = string(d);

  // split into subpath data
  vector<string> subpaths;
  size_t posl = dstr.find_first_of("mM");
  while (posl != string::npos) {
    size_t posr = dstr.find_first_of("mM", posl + 1);
    if (posr) {
      subpaths.push_back(dstr.substr(posl, posr - posl));
      posl = posr;
    } else {
      subpaths.push_back(dstr.substr(posl));
      break;
    }
  }

  // parse each subpath
  for (auto p = subpaths.begin(); p != subpaths.end(); p++) {

    bool closed = false; // polygon or polyline
    vector<Vector2D> points;

    // split into list of individual commands and unify argument format
    string &subpathstr = *p;
    vector<string> commands;
    size_t posl = subpathstr.find_first_of("mMlLhHvVzZ");
    while (posl != string::npos) {
      size_t posr = subpathstr.find_first_of("mMlLhHvVzZ", posl + 1);
      if (posr) {
        string command = subpathstr.substr(posl, posr - posl);
        replace(command.begin(), command.end(), ',', ' ');
        commands.push_back(command);
        posl = posr;
      } else {
        string command = subpathstr.substr(posl);
        replace(command.begin(), command.end(), ',', ' ');
        commands.push_back(command);
        break;
      }
    }

    for (auto c = commands.begin(); c != commands.end(); c++) {

      // current point - used to handle absolute & relative coordinates
      Vector2D current = Vector2D();

      // parse command
      stringstream data(*c);
      char command;
      float x, y;
      data >> command;

      switch (command) {
      case 'm':

        // first coordinate is implicitly absolute
        data >> x >> y;
        current = Vector2D(x, y);
        points.push_back(current);

        // implicit lineto's are relative
        while (data >> x >> y) {
          current.x += x;
          current.y += y;
          points.push_back(current);
        }
        break;

      case 'M':

        // everything is absolute
        while (data >> x >> y) {
          current.x = x;
          current.y = y;
          points.push_back(current);
        }
        break;

      case 'l':

        // lineto's with relative coordinates
        while (data >> x >> y) {
          current.x += x;
          current.y += y;
          points.push_back(current);
        }
        break;

      case 'L':

        // lineto's with absolute coordinates
        while (data >> x >> y) {
          current.x = x;
          current.y = y;
          points.push_back(current);
        }
        break;

      case 'h':

        // horizontal with relative coordiantes
        data >> x;
        current.x += x;
        points.push_back(current);
        break;

      case 'H':

        // horizontal with absolute coordiantes
        data >> x;
        current.x = x;
        points.push_back(current);
        break;

      case 'v':

        // vertical with relative coordiantes
        data >> y;
        current.y += y;
        points.push_back(current);
        break;

      case 'V':

        // vertical with absolute coordiantes
        data >> y;
        current.y = y;
        points.push_back(current);
        break;

      case 'z':
      case 'Z':
        closed = true;
      }
    }

    // construct element
    if (closed) {
      Polygon *polygon = new Polygon();
      parseElement(xml, polygon);
      polygon->points = points;
      svg->elements.push_back(polygon);
    } else {
      Polyline *polyline = new Polyline();
      parseElement(xml, polyline);
      polyline->points = points;
      svg->elements.push_back(polyline);
    }

    // reset for next subpath
    closed = false;
    points.clear();
  }
}

void SVGParser::parseElement(XMLElement *xml, SVGElement *element) {

  Style *style = &element->style;

  const char *s = xml->Attribute("style");
  if (s) { // parse style string

    // creat a map of individual attribute strings
    string astr = string(s);
    map<string,string> attributes;
    size_t posl = 0;
    while (posl != string::npos) {
      size_t posr = astr.find_first_of(";", posl + 1);
      if (posr) {
        string attribute = astr.substr(posl, posr - posl);
        size_t pos = attribute.find_first_of(":");
        string key = attribute.substr(0, pos);
        string val = attribute.substr(pos + 1);
        attributes[key] = val;
        posl = posr == string::npos ? posr : posr + 1;
      } else {
        string attribute = astr.substr(posl);
        size_t pos = attribute.find_first_of(":");
        string key = attribute.substr(0, pos);
        string val = attribute.substr(pos + 1);
        attributes[key] = val;
        break;
      }

      // parse from map
      auto nope = attributes.end();

      auto fill = attributes.find("fill");
      if (fill != nope) {
        style->fillColor = Color::fromHex(fill->second.c_str());
      }

      auto opacity = attributes.find("opacity");
      if (opacity != nope) {
        style->fillColor.a = atof(opacity->second.c_str());
      }

      auto fill_opacity = attributes.find("fill-opacity");
      if (fill_opacity != nope) {
        style->fillColor.a = atof(fill_opacity->second.c_str());
      }

      auto stroke = attributes.find("stroke");
      auto stroke_opacity = attributes.find("stroke-opacity");
      if (stroke != nope) {
        style->strokeColor = Color::fromHex(stroke->second.c_str());
        if (stroke_opacity != nope) {
          style->strokeColor.a = atof(stroke_opacity->second.c_str());
        }
      } else {
        style->strokeColor = Color::Black;
        style->strokeColor.a = 0;
      }

      auto stroke_width = attributes.find("stroke-width");
      if (stroke_width != nope) {
        style->strokeWidth = atof(stroke_width->second.c_str());
      }

      auto stroke_miterlimit = attributes.find("stroke-miterlimit");
      if (stroke_miterlimit != nope) {
        style->miterLimit = atof(stroke_miterlimit->second.c_str());
      }
    }

  } else { // parse individual properties
    const char *fill = xml->Attribute("fill");
    if (fill)
      style->fillColor = Color::fromHex(fill);

    const char *opacity = xml->Attribute("opacity");
    if (opacity)
      style->fillColor.a = atof(opacity);

    const char *fill_opacity = xml->Attribute("fill-opacity");
    if (fill_opacity)
      style->fillColor.a = atof(fill_opacity);

    const char *stroke = xml->Attribute("stroke");
    const char *stroke_opacity = xml->Attribute("stroke-opacity");
    if (stroke) {
      style->strokeColor = Color::fromHex(stroke);
      if (stroke_opacity)
        style->strokeColor.a = atof(stroke_opacity);
    } else {
      style->strokeColor = Color::Black;
      style->strokeColor.a = 0;
    }

    xml->QueryFloatAttribute("stroke-width", &style->strokeWidth);
    xml->QueryFloatAttribute("stroke-miterlimit", &style->miterLimit);
  }

  // parse transformation
  const char *trans = xml->Attribute("transform");
  if (trans) {
    // NOTE (sky):
    // This implements the SVG transformation specification. All the SVG
    // transformations are supported as documented in the link below:
    // https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/transform

    // consolidate transformation
    Matrix3x3 transform = Matrix3x3::identity();

    string trans_str = trans;
    size_t paren_l, paren_r;
    while (trans_str.find_first_of('(') != string::npos) {
      paren_l = trans_str.find_first_of('(');
      paren_r = trans_str.find_first_of(')');

      string type = trans_str.substr(0, paren_l);
      string data = trans_str.substr(paren_l + 1, paren_r - paren_l - 1);

      if (type == "matrix") {
        string matrix_str = data;
        replace(matrix_str.begin(), matrix_str.end(), ',', ' ');

        stringstream ss(matrix_str);
        float a;
        float b;
        float c;
        float d;
        float e;
        float f;
        ss >> a;
        ss >> b;
        ss >> c;
        ss >> d;
        ss >> e;
        ss >> f;

        Matrix3x3 m;
        m(0, 0) = a;
        m(0, 1) = c;
        m(0, 2) = e;
        m(1, 0) = b;
        m(1, 1) = d;
        m(1, 2) = f;
        m(2, 0) = 0;
        m(2, 1) = 0;
        m(2, 2) = 1;
        transform = transform * m;

      } else if (type == "translate") {
        stringstream ss(data);
        float x;
        if (!(ss >> x))
          x = 0;
        float y;
        if (!(ss >> y))
          y = 0;

        Matrix3x3 m = Matrix3x3::identity();

        m(0, 2) = x;
        m(1, 2) = y;

        transform = transform * m;

      } else if (type == "scale") {
        stringstream ss(data);
        float x;
        if (!(ss >> x))
          x = 1;
        float y;
        if (!(ss >> y))
          y = 1;

        Matrix3x3 m = Matrix3x3::identity();

        m(0, 0) = x;
        m(1, 1) = y;

        transform = transform * m;

      } else if (type == "rotate") {
        stringstream ss(data);
        float a;
        if (!(ss >> a))
          a = 0;
        float x;
        if (!(ss >> x))
          x = 0;
        float y;
        if (!(ss >> y))
          y = 0;

        if (x != 0 || y != 0) {
          Matrix3x3 m = Matrix3x3::identity();

          m(0, 0) = cos(a * PI / 180.0f);
          m(0, 1) = -sin(a * PI / 180.0f);
          m(1, 0) = sin(a * PI / 180.0f);
          m(1, 1) = cos(a * PI / 180.0f);

          m(0, 2) = -x * cos(a * PI / 180.0f) + y * sin(a * PI / 180.0f) + x;
          m(1, 2) = -x * sin(a * PI / 180.0f) - y * cos(a * PI / 180.0f) + y;

          transform = transform * m;

        } else {
          Matrix3x3 m = Matrix3x3::identity();

          m(0, 0) = cos(a * PI / 180.0f);
          m(0, 1) = -sin(a * PI / 180.0f);
          m(1, 0) = sin(a * PI / 180.0f);
          m(1, 1) = cos(a * PI / 180.0f);

          transform = transform * m;
        }

      } else if (type == "skewX") {
        stringstream ss(data);
        float a;
        ss >> a;

        Matrix3x3 m = Matrix3x3::identity();

        m(0, 1) = tan(a * PI / 180.0f);

        transform = transform * m;

      } else if (type == "skewY") {
        stringstream ss(data);
        float a;
        ss >> a;

        Matrix3x3 m = Matrix3x3::identity();

        m(1, 0) = tan(a * PI / 180.0f);

        transform = transform * m;

      } else {
        cout << "Error: Unknown transformation type: " << type;
        exit(EXIT_FAILURE);
      }

      size_t end = paren_r + 2;
      trans_str.erase(0, end);
    }

    element->transform = transform;
  }
}

void SVGParser::parsePoint(XMLElement *xml, Point *point) {
  point->position =
      Vector2D(xml->FloatAttribute("x"), xml->FloatAttribute("y"));
}

void SVGParser::parseLine(XMLElement *xml, Line *line) {
  line->from = Vector2D(xml->FloatAttribute("x1"), xml->FloatAttribute("y1"));
  line->to = Vector2D(xml->FloatAttribute("x2"), xml->FloatAttribute("y2"));
}

void SVGParser::parsePolyline(XMLElement *xml, Polyline *polyline) {
  stringstream points(xml->Attribute("points"));

  float x, y;
  char c;

  while (points >> x >> c >> y) {
    polyline->points.push_back(Vector2D(x, y));
  }
}

void SVGParser::parseRect(XMLElement *xml, Rect *rect) {
  rect->position = Vector2D(xml->FloatAttribute("x"), xml->FloatAttribute("y"));
  rect->dimension =
      Vector2D(xml->FloatAttribute("width"), xml->FloatAttribute("height"));
}

void SVGParser::parsePolygon(XMLElement *xml, Polygon *polygon) {
  stringstream points(xml->Attribute("points"));

  float x, y;
  char c;

  while (points >> x >> c >> y) {
    polygon->points.push_back(Vector2D(x, y));
  }
}

void SVGParser::parseEllipse(XMLElement *xml, Ellipse *ellipse) {
  ellipse->center =
      Vector2D(xml->FloatAttribute("cx"), xml->FloatAttribute("cy"));

  ellipse->radius =
      Vector2D(xml->FloatAttribute("rx"), xml->FloatAttribute("ry"));
}

void SVGParser::parseCircle(XMLElement *xml, Circle *circle) {
  circle->center =
      Vector2D(xml->FloatAttribute("cx"), xml->FloatAttribute("cy"));

  circle->radius = xml->FloatAttribute("r");
}

void SVGParser::parseImage(XMLElement *xml, Image *image) {
  image->position =
      Vector2D(xml->FloatAttribute("x"), xml->FloatAttribute("y"));
  image->dimension =
      Vector2D(xml->FloatAttribute("width"), xml->FloatAttribute("height"));

  // read png data
  const char *data = xml->Attribute("xlink:href");
  while (*data != ',')
    data++;
  data++;

  // decode base64 encoded data
  string encoded = data;
  encoded.erase(remove(encoded.begin(), encoded.end(), ' '), encoded.end());
  encoded.erase(remove(encoded.begin(), encoded.end(), '\t'), encoded.end());
  encoded.erase(remove(encoded.begin(), encoded.end(), '\n'), encoded.end());
  string decoded = base64_decode(encoded);

  // load decoded data into buffer
  const unsigned char *buffer = (unsigned char *)decoded.c_str();
  size_t size = decoded.size();

  // load into png
  PNG png;
  PNGParser::load(buffer, size, png);

  // create bitmap texture from png (mip level 0)
  MipLevel mip_start;
  mip_start.width = png.width;
  mip_start.height = png.height;
  mip_start.texels = png.pixels;

  // add to svg
  image->tex.width = mip_start.width;
  image->tex.height = mip_start.height;
  image->tex.mipmap.push_back(mip_start);
}

// Mass & moments of inertia ---------------------------------------------------
//
// Throughout we assume a uniform mass density based on the shape
// style's alpha value, but of course one can get the moment of
// inertia for any other (uniform) density by simply multiplying
// this value by a constant.

double strokeStyleToMassDensity(const Style &s) {
  // This constant effectively says how much mass
  // is contributed by a stroke of width 1.
  const double baseLineDensity = 0.0001;

  // This constant determines how much every additional point of
  // stroke thickness (past 1.0) contributes to the mass density.
  const double strokeWidthDensityFactor = .2;

  return baseLineDensity * s.strokeColor.a *
         (1. + strokeWidthDensityFactor * (s.strokeWidth - 1.));
}

double shapeStyleToMassDensity(const Style &s) {
  if (s.fillColor.a != 0.) {
    return s.fillColor.a;
  }
  return strokeStyleToMassDensity(s);
}

double Group::mass(void) const {
  // Could also return children's total mass...
  return 1.;
}

double Group::momentOfInertia(const Vector2D &center) const {
  // Could also return children's total inertia...
  return 1.;
}

Vector2D Group::centroid(void) const {
  // Could also return children's centroid...
  return Vector2D(0., 0.);
}

double Point::mass(void) const {
  double rho = style.strokeColor.a;

  return rho;
}

double Point::momentOfInertia(const Vector2D &center) const {
  double rho = style.strokeColor.a;
  Vector2D r = center - position;

  return rho * r.norm2();
}

Vector2D Point::centroid(void) const { return position; }

double Line::mass(void) const {
  double rho = strokeStyleToMassDensity(style);
  double L = (to - from).norm();

  return rho * L;
}

double Line::momentOfInertia(const Vector2D &center) const {
  double rho = strokeStyleToMassDensity(style);

  Vector2D a = from - center;
  Vector2D b = to - center;
  Vector2D m = (a + b) / 2.;
  double L = (b - a).norm();

  return rho * (4. / 3.) * L * dot(m, m);
}

Vector2D Line::centroid(void) const { return (from + to) / 2.; }

double Polyline::mass(void) const {
  double rho = strokeStyleToMassDensity(style);
  double totalLength = 0.;

  int n = points.size();
  for (int i = 0; i < n - 1; i++) {
    Vector2D p = points[i + 0];
    Vector2D q = points[i + 1];
    double L = (q - p).norm();

    totalLength += L;
  }

  return rho * totalLength;
}

double Polyline::momentOfInertia(const Vector2D &center) const {
  double rho = strokeStyleToMassDensity(style);
  double I = 0.;

  int n = points.size();
  for (int i = 0; i < n - 1; i++) {
    Vector2D p = points[i + 0];
    Vector2D q = points[i + 1];

    Vector2D a = p - center;
    Vector2D b = q - center;
    Vector2D m = (a + b) / 2.;
    double L = (b - a).norm();

    I += (4. / 3.) * L * dot(m, m);
  }

  return rho * I;
}

Vector2D Polyline::centroid(void) const {
  Vector2D c(0., 0.);

  int n = points.size();
  for (int i = 0; i < n - 1; i++) {
    Vector2D p = points[i + 0];
    Vector2D q = points[i + 1];

    c += (p + q) / 2.;
  }

  return c / (double)n;
}

double polygonArea(const vector<Vector2D> &points) {
  double area = 0.;

  int n = points.size();
  for (int i = 0; i < n; i++) {
    Vector2D p = points[(i + 0) % n];
    Vector2D q = points[(i + 1) % n];
    double pxq = (p.x * q.y - p.y * q.x);

    area += pxq / 2.;
  }

  return fabs(area);
}

double polygonMomentOfInertia(const Vector2D &center,
                              const vector<Vector2D> &points) {
  double I = 0.;
  double d = 0.;

  int n = points.size();
  for (int i = 0; i < n; i++) {
    Vector2D p = points[(i + 0) % n];
    Vector2D q = points[(i + 1) % n];

    Vector2D a = p - center;
    Vector2D b = q - center;
    Vector2D m = (a + b) / 2.;
    double axb = (a.x * b.y - a.y * b.x);

    I += (2. / 3.) * dot(m, m) * axb;
    d += axb;
  }

  return fabs(I / d);
}

Vector2D polygonCentroid(const vector<Vector2D> &points) {
  Vector2D c(0., 0.);
  double A = 0.;

  int n = points.size();
  for (int i = 0; i < n; i++) {
    Vector2D p = points[(i + 0) % n];
    Vector2D q = points[(i + 1) % n];
    Vector2D m = (p + q) / 2.;
    double pxq = (p.x * q.y - p.y * q.x);

    c += m * pxq;
    A += pxq / 2.;
  }

  return c / (3. * A);
}

double Polygon::mass(void) const {
  double rho = shapeStyleToMassDensity(style);

  return rho * polygonArea(points);
}

double Polygon::momentOfInertia(const Vector2D &center) const {
  double rho = shapeStyleToMassDensity(style);

  return rho * polygonMomentOfInertia(center, points);
}

Vector2D Polygon::centroid(void) const { return polygonCentroid(points); }

double Rect::mass(void) const {
  double rho = shapeStyleToMassDensity(style);

  return rho * (dimension.x * dimension.y);
}

double Rect::momentOfInertia(const Vector2D &center) const {
  double rho = shapeStyleToMassDensity(style);

  Vector2D p = position - center;
  Vector2D w = Vector2D(dimension.x, 0.);
  Vector2D h = Vector2D(0., dimension.y);

  Vector2D p0 = p;
  Vector2D p1 = p + w;
  Vector2D p2 = p + w + h;
  Vector2D p3 = p + h;

  vector<Vector2D> points;
  points.push_back(p0);
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);

  return rho * polygonMomentOfInertia(center, points);
}

Vector2D Rect::centroid(void) const {
  return (position + (position + dimension)) / 2.;
}

double Ellipse::mass(void) const {
  double rho = shapeStyleToMassDensity(style);
  double a = radius.x;
  double b = radius.y;
  double area = M_PI * a * b;

  return rho * area;
}

double Ellipse::momentOfInertia(const Vector2D &_center) const {
  double rho = shapeStyleToMassDensity(style);

  double a = radius.x;
  double b = radius.y;
  double area = M_PI * a * b;
  double mass = rho * area;
  double d2 = (center - _center).norm2();
  double I = mass * (a * a + b * b) / 4.;

  return I + mass * d2;
}

Vector2D Ellipse::centroid(void) const { return center; }

double Circle::mass(void) const {
  double rho = shapeStyleToMassDensity(style);
  double r = radius;
  double area = M_PI * r * r;

  return rho * area;
}

double Circle::momentOfInertia(const Vector2D &_center) const {
  double rho = shapeStyleToMassDensity(style);

  double r = radius;
  double area = M_PI * r * r;
  double mass = rho * area;
  double d2 = (center - _center).norm2();
  double I = mass * r * r / 2.;

  return I + mass * d2;
}

Vector2D Circle::centroid(void) const { return center; }

double Image::mass(void) const {
  // just return the moment of inertia for the bounding box
  double rho = 1.;

  return rho * (dimension.x * dimension.y);
}

double Image::momentOfInertia(const Vector2D &center) const {
  // just return the moment of inertia for the bounding box
  double rho = 1.;

  Vector2D p = position - center;
  Vector2D w = Vector2D(dimension.x, 0.);
  Vector2D h = Vector2D(0., dimension.y);

  Vector2D p0 = p;
  Vector2D p1 = p + w;
  Vector2D p2 = p + h;
  Vector2D p3 = p + w + h;

  vector<Vector2D> points;
  points.push_back(p0);
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);

  return rho * polygonMomentOfInertia(center, points);
}

Vector2D Image::centroid(void) const {
  return (position + (position + dimension)) / 2.;
}

} // namespace CMU462
