// HelloOpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define GL_GLEXT_PROTOTYPES
#include <iostream>
#include <GL/glut.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <IL/il.h>
#include <IL\ilu.h>
#include <IL\ilut.h>
#define pi 3.14159265
//provaa
using namespace std;


const string fileName = "prova.jpg";
unsigned int id;
bool success;
int width;
int height;

unsigned int fbo;

//float angleZ =	0.0f;
float angleY =	0.0f;
float cam_x =	0.0f;
float cam_z =	0.0f;
float loc_x =	1.0f;
float loc_z =	0.0f;
float dX =		0.0f;
float dZ =		0.0f;
float dAngleY =	0.0f;

void resize(int w, int h) {
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
			h = 1;

	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45,ratio,1, 100);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}



void drawRectangle(int x, int y, int z, int w, int h) {
	glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(x, y, z);
		glVertex3f(x+w, y, z);
		glVertex3f(x, y+h, z);
		glVertex3f(x+w, y+h, z);
	glEnd();

}

void renderMap(void) {
	glColor3f(1, 1, 1);


	
	//Draw Floor
	//loadtexture:

	if (!id) {
		ilGenImages(1, &id);
		ilBindImage(id);
		success = ilLoadImage((ILstring)fileName.c_str());
		if (!success) printf("image failed \n");
		success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
		if (!success) printf("conversion failed \n");
		glGenTextures(1, &id);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, id);
		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH),
			ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
			ilGetData());
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Linear Filtered
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Linear Filtered
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBegin(GL_QUADS);
		glVertex3f(-100.0f, 0.0f, -100.0f); glTexCoord2f(-100, -100);
		glVertex3f(-100.0f, 0.0f,  100.0f); glTexCoord2f(-100, 100);
		glVertex3f( 100.0f,	0.0f,  100.0f); glTexCoord2f(100, 100);
		glVertex3f( 100.0f, 0.0f, -100.0f); glTexCoord2f(100, -100);
	glEnd();
}

void drawCube(void) {
	//Draw a Cube
	glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();
	glTranslatef(10.0f, 1.0f, 0.0f);
	glutSolidCube(2.0f);
	glPopMatrix();
}

void drawSphere(void) {
	//Draw a Sphere
	glColor3f(5.0f, 1.0f, 0.0f);
	glPushMatrix();
	glTranslatef(0.0f, 1.0f, 10.0f);
	glutWireSphere(1.0f, 8, 4);
	glPopMatrix();
}

void drawCone(void) {
	//Draw a Cone
	glColor3f(0.0f, 1.0f, 1.0f);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -10.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutWireCone(1.0f, 2.0f, 6, 2);
	glPopMatrix();
}

void drawSkybox(void) {
	//Draw inverse cube as skybox
	glColor3f(1.0f, 0.0f, 1.0f);
	glutSolidCube(100);
}

//GLuint loadTexture(void) {
//	
//}

void renderScene(void) {
	glEnable(GL_TEXTURE_2D);
	//Clear Color and Depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Draw Skybox
	//drawSkybox();

	//Reset transformations
	glLoadIdentity();
	
	//Compute movements
	angleY += dAngleY;
	loc_x = cos(angleY);
	loc_z = sin(angleY);
	cam_x += dX * loc_x;
	cam_z += dZ * loc_z;


	//Camera setup
	gluLookAt(	cam_x,			1.0f,	cam_z,				//Camera Position  
				cam_x + loc_x,	1.0f,	cam_z + loc_z,		//Look at
				0.0f,			10.0f,	0.0f);				//Up vector

	renderMap();
	drawCube();
	drawSphere();
	drawCone();

	//glColor3f(.0f, 1.0f, 1.0f);
	//glBegin(GL_TRIANGLES);
	//	glVertex3f(-2, -2, 0);
	//	glVertex3f(2, 0.0, 0);
	//	glVertex3f(0.0, 2, 0);
	//glEnd();

	//drawRectangle(0, 0, -5, 1, 1);

	glutSwapBuffers();
}

void processNKeys(unsigned char key, int x, int y) {

	switch (key) {
		case 'w':
			dX += 0.1f;
			dZ += 0.1f;
			break;
		case 's':
			dX -= 0.1f;
			dZ -= 0.1f;
			break;
		case 'd':
			dAngleY += 0.05f;
			break;
		case 'a':
			dAngleY -= 0.05f;
			break;
		case 'r':
			angleY = 0.0f;
			loc_x =  1.0f;
			loc_z =  0.0f;
			cam_x =	 0.0f;
			cam_z =  0.0f;
			break;
	}

}

void processSKeys(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_UP:
			break;
		case GLUT_KEY_RIGHT:
			angleY += 0.1f;
			loc_x = cos(angleY);
			loc_z = sin(angleY);
			break;
		case GLUT_KEY_DOWN:
			break;
		case GLUT_KEY_LEFT:
			angleY -= 0.1f;
			loc_x = cos(angleY);
			loc_z = sin(angleY);
			break;
	}

}

//void processSkeysReleased(int key, int x, int y) {
//	
//}

void processNKeysReleased(unsigned char key, int x, int y) {
	switch (key) {
		case 'w': 
		case 's': dX = 0; dZ = 0;break;
		case 'a':
		case 'd': dAngleY = 0.0f;break;
	}
}


int main(int argc, char** argv) {

	// init DevIL to load images
	ilInit();

	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize( 1000, 500);
	glutCreateWindow("Engine3D");

	// register callbacks
	glutDisplayFunc(renderScene);

	// fix proportions
	glutReshapeFunc(resize);

	// idle func
	glutIdleFunc(renderScene);

	// handle input
	glutKeyboardFunc(processNKeys);
	glutSpecialFunc(processSKeys);
	//glutSpecialUpFunc(processSkeysReleased);
	glutKeyboardUpFunc(processNKeysReleased);

	// enable depth test
	glEnable(GL_DEPTH_TEST);

	// ignore repeated key pressed callbacks
	glutIgnoreKeyRepeat(1);

	// enter GLUT event processing loop
	glutMainLoop();


	return 1;
}

