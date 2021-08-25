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
#include "engine.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define pi 3.14159265

using namespace std;
// using namespace engine;


// const string fileName = "prova.jpg";
// unsigned int id;
// bool success;
// int width;
// int height;

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


// bool DoTheImportThing( const std::string& pFile) {
//   // Create an instance of the Importer class
//   Assimp::Importer importer;

//   // And have it read the given file with some example postprocessing
//   // Usually - if speed is not the most important aspect for you - you'll
//   // probably to request more postprocessing than we do in this example.
//   const aiScene* scene = importer.ReadFile( pFile,
//     aiProcess_CalcTangentSpace       |
//     aiProcess_Triangulate            |
//     aiProcess_JoinIdenticalVertices  |
//     aiProcess_SortByPType);

//   // If the import failed, report it
//   if( !scene) {
//     printf( importer.GetErrorString());
//     return false;
//   }

//   // Now we can access the file's contents.
// //   DoTheSceneProcessing( scene);

//   // We're done. Everything will be cleaned up by the importer destructor
//   return true;
// }

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
		case 'c':
			printf("x: %f, y: %f, ang: %f \n", cam_x, cam_z, angleY);
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
		case 's': dX = 0; dZ = 0; break;
		case 'a':
		case 'd': dAngleY = 0.0f; break;
	}
}


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

	glColor3f(0.0f, 0.2f, 0.1f);
	glutSolidCube(100);
}


bool sceneLoaded = false;

//Scene objects
int isPlayerColliding = 0;
int plaza = 0;
std::vector<engine::Area> collisions;
// std::vector<engine::Model> maps;
engine::Model map1("Kiricomap.obj");
engine::Model map2("Kiricomap2.obj");
std::vector<engine::CollisionMap> collisionmaps;
// engine::Model table("Table.obj");
// engine::Model crate("Crate.obj");
engine::Model plazabase("base.obj");
engine::Terrain ground;
engine::Vector2 pos;
engine::Texture sand;
engine::Texture brick;
engine::Area trigger(engine::Vector2(5.0f, -13.0f), 5.0f);
engine::Area cbox1;
engine::Color 	col;
engine::Cube 	cube1;
engine::Camera camera;

void renderScene(void) {
	glEnable(GL_TEXTURE_2D);
	//Clear Color and Depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Draw Skybox
	// drawSkybox();

	//Reset transformations
	glLoadIdentity();
	
	//Instantiate scene objects
	if(!sceneLoaded){
	// 	table.scale = 1.5f;
	// 	crate.scale = 1.00f;
		// collisions.push_back(engine::Area(engine::Vector2(2.5f, -5.5f), 1.0f));
		collisionmaps.push_back(engine::CollisionMap("KiricomapC.obj"));
		collisionmaps.push_back(engine::CollisionMap("Kiricomap2C.obj"));


		trigger = engine::Area(engine::Vector2(9.0f, -13.0f), 5.0f);
		// kmap.scale = 1.00f;
		// crate.text = engine::Texture("box.jpg");
		brick = engine::Texture("prova.jpg");
		// crate.pos = engine::Vector3(0.0f, 0.5f, 0.0f);
		ground = engine::Terrain(100.0f, brick);
		pos = engine::Vector2(10, 0);	
		col = engine::Color(0.0f, 1.0f, 0.0f);
		cube1 = engine::Cube(pos, 2, col);
		camera = engine::Camera(engine::Vector2(0,0), 0);
		camera.setActive();
		sceneLoaded = true;
	}


	//Compute movements
	angleY += dAngleY;
	loc_x = cos(angleY);
	loc_z = sin(angleY);
	// for(int cb = 0; cb < collisions.size(); cb++){
	// 	if(collisions[cb].isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z)){
	// 		isPlayerColliding = 1;
	// 		printf("Colliding! \n");
	// 		break;
	// 	}
	// }

	if(trigger.isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z)){
		plaza = 1;
	}

	// if(!isPlayerColliding){
	// 	cam_x += dX * loc_x;
	// 	cam_z += dZ * loc_z;
	// }

	if(!collisionmaps[0].isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z)){
		cam_x += dX * loc_x;
		cam_z += dZ * loc_z;
	}

	camera.update(engine::Vector2(cam_x, cam_z), angleY);

	// //Camera setup
	// gluLookAt(	cam_x,			1.0f,	cam_z,				//Camera Position  
	// 				cam_x + loc_x,	1.0f,	cam_z + loc_z,		//Look at
	// 				0.0f,			10.0f,	0.0f);				//Up vector

	// cmap.draw();
	trigger.draw();
	plazabase.draw();
	switch (plaza)
	{
	case 0:
		map1.draw();
		break;
	case 1:
		map2.draw();
		break;
	default:
		break;
	}
	// table.draw();
	// crate.draw();
	for(int cb = 0; cb < collisions.size(); cb++){
		collisions[cb].draw();
	}
	// ground.draw();
	// renderMap();
	
	// printf("x: %f, y: %f, ang: %f \n", cam_x, cam_z, angleY);
	for(int cb = 0; cb < collisions.size(); cb++){
		if (collisions[cb].isColliding(cam_x, cam_z)){
			col.set(1.0f, 0.0f, 0.0f);
		} 
		else col.set(0.0f, 1.0f, 0.0f);	
	}
	
	
	
	
	
	// c.setColor(col);
	// c.draw();
	// drawCube();
	// drawSphere();
	// drawCone();

	glutSwapBuffers();
	isPlayerColliding = 0;
}

int main(int argc, char** argv) {

	// init DevIL to load images
	ilInit();

	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize( 1000, 500);
	glutCreateWindow("The Kirico");

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

	// // preload the scene datas
	// loadScene();

	// enter GLUT event processing loop
	glutMainLoop();

	return 0;
}

