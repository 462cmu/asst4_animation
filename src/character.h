#ifndef CHARACTER_H
#define CHARACTER_H

/*
 * Character class and Joint Class.
 * Written by Bryce Summers on October 29, 2015.
 *
 * Purpose : These classes represent a rigged character that can be animated.
 *
 */

#include <vector>
#include "svg.h"
#include "spline.h"
#include "svg_renderer.h"

using namespace std;

namespace CMU462
{
   class Character;

   // JointType specifies how a given joint gets animated: using
   // keyframed spline animation, or dynamic simulation.
   enum JointType
   {
      KEYFRAMED,
      DYNAMIC
   };

   class Joint
   {
      public:
         Joint();
         ~Joint();

         // Each joint has some number of children (possibly zero).
         vector<Joint*> kids;
         
         // Type of joint; this value specifies which kind of
         // motion should be used (keyframing or dynamics).
         JointType type;

         // Location of the center of rotation, *before* any transformations
         // are applied to the character.  Note that this value does NOT
         // represent the current state of the character; it represents the
         // state of the character in its original (rest) pose.  The current
         // center is stored in Joint::currentCenter.
         Vector2D center;

         // Location of the center of rotation at the current time, *after* transformations
         // (rotations and translations) have been applied to the character and its joints.
         // The value of Joint::currentCenter is updated by calls to Character::update().
         Vector2D currentCenter;

         // Value of the current joint transformation, taking into account translation
         // and rotation of all joints above it.  This value gets updated during calls
         // to Character::update().  Note that at all times, applying this transformation
         // to the center should yield the current center.  The reason we need to keep
         // track of both pieces of data is that we might also need to apply the same
         // transformation to other data.  (Likewise, it is often convenient to have
         // the current center, without needing to apply the transformation every time.)
         Matrix3x3 currentTransformation;

         // For convenience, we also store the current transformation of the parent joint.
         // This value is mainly useful for ensuring that pendulum joints hang "down"
         // correctly, rather than being transformed by their parent.
         Matrix3x3 currentParentTransformation;
         
         // Gradient of IK energy with respect to this joint angle, which will
         // be used to update the joint configurations.  This value is updated
         // by calls to Character::reachForTarget().
         double ikAngleGradient;

         // Returns the joint angle.  If the joint motion is determined by keyframe animation, this
         // angle will be the interpolated angle at the given time; if the joint motion is determined
         // by dynamics, this angle will simply be the most recently computed angle.
         double getAngle( double time );

         // Sets the joint angle.  If the joint motion is determined by keyframe animation, this method
         // sets the angle for the spline at the specified time; if it is determined by dynamics, this
         // method simply sets the dynamical variable at the current time to the specified value.
         void setAngle( double time, double value );

         // Removes any keyframe corresponding to the specified time.
         bool removeAngle( double time );

         // Reset dynamical variables (or just velocities) to defaults (zero).
         void resetDynamics( void );
         void resetVelocity( void );

         // Computes the gradient of IK energy for this joint and, recursively,
         // for all of its children, storing the result in Joint::ikAngleGradient.
         bool calculateAngleGradient( Joint* goalJoint, Vector2D p, Vector2D ptilde );

         // Recursively update the current transformation and center
         // for this joint and all its children.
         void update( double time, Matrix3x3 transform );

         // Computes the total mass, moment of inertia, and center of mass relative to
         // the given center point using the joint shape as described in the SVG file.
         void physicalQuantities( double& m, double& I, Vector2D& c, Vector2D center ) const;

         // Recursively performs time integration for any dynamic joint.
         void integrate( double time, double timestep, Vector2D cumulativeAcceleration );

         // Recursively draw this joint and all child joints.
         // If in picking mode, will use pseudocolors based on index.
         void draw( SVGRenderer* renderer, bool pick, Joint* hovered, Joint* selected );

         // Parses a Joint from the given group.
         void parse_from_group(Group * G, Character & C);

         // Set the joint type based on the style of the given circle.
         void setJointType( Circle* circle );

         // Index into the "joints" array of this Joint's Character.
         // This value is used exclusively for OpenGL picking; it
         // should not be used (or needed) by any other routine.
         int index;

         // Accessors for dynamical angle variables.
         double getTheta(){ return theta; };
         double getOmega(){ return omega; };

      private:
         // For keyframed joints, "angle" stores the angle of the joint
         // relative to its initial rest pose.  These values are accumulated
         // along the kinematic chain to determine the current configuration
         // of the character.  Note that this value is used ONLY for keyframed
         // joints; for dynamic joints, the angle is determined by the value
         // "theta", defined below.
         Spline<double> angle;

         // For pendulum joints, "theta" stores the angle with the vertical and
         // "omega" stores the associated angular velocity.  Note that these
         // values are used ONLY for dynamic joints; for keyframed joints, the
         // angle is determined by the spline "angle", defined above.
         double theta; // dynamical configuration
         double omega; // dynamical angular velocity
         
         // An array of shapes describes the appearance of the joint,
         // which get drawn back-to-front in first-to-last order.  These
         // values should NOT be needed to determine the joint motion,
         // except by the method Joint::physicalQuantities().
         vector<SVGElement*> shapes;

   };

   // A Character is a tree of Joints, together with some additional information.
   class Character
   {
      public:
         // This spline determines the overall translation of the character.
         Spline<Vector2D> position;

         // The root of the tree.
         Joint* root;

         // In principal, the character could store only its root joint
         // and access its children via a tree traversal.  However, it
         // is often convenient to be able to simply iterate over a list
         // of children.
         vector<Joint*> joints;

         // Transformation of the character at the current time,
         // as computed by the last call to Character::update().
         Matrix3x3 currentTransformation;

         // Computes the joint transformations and joint center for the
         // specified time, storing these values in Joint::currentTransformation and
         // Joint::currentCenter, respectively.
         void update( double time );

         // For any joint whose motion is determined by dynamics rather than spline
         // animation, integrate() updates the dynamic variables theta and omega
         // via numerical integration using the given time step.
         void integrate( double time, double timestep );

         // draws the character using the specified renderer;
         // note that the layering of joints is determined by
         // a depth-first traversal of the tree (depth-first
         // ordering rather than breadth-first ordering preserves
         // the coherence of limbs, i.e., a whole arm occludes a
         // whole leg)
         void draw( SVGRenderer* renderer, bool pick = false, Joint* hovered = NULL, Joint* selected = NULL );

         // The method reachForTarget() optimizes all of the angles in this character
         // in order to bring a source point p on some joint as close as possible to the given
         // target point q.  The source point p is specified in the original coordinate system, i.e.,
         // before applying any joint transformations.  The target point q is specified as a
         // point in world coordinates.  Optimization should be performed by applying gradient
         // descent to the squared norm of the difference between the transformed position of p
         // and the world-space position of q.
         void reachForTarget( Joint* goalJoint,
               Vector2D sourcePoint, // source point p, expressed in the original coordinate system (i.e., before joint rotations and character translation)
               Vector2D targetPoint, // target point q, expressed in the world coordinate system (this is the mouse cursor position, so there is no notion of "before" and "after" transformation)
               double time );

         // Loads this character from an svg grouping representation.
         void load_from_SVG(SVG & svg);
   };
}

#endif // CHARACTER_H
