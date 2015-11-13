![Animator](http://15462.courses.cs.cmu.edu/fall2015content/misc/asst4_images/Animator.jpg)

### Due Date
Mon Nov 23rd, 11:59pm

### Overview 
In this project you will build on your previous work with 2D rasterization to implement an application that allows you to manipulate and animate jointed cartoon characters.  When you are done, you will have a tool that allows you to produce animations that use three of the major techniques used in production computer animation: keyframing, numerical optimization, and physically-based simulation.  (You can create your own SVG characters in free software like [Inkscape](http://https://inkscape.org/en/), or commercial software like [Adobe Illustrator](http://www.adobe.com/Illustrator/); more details are given below about exactly how to construct one of these characters.)

### Getting started
We will be distributing assignments with git. You can find the repository for this assignment at <http://462cmu.github.io/asst4_animator/>. If you are unfamiliar with git, here is what you need to do to get the starter code:
```
$ git clone https://github.com/462cmu/asst4_animator.git
```
This will create a <i class="icon-folder"> </i> *asst4_animator* folder with all the source files. 

### Build Instructions
In order to ease the process of running on different platforms, we will be using [CMake](http://www.cmake.org "CMake Homepage") for our assignments. You will need a CMake installation of version 2.8+ to build the code for this assignment. The GHC 5xxx cluster machines have all the packages required to build the project. It should also be relatively easy to build the assignment and work locally on your OSX or Linux. Building on Windows is currently not supported.

If you are working on OS X and do not have CMake installed, we recommend installing it through [Macports](https://www.macports.org/):
```
sudo port install cmake
```
Or [Homebrew](http://brew.sh/):
```
brew install cmake
```

To build your code for this assignment:

- Create a directory to build your code:
```
$ cd asst4_animator && mkdir build && cd build
``` 
- Run CMake to generate makefile:
```
$ cmake ..
```
- Build your code:
```
$ make
```
- Install the executable (to asst4_animator/bin):
```
$ make install
```

### Using the Animator App

When you have successfully built your code, you will get an executable named **animator**. The **animator** executable takes exactly one argument from the command line, specifying either a single SVG character, or a directory containing multiple characters.  (Every SVG file in the directory must follow the character format, described below.) For example, to load the example file <i class="icon-file"> </i>*scenes/character.svg* from your build directory:

```
./meshedit ../scenes/character.svg
```

Likewise, to load the directory <i class="icon-file"> </i>*scenes/testscene/* you can run

```
./meshedit ../scenes/testscene
```

When you first run the application, you will see a collection of one or more characters, possibly overlapping.  The editor already supports some basic functionality like rotating the joints of characters, which you can do by (left-)clicking and dragging on the joint.  You can also translate the character by (left-)clicking and dragging on its root.  Eventually, you will be able to right-click and drag in order to modify the character configuration via inverse kinematics (IK).  As you move the cursor around the screen, you'll notice that the character joint under the cursor gets highlighted.  Clicking on this joint will display its attributes and current state.

![Animator GUI](http://15462.courses.cs.cmu.edu/fall2015content/misc/asst4_images/AnimatorGUIScreenshot.png)

In this assignment, you will add functionality to the program that allows you to give motion to the characters in a variety of ways.  There will be three basic types of animation:

1. Keyframed - At each point in time, the user can set the joint angles and translation for each of the characters.  These events are inserted into a spline data structure, that interpolates the values over time.  Your task will be to implement the interpolation routines, which return the interpolated value and its derivatives.
2. Inverse Kinematics (IK) - Setting every joint in a character is difficult and time consuming.  In the second part of the assignment you will implement a simple inverse kinematics scheme that allows the character configuration to be modified by dragging a point on the character toward a target point.  Your key tasks will be computing the gradient of the IK energy, and implementing a simple gradient descent scheme to update the joint angles.
3. Dynamics - Some of the characters have special "pendulum" joints whose motion will be dictated by physically-based simulation rather than keyframe interpolation.  Your job will be to evaluate the forces and integrate the equations of motion for a swinging pendulum.  Note that the axis of rotation for this pendulum may be driven by the motion of a keyframed joint; these additional forces also need to be accounted for in the dynamics so that, for instance, an image tossed around the scene will exhibit lively dynamics.

The user interface for each of these animation modes is described in the table below.  Notice that currently, nothing happens when the scene is animated - this is because you haven't yet implemented any of these modes of animation!  Unlike the previous assignment, no reference solution is provided.  However, we provide visual debugging feedback that should help determine if your solutions are correct.  We will also provide several examples (images and video) of correct interpolation, IK, and dynamics.

### Summary of Viewer Controls
A table of all the mouse and keyboard controls in the **animator** application is provided below.

| Command                              | Key                                                |
| ------------------------------------ |:--------------------------------------------------:|
| Rotate a joint                       | (left click and drag on joint)                     |
| Move a character                     | (left click and drag on root joint)                |
| Manipulate a joint via IK            | (right click and drag on a joint)                  |
| Delete current keyframe              | <kbd>BACKSPACE/DELETE</kbd>                        |
| Show/hide debugging information      | <kbd>d</kbd>                                       |
| Save animation to image sequence     | <kbd>s</kbd>                                       |
| Play the animation                   | <kbd>SPACE</kbd>                                   |
| Step timeline forward by one frame   | <kbd>RIGHT KEY</kbd>                               |
| Step timeline forward by ten frames  | <kbd>RIGHT KEY + SHIFT</kbd>                       |
| Step timeline back by one frame      | <kbd>LEFT KEY</kbd>                                |
| Step timeline back by ten frames     | <kbd>LEFT KEY + SHIFT</kbd>                        |
| Move to next keyframe in timeline    | <kbd>RIGHT KEY + ALT</kbd>                         |
| Move to next keyframe in timeline    | <kbd>LEFT KEY + ALT</kbd>                          |
| Move to beginning of timeline        | <kbd>UP KEY/HOME</kbd>                             |
| Move to end of timeline              | <kbd>DOWN KEY/END</kbd>                            |
| Make timeline shorter                | <kbd>[</kbd>                                       |
| Make timeline shorter                | <kbd>]</kbd>                                       |

The timeline can also be manipulated by clicking on the timeline itself.  Clicking and dragging will "scrub" through the animation; clicking on the buttons will play, pause, loop, etc.

### What You Need to Do

The assignment is divided into three parts:
* Keyframe animation
* Inverse kinematics
* Dynamics simulation

As with all of our assignments, this assignment __involves significant implementation effort.__ Also, be advised that there are some dependencies, e.g., IK and dynamics both depend on properly-working keyframe interpolation.  Therefore, you are highly advised to compete the interpolation tasks first, and to carefully consider the **correctness** of this code, since it will be used by subsequent tasks.

#### Getting Acquainted with the Starter Code 
Before you start, here are some basic information on the structure of the starter code.

Your work will be constrained to implementing part of the classes `Spline`, `Joint`, and `Character` in <i class="icon-file"> </i> *spline.h* and *character.cpp*.  The routines you need to implement will be clearly marked at the top of the file.  You should be able to implement all the necessary routines without adding additional members or methods to the classes.  You can also assume that a character has already been correctly loaded and built from an SVG file; you do not have to worry about malformed input.  (Of course, if you create your own malformed SVG files, you might run into trouble!)

#### The Character and Joint classes

![Joint diagram](http://15462.courses.cs.cmu.edu/fall2015content/misc/asst4_images/JointDiagram.jpg)

The main object you will be working with is a `Character`, which is a tree-like collection of `Joints` together with some additional information, such as its position as a function of time.  Each `Joint` is a node in the tree, which stores pointers to all of its children plus additional information encoding its center of rotation, and rotation angle as a function of time (relative to the parent).  Each joint also has a flag specifying whether its motion should be determined by keyframe interpolation, or by dynamics.

When working with characters, it is very important to understand how its state (positions, rotation centers, and angles) encodes its configuration.  In particular, let x_0(t) denote the translation of the character as a function of time, encoded by the member `Character::position`.  Let c_i denote the fixed center of rotation for joint i, encoded by the member `Character::center` (note that this value is __not__ the geometric center of the object; it is an arbitrary point that could be anywhere on the object---or even somewhere __off__the object!).  Finally, let theta_i(t) denote the angle of rotation for joint i as a function of time, encoded by either the member `Character::angle` (for keyframe interpolated joints), or `Character::theta` (for dynamic joints).  At any given time t, the rotation of joints and translation of characters will move each joint center to a new location p_i, which we will ultimately store in the member `Character::currentCenter`.  If joint j is the parent of joint i, then the current position of the root can be expressed as

   p_0 = c_0 + x_0(t),

and the current position of any (non-root) node i can be expressed as

   p_i = p_j + R(Theta_i(t)) ( c_i - c_j ) + x_0(t),

where R(s) denotes a counter-clockwise rotation by the angle s (in radians), and Theta_i(t) denotes the cumulative rotation from the root node all the way down to joint i (at time t).  In other words, to get the position of any joint, we start at the parent joint, and add the difference between the two joints' centers, rotated by all of the angles "above" the current joint.  Finally, we add the translation of the character itself.  The cumulative rotation can be written explicitly as

   Theta_i(t) = sum_k=0^{i-1} theta_k(t).

Consider what happens, for instance, when the root translation and all the angles are zero.  Then we have p_0 = c_0, and hence

   p_1 = p_0 + R(0) ( c_1 - c_0 ) = c_0 + c_1 - c_0 = c_1,
   p_2 = p_1 + R(0) ( c_2 - c_1 ) = c_1 + c_2 - c_1 = c_2,

and so forth.  In other words, we just recover the original joint centers.

In your code, you will not compute these cumulative rotations explicitly, but rather by recursively traversing the tree.  I.e., each joint will compute its transformation relative to its parent, and then pass this updated transformation to all of its children so that they can compute their own transformations.  This kind of hierarchical _scene graph_ is typical in many computer graphics applications (both 2D and 3D).

The `Character` and `Joint` classes have some additional members that will help you to compute their motion.  For instance, each joint stores its current transformation and the current transformation of its parent; it also stores values to encode the current angle and angular velocity for dynamical simulation.  You should take a close look at the comments in the header file `character.h` to get a more detailed understanding of how these classes work.

#### Task 1: Keyframe Interpolation

As we discussed in class, data points in time can be interpolated by constructing an approximating piecewise polynomial or _spline_.  In this assignment you will implement a particular kind of spline, called a _Catmull-Rom spline_.  A Catmull-Rom spline is a piecewise cubic spline defined purely in terms of the points it interpolates.  It is a popular choice in real animation systems, because the animator does not need to define additional data like tangents, etc.  (However, your code may still need to numerically evaluate these tangents after the fact; more on this point later.)  All of the methods relevant to spline interpolation can be found in `spline.h` with implementations in 'spline.inl'.

##### Task 1A: Hermite curve over the unit interval

Recall that a __cubic polynomial__ is a function of the form

   p(t) = at^3 + bt^2 + ct + d,

where a, b, c, and d are fixed coefficients.  However, there are many different ways of specifying a cubic polynomial.  In particular, rather than specifying the coefficients directly, we can specify the endpoints and tangents we wish to interpolate.  This construction is called the "Hermite form" of the polynomial.  In particular, the Hermite form is given by

   p(t) = h00(t)p0 + h10(t)m0 + h01(t)p1 + h11(t)m1,

where p0,p1 are the endpoint positions, m0,m1 are the endpoint tangents, and hij are the Hermite bases

   h00(t) = 2t^3 - 3t^2 + 1
   h10(t) = t3 - 2t^2 + t
   h01(t) = -2t^3 + 3t^2
   h11(t) = t3 - t2

Your first task is to implement the method `Spline::cubicSplineUnitInterval()`, which evaluates a spline over defined over the time interval [0,1] given a pair of endpoints and tangents at endpoints.  Optionally, the user can also specify that they want one of the time derivatives of the spline (1st or 2nd derivative), which will be needed for our dynamics calculations.

Your basic strategy for implementing this routine should be:
1. Evaluate the time, its square, and its cube.  (For readability, you may want to make a local copy of `normalizedTime` called simply `t`.)
2. Using these values, as well as the position and tangent values, compute the four basis functions h00, h01, h10, and h11 of a cubic polynomial in Hermite form.  Or, if the user has requested the nth derivative, evaluate the nth derivative of each of the bases.
3. Finally, combine the endpoint and tangent data using the evaluated bases, and return the result.

Notice that this function is __templated__ on a type `T`.  In C++, a templated class can operate on data of a variable type.  In the case of a spline, for instance, we want to be able to interpolate all sorts of data: angles, vectors, colors, etc.  So it wouldn't make sense to rewrite our spline class once for each of these types; instead, we use templates.  In terms of implementation, your code will look no different than if you were operating on a basic type (e.g., doubles).  However, the compiler will complain if you try to interpolate a type for which interpolation doesn't make sense!  For instance, if you tried to interpolate `Character` objects, the compiler would likely complain that there is no definition for the sum of two characters (via a + operator).  In general, our spline interpolation will only make sense for data that comes from a _vector space_, since we need to add T values and take scalar multiples.

##### Task 1B: Evaluation of a Catmull-Rom spline

Using the routine from part 1A, you will now implement the method `Spline::evaluate()` which evaluates a general Catmull-Rom spline (and possibly one of its derivatives) at the specified time.  Since we now know how to interpolate a pair of endpoints and tangents, the only task remaining is to find the interval closest to the query time, and evaluate its endpoints and tangents.

The basic idea behind Catmull-Rom is that for a given time t, we first find the four closest knots at times

   t0 < t1 <= t < t2 < t3.
   
We then use t1 and t2 as the endpoints of our cubic "piece," and for tangents we use the values

   m1 = ( p2 - p0 ) / 2,
   m2 = ( p3 - p1 ) / 2.

In other words, a reasonable guess for the tangent is given by the difference between neighboring points.  (See the Wikipedia and our course slides for more details.)

![Spline boundary](http://15462.courses.cs.cmu.edu/fall2015content/misc/asst4_images/SplineBoundary.jpg)

This scheme works great if we have two well-defined knots on either side of the query time t.  But what happens if we get a query time near the beginning or end of the spline?  Or what if the spline contains fewer than four knots?  We still have to somehow come up with a reasonable definition for the positions and tangents of the curve at these times.  For this assignment, your Catmull-Rom spline interpolation should satisfy the following properties:
* If there are no knots at all in the spline, interpolation should return the default value for the interpolated type.  This value can be computed by simply calling the constructor for the type: `T()`.  For instance, if the spline is interpolating `Vector2D` objects, then the default value will be (0,0).
* If there is only one knot in the spline, interpolation should always return the value of that knot (independent of the time).  In other words, we simply have a _constant_ interpolant.  (What, therefore, should we return for the 1st and 2nd derivatives?)
* If the query time is less than or equal to the initial knot, return the initial knot's value. (What do derivatives look like in this region?)
* If the query time is greater than or equal to the final knot, return the final knot's value. (What do derivatives look like in this region?)

Once we have two or more knots, interpolation can be handled using general-purpose code; we no longer have to consider individual cases (two knots, three knots, ...).  In particular, we can adopt the following "mirroring" strategy:
* Any query time between the first and last knot will have at least one knot "to the left" (k1) and one "to the right" (k2).
* Suppose we don't have a knot "two to the left" (k0).  Then we will define a "virtual" knot k0 = k1 - (k2-k1).  In other words, we will "mirror" the difference be observe between k1 and k2 to the other side of k1.
* Likewise, if we don't have a knot "two to the left" of t (k3), then we will "mirror" the difference to get a "virtual" knot k3 = k2 + (k2-k1).
* At this point, we have four valid knot values (whether "real" or "virtual"), and can compute our tangents and positions as usual.
* These values are then handed off to our subroutine that computes cubic interpolation over the unit interval.

An important thing to keep in mind is that `Spline::cubicSplineUnitInterval()` assumes that the time value t is between 0 and 1, whereas the distance between two knots on our Catmull-Rom spline can be arbitrary.  Therefore, when calling this subroutine you will have to normalize t such that it is between 0 and 1, i.e., you will have to divide by the length of the current interval over which you are interpolating.  You should think very carefully about how this normalization affects the value and derivatives computed by the subroutine, in comparison to the values and derivatives we want to return.

Internally, a `Spline` object stores its data in an STL `map` that maps knot times to knot values.  A nice thing about an STL `map` is that it automatically keeps knots in sorted order.  Therefore, we can quickly access the knot _closest_ to a given time using the method `map::upper_bound()`, which returns an iterator to knot with the smallest time greater than the given query time (you can find out more about this method via online documentation for the Standard Template Library).

##### Trying it out

As soon as spline animation is working, you should be able to edit and playback animations in the timeline.  The first thing you should try is translating a character along a spline path by dragging the root node of the character to different locations in the scene at different points in time (the root node is usually something like the character's torso).  (A good example is `pencil.svg`.)  If position interpolation is working properly, you should see a white curve interpolating the points of translation.  If derivative interpolation is working, you should also see blue arrows visualizing the velocity, and red arrows visualizing the acceleration.  Compare with the example below.

![Catmull Rom curve drawn with a pencil](http://15462.courses.cs.cmu.edu/fall2015content/misc/asst4_images/CatmullRomPencil.jpg)

Now make something fun!

__Possible Extra Credit Extensions:__ 

* There are a variety of variants on the Catmull-Rom spline that are used in computer animation such as the [Centripetal Catmull-Rom spline](https://en.wikipedia.org/wiki/Centripetal_Catmull–Rom_spline).  Find and implement different types of spline interpolation that are well-suited to our 2D keyframe animation, and provide some justification for why they should improve the motion or the ease of generating an animation.  Produce animations that demonstrate the pros and cons of the different interpolation schemes.  (Another interesting example are the so-called [wiggly splines](http://graphics.pixar.com/library/WigglySplinesA/).)

#### Task 2: Inverse Kinematics

After playing with spline interpolation a bit, you may have noticed that it takes a lot of work to manipulate characters into the desired pose, especially if they have many joints (check out `character.svg`, for instance).  In this task you will implement a basic inverse kinematics (IK) solver that optimizes joint angles in order to meet a given target.  In particular, it will try to match a selected point on the character to the current position of the cursor.

Above, we already described how positions in our kinematic chain are determined from the current angles theta_i.  Our IK algorithm will try to adjust these angles so that our character "reaches for" the cursor.  In particular, suppose the user clicks on a point p on joint i, then moves the cursor to a point q on the screen.  We want to minimize the objective

   f_0(theta) = | p(theta) - q |^2

with respect to the joint angles theta.  To keep things simple, we're going to adopt the following convention:

   _The source position p is expressed with respect to the original coordinate system of the joint, i.e., before its parent character translation and joint rotations are applied._

This way, p(theta) is the position of the source point after transformation by theta (and the character center).  For instance, if p were the joint center, then p(theta) would just be equal to p_i(theta), i.e., the transformed joint center.

#### Task 2A: Evaluate the gradient

The question now is: what is the derivative of f_0 with respect to one of the angles theta_j?  Of course, we only have to consider angles theta_j "further up the chain" from joint i, because angles below joint i (or angles on other branches of the tree) don't affect the position of the transformed source point.  Suppose, then, that joint j is an ancestor of joint i.  Then the derivative of f_0 with respect to theta_j is given by

   < q-p(theta), u/|u|^2 >,

where <.,.> denotes the usual dot product and u is the vector

   R(pi/2)( p(theta) - p_j(theta) ),

i.e., we take the difference between the current source point and the current location of the jth joint, rotated by 90 degrees.  (These expressions may be hard to look at on GitHub; we will also provide a PDF version at a later date.)

This formula will be evaluated in the method `Joint::calculateAngleGradient()` found in the 'character.h' file. This method takes three parameters:
* a pointer to the joint i that contains the source point,
* the current position p(theta) of the source point (i.e., the transformed click point), and
* the current position q of the target point (i.e., the mouse cursor).
It should compute the gradient of f_0 with respect to joint's angle and store it in the member `Joint::ikAngleGradient`.  It should also recursively evaluate the gradient for all children, i.e., it should call `calculateAngleGradient()` for each of the "kids."  However, you should be sure _not_ to set an angle gradient for any joint that doesn't affect the source point; the angle gradient for all other joints must be set to zero.  To aid in this process, you can use the boolean return value of `Joint::calculateAngleGradient()` to help determine whether the current branch is connected to the joint containing the source point.

Once you have implemented this method, you will be able to see a visualization of the angle gradients by right-clicking and dragging on a joint.  For instance, if you load up the example file `scenes/testscene/telescope.svg`, your implementation should match the gradients visualized below; here the red dot is the initial click point, and the blue dot is the position of the cursor.  However, the configuration of the character should not change (yet).  

![IK angle gradients on the telescope](http://15462.courses.cs.cmu.edu/fall2015content/misc/asst4_images/IKGradientTelescope.jpg)

#### Task 2B: Ski downhill

From here, there's not much more to do: we simply need to "ski downhill" (i.e., apply gradient descent) using the gradient we computed in part 2B.  In particular, you will need to implement the method `Character::reachForTarget()`, which takes three parameters:
* a pointer to the joint containing the click point,
* the location of the un-transformed source point (i.e., without the character translation or joint rotations applied),
* the target point (i.e., the location of the cursor), and
* the current animation time.

This method will need to do several things:
* First, it needs to call `update()`, which computes the current configuration of each joint.  (These values are used by your gradient subroutine, therefore they need to be updated for the current animation time.)
* Also, it needs to compute the angle gradient for every joint.  Joints that do not influence the objective should have zero gradient.  **IMPORTANTLY** you must transform the source point into its current configuration before passing it to `Joint::calculateGradient()`; the joint's current transformation is stored in the member `Joint::currentTransformation`.
* Finally, it needs to advance the joint angles via gradient descent.  This can be done by iterating over all joints and adding a little bit of the angle derivative to the current angle (forward Euler).

How do you pick the size of the time step?  For this assignment, we will just use a "very small" time step (the reference implementation uses a time step of tau = 0.1, but feel free to experiment).  Also, to get snappier feedback, you can take multiple gradient descent steps per call to `Character::reachForTarget()` (we used 10... but please experiment so you get a sense of how this all works!).  When things are working properly, the telescope should bend as in the picture below.  (Now try generating some animation, using your new IK tool to accelerate the process!)

![Telescope under the infuence of IK](http://15462.courses.cs.cmu.edu/fall2015content/misc/asst4_images/TelescopeBent.jpg)

#### Task 3: Secondary Dynamics

(Details forthcoming.)

#### Task 4: Get Creative!

Now that you have an awesome animation system, make an awesome animation!  You can save your animation to an image sequence by hitting 's' in the viewer, and compress it into a movie using the script in the p4 directory.  In "getting creative," you should also try creating your own characters, either using a program like Inkscape or by just creating your own SVG file by hand (see below for more details).  Have fun!

### Grading

Your code must run on the GHC 5xxxx cluster machines as we will grade on those machines. Do not wait until the submission deadline to test your code on the cluster machines. Keep in mind that there is no perfect way to run on arbitrary platforms. If you experience trouble building on your computer, while the staff may be able to help, but the GHC 5xxx machines will always work and we recommend you work on them.

Each task will be graded on the basis of correctness, except for Task 7 which will be graded (gently) on effort.  You are not expected to completely reproduce the reference solution vertex-for-vertex as slight differences in implementation strategy or even the order of floating point arithmetic will causes differences, but your solution should not be very far off from the provided input/output pairs.  If you have any questions about whether your implementation is "sufficiently correct", just ask.

The assignment consists of a total of 100 pts. The point breakdown is as follows:

* Task 1:  14
* Task 2:  14 
* Task 3:  14
* Task 4:  17
* Task 5:  17
* Task 6:  17
* Task 7:  7

### Handin Instructions


Your handin directory is on AFS under:
```
/afs/cs/academic/class/15462-f15-users/ANDREWID/asst4/
```
Note that you may need to create the `asst4` directory yourself. All your files should be placed there. Please make sure you have a directory and are able to write to it well before the deadline; we are not responsible if you wait until 10 minutes before the deadline and run into trouble. Also, you may need to run `aklog cs.cmu.edu` after you login in order to read from/write to your submission directory.

You should submit all files needed to build your project, this include:

* The `src` folder with all your source files

Please do not include:

* The`build` folder
* Executables
* Any additional binary or intermediate files generated in the build process.

You should include a `README` file (plaintext or pdf) if you have implemented any of the extra credit features. In your `REAME` file, clearly indicate which extra credit features you have implemented. You should also briefly state anything that you think the grader should be aware of.

Do not add levels of indirection when submitting. And please use the same arrangement as the handout. We will enter your handin directory, and run:
```
mkdir build && cd build && cmake .. && make
```
and your code should build correctly. The code must compile and run on the GHC 5xxx cluster machines. Be sure to check to make sure you submit all files and that your code builds correctly.


### Friendly Advice from your TAs
* As always, start early.  There is a lot to implement in this assignment, and no official checkpoint, so don't fall behind!
 
* Be careful with memory allocation, as too many or too frequent heap allocations will severely degrade performance.

* Make sure you have a submission directory that you can write to as soon as possible. Notify course staff if this is not the case.

* While C has many pitfalls, C++ introduces even more wonderful ways to shoot yourself in the foot. It is generally wise to stay away from as many features as possible, and make sure you fully understand the features you do use.

### Rigged Character Definitions

Specially-constructed SVG files are used to define used to describe a kinematic chain that is a hierarchical representation of a character.  The _grouping_ and _layering_ of objects is used to determine the hierarchy and dynamic behavior of the character.  In particular:

* A __shape__ is an elementary SVG shape; currently-supported shapes include: circle, ellipse, rectangle, line segment, polyline, and polygon.
* Each __joint__ consists of a single group of __shapes__, where the last shape in the group _must_ be a circle.  This list must be "flat," i.e., it cannot be a group of groups.  The circle specifies the center of rotation for the joint (its radius is ignored).  The fill color of the joint determines its motion type: black for keyframed joints, white for dynamic joints.
* A __character__ is a collection of hierarchically grouped __joints__.  The grouping determines a dependency tree among __joints__; in particular, in any group of __joints__ the first __joint__ is the parent, and all remaining __joints__ are children.  All __joints__ in a file must ultimately be grouped into a single __character__; there is no support for multiple characters in a single file.

_Example_: Suppose we want to create a character consisting of a shoulder A, an arm B, a hand C, and three fingers x, y, and z.  Each of A, B, C, x, y, and z must be __joints__, i.e., they must each be a _flat_ list of shapes, including a circle at the very end specifying the center of rotation.  For instance, one of the fingers might look like a = (ellipse,ellipse,ellipse,circle), consisting of three finger joints and an axis of rotation.  In this case, the three finger joints would be fixed relative to each other; only the whole finger would be able to rotate.  To construct the __character__, we would then construct the group (x,y,z,C), which attaches the five fingers to the hand C.  We would then put this group into a group ((x,y,z,C),B) attaching the hand to the arm, and finally (((x,y,z,C),B),A) attaching the arm to the shoulder.  Here is the corresponding SVG code; note that comments (beginning with !--) are ignored:

``` xml
<svg>
   <g> <!-- CHARACTER: jointed arm with fingers -->
      <g> <!-- SHAPE: shoulder (C) -->
         <ellipse cx="40" cy="340" rx="40" ry="50"/>
         <circle cx="40" cy="380" r="1" fill="#000000" /> <!-- center of rotation -->
      </g>
      <g>
         <g> <!-- SHAPE: arm (B) -->
            <rect x="0" y="140" width="80" height="200"/>
            <circle cx="40" cy="340" r="1" fill="#000000" /> <!-- center of rotation -->
         </g>
         <g>
            <g> <!-- SHAPE: hand (C) -->
               <rect x="0" y="60" width="80" height="80"/>
               <circle cx="40" cy="140" r="1" fill="#000000" /> <!-- center of rotation -->
            </g>
            <g> <!-- SHAPE: finger (x) -->
               <ellipse cx="0" cy="0" rx="5" ry="20"/>
               <ellipse cx="0" cy="20" rx="5" ry="20"/>
               <ellipse cx="0" cy="40" rx="5" ry="20"/>
               <circle cx="0" cy="60" r="1" fill="#000000" /> <!-- center of rotation -->
            </g>
            <g> <!-- SHAPE: finger (y) -->
               <ellipse cx="40" cy="0" rx="5" ry="20"/>
               <ellipse cx="40" cy="20" rx="5" ry="20"/>
               <ellipse cx="40" cy="40" rx="5" ry="20"/>
               <circle cx="40" cy="60" r="1" fill="#000000" /> <!-- center of rotation -->
            </g>
            <g> <!-- SHAPE: finger (z) -->
               <ellipse cx="80" cy="0" rx="5" ry="20"/>
               <ellipse cx="80" cy="20" rx="5" ry="20"/>
               <ellipse cx="80" cy="40" rx="5" ry="20"/>
               <circle cx="80" cy="60" r="1" fill="#000000" /> <!-- center of rotation -->
            </g>
         </g>
      </g>
   </g>
</svg>
```

Note that in most SVG editors the topmost shape in the drawing is saved as the last element in the SVG group.  In general one must be very careful that each joint cointains a circle as its last (topmost) element, and that grouping and layering are done in the proper order.  A good way to "debug" a character file is to try out small pieces of the character joint by joint.

### Using p1 Rasterization Routines.

Those so inclined are welcome to try replacing the opengl rasterization calls in 'hardware_renderer.cpp' with their own software implementations, much like the those implemented in project 1.

### Resources and Notes

* [Bryce's C++ Programming Guide](https://github.com/Bryce-Summers/Writings/blob/master/Programming%20Guides/C_plus_plus_guide.pdf)
