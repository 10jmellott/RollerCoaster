#Readme on RollerCoaster#
######https://github.com/kinikibu/RollerCoaster######

##information##
```
Programmer: Joshua Mellott-Lillie
      Date: 10/4/2012
   Program: Roller Coaster
```

##description##
```
You will create a
rollercoaster track along which the camera will move. You will include an
environment for your rollercoaster; this may be a simple skybox, or may also
include a terrain (using your heightmap code?) or other scenery. You may
use the GLUT toolkit for interaction with the mouse and keyboard to give
the user interactive control over rotation of the camera or other control over
the motion / view.
```

###completeness###
```
2. Environment: 100%
3. Appearance: 80%
4. File Types: 100%
5. Interpolation: 100%
6. Physics: 80%
7. Animation: 90%
```

###extra credit###
+ Support Columns have been added
+ Color scheme is planned
+ Also, I attempted to create my own personal track. I figured out, relatively intuitively
  that if the spline file numbers add up to 0, then the track is able to connect to itself.
  That being said I took some time and created a track with such properties. The only thing
  is that you ram your head against multiple support beams every so often. Attempting to 
  fix one beam only made another section just as dangerous so as of now, the ride will end
  up with a broken skull.
+ Anti-Aliased Tracks



##progress##
###Ver. 2.0###
Base Finished Product

3D Rail Added
Support Columns Added

Lighting Added
Anti-Aliasing Added

Physics system smoothed out so that the system will work for all tracks
Note: The physics system has yet to take into account the ```TRACK_DENSITY```

Track turning around with limited velocity was removed
+ this is due to the fact that live roller coasters do not just turn around
+ In deference to this, the system was changed s.t. the program will pause after each run

Code was used from https://github.com/curran/renderCyliner.git to manage the rotation for
the cylinders. The code has been altered for this project.

###Ver. 1.5###
Code has not been altered much. This submission won't be as visible in the code.
Instead I worked out the values for the rails(perpendicular rails) on paper.
Also I managed to sort out the basic problems with the velocity, which is 
a constant at this point.

###Ver. 1###
Code has been heavily reworked!

For the Track
+ Track is now in a display list
+ Track points have values stored in ```*track_view```
+ Catmull-Rom splines have been implemented
+ there are ```TRACK_DENSITY``` sub-points created with spline

Physics added
+ physics is not completed as of yet, still a few problems
+ track requires an initial velocity
+ track will turn around if it does not have enough velocity
+ Velocities at various points are recorded in ```*velocity```

Animation
+ Animation uses a focused ```glutTimerFunc()``` for its animation
+ The animation uses the vector ```track_view```
+ Animation is located in ```Timer(int iunused)```

###Ver. 0###
+ Starter code looked over and analyzed
+ Control system from Heightmap program has been transferred
+ Skybox system from Heightmap program has been transferred
