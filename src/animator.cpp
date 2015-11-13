#include "animator.h"

#include <sstream>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <iomanip>

using namespace std;

// Purely for the purpose of simulation, this number specifies how
// long a logical animation frame is in metric seconds; it may or
// may not actually correspond to the animation playback rate.
const double simulationFramesPerSecond = 60.;
const double simulationTimestep = 1. / simulationFramesPerSecond;

namespace CMU462
{

   void Animator::parseNewCharacter( SVG * svg )
   {
      actors.push_back(Character());
      Character & actor = actors.back();
      actor.load_from_SVG(*svg);
   }

   string Animator::name() {
      return "CMU 15-462 Animator";
   }

   string Animator::info() {
      return "";
   }

   void Animator::init() {

      // Initialize this application by default to have a 300 frame timeline.
      timeline.setMaxFrame(300);
      timeline.markTime(0);

      text_drawer.init(use_hdpi);
      b_HUD = true;
      cursor_moving_element = false;
   }

   // The root of all drawing calls in this project.
   void Animator::render()
   {
      double time = timeline.getCurrentFrame();
      static double lastTime = time;

      if( time != lastTime )
      {
         if( time == 0 )
         {
            for( vector<Character>::iterator character = actors.begin(); character != actors.end(); character++ )
            {
               for( vector<Joint*>::iterator j = character->joints.begin(); j != character->joints.end(); j++ )
               {
                  (*j)->resetDynamics();
               }
            }
         }

         if( time == lastTime+1 )
         {
            for( vector<Character>::iterator character  = actors.begin();
                  character != actors.end();
                  character++ )
            {
               character->integrate( time, simulationTimestep );
            }
         }
      }

      enter_2D_GL_draw_mode();

      //==========================================
      // FIXME: Separate rendering from updating.
      timeline.step();
      //==========================================

      renderer->clear( Color( .6, .6, .9, 0. ) );

      // Draw each character in they order they appear in the "actors"
      // list.  Note that this ordering effectivly determines the layering/
      // occlusion of objects in the scene.
      for( vector<Character>::iterator character  = actors.begin();
            character != actors.end();
            character ++ )
      {
         bool pick = false;
         character->update( time );
         character->draw( renderer, pick, hoveredJoint, selectedJoint );
      }

      if( showDebugWidgets )
      {
         drawSplines();
      }

      // Draw the timeline last, so that it doesn't get occluded by the scene.
      timeline.draw();

      // As long as the right mouse button is being held down, apply IK optimization.
      // Note that this code must be run here in the (so-called) "render" method, because
      // this method is updated every frame, independent of whether the cursor is moving,
      // etc.  By calling Character::reachForTarget() here, the IK optimization is run
      // continuously (rather than just during cursor/click/key events).
      if( followCursor )
      {
         // draw round points
         glEnable( GL_BLEND );
         glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
         glEnable( GL_POINT_SMOOTH );
         glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );

         double time = timeline.getCurrentFrame();
         if( selectedJoint && selectedJointCharacter )
         {
            selectedJointCharacter->reachForTarget( selectedJoint, ikSourcePoint, cursorPoint, time );
            timeline.markTime((int)time);

            if( showDebugWidgets )
            {
               drawIKDebugWidgets();
            }
         }
      }

      // Draw the HUD.
      if(b_HUD)
      {
         drawHUD();
      }


      exit_2D_GL_draw_mode();

      lastTime = time;


   }

   void Animator::render_frames()
   {
      // rewind to begining
      for( vector<Character>::iterator character = actors.begin(); character != actors.end(); character++ )
      {
         for( vector<Joint*>::iterator j = character->joints.begin(); j != character->joints.end(); j++ )
         {
            (*j)->resetVelocity();
         }
      }

      // allocate frame memory
      uint32_t* frame_out = new uint32_t[width * height];

      // output parameters
      const size_t frame_total = timeline.getMaxFrame();

      glReadBuffer(GL_BACK);
      glDrawBuffer(GL_BACK);

      glPushAttrib( GL_VIEWPORT_BIT );
      glViewport( 0, 0, width, height );

      glMatrixMode( GL_PROJECTION );
      glPushMatrix();
      glLoadIdentity();
      glOrtho( 0, width, 0, height, 0, 1 ); // flip y

      glMatrixMode( GL_MODELVIEW );
      glPushMatrix();
      glLoadIdentity();
      glTranslatef( 0, 0, -1 );

      double time;
      size_t frame_count = 0;
      while( frame_count < frame_total )
      {
         renderer->clear( Color( .6, .6, .95, .2 ) );

         time = frame_count;

         // Update character state for the current time step
         for( vector<Character>::iterator character  = actors.begin();
               character != actors.end();
               character ++ )
         {
            character->update( time );
         }

         // Update characters
         for( vector<Character>::iterator character  = actors.begin();
               character != actors.end();
               character++ )
         {
            character->integrate( time, simulationTimestep );
         }

         // Draw each character in they order they appear in the "actors"
         // list.  Note that this ordering effectivly determines the layering/
         // occlusion of objects in the scene.
         for( vector<Character>::iterator character  = actors.begin();
               character != actors.end();
               character ++ )
         {
            character->draw( renderer );
         }

         // Read pixels
         glFlush();
         glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frame_out);
         fprintf(stdout, "\rWriting frames: %lu/%lu", frame_count + 1, frame_total);
         fflush(stdout);

         // Write to image
         ostringstream filename;
         filename << "frame_";
         filename << std::setw(4) << std::setfill('0') << frame_count;
         filename << string(".png");
         lodepng::encode(filename.str(), (unsigned char*) frame_out, width, height);

         frame_count++;
      }

      std::cout << std::endl;

      glMatrixMode( GL_PROJECTION );
      glPopMatrix();

      glMatrixMode( GL_MODELVIEW );
      glPopMatrix();
      glPopAttrib();

   }


   void Animator :: drawIKDebugWidgets( void )
   {
      // draw source and target IK points
      Vector3D p( ikSourcePoint.x, ikSourcePoint.y, 1. );
      p = selectedJoint->currentTransformation * p;
      glPointSize( 25. );
      glBegin( GL_POINTS );
      glColor4f( 0., 0., 1., 1. ); glVertex2d( cursorPoint.x, cursorPoint.y );
      glColor4f( 1., 0., 0., 1. ); glVertex2d( p.x/p.z, p.y/p.z );
      glEnd();

      // visualize gradients of IK energy
      for( vector<Joint*>::iterator j = selectedJointCharacter->joints.begin(); j != selectedJointCharacter->joints.end(); j++ )
      {
         const double gradientLengthScale = 2000.;
         const Joint& joint( **j );

         if( joint.kids.size() > 0 )
         {
            Vector2D p = joint.currentCenter;
            Vector2D q = joint.kids[0]->currentCenter;
            Vector2D e1 = ( q-p ).unit();
            Vector2D e2( -e1.y, e1.x );
            double dTheta = joint.ikAngleGradient;
            glColor4f( 1., 1., 0., 1. );
            drawArrow( q, q + gradientLengthScale*dTheta*e2 );
         }
      }
   }

   void Animator :: drawArrow( const Vector2D& from, const Vector2D& to )
   {
      const double lineWidth = 5.;
      const double arrowheadScale = 15.;

      // arrowhead shape
      const Vector2D a0(  0.65,  0.0 );
      const Vector2D a1( -0.65,  0.5 );
      const Vector2D a2( -0.35,  0.0 );
      const Vector2D a3( -0.65, -0.5 );

      glLineWidth( lineWidth );
      glBegin( GL_LINES );
      glVertex2dv( &from.x );
      glVertex2dv( &to.x );
      glEnd();

      Vector2D e1 = ( to - from ).unit();
      Vector2D e2( -e1.y, e1.x );
      Vector2D p0 = to + arrowheadScale*( a0.x*e1 + a0.y*e2 );
      Vector2D p1 = to + arrowheadScale*( a1.x*e1 + a1.y*e2 );
      Vector2D p2 = to + arrowheadScale*( a2.x*e1 + a2.y*e2 );
      Vector2D p3 = to + arrowheadScale*( a3.x*e1 + a3.y*e2 );

      glBegin( GL_TRIANGLES );
      glVertex2dv( &p0.x ); glVertex2dv( &p1.x ); glVertex2dv( &p2.x );
      glVertex2dv( &p2.x ); glVertex2dv( &p3.x ); glVertex2dv( &p0.x );
      glEnd();
   }

   void Animator::drawSplines( void )
   {
      const double nSamplesPerTick = 1.;
      const double tangentLengthScale = 5.;
      const double controlPointScale = 10.;

      glPushAttrib( GL_ALL_ATTRIB_BITS );

      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

      // use round (rather than square) points
      glEnable( GL_POINT_SMOOTH );
      glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );

      for( vector<Character>::iterator character = actors.begin(); character != actors.end(); character++ )
      {
         bool selected = ( &*character == selectedCharacter );
         double alpha = selected ? 1. : .2;

         Spline<Vector2D>& spline = character->position;

         // approximate spline by line segments
         const double maxTime = timeline.getMaxFrame();
         Vector2D c = character->root->center;
         glLineWidth( 3. );
         glColor4f( 1., 1., 1., alpha );
         glBegin( GL_LINE_STRIP );
         for( double t = 0; t <= maxTime; t += 1./nSamplesPerTick )
         {
            Vector2D p = spline( t ) + c;
            glVertex2d( p.x, p.y );
         }
         glEnd();

         if( selected )
         {
            // draw dot for current position
            double t0 = timeline.getCurrentFrame();
            Vector2D p0 = c + spline( t0 );
            glColor4f( 0., 0., 0., alpha );
            glPointSize( controlPointScale * 1.5 );
            glBegin( GL_POINTS );
            glVertex2d( p0.x, p0.y );
            glEnd();
            
            // draw control points, tangents, and curvature
            for( Spline<Vector2D>::KnotCIter k  = spline.knots.begin(); k != spline.knots.end(); k++ )
            {
               double t = k->first;
               Vector2D P = c + k->second; // position
               Vector2D T = tangentLengthScale * spline.evaluate( t, 1 ); // tangent
               Vector2D A = tangentLengthScale * spline.evaluate( t, 2 ); // acceleration

               glColor4f( 1., 1., 1., alpha );
               glPointSize( controlPointScale );
               glBegin( GL_POINTS );
               glVertex2d( P.x, P.y );
               glEnd();

               if( selected )
               {
                  glColor4f( 0., 0., 1., alpha ); drawArrow( P, P+T );
                  glColor4f( 1., 0., 0., alpha ); drawArrow( P, P+A );
               }
            }
         }
      }

      glPopAttrib();
   }

   void Animator::resize( size_t width, size_t height ) {

      this->width  = width;
      this->height = height;

      renderer->resize( width, height );

      text_drawer.resize(width, height);

      timeline.resize(width, 64);
      timeline.move(0, height - 64);
   }

   void Animator::keyboard_event( int key, int event, unsigned char mods  )
   {
      // Handle arrow keys to move around timeline.
      if( event == EVENT_PRESS || event == EVENT_REPEAT )
      {
         switch( mods )
         {
            case MOD_ALT:
               if( key == KEYBOARD_LEFT  ) timeline.action_goto_prev_key_frame();
               if( key == KEYBOARD_RIGHT ) timeline.action_goto_next_key_frame();
               break;
            case MOD_SHIFT:
               if( key == KEYBOARD_LEFT  ) timeline.action_step_backward( 10 );
               if( key == KEYBOARD_RIGHT ) timeline.action_step_forward( 10 );
               break;
            default:
               if( key == KEYBOARD_LEFT  ) timeline.action_step_backward();
               if( key == KEYBOARD_RIGHT ) timeline.action_step_forward();
               break;
         }
      }

      if(event == EVENT_PRESS)
      {
         switch(key)
         {
            case KEYBOARD_UP:
            case KEYBOARD_HOME:
               timeline.action_rewind();
               break;
            case KEYBOARD_DOWN:
            case KEYBOARD_END:
               timeline.action_goto_end();
               break;
            case ' ':
               timeline.action_toggle_playing();
               break;
            case 'd':
            case 'D':
                      showDebugWidgets = !showDebugWidgets;
                      b_HUD = !b_HUD;
                      break;

                      // Export a sequence of animation frames.
            case 's':
            case 'S':
                      render_frames();
                      break;

                      // Remove KeyFrames.
            case KEYBOARD_BACKSPACE:
            case KEYBOARD_DELETE:
                      if(timeline.getCurrentFrame() > 0)
                      {
                         removeUniversalKeyFrame(timeline.getCurrentFrame());
                         timeline.unmarkTime(timeline.getCurrentFrame());
                      }
                      break;

            case '[':
                      timeline.makeShorter(100);
                      break;
            case ']':
                      timeline.makeLonger(100);
                      break;
         }
      }

   }

   Joint* Animator :: pickJoint( float x, float y )
   {
      // Initially assume that there is no joint under the cursor, and
      // that this (null) joint is not associated with any character.
      Joint* j = NULL;
      hoveredJointCharacter = NULL;

      // Picking is implemented by drawing each character with special
      // pseudocolors determined by the index of each joint in the
      // character's "joints" array.  We then read back the pixel color
      // at the cursor location, and (if it's nonzero) use it to grab
      // the appropriate Joint pointer from the current character.
      // Note that indices are offset by 1, so that the black background
      // can be used to indicate "no joint."

      // Like any other drawing, we need to make sure that our draw
      // calls reflect drawing into a 2D canvas.  (For the software
      // renderer, this should be the default behavior, but for the
      // hardware renderer we need to setup some OpenGL state.)
      enter_2D_GL_draw_mode();

      // We also need to make sure we're drawing
      // the actors at the current time.
      double time = timeline.getCurrentFrame();

      // We iterate over actors in *reverse* order,
      // so that the actor on top gets picked first.
      for( int i = actors.size()-1; i >= 0; i-- )
      {
         Character& character( actors[i] );

         // Since indices are local to each character, we
         // need to clear the buffer of any values that
         // refer to a previous character.
         renderer->clear();

         // Now draw the character, setting a special flag that
         // indicates that we are in picking mode (which means
         // joints will get drawn with pseudocolors).
         const bool pick = true;
         character.draw( renderer, pick );

         // Grab the pixel under the cursor from the renderer.
         Color c = renderer->readPixel( x, y );

         // Convert the pixel color to a picking index, subtracting
         // 1 to get back to the original 0-based indexing of our
         // "joints" array.
         int pickIndex = c.toPickIndex() - 1;

         // If the index is in bounds, we pick this joint; we also
         // want to skip the rest of the characters, since we care
         // only about the "first hit," i.e., the character at the
         // top of the drawing.  Note that occlusion of joints *within*
         // a character is already taken care of, by the mere fact that
         // we are drawing the character from the root to the leaves.
         if( 0 <= pickIndex&&pickIndex < character.joints.size() )
         {
            j = character.joints[pickIndex];
            hoveredJointCharacter = &character;
            break;
         }
      }

      // We now leave the 2D canvas, in case other draw routines
      // need to use OpenGL in a different way.
      exit_2D_GL_draw_mode();

      return j;
   }

   void Animator::cursor_event( float x, float y)
   {
      cursorPoint = Vector2D( x, y );

      if(!cursor_moving_element && timeline.mouse_over(x, y) )
      {
         // Dragging functionality over the timeline.
         if(timeline.mouse_over_timeline(x, y) && leftDown)
         {
            timeline.mouse_click( cursorPoint.x, cursorPoint.y );
         }

         return;
      }

      // Note (Bryce): Timelines actually give integers...
      double time = timeline.getCurrentFrame();

      // Handle dragging w/ left mouse button.
      if( leftDown && !draggingTimeline )
      {
         if( selectedCharacter )
         {
            Vector2D updatedPosition = originalCharacterPosition + ( Vector2D(x,y) - mouseDownPosition );
            selectedCharacter->position.setValue( time, updatedPosition );
            timeline.markTime((int)time);

            setUniversalKeyFrame(time);

            cursor_moving_element = true;
         }
         else if( selectedJoint )
         {
            Vector2D c = selectedJoint->currentCenter;
            Vector2D a = Vector2D( x, y ) - c;
            Vector2D b = mouseDownPosition - c;
            double delta = atan2( a.x*b.y-a.y*b.x, a.x*b.x + a.y*b.y );
            double updatedAngle = originalJointAngle + delta;

            selectedJoint->setAngle( time, updatedAngle );
            timeline.markTime((int)time);

            setUniversalKeyFrame(time);

            cursor_moving_element = true;
         }
      }

      // Handle "hovering," i.e., picking the object under the cursor so that it can be highlighted.
      hoveredJoint = pickJoint( x, y );
      hoveredCharacter = NULL;

      for( vector<Character>::iterator character = actors.begin();
            character != actors.end();
            character ++ )
      {
         if( hoveredJoint == character->root )
         {
            hoveredCharacter = &*character;
         }
      }
   }

   // Sets all Characters and joints in the scene to have a keyframe at
   // the given time with the value that they currenly would currently
   // have at that time.
   void Animator::setUniversalKeyFrame(double time)
   {
      for(vector<Character>::iterator C_iter = actors.begin();
            C_iter != actors.end();
            C_iter++)
      {
         Character & C = *C_iter;
         for(vector<Joint*>::iterator iter = C.joints.begin();
               iter != C.joints.end();
               iter++)
         {
            Joint * joint = *iter;
            joint -> setAngle(time, joint -> getAngle(time));
         }

         Spline<Vector2D> & pos = C.position;
         pos.setValue(time, pos.evaluate(time));
      }

   }

   // Removes the given keyframe from all nodes.
   void Animator::removeUniversalKeyFrame(double time)
   {
      for(vector<Character>::iterator C_iter = actors.begin();
            C_iter != actors.end();
            C_iter++)
      {
         Character & C = *C_iter;
         for(vector<Joint*>::iterator iter = C.joints.begin();
               iter != C.joints.end();
               iter++)
         {
            Joint * joint = *iter;
            joint->removeAngle(time);
         }

         Spline<Vector2D> & pos = C.position;
         pos.removeKnot(time, .1); // Assuming times are on the integers.
      }

   }


   // FIXME : This code is the same as the helpful decomposition in p3.
   //         We should probably put this function in the standard library.
   //         I think that we should inherit mouse_pressed(), released() etc
   //         from the base class.
   void Animator::mouse_event(int key, int event, unsigned char mods)
   {
      switch(event)
      {
         case EVENT_PRESS:
            switch(key) {
               case MOUSE_LEFT:
                  mouse_pressed(LEFT);
                  break;
               case MOUSE_RIGHT:
                  mouse_pressed(RIGHT);
                  break;
               case MOUSE_MIDDLE:
                  mouse_pressed(MIDDLE);
                  break;
            }
            break;
         case EVENT_RELEASE:
            switch(key) {
               case MOUSE_LEFT:
                  mouse_released(LEFT);
                  break;
               case MOUSE_RIGHT:
                  mouse_released(RIGHT);
                  break;
               case MOUSE_MIDDLE:
                  mouse_released(MIDDLE);
                  break;
            }
            // Signal the end of any joint or character movement.
            cursor_moving_element = false;
            break;
      }
   }

   void Animator::mouse_pressed( e_mouse_button b )
   {
      double time = timeline.getCurrentFrame();
      mouseDownPosition = cursorPoint;
      draggingTimeline = false;

      switch( b )
      {
         case LEFT:
            leftDown = true;
            followCursor = false;
            if( timeline.mouse_click( cursorPoint.x, cursorPoint.y ) )
            {
               draggingTimeline = true;
            }
            else if( hoveredCharacter )
            {
               selectedCharacter = hoveredCharacter;
               selectedJoint = selectedCharacter->root;
               selectedJointCharacter = hoveredCharacter;
               originalCharacterPosition = hoveredCharacter->position( time );
            }
            else
            {
               selectedCharacter = NULL;

               if( hoveredJoint )
               {
                  selectedJoint = hoveredJoint;
                  selectedJointCharacter = hoveredJointCharacter;
                  originalJointAngle = selectedJoint->getAngle( time );
               }
               else
               {
                  selectedJoint = NULL;
                  selectedJointCharacter = NULL;
               }
            }
            break;
         case RIGHT:
            rightDown = true;
            if( !followCursor )
            {
               setIKSourcePoint();
               followCursor = true;
            }
            else
            {
               followCursor = false;
            }
            break;
         case MIDDLE:
            middleDown = true;
            break;
      }
   }

   void Animator :: setIKSourcePoint( void )
   {
      if( hoveredJoint )
      {
         selectedJoint = hoveredJoint;
         selectedJointCharacter = hoveredJointCharacter;

         // determine where the point we clicked on would have
         // been in the pre-transformed coordinate system
         Vector3D p( mouseDownPosition.x, mouseDownPosition.y, 1. );
         p = selectedJoint->currentTransformation.inv() * p;
         ikSourcePoint = Vector2D( p.x/p.z, p.y/p.z );
      }
      else
      {
         selectedJoint = NULL;
         selectedJointCharacter = NULL;
      }
   }

   void Animator::mouse_released( e_mouse_button b )
   {
      mouseUpPosition = cursorPoint;
      draggingTimeline = false;

      switch( b )
      {
         case LEFT:
            leftDown = false;
            break;
         case RIGHT:
            rightDown = false;
            break;
         case MIDDLE:
            middleDown = false;
            break;
      }
   }

   void Animator::scroll_event( float offset_x, float offset_y )
   {
      if (offset_x || offset_y)
      {
         // FIXME : Put Relevant scroll behaviors here.
      }
   }

   // ====== DRAWING ========

   void Animator::enter_2D_GL_draw_mode()
   {
      // FIXME: Double check these they might need to be screen space, instead of window space.
      int screen_w = width;
      int screen_h = height;

      glPushAttrib( GL_VIEWPORT_BIT );
      glViewport( 0, 0, screen_w, screen_h );

      glMatrixMode( GL_PROJECTION );
      glPushMatrix();
      glLoadIdentity();
      glOrtho( 0, screen_w, screen_h, 0, 0, 1 ); // Y flipped !

      glMatrixMode( GL_MODELVIEW );
      glPushMatrix();
      glLoadIdentity();
      glTranslatef( 0, 0, -1 );
      //  glDisable(GL_DEPTH_TEST);
   }

   void Animator::exit_2D_GL_draw_mode()
   {
      glMatrixMode( GL_PROJECTION );
      glPopMatrix();

      glMatrixMode( GL_MODELVIEW );
      glPopMatrix();
      glPopAttrib();
   }

   // -- TEXT Drawing for the HUD.

   void Animator::drawHUD()
   {
      text_drawer.clear();

      // The selected Joint.
      if(selectedJoint == NULL)
      {
         return;
      }

      Joint & joint = *selectedJoint;
      double time = timeline.getCurrentFrame();

      ostringstream m1, m2, m3, m4, m5, m6;

      m1 << "type: ";
      switch(selectedJoint->type)
      {
         case KEYFRAMED:
            m1 << "keyframed";
            m2 << "angle: " << joint.getAngle(time);
            break;
         case DYNAMIC:
            m1 << "dynamic";
            m2 << "theta: " << joint.getTheta();
            m3 << "omega: " << joint.getOmega();
            break;
      }

      if( selectedJointCharacter )
      {
         m4 << "position: " << selectedJointCharacter->position(time);
         m5 << "velocity: " << selectedJointCharacter->position.evaluate(time,1);
         m6 << "acceleration: " << selectedJointCharacter->position.evaluate(time,2);
      }

      const size_t size = 12;
      const float x0 = use_hdpi ? width - 200 * 2 : width - 200;
      const float y0 = use_hdpi ? 32*2 : 32;
      const int inc  = use_hdpi ? 40 : 20;
      const int indent = use_hdpi ? 20 : 10;
      float y = y0 + inc - size;

      Color text_color = Color(1.0, 1.0, 1.0);

      drawString(x0,        y, "CHARACTER", size, text_color); y += inc;
      drawString(x0+indent, y, m4.str(),    size, text_color); y += inc;
      drawString(x0+indent, y, m5.str(),    size, text_color); y += inc;
      drawString(x0+indent, y, m6.str(),    size, text_color); y += inc;
      drawString(x0,        y, "JOINT",     size, text_color); y += inc;
      drawString(x0+indent, y, m1.str(),    size, text_color); y += inc;
      drawString(x0+indent, y, m2.str(),    size, text_color); y += inc;
      drawString(x0+indent, y, m3.str(),    size, text_color); y += inc;

      glColor4f(0.0, 0.0, 0.0, 0.8);
      timeline.drawRectangle(x0 - size, y0 - size, width, y);

      text_drawer.render();
   }

   inline void Animator::drawString( float x, float y, string str, size_t size, Color c )
   {
      text_drawer.add_line( ( x*2/width)  - 1.0,
                            (-y*2/height) + 1.0,
                            str, size, c );
   }

} // namespace CMU462
