#ifndef CMU462_ANIMATOR_H
#define CMU462_ANIMATOR_H

/*
 * 2D Key Framed Animation Editor.
 *
 * Writing began by Bryce Summers on 11 / 2 / 2015.
 *
 * This is an important class for project 4,
 * 15462 at Carnegie Mellon University Fall 2015.
 *
 * Adapted from the drawsvg class in project 1.
 *
 */

#include "CMU462/CMU462.h"
#include "svg.h"
#include "GL/glew.h"
#include "CMU462/application.h"
#include "CMU462/lodepng.h"
#include "CMU462/osdtext.h"
#include "character.h"
#include "timeline.h"
#include "svg_renderer.h"
#include "hardware_renderer.h"

using namespace std;

namespace CMU462 {

   /**
    * The Animator class allows for the editing and playback of animations
    * involving characters defined according to the strict specifications of the
    * CMU462 character svg format.
    */
   class Animator : public Application {

      public:

         Animator()
         : hoveredJoint( NULL ),
           selectedJoint( NULL ),
           hoveredCharacter( NULL ),
           selectedCharacter( NULL ),
           showDebugWidgets( true ),
           followCursor( false ),
           draggingTimeline( false ),
           cursor_moving_element( false )
         {
            renderer = new HardwareRenderer();
         }
         ~Animator()
         {
            delete renderer;
         }

         // BEGIN interface declaration of base class methods specified in Application.

         std::string name ( void );
         std::string info ( void );

         void init  ( void );
         void render( void );

         void render_frames( void );

         void resize( size_t width, size_t height );

		 void keyboard_event( int key, int event, unsigned char mods  );
         void cursor_event( float x, float y);
         void mouse_event ( int key, int event, unsigned char mods );
         void scroll_event( float offset_x, float offset_y );

         // END base class interface declarations

         /**
          * Clears out the previous render.
          */
         void clear( void );

         // --------------------------------------------------------------
         // Application functions.
         //---------------------------------------------------------------

         /**
          * Adds a character to the scene by parsing the given SVG file.
          * The SVG file must obey the 462 Rigged Actor Specification.
          */
         void parseNewCharacter( SVG * svg );

         /**
          * Draws All of the characters to the scene at the given time.
          */
         void drawScene(double time);

         // For our drawing pipeline, we assume in many of our drawing routines
         // that GL is set to a standardized orthogonal projection.
         // These routines setup this projection and tear it down.
         //
         // SVGRenderers perform similar functions internally, but we need to set them
         // for components like Timelines that have no notion of the screen.
         void enter_2D_GL_draw_mode();
         void exit_2D_GL_draw_mode();

         /**
          * Draw a rigged Character.
          */
         void drawCharacter( Character & character );

      private:

         // Provides functions for drawing primitive svg shapes.
         SVGRenderer* renderer;// This can either be a hardware application (OpenGL)
         // Of a software application implemented by the students!

         /* window size */
         size_t width, height;

         /* cursor position */
         Vector2D cursorPoint;

         // The stored array of characters.
         vector<Character> actors;

         Timeline timeline;

         // Internal event system (Copied from p3!!) //

         float mouseX, mouseY;
         enum e_mouse_button {
            LEFT   = MOUSE_LEFT,
            RIGHT  = MOUSE_RIGHT,
            MIDDLE = MOUSE_MIDDLE
         };

         bool leftDown;
         bool rightDown;
         bool middleDown;

         // Event handling //

         void mouse_pressed(e_mouse_button b);   // Mouse pressed.
         void mouse_released(e_mouse_button b);  // Mouse Released.

         Vector2D mouseDownPosition;
         Vector2D mouseUpPosition;
         Vector2D originalCharacterPosition;
         Vector2D ikSourcePoint;
         double originalJointAngle;

         // Initializes the source point for IK
         // using the current values of mouseDownPosition
         // and hoveredJoint.
         void setIKSourcePoint( void );

         // returns the joint under the cursor, or
         // NULL if no joint is under the cursor
         Joint* pickJoint( float x, float y );
         Joint* hoveredJoint;
         Joint* selectedJoint;
         Character* hoveredCharacter; // if the cursor is hovering over the root joint of some character, this value will be non-NULL
         Character* selectedCharacter; // if we clicked on the root joint of some character, this value will be non-NULL
         Character* hoveredJointCharacter; // if *any* joint of a character is currently hovered (i.e., if hoveredJoint is non-NULL), then this value will point to the corresponding character
         Character* selectedJointCharacter; // if *any* joint of a character is currently selected (i.e., if selectedJoint is non-NULL), then this value will point to the corresponding character

         // visualization of interpolating splines
         void drawSplines( void );
         void drawArrow( const Vector2D& from,
                         const Vector2D& to );

         // toggles whether to draw debugging widgets
         bool showDebugWidgets;

         // draws the IK source, goal, and gradient vectors
         void drawIKDebugWidgets( void );

         // toggles whether the current source point should follow
         // the cursor using IK; this value is toggled off/on upon
         // right-clicks
         bool followCursor;

		 // Adds a spline value to all characters and joints in the scene.
		 // This is necessary for to ensure that artfully designed poses
		 // are followed faithfully even if only a few parameters were modified.
		 void setUniversalKeyFrame(double time);
		 // Removes a key frame at the given time.
		 // Assumes all times are on the integers.
		 void removeUniversalKeyFrame(double time);


		 // HUD -- drawing functions.

		 // "On Screen display class handles drawing text to the screen."
		 OSDText text_drawer;

		 // Controls whether the HUD is drawn or not.
		 bool b_HUD;

		 // Draws the HUD to the screen.
		 void drawHUD();
		 void drawString(float x, float y,
						 string str,
						 size_t size,
						 Color c);

		 // This variable is use to ensure that when the
                 // cursor is dragging a joint and enters the
                 // timeline region, it does not then begin dragging
                 // the timeline.
		 bool cursor_moving_element;

                 // This flag ensures that when the cursor is
                 // dragging the timeline and then enters the draw
                 // region, it does not then begin dragging joints.
                 bool draggingTimeline;
   };

} // namespace CMU462

#endif // CMU462_ANIMATOR_H
