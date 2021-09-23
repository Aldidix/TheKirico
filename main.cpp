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
#include <fstream>
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

int width, height;

bool menu = true;
bool gameover = false;

engine::Timer timer(5);

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

	if(!menu){
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
	else{
		if(key == ' '){
			menu = false;
			timer.start();
		}
	}
}

void processSKeys(int key, int x, int y) {
	if(!menu){
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
	height = h;
	width = w;
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45,ratio,1, 100);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
	int hh = glutGet(GLUT_WINDOW_HEIGHT);
	printf("h %d \n", hh);
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
int numfound = 0;


//Menu options
engine::GuiText startText;

//Scene objects
bool isPlayerColliding = false;
int reset = 0;
int plaza = 0;

engine::Part parts[3];
engine::Model mannequin_complete1("Bottom.obj");
engine::Model mannequin_complete2("Middle.obj");
engine::Model mannequin_complete3("Top.obj");

engine::GuiText timerText;
engine::GuiText piecesfound;
std::vector<engine::Area> collisions;
// std::vector<engine::Model> maps;
engine::Model specialDoor("door.obj");
engine::Area  specialDoorCol(3.6f, 9.25f, 2.0f);
engine::Model map1("Kiricomap.obj");
engine::Model map2("Kiricomap2.obj");
engine::Model map3("Kiricomap3.obj");
std::vector<engine::CollisionMap> collisionmaps;
// engine::Model table("Table.obj");
engine::Model silhouette("Silhouette.obj");
engine::Area silArea(engine::Vector2(15.f, -6.f), 2.0f);
engine::Model crate("Crate.obj");
engine::Model lightbox("Crate.obj");
engine::Model plazabase("base.obj");
engine::Model train("train.obj");
engine::Terrain ground;
engine::Vector2 pos;
engine::Texture sand;
engine::Texture brick;
engine::Area trigger(engine::Vector2(5.0f, -13.0f), 4.5f);
engine::Area cbox1;
engine::Color 	col;
engine::Cube 	cube1;
engine::Camera camera;

bool silFalling = false;
float trainpos = 0;

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

		parts[0] = engine::Part("Legs.obj");
		parts[1] = engine::Part("Torso.obj");
		parts[2] = engine::Part("Head.obj");
		
		// 	table.scale = 1.5f;
		// 	crate.scale = 1.00f;
		// collisions.push_back(engine::Area(engine::Vector2(2.5f, -5.5f), 1.0f));
		startText = engine::GuiText(width, height, engine::Color(1.0f, 1.0f, 1.0f), "Press SPACE to Start");
		collisionmaps.push_back(engine::CollisionMap("KiricomapC.obj"));
		collisionmaps.push_back(engine::CollisionMap("Kiricomap2C.obj"));
		timerText = engine::GuiText(width, height, engine::Color(1.0f, 1.0f, 1.0f), std::string("10:00"));
		trigger = engine::Area(engine::Vector2(9.0f, -13.0f), 3.0f);
		// kmap.scale = 1.00f;
		crate.text = engine::Texture("box.jpg");
		// train.text = engine::Texture("Train.png");
		crate.rotation = 45.0f;
		brick = engine::Texture("prova.jpg");
		// crate.pos = engine::Vector3(0.0f, 0.5f, 0.0f);
		ground = engine::Terrain(100.0f, brick);
		pos = engine::Vector2(10, 0);	
		col = engine::Color(0.0f, 1.0f, 0.0f);
		cube1 = engine::Cube(pos, 2, col);
		camera = engine::Camera(engine::Vector2(0,0), 0);
		camera.setActive();
		piecesfound.col = engine::Color(1.0f, 1.0f, 1.0f);
		sceneLoaded = true;	
	}

	piecesfound.text = std::string(std::to_string(numfound) + "/3");
	// engine::GuiText(width, height, engine::Color(1.0f, 1.0f, 1.0f), std::string(std::to_string(numfound) + "/3"));
	timerText.text = timer.ToString();
	// timerText = engine::GuiText(width, height, engine::Color(1.0f, 1.0f, 1.0f), timer.ToString());

	if(!menu && !gameover){

		GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 0.0f};
		GLfloat light_position1[] = {1.0f, 1.0f, 1.0f, 0.0f};
		GLfloat light_amb[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_amb);
		// glLightModelfv()
		// GLfloat light_position2[] = {0.0, 0.0, 1.0, 0.0};
		// glLightfv(GL_LIGHT1, GL_POSITION, light_position2);

		// GLfloat light_position[] = {1.0, 1.0, 1.0, 1.0};
		// glLightfv(GL_LIGHT0, GL_AMBIENT, light_position);

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
			if(!reset){
				switch (plaza)
				{
				case 0:	
					if(numfound == 1)
						plaza = 1;
					break;
				case 1:
					if(numfound == 2)
						plaza = 2;
				default:
					break;
				}
			}
			reset = 1;
		}
		else reset = 0;
		// if(!isPlayerColliding){
		// 	cam_x += dX * loc_x;
		// 	cam_z += dZ * loc_z;
		// }

		if(silArea.isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z))
			silFalling = true;

		if(collisionmaps[0].isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z)){
			isPlayerColliding = true;
		}

		if(plaza < 2 && specialDoorCol.isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z)){
			isPlayerColliding = true;
		}

		if(!isPlayerColliding){
			cam_x += dX * loc_x;
			cam_z += dZ * loc_z;
		}

		isPlayerColliding = false;	
		camera.update(engine::Vector2(cam_x, cam_z), angleY);

		// collisionmaps[0].draw();
		// trigger.draw();

		silhouette.pos = engine::Vector3(15.f, 0.0f, -6.f);
		if(silhouette.tilt > -90.0f && silFalling)
			silhouette.tilt -= 3.0f;
		if(numfound == 0)
			silhouette.draw();
		plazabase.draw();

		glDisable(GL_LIGHTING);

		trainpos += 0.01f;
		if(train.pos.z > -45.0f)
			train.pos = engine::Vector3(train.pos.x, train.pos.y, train.pos.z - 0.05f);
		else train.pos.z = 0.0f;
		train.draw();

		timerText.pos = engine::Vector2(width/2, height- 20.0f);
		timerText.draw();

		// for(int i = 0; i <= numfound - 1; i++){
		// 	mannequin_complete[i].draw();
		// }

		if(numfound == 0 && plaza == 0){
			parts[0].translate(5.0f, 1.0f);
			parts[0].draw();
		}
		if(numfound == 1 && plaza == 1){
			parts[1].translate(10.0f, -21.0f);
			parts[1].draw();
		}
		if(numfound == 2 && plaza == 2){
			parts[2].translate(21.0f, 0.0f);
			parts[2].draw();
		}


		
		// lightbox.pos = engine::Vector3(light_position1[0], light_position1[1], light_position1[2]);
		// lightbox.scale = 0.05f;
		// lightbox.draw();
		piecesfound.pos = engine::Vector2(width - 40.0f, height- 20.0f);
		piecesfound.draw();
		glEnable(GL_LIGHTING);


		if(parts[0].isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z)){
			parts[0].take();
			numfound++;
			std::ofstream outfile("savegame.txt");
			outfile << "1 1\n";
			outfile.close();
			std::cout << "colliding\n";
		}
		if(parts[1].isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z) && numfound == 1){
			parts[1].take();
			numfound++;
			std::ofstream outfile("savegame.txt");
			outfile << "2 2\n";
			outfile.close();
			std::cout << "colliding\n";
		}
		if(parts[2].isColliding(cam_x + dX * loc_x, cam_z + dZ * loc_z) && numfound == 2){
			parts[2].take();
			numfound++;
			std::ofstream outfile("savegame.txt");
			outfile << "3 3\n";
			outfile.close();
			std::cout << "colliding\n";
		}

		if(timer.hasExpired())
			gameover = true;

		switch (plaza)
		{
		case 0:
			map1.draw();
			break;
		case 1:
			map2.draw();
			break;
		case 2:
			map3.draw();
		default:
			break;
		}

		if(numfound < 3)
			specialDoor.draw();

		switch (numfound)
		{
		case 1:
			mannequin_complete1.draw();
			break;
		case 2:
			mannequin_complete2.draw();
			break;
		case 3:
			mannequin_complete3.draw();
			break;
		default:
			break;
		}
		

		// table.draw();
		// crate.draw();

		// for(int cb = 0; cb < collisions.size(); cb++){
		// 	collisions[cb].draw();
		// }
		// ground.draw();
		// renderMap();
		
		// printf("x: %f, y: %f, ang: %f \n", cam_x, cam_z, angleY);
		for(int cb = 0; cb < collisions.size(); cb++){
			if (collisions[cb].isColliding(cam_x, cam_z)){
				col.set(1.0f, 0.0f, 0.0f);
			} 
			else col.set(0.0f, 1.0f, 0.0f);
		}
	}
	else if(menu){		
		startText.pos = engine::Vector2(width/2 - 100.0f, height/2);
		startText.font = GLUT_BITMAP_TIMES_ROMAN_24;
		startText.draw();
	}	
	else if(gameover){		
		startText.pos = engine::Vector2(width/2 - 100.0f, height/2);
		startText.font = GLUT_BITMAP_TIMES_ROMAN_24;
		startText.text = std::string("Game Over");
		startText.draw();
	}
	
	
	
	
	// c.setColor(col);
	// c.draw();
	// drawCube();
	// drawSphere();
	// drawCone();

	glutSwapBuffers();
}

int main(int argc, char** argv) {

	
	//ToDO Open and Read SaveFile
	std::string line;
	std::ifstream myfile("savegame.txt");
	if(myfile.is_open()){
		getline(myfile, line);
		char charline[line.length()];
		std::strcpy(charline, line.c_str());
		numfound = charline[0] - '0';
		plaza = (charline[2] - 1) -'0';
	}
	else std::cout << "No Savegame found\n";

	// init DevIL to load images
	ilInit();

	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	width = 1000;
	height = 500;
	glutInitWindowSize( width, height);
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
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);

	// ignore repeated key pressed callbacks
	glutIgnoreKeyRepeat(1);

	// // preload the scene datas
	// loadScene();

	// enter GLUT event processing loop
	glutMainLoop();

	return 0;
}

