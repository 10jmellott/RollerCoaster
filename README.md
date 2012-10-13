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
Started
```

###extra credit###
+ None as of yet



##progress##
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
