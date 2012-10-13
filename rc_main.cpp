#ifdef WIN32
// For VC++ you need to include this file as glut.h and gl.h refer to it
#include <windows.h>
#endif

#include <stdio.h>     // Standard Header For Most Programs
#include <stdlib.h>    // Additional standard Functions (exit() for example)
#include <math.h>

#ifdef __APPLE__

// This ONLY APPLIES to OSX
//
// Remember to add the -framework OpenGL - framework GLUT to
// to the gcc command line or include those frameworks
// using Xcode

#include <OpenGL/gl.h>     // The GL Header File
#include <OpenGL/glu.h>    // The GL Utilities (GLU) header
#include <GLUT/glut.h>   // The GL Utility Toolkit (Glut) Header

#else
#include <GL/gl.h>     // The GL Header File
#include <GL/glu.h>    // The GL Utilities (GLU) header
#include <GL/glut.h>   // The GL Utility Toolkit (Glut) Header
#endif

// Interface to libpicio, provides functions to load/save jpeg files
#include <pic.h>
#include "rc_spline.h"

/* Here we will load the spline that represents the track */
rc_Spline g_Track;

/* glut Id for the context menu */
int g_iMenuId;

/* State of the mouse */
int mousePos[2] = {0, 0};
int leftMouseButtonState = 0;    /* 1 if pressed, 0 if not */
int middleMouseButtonState = 0;
int rightMouseButtonState = 0;

/* Current control mode the program is in (what will happen when the mouse is moved */
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE currentControlState = ROTATE;

/* transformation variables */
GLfloat currentRotation[3] = {0.0, 0.0, 0.0};
GLfloat currentTranslation[3] = {0.0, 0.0, 0.0};
GLfloat currentScaling[3] = {1.0, 1.0, 1.0};

#define PI 3.14159265359
GLfloat currentViewAngle[2] = {0.0, PI / 2};
GLfloat currentView[3] = {0.0, 0.0, 0.0};

// if the skybox is to be scaled
#define SCALE_SKYBOX 0

#define DRAW_SKYBOX 0
#define DRAW_TRACK 1

// defined to remove the black edges along the textures
#define GL_CLAMP_TO_EDGE 0x812F

/* Texture ids */
GLuint frontTextureId;
GLuint leftTextureId;
GLuint backTextureId;
GLuint rightTextureId;
GLuint topTextureId;
GLuint bottomTextureId;


#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

/* OpenGL callback declarations 
definitions are at the end to avoid clutter */

/* This function takes the name of a jpg file and a texture ID (by reference)
* It creates a texture from the image specified and sets the ID specified to the value OpenGL generates for that texture */
void loadTexture (char *filename, GLuint &textureID);

// initGL will perform the one time initialization by
// setting some state variables that are not going to
// be changed
void InitGL ( GLvoid );

/* This function is called by GL whenever it is idle, usually after calling
* display.
*/
void doIdle();

/* Main GL display loop.
* This function is called by GL whenever it wants to update the display.
* We will assume that the resulting image has already been created, and 
we will just paint it to the display.
*/
void display ( void );

/* This function will be called by GL whenever a keyboard key is pressed.
* It recieves the ASCII value of the key that was pressed, along with the x
* and y coordinates of the mouse at the time the key was pressed.
*/
void keyboardfunc (unsigned char key, int x, int y) ;

/* Function that GL runs once a menu selection has been made.
* This receives the number of the menu item that was selected
*/
void menufunc(int value);

/* sets the reshape callback for the current window */
void reshape(int w, int h);	

/* converts mouse drags into information about rotation/translation/scaling
* This is run by GL whenever the mouse is moved and a mouse button is being
* held down. */
void mousedrag(int x, int y);

/* This function is called by GL when the mouse moves passively.
* "Passively" means that no mouse button is being held down */
void mouseidle(int x, int y);

/* This is called by GL whenever a mouse button is pressed. */
void mousebutton(int button, int state, int x, int y);

/* this will draw the skybox */
GLvoid drawSkybox();

/* this will draw the track */
GLvoid drawTrack(float initx, float inity, float initz, int regpoints);
#define TRACK_DENSITY 60			// number of points registered between control points
#define TRACK_WIDTH 0.5
GLuint track;
GLuint max_point;
Vec3f *track_view = NULL;
GLfloat *velocity = NULL;
GLuint track_point = 0;
GLuint track_dir = 1;			// 1 is forward, -1 is backward

/* this function is to be used to animate the roller coaster's movement */
GLvoid Timer(int iunused);

// The ubiquituous main function.
int main ( int argc, char** argv )   // Create Main Function For Bringing It All Together
{
	/* load the track, this routine aborts if it fails */
	g_Track.loadSplineFrom("track");



	/*** The following are commands for setting up GL      ***/
	/*** No OpenGl call should be before this sequence!!!! ***/

	/* Initialize glut */
	glutInit(&argc,argv);

	/* Set up window modes */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	/* Set window position (where it will appear when it opens) */
	glutInitWindowPosition(0,0);
	/* Set size of window */
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	/* Create the window */
	glutCreateWindow    ( "CMPSC 458: Rollercoaster" );

	/**** Call to our initialization routine****/
	InitGL ();

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
	/*sets the reshape callback for the current window*/
	glutReshapeFunc(reshape);
	/* Set idle function.  You can change this to call code for your animation,
	* or place your animation code in doIdle */
	glutIdleFunc(doIdle);

	/* callback for keyboard input */
	glutKeyboardFunc(keyboardfunc);

	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);

	/** allow the user to quit using the right mouse button menu **/
	/* Set menu function callback */
	g_iMenuId = glutCreateMenu(menufunc);
	/* Set identifier for menu */
	glutSetMenu(g_iMenuId);
	/* Add quit option to menu */
	glutAddMenuEntry("Quit",0);
	/* Attach menu to right button clicks */
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	Timer(0);				// this is the animation trigger

	glutMainLoop ( );       // Initialize The Main Loop

	free(track_view);

	return 0;
}

/* Here are all the standard OpenGL callbacks */

void loadTexture (char *filename, GLuint &textureID)
{
	Pic *pBitMap = pic_read(filename, NULL);

	if(pBitMap == NULL)
	{
		printf("Could not load the texture file\n");
		exit(1);
	}

	glEnable(GL_TEXTURE_2D); // Enable texturing
	glGenTextures(1, &textureID); // Obtain an id for the texture

	glBindTexture(GL_TEXTURE_2D, textureID); // Set as the current texture

	/* set some texture parameters */
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	/* Load the texture into OpenGL as a mipmap. !!! This is a very important step !!! */
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, pBitMap->nx, pBitMap->ny, GL_RGB, GL_UNSIGNED_BYTE, pBitMap->pix);
	glDisable(GL_TEXTURE_2D);
	pic_free(pBitMap); // now that the texture has been copied by OpenGL we can free our copy
}

void InitGL ( GLvoid )     // Create Some Everyday Functions
{

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background

	// load textures
	loadTexture("texture/sky25/front.jpg", frontTextureId);
	loadTexture("texture/sky25/left.jpg", leftTextureId);
	loadTexture("texture/sky25/back.jpg", backTextureId);
	loadTexture("texture/sky25/right.jpg", rightTextureId);
	loadTexture("texture/sky25/up.jpg", topTextureId);
	loadTexture("texture/sky25/down.jpg", bottomTextureId);

	glPointSize(3.0);

	track = glGenLists(1);
	glNewList(track, GL_COMPILE);
	drawTrack(-TRACK_WIDTH, 0, 0, 0);
	drawTrack(0, 0, 0, 1);
	drawTrack(TRACK_WIDTH, 0, 0, 0);
	glEndList();
}

void reshape(int w, int h)
{
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, aspect, 1, 500.0);
	glMatrixMode(GL_MODELVIEW);
}

void display ( void )   // Create The Display Function
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* set view of the camera, using polar coordinates of a sphere */

	currentView[0] = cos(currentViewAngle[0]) * sin(currentViewAngle[1]);
	currentView[1] = sin(currentViewAngle[0]) * sin(currentViewAngle[1]);
	currentView[2] = cos(currentViewAngle[1]);

	gluLookAt(currentTranslation[0], currentTranslation[1], currentTranslation[2], currentView[0] + currentTranslation[0], currentView[1] + currentTranslation[1], currentView[2] + currentTranslation[2], 0, 0, 1);
	glScalef(currentScaling[0], currentScaling[1], currentScaling[2]);

	/* Clear buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* draw skybox */

	if(DRAW_SKYBOX)
		drawSkybox();	

	if(DRAW_TRACK)
		glCallList(track);

	/* Swap buffers, so one we just drew is displayed */
	glutSwapBuffers();
}

void keyboardfunc (unsigned char key, int x, int y) {

	/* User pressed quit key */
	if(key == 'q' || key == 'Q' || key == 27) {
		exit(0);
	}
}

void menufunc(int value)
{
	switch (value)
	{
	case 0:
		exit(0);
		break;
	}
}

void doIdle()
{
	/* do some stuff... */

	/* make the screen update. */
	glutPostRedisplay();
}

void mousedrag(int x, int y)
{
	int mousePosChange[2] = {x-mousePos[0], y-mousePos[1]};

	/* Check which state we are in. */
	switch (currentControlState)
	{
		/*
	case TRANSLATE:
		if (leftMouseButtonState)
		{
			float theta = PI - currentViewAngle[0];

			// move mouse left or right
			currentTranslation[0] -= (mousePosChange[0] * sin(theta)) * 0.01;
			currentTranslation[1] -= (mousePosChange[0] * cos(theta)) * 0.01;

			theta = currentViewAngle[0];

			// move mouse up or down
			currentTranslation[0] += (mousePosChange[1] * cos(theta)) * 0.01;
			currentTranslation[1] += (mousePosChange[1] * sin(theta)) * 0.01;
		}
		if (middleMouseButtonState)
		{
			currentTranslation[2] += mousePosChange[1]*0.01;
		}
		break;
		*/
	case ROTATE:
		if (leftMouseButtonState)
		{
			currentViewAngle[0] -= mousePosChange[0]*0.004;
			currentViewAngle[1] += mousePosChange[1]*0.004;
		}
		if (middleMouseButtonState)
		{
			currentViewAngle[1] += mousePosChange[1]*0.005;
		}

		// limits the view angles
		if(currentViewAngle[1] > PI)
			currentViewAngle[1] = PI - .0001;
		if(currentViewAngle[1] < 0)
			currentViewAngle[1] = .0001;

		break;
	case SCALE:
		if (leftMouseButtonState)
		{
			currentScaling[0] *= 1.0+mousePosChange[0]*0.001;
			currentScaling[1] *= 1.0-mousePosChange[1]*0.001;
		}
		if (middleMouseButtonState)
		{
			currentScaling[2] *= 1.0-mousePosChange[1]*0.001;
		}
		break;
	}

	/* Update stored mouse position */
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseidle(int x, int y)
{
	/* Update mouse position */
	mousePos[0] = x;
	mousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

	/* Check which button was pressed and update stored mouse state */
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		leftMouseButtonState = (state==GLUT_DOWN);
		break;
	case GLUT_MIDDLE_BUTTON:
		middleMouseButtonState = (state==GLUT_DOWN);
		break;
	case GLUT_RIGHT_BUTTON:
		rightMouseButtonState = (state==GLUT_DOWN);
		break;
	}

	/* Check what modifier keys (shift, ctrl, etc) are pressed and update the
	* control mode based off of which keys are pressed */
	switch(glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		currentControlState = TRANSLATE;
		break;
	case GLUT_ACTIVE_SHIFT:
		currentControlState = SCALE;
		break;
	default:
		currentControlState = ROTATE;
		break;
	}

	/* Update stored mouse position */
	mousePos[0] = x;
	mousePos[1] = y;
}

GLvoid drawSkybox()
{
	// Store the current matrix
	glPushMatrix();

	// Reset and transform the matrix.
	glLoadIdentity();

	// sets the viewing angle to the values calculated in display()
	gluLookAt(0, 0, 0, currentView[0], currentView[1], currentView[2], 0, 0, 1);

	// Enable/Disable features
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	if(SCALE_SKYBOX)
		glScalef(currentScaling[0], currentScaling[1], currentScaling[1]);

	/* Texture coordinates */
	float t1[] = {1.0, 1.0};
	float t2[] = {1.0, 0.0};
	float t3[] = {0.0, 0.0};
	float t4[] = {0.0, 1.0};

	/* vertices of the cube */
	float v1[] = { 1.0, -1.0, -1.0};
	float v2[] = { 1.0, -1.0,  1.0};
	float v3[] = { 1.0,  1.0,  1.0};
	float v4[] = { 1.0,  1.0, -1.0};
	float v5[] = {-1.0, -1.0, -1.0};
	float v6[] = {-1.0, -1.0,  1.0};
	float v7[] = {-1.0,  1.0,  1.0};
	float v8[] = {-1.0,  1.0, -1.0};

	glEnable(GL_TEXTURE_2D); // Enable texturing from now on

	glColor3f(1.0, 1.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, frontTextureId); 
	glBegin(GL_QUADS);
	glTexCoord2fv(t1);
	glVertex3fv(v4);

	glTexCoord2fv(t2);
	glVertex3fv(v3);

	glTexCoord2fv(t3);
	glVertex3fv(v2);

	glTexCoord2fv(t4);
	glVertex3fv(v1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, rightTextureId); 
	glBegin(GL_QUADS);
	glTexCoord2fv(t1);
	glVertex3fv(v1);

	glTexCoord2fv(t2);
	glVertex3fv(v2);

	glTexCoord2fv(t3);
	glVertex3fv(v6);

	glTexCoord2fv(t4);
	glVertex3fv(v5);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, backTextureId); 
	glBegin(GL_QUADS);
	glTexCoord2fv(t1);
	glVertex3fv(v5);

	glTexCoord2fv(t2);
	glVertex3fv(v6);

	glTexCoord2fv(t3);
	glVertex3fv(v7);

	glTexCoord2fv(t4);
	glVertex3fv(v8);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, leftTextureId); 
	glBegin(GL_QUADS);
	glTexCoord2fv(t1);
	glVertex3fv(v8);

	glTexCoord2fv(t2);
	glVertex3fv(v7);

	glTexCoord2fv(t3);
	glVertex3fv(v3);

	glTexCoord2fv(t4);
	glVertex3fv(v4);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, topTextureId); 
	glBegin(GL_QUADS);
	glTexCoord2fv(t1);
	glVertex3fv(v2);

	glTexCoord2fv(t2);
	glVertex3fv(v3);

	glTexCoord2fv(t3);
	glVertex3fv(v7);

	glTexCoord2fv(t4);
	glVertex3fv(v6);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, bottomTextureId); 
	glBegin(GL_QUADS);
	glTexCoord2fv(t1);
	glVertex3fv(v4);

	glTexCoord2fv(t2);
	glVertex3fv(v1);

	glTexCoord2fv(t3);
	glVertex3fv(v5);

	glTexCoord2fv(t4);
	glVertex3fv(v8);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	// Restore enable bits and matrix
	glPopAttrib();
	glPopMatrix();
}

GLvoid drawTrack(float initx, float inity, float initz, int regpoints)
{
	/*
	{{1, u, u^2, u^3}}*{{0, 1, 0, 0}, {-t, 0, t, 0}, {2t, t-3, 3-2t, -t}, {-t, 2-t, t-2, t}} =>
	{{
	-(t u) + 2 t u^2 - t u^3,
	1 + (-3 + t) u^2 + (2 - t) u^3,
	t u + (3 - 2 t) u^2 + (-2 + t) u^3,
	-(t u^2) + t u^3
	}}
	*{{A1, A2, A3}, {B1, B2, B3}, {C1, C2, C3}, {D1, D2, D3}} =>
	Catmull-Rom Spline => with the help of Wolfram Alpha
	{{  
	B1 (1 + (-3 + t) u^2 + (2 - t) u^3) + C1 (t u + (3 - 2 t) u^2 + (-2 + t) u^3) + A1 (-(t u) + 2 t u^2 - t u^3) + D1 (-(t u^2) + t u^3),
	B2 (1 + (-3 + t) u^2 + (2 - t) u^3) + C2 (t u + (3 - 2 t) u^2 + (-2 + t) u^3) + A2 (-(t u) + 2 t u^2 - t u^3) + D2 (-(t u^2) + t u^3),
	B3 (1 + (-3 + t) u^2 + (2 - t) u^3) + C3 (t u + (3 - 2 t) u^2 + (-2 + t) u^3) + A3 (-(t u) + 2 t u^2 - t u^3) + D3 (-(t u^2) + t u^3)}}

	Prototype Call
	Vec3f mov(
		B.x() * (1 + (-3 + t) * pow(u, 2) + (2 - t) * pow(u, 3)) + C.x() * (t * u + (3 - 2 * t) * pow(u, 2) + (-2 + t) * pow(u, 3)) + 
		A.x() * (-(t * u) + 2 * t * pow(u, 2) - t * pow(u, 3)) + D.x() * (-(t * pow(u, 2)) + t * pow(u, 3)),
		B.y() * (1 + (-3 + t) * pow(u, 2) + (2 - t) * pow(u, 3)) + C.y() * (t * u + (3 - 2 * t) * pow(u, 2) + (-2 + t) * pow(u, 3)) + 
		A.y() * (-(t * u) + 2 * t * pow(u, 2) - t * pow(u, 3)) + D.y() * (-(t * pow(u, 2)) + t * pow(u, 3)),
		B.z() * (1 + (-3 + t) * pow(u, 2) + (2 - t) * pow(u, 3)) + C.z() * (t * u + (3 - 2 * t) * pow(u, 2) + (-2 + t) * pow(u, 3)) + 
		A.z() * (-(t * u) + 2 * t * pow(u, 2) - t * pow(u, 3)) + D.z() * (-(t * pow(u, 2)) + t * pow(u, 3))
		);
	
	*/
 
	if(regpoints)
	{
		int track_len = 0;

		for(pointVectorIter ptsiter = g_Track.points().begin(); ptsiter  !=  g_Track.points().end(); ptsiter++)
			track_len++;

		max_point = (TRACK_DENSITY + 1) * track_len;
		track_view = (Vec3f *)calloc(max_point, sizeof(Vec3f));
		velocity = (GLfloat *)calloc(max_point + 1, sizeof(GLfloat));

		int track_point = 0;

		velocity[track_point] = 10;				// initial speed
	}
	

	

 	glPointSize(3.0);
	glBegin(GL_POINTS);
	
	Vec3f A, B, C, D;
	Vec3f curpos(initx, inity, initz);							// initial position of the track rail
	pointVectorIter ptsiter2 = g_Track.points().begin();
	pointVectorIter ptsIter3 = g_Track.points().end();
	ptsIter3--;

	GLfloat t = 0.5;
	GLfloat u;

	

	for(pointVectorIter ptsiter = g_Track.points().begin(); ptsiter  !=  g_Track.points().end(); ptsiter++)
	{
		/* denotes the control points at any given iteration */
		if(ptsiter+1 == g_Track.points().end())
		{
			A = *(ptsiter + 0) + curpos;
			B = *(ptsiter2 + 0) + A;
			C = *(ptsiter2 + 1) + B;
			D = *(ptsiter2 + 2) + C;
		}
		else if(ptsiter+2 == g_Track.points().end())
		{
			A = *(ptsiter + 0) + curpos;
			B = *(ptsiter + 1) + A;
			C = *(ptsiter2 + 0) + B;
			D = *(ptsiter2 + 1) + C;
		}
		else if(ptsiter+3 == g_Track.points().end())
		{
			A = *(ptsiter + 0) + curpos;
			B = *(ptsiter + 1) + A;
			C = *(ptsiter + 2) + B;
			D = *(ptsiter2 + 0) + C;
		}
		else
		{
			A = *(ptsiter + 0) + curpos;
			B = *(ptsiter + 1) + A;
			C = *(ptsiter + 2) + B;
			D = *(ptsiter + 3) + C;
		}

		for(int i = 0; i <= TRACK_DENSITY; i++)
		{
			u = (GLfloat)(i) / (GLfloat)(TRACK_DENSITY);
			
			Vec3f point(
				B.x() * (1 + (-3 + t) * pow(u, 2) + (2 - t) * pow(u, 3)) + 
				C.x() * (t * u + (3 - 2 * t) * pow(u, 2) + (-2 + t) * pow(u, 3)) + 
				A.x() * (-(t * u) + 2 * t * pow(u, 2) - t * pow(u, 3)) + 
				D.x() * (-(t * pow(u, 2)) + t * pow(u, 3)),

				B.y() * (1 + (-3 + t) * pow(u, 2) + (2 - t) * pow(u, 3)) + 
				C.y() * (t * u + (3 - 2 * t) * pow(u, 2) + (-2 + t) * pow(u, 3)) + 
				A.y() * (-(t * u) + 2 * t * pow(u, 2) - t * pow(u, 3)) + 
				D.y() * (-(t * pow(u, 2)) + t * pow(u, 3)),

				B.z() * (1 + (-3 + t) * pow(u, 2) + (2 - t) * pow(u, 3)) + 
				C.z() * (t * u + (3 - 2 * t) * pow(u, 2) + (-2 + t) * pow(u, 3)) + 
				A.z() * (-(t * u) + 2 * t * pow(u, 2) - t * pow(u, 3)) + 
				D.z() * (-(t * pow(u, 2)) + t * pow(u, 3))
			);

			if(regpoints)
			{
				track_view[track_point++]  = Vec3f(point);

				float det = pow(velocity[track_point-1], 2) - 18.6 * (track_view[track_point-1].z() - track_view[track_point].z());
				if(det < 0)
					velocity[track_point] = -1 * sqrt(abs(det));
				else
					velocity[track_point] = sqrt(det);
			}
			

			
			glColor3f(0,1,1);
			glVertex3f(point.x(), point.y(), point.z());
		}

		// increment the positional control point
		curpos+=*ptsiter;
	}

	glEnd();
}

GLvoid Timer(int iunused)
{
	currentTranslation[0] = track_view[track_point].x();
	currentTranslation[1] = track_view[track_point].y();
	currentTranslation[2] = track_view[track_point].z() + 0.02;

	/*
	if(velocity[track_point] <= 0)
		track_dir*=-1;
	
	track_point+=track_dir;
	*/

	track_point++;

	if(track_point > max_point - 1)
			track_point = 0;
	else if(track_point == 0)
			track_point = max_point;
	
	glutPostRedisplay();

	glutTimerFunc(40, Timer, 0);
}