#pragma once
#include <iostream>
#include <GL/glut.h>
#include <GL/gl.h>
#include <IL/il.h>
#include <IL\ilu.h>
#include <IL\ilut.h>
#include <ctime>
#include <chrono>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>  
#include <vector>
#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)


namespace engine
{



Assimp::Importer importer;

class Timer
{
public:
	Timer(int min) : min(min*60){}
    void start()
    {
        m_StartTime = std::chrono::system_clock::now();
        m_bRunning = true;
    }
    
    void stop()
    {
        m_EndTime = std::chrono::system_clock::now();
        m_bRunning = false;
    }
    
    double elapsedMilliseconds()
    {
        std::chrono::time_point<std::chrono::system_clock> endTime;
        
        if(m_bRunning)
        {
            endTime = std::chrono::system_clock::now();
        }
        else
        {
            endTime = m_EndTime;
        }
        
        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
    }
    
    double elapsedSeconds()
    {
        return elapsedMilliseconds() / 1000.0;
    }

	std::string ToString(){
		std::string temp;
		int elapsedSec = (int) elapsedSeconds();
		int tempmin = (min - elapsedSec)/60; 
		int tempsec = min - elapsedSec - tempmin*60;
		temp = std::to_string(tempmin) + ":" + std::to_string(tempsec);
		return temp;
	}

	bool hasExpired(){
		if (min - elapsedSeconds() < 0)
			return true;
		else return false;
	}

private:
	int min;
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    bool                                               m_bRunning = false;
};

class Color{
public:
	float r;
	float g;
	float b;
	Color(){};
	Color(float r, float g, float b):r(r),g(g),b(b){}
	void set(float r, float g, float b){
		this->r = r;
		this->g = g;
		this->b = b;
	}
	float getR(){return r;};
	float getG(){return g;};
	float getB(){return b;};
};

class Texture{
	unsigned int id = 0;
	unsigned int image;
	bool success;
	int width;
	int height;
public:
	std::string path;
	Texture(){};
	Texture(std::string path) : path(path){
		std::cout << "Loading texture: " << path << std::endl;
		// printf("Loading texture: %s\n", path);
		ilGenImages(1, &id);
		std::cout << "" << id << std::endl;
		ilBindImage(id);
		success = ilLoadImage((ILstring)path.c_str());
		if (!success) printf("image failed \n");
		success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
		if (!success) printf("conversion failed \n");
		glGenTextures(1, &image);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, image);
		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH),
			ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
			ilGetData());
	}

	void bind(){
		// ilBindImage(id);
		// std::cout << "Binding image with id: " << image << std::endl; 
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, image);		
		// glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH),
		// 	ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
		// 	ilGetData());
	}
	unsigned int getId(){return id;}
};

class Vector2{
public:
	float x;
	float y;
	Vector2(){}
	Vector2(float x, float y):x(x), y(y){}
};

class Vector3{
public:
	float x;
	float y;
	float z;
	Vector3(){}
	Vector3(float x, float y, float z):x(x), y(y), z(z){}
};

class Vertex{
public:
	Vector3 position;
	Vector3 normal;
	Vector3 textCoords;
	Vertex(){}
	Vertex(float x, float y, float z){
		this->position = Vector3(x, y, z);
	}
	Vertex(Vector3 pos): position(pos){}
};

class Triangle{
	Vertex vertices[3];
public:
	Triangle(){}
	Triangle(Vector3 v1, Vector3 v2, Vector3 v3){
		vertices[0] = Vertex(v1);
		vertices[1] = Vertex(v2);
		vertices[2] = Vertex(v3);
	}
	Triangle(float v1[3], float v2[3], float v3[3]){
		vertices[0] = Vertex(v1[0], v1[1], v1[2]);
		vertices[1] = Vertex(v2[0], v2[1], v2[2]);
		vertices[2] = Vertex(v3[0], v3[1], v3[2]);
	}
	void draw(){
		glBegin(GL_TRIANGLES);
			for(int i = 0 ; i<3; i++){
				glVertex3f(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
			}
		glEnd();
	}
};

class Material{
	int id;
public:
	Texture text;
	std::string texturepath;
	Color diffuse;
	Material(){}
	Material(int id):id(id){}
	void setColor(Color c){this->diffuse = c;}
	void setTexture(Texture t){this->text = t;}
	void setTexturePath(std::string path){this->texturepath = path;}
};

class Face{
public:
	Face(const aiFace f){
		for(int i = 0; i < f.mNumIndices; i++){
			indices.push_back(f.mIndices[i]);
		}
		nIndices = indices.size();
	}
	std::vector<int> indices;
	int nIndices;

private:

};



class Mesh{
private:
	std::vector<Vertex> vertices;
	std::vector<Vector3> normals;
	std::vector<Face> faces;
	std::vector<Vector2> textcoord;
	Texture text;
	int isTextureLoaded;
public:
	Material* mat;
	Color color;
	Mesh(){}
	Mesh(std::vector<Vertex> vertices):vertices(vertices){}
	Mesh(std::vector<Vertex> vertices, Vector3 pos, float scale):vertices(vertices){}
	Mesh(const aiMesh* m){
		isTextureLoaded = 0;
		printf("Mesh name: %s\n",m->mName.data);
		for(int v = 0; v < m->mNumVertices; v++){
			vertices.push_back(Vertex(m->mVertices[v].x, m->mVertices[v].y, m->mVertices[v].z));
			textcoord.push_back(Vector2(m->mTextureCoords[0][v].x, 1 - m->mTextureCoords[0][v].y));
			normals.push_back(Vector3(m->mNormals[v].x, m->mNormals[v].y, m->mNormals[v].z));
			// if(m->mColors[0] != NULL)
			// 	colors.push_back(Color(m->mColors[0][v].r, m->mColors[0][v].g, m->mColors[0][v].b));
		}
		for(int f = 0; f < m->mNumFaces; f++){
			faces.push_back(Face(m->mFaces[f]));
		}
	}
	void draw(float scale = 1, Vector3 position = Vector3(0, 0, 0), float rot = 0.0f, float tilt = 0.0f){
		int i = 0;

		glColor3f(1, 1, 1);

		if(mat->texturepath.length()){
			if(!isTextureLoaded){
				text = Texture(mat->texturepath);
				isTextureLoaded = 1;
			}
			text.bind();
		}
		// else defaultTexture.bind();
		else mat->texturepath = std::string("base.png"); 

		glPushMatrix();
		glTranslatef(position.x, position.y, position.z);
		glRotatef(rot, 0.0f, 1.0f, 0.0f);
		glRotatef(tilt, 0.0f, 0.0f, 1.0f);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			for(int f = 0; f < faces.size(); f++){
				glBegin(GL_TRIANGLES);
				for(int i = 0; i < faces[f].nIndices; i++){
					// if(colors.size() > 0)
					// 	glColor3f(colors[faces[f].indices[i]].r, colors[faces[f].indices[i]].g, colors[faces[f].indices[i]].b);
					// glColor3f(color.r, color.g, color.b);
					if(!mat->texturepath.length())
						glColor3f(mat->diffuse.r, mat->diffuse.g, mat->diffuse.b);
					else if(mat->texturepath.compare("base.png") == 0)
						glColor3f(mat->diffuse.r, mat->diffuse.g, mat->diffuse.b);
					glTexCoord2f(textcoord[faces[f].indices[i]].x, textcoord[faces[f].indices[i]].y);
					glNormal3f(normals[faces[f].indices[i]].x, normals[faces[f].indices[i]].y, normals[faces[f].indices[i]].z);
					glVertex3f(vertices[faces[f].indices[i]].position.x*scale, vertices[faces[f].indices[i]].position.y*scale, vertices[faces[f].indices[i]].position.z*scale);
				}
				glEnd();
			}

		glPopMatrix();
	}
};

class Model{
	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	std::vector<Texture> textures;
	void addTexture(std::string path){}
public:
	float rotation;
	float tilt;
	Texture text;
	float scale;
	Vector3 pos;
	Model(){}
	Model(std::string path){
		this->scale = 1.0f;
		this->rotation = 0.0f;
		this->tilt = 0.0f;
		printf("Loading file: %s\n", path.c_str());
		// text = Texture("sand.jpg");
		const aiScene* scene;
		scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);
		if(!scene) printf("Cannot open scene\n");
		else{
			for(int i = 0; i < scene->mNumMaterials; i++){
				aiColor4D c;
				aiGetMaterialColor(scene->mMaterials[i], AI_MATKEY_COLOR_DIFFUSE, &c);
				Material m;
				aiString texpath;
				scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texpath);
				if(texpath.length > 0){
					std::string s(texpath.data);
					printf("Texture path: %s\n", texpath.data);
					m.setTexturePath(std::string(texpath.C_Str()));
				}
				m.setColor(Color(c.r, c.g, c.b));
				materials.push_back(m);
			}

			for(int i = 0; i < scene->mNumMeshes; i++){
				meshes.push_back(Mesh(scene->mMeshes[i]));
				// meshes[i].color = diffuse[scene->mMeshes[i]->mMaterialIndex];
				meshes[i].mat = &materials[scene->mMeshes[i]->mMaterialIndex];
			} 
			printf("Scene loaded\n");
		}
	}

	void draw(){
		for(int i = 0; i < meshes.size(); i++){
			text.bind();
			meshes[i].draw(scale, pos, rotation, tilt);
		} 
	}
};

class Terrain{
	Texture text;
	float size;
public:
	Terrain(){};
	Terrain(float size, Texture text):text(text){}
	Terrain(float size):size(size){}
	void draw(){	
		glColor3f(50, 100, 65);
		
		text.bind();

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

	void changeTexture(Texture text){
		this->text = text;
	}
};

class Camera{
	Vector2 pos;
	float angle;
	bool isActive = false;
public:
	Camera(){}
	Camera(Vector2 pos, float angle):pos(pos), angle(angle){};
	void setActive(){isActive = true;}
	void update(Vector2 pos, float angle){
		this->pos = pos;
		this->angle = angle;
		if (isActive) 
			gluLookAt(	pos.x,				1.0f,	pos.y,						//Camera Position  
						pos.x + cos(angle),	1.0f,	pos.y + sin(angle),		//Look at
						0.0f,				10.0f,	0.0f);						//Up vector
	}
};

class GuiRectangle{
	Vector2 pos;
	Color col;
	float size;
public:
	GuiRectangle(){}
	GuiRectangle(float x, float y, float size, Color col) : pos(Vector2(x, y)), size(size), col(col){}
	void draw(){
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D( 0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();
		glColor3f(col.r, col.g, col.b);
		//Draw Here
		glBegin(GL_POLYGON);
		glVertex2f(pos.x, 			pos.y		);
		glVertex2f(pos.x + size, 	pos.y		);
		glVertex2f(pos.x + size, 	pos.y + size);
		glVertex2f(pos.x, 			pos.y + size);
		glEnd();
		glFlush();
		glPopMatrix();

		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );
	}
};

class GuiText{
public:
	void* font = GLUT_BITMAP_HELVETICA_18;
	Vector2 pos;
	Color col;
	int width, height;
	std::string text;
	GuiText(){}
	GuiText(Vector2 pos, Color col, std::string text) : pos(pos), col(col), text(text), width(1000), height(500){}
	GuiText(float x, float y, Color col, std::string text) : pos(Vector2(x, y)), col(col), text(text){}
	void draw(){
		glDisable(GL_LIGHTING);
		int len = text.length();
		char text_char[len + 1];
		strcpy(text_char, text.c_str());
		
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D( 0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();
		glColor3f(col.r, col.g, col.b);
		glRasterPos2f( pos.x, pos.y );
		for ( int i = 0; i < len ; ++i ) {
			glutBitmapCharacter(font, text_char[i]);
		}
		glPopMatrix();

		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );
		glEnable(GL_LIGHTING);
	}
};

class Player{
	Camera cam;
public:
	Vector2 pos;
	Player(){}
	Player(Vector2 pos):pos(pos){
		cam = engine::Camera(pos, 0);
	}
};

class Cube{
	Vector2 center;
	float size;
	Color color;
public:
	Cube(){}
	Cube(int x, int y, int size, Color c):size(size), color(c), center(Vector2(x,y)){}
	Cube(Vector2 center, int size, Color c):center(center), color(c), size(size){}
	void setColor(Color c){this->color = c;}
	void draw(){
		glColor3f(color.r, color.g, color.b);
		glPushMatrix();
		glTranslatef(center.x, 1.0f, center.y);
		glutSolidCube(size);
		glPopMatrix();
	}
};

class Sphere{
	Vector2 center;
	float size;
	Color color;
public:
	Sphere(){}
	Sphere(float x, float y, float size, Color c):size(size), color(c), center(Vector2(x,y)){}
	Sphere(Vector2 center, int size, Color c):center(center), color(c), size(size){}
	void setColor(Color c){this->color = c;}
	void draw(){
		glColor3f(color.r, color.g, color.b);
		glPushMatrix();
		glTranslatef(center.x, 0.01f, center.y);
		glutSolidSphere(size, 16, 8);
		glPopMatrix();
	}
};

class Area{
	Vector2 center;
	float size;
public:
	Area(){}
	Area(float x, float y, float size):size(size), center(Vector2(x,y)){}
	Area(Vector2 center, int size):center(center), size(size){}
	bool isColliding(float x, float y){
		if(	x >= center.x - size && 
			x <= center.x + size && 
			y >= center.y - size && 
			y <= center.y + size) return true;
		return false;
	}
	void draw(){
		glColor3f(0.0f, 1.0f, 0.0f);
		glPushMatrix();
		glTranslatef(center.x, 1.0f - size/2, center.y);
		glutWireCube(size);
		glPopMatrix();
	}
};

class CollisionBox{
	Vector2 center;
	Vector2 size;
	float margin = 0.9f;
public:
	std::vector<Sphere> vertex;
	CollisionBox(){}
	CollisionBox(float x, float y, float width, float height):center(Vector2(x,y)), size(Vector2(width, height)){
		vertex.push_back(Sphere(center.x - size.x/2, center.y - size.y/2, 0.05f, Color(1.0f, 0.0f, 0.0f)));
		vertex.push_back(Sphere(center.x - size.x/2, center.y + size.y/2, 0.05f, Color(1.0f, 0.0f, 0.0f)));
		vertex.push_back(Sphere(center.x + size.x/2, center.y - size.y/2, 0.05f, Color(1.0f, 0.0f, 0.0f)));
		vertex.push_back(Sphere(center.x + size.x/2, center.y + size.y/2, 0.05f, Color(1.0f, 0.0f, 0.0f)));
	}
	// CollisionBox(Vector2 center, int size):center(center), size(size){}
	bool isColliding(float x, float y){
		if(	x >= center.x - size.x/2 - margin && 
			x <= center.x + size.x/2 + margin && 
			y >= center.y - size.y/2 - margin && 
			y <= center.y + size.y/2 + margin) return true;
		return false;
	}

	void draw(){
		for(int i = 0; i < vertex.size(); i++)
			vertex[i].draw();
	}
};

class Part{
	Area cbox;
	bool taken = 0;
	float rot = 0;
public:
	Model mesh;
	Vector2 pos;
	Part(){}
	Part(std::string path){
		mesh = Model(path);
		cbox = Area(10000.0f, 10000000.0f, 1.0f);
	}
	void translate(float x, float y){
		mesh.pos = Vector3(x, 0, y);
		cbox = Area(x, y, 1.0f);
	}
	void draw(){
		if(!taken){
			mesh.rotation += 5.0f;
			mesh.draw();
		}

	}
	void take(){
		taken = 1;
	}
	bool isColliding(float x, float y){
		if(!taken)
			return cbox.isColliding(x, y);
		else return false;
	}
};

class CollisionMap{
private:
	std::vector<CollisionBox> collisions;
public:
	CollisionMap(){}
	CollisionMap(std::string path){
		printf("Loading file: %s\n", path.c_str());
		const aiScene *scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);
		if(!scene) printf("Cannot open collision map\n");
		else{
			for(int i = 0; i < scene->mNumMeshes; i++){
				CollisionBox cb;
				aiMesh* m = scene->mMeshes[i];
				float minx = m->mVertices[0].x;
				float maxx = minx;
				float miny = m->mVertices[0].z;
				float maxy = miny;
				for(int v = 1; v < scene->mMeshes[i]->mNumVertices; v++){
					cb.vertex.push_back(Sphere(m->mVertices[v].x, m->mVertices[v].z, 0.05f, Color(1.0f, 0.0f, 0.0f)));
					// printf("Vertex at: %f, %f\n", m->mVertices[v].x, m->mVertices[v].z);
					minx = aisgl_min(minx, m->mVertices[v].x);
					miny = aisgl_min(miny, m->mVertices[v].z);
					// printf("Min: x: %f, y: %f\n", minx, miny);
					maxx = aisgl_max(maxx, m->mVertices[v].x);
					maxy = aisgl_max(maxy, m->mVertices[v].z);
					// printf("Max: x: %f, y: %f\n", maxx, maxy);
				}
				float width = maxx - minx;
				float height = maxy - miny;
				// collisions.push_back(cb);
				collisions.push_back(CollisionBox(minx + width/2, miny + height/2, width, height));
			} 
			printf("Collisions loaded\n");
		}
	}
	bool isColliding(float x, float y){
		for(int i = 0; i < collisions.size(); i++)
			if(collisions[i].isColliding(x, y)) return true;
		return false;
	}

	void draw(){
		for(int i = 0; i < collisions.size(); i++)
			collisions[i].draw();
	}
};

}