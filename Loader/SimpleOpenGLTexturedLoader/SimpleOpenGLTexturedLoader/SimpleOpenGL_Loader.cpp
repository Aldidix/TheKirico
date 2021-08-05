// ----------------------------------------------------------------------------
// Simple sample to prove that Assimp is easy to use with OpenGL.
// It takes a file name as command line parameter, loads it using standard
// settings and displays it.
//
// If you intend to _use_ this code sample in your app, do yourself a favour 
// and replace immediate mode calls with VBOs ...
//
// The vc8 solution links against assimp-release-dll_win32 - be sure to
// have this configuration built.
// ----------------------------------------------------------------------------


// assimp include files. These three are usually needed.
#include "assimp.h"
#include "aiPostProcess.h"
#include "aiScene.h"

#include "GL/glut.h"
#include <IL/il.h>

//to map image filenames to textureIds
#include <string.h>
#include <map>

// currently this is hardcoded
//static const std::string basepath = "./models/textures/"; //obj..
static const std::string basepath = "./models/"; //per i file blend

// the global Assimp scene object
const struct aiScene* scene = NULL;
GLuint scene_list = 0;
struct aiVector3D scene_min, scene_max, scene_center;

// current rotation angle
static float angle = 0.f;

// images / texture
std::map<std::string, GLuint*> textureIdMap;	// map image filenames to textureIds
GLuint*		textureIds;							// pointer to texture Array

GLfloat LightAmbient[]= { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[]= { 0.0f, 0.0f, 15.0f, 1.0f };

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)
#define TRUE                1
#define FALSE               0


//--------
int uno = 1;

const std::string fileName = "prova.jpg";
unsigned int id;
bool success;
int width;
int height;

unsigned int fbo;

//float angleZ =	0.0f;
float angleY = 0.0f;
float cam_x = 0.0f;
float cam_z = 0.0f;
float loc_x = 1.0f;
float loc_z = 0.0f;
float dX = 0.0f;
float dZ = 0.0f;
float dAngleY = 0.0f;

//--------


// ----------------------------------------------------------------------------
void reshape(int width, int height)
{
	const double aspectRatio = (float) width / height, fieldOfView = 45.0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fieldOfView, aspectRatio,
		1.0, 1000.0);  // Znear and Zfar 
	glViewport(0, 0, width, height);
}

// ----------------------------------------------------------------------------
void get_bounding_box_for_node (const struct aiNode* nd, 
	struct aiVector3D* min, 
	struct aiVector3D* max, 
	struct aiMatrix4x4* trafo
){
	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

// ----------------------------------------------------------------------------

void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max)
{
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}

// ----------------------------------------------------------------------------

void color4_to_float4(const struct aiColor4D *c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

// ----------------------------------------------------------------------------

void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

// ----------------------------------------------------------------------------
void apply_material(const struct aiMaterial *mtl)
{
	float c[4];

	GLenum fill_mode;
	int ret1, ret2;
	struct aiColor4D diffuse;
	struct aiColor4D specular;
	struct aiColor4D ambient;
	struct aiColor4D emission;
	float shininess, strength;
	int two_sided;
	int wireframe;
	int max;

int texIndex = 0;
aiString texPath;	//contains filename of texture
if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath))
{
	//bind texture
	unsigned int texId = *textureIdMap[texPath.data];
	glBindTexture(GL_TEXTURE_2D, texId);
}

	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
		color4_to_float4(&diffuse, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
		color4_to_float4(&specular, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
		color4_to_float4(&ambient, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
		color4_to_float4(&emission, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

	max = 1;
	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, (unsigned int *)&max);
	max = 1;
	ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, (unsigned int *)&max);
	if((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS))
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
	else {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	max = 1;
	if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, (unsigned int *)&max))
		fill_mode = wireframe ? GL_LINE : GL_FILL;
	else
		fill_mode = GL_FILL;
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

	max = 1;
	if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, (unsigned int *)&max)) && two_sided)
		glEnable(GL_CULL_FACE);
	else 
		glDisable(GL_CULL_FACE);
}

// ----------------------------------------------------------------------------

// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
void Color4f(const struct aiColor4D *color)
{
	glColor4f(color->r, color->g, color->b, color->a);
}

// ----------------------------------------------------------------------------

void recursive_render (const struct aiScene *sc, const struct aiNode* nd, float scale)
{
	unsigned int i;
	unsigned int n=0, t;
	struct aiMatrix4x4 m = nd->mTransformation;

	printf("Node name: %s\n",nd->mName.data);	

	//m.Scaling(aiVector3D(scale, scale, scale), m);
	
	// update transform
	m.Transpose();
	glPushMatrix();
	glMultMatrixf((float*)&m);

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n)
	{
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

		///
		printf("Drawing MESH with this name: %s\n",mesh->mName.data);

		apply_material(sc->mMaterials[mesh->mMaterialIndex]);


		if(mesh->HasTextureCoords(0))
			glEnable(GL_TEXTURE_2D);
		else
			glDisable(GL_TEXTURE_2D);
		if(mesh->mNormals == NULL)
		{
			glDisable(GL_LIGHTING);
		}
		else
		{
			glEnable(GL_LIGHTING);
		}

		if(mesh->mColors[0] != NULL)
		{
			glEnable(GL_COLOR_MATERIAL);
		}
		else
		{
			glDisable(GL_COLOR_MATERIAL);
		}



		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch(face->mNumIndices)
			{
				case 1: face_mode = GL_POINTS; break;
				case 2: face_mode = GL_LINES; break;
				case 3: face_mode = GL_TRIANGLES; break;
				default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);

			for(i = 0; i < face->mNumIndices; i++)		// go through all vertices in face
			{
				int vertexIndex = face->mIndices[i];	// get group index for current index
				if(mesh->mColors[0] != NULL)
					Color4f(&mesh->mColors[0][vertexIndex]);
				if(mesh->mNormals != NULL)

					if(mesh->HasTextureCoords(0))		//HasTextureCoords(texture_coordinates_set)
					{
						glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, 1 - mesh->mTextureCoords[0][vertexIndex].y); //mTextureCoords[channel][vertex]
					}

					glNormal3fv(&mesh->mNormals[vertexIndex].x);
					glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}

			glEnd();

		}

	}


	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n)
	{
		recursive_render(sc, nd->mChildren[n], scale);
	}

	glPopMatrix();
}

// ----------------------------------------------------------------------------
void do_motion (void)
{
	static GLint prev_time = 0;

	int time = glutGet(GLUT_ELAPSED_TIME);
	angle += (time-prev_time)*0.01;
	prev_time = time;

	glutPostRedisplay ();
}

// ----------------------------------------------------------------------------
//-----------


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
	glVertex3f(-100.0f, 0.0f, 100.0f); glTexCoord2f(-100, 100);
	glVertex3f(100.0f, 0.0f, 100.0f); glTexCoord2f(100, 100);
	glVertex3f(100.0f, 0.0f, -100.0f); glTexCoord2f(100, -100);
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

//-----------


void display(void)
{
	float tmp;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.f,0.f,3.f,0.f,0.f,-5.f,0.f,1.f,0.f);
	
	// rotate it around the y axis
	//glRotatef(angle,0.f,1.f,0.f);

	// scale the whole asset to fit into our view frustum 
	tmp = scene_max.x-scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y,tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z,tmp);
	tmp = 1.f / tmp;
	glScalef(tmp, tmp, tmp);

	//Compute movements
	angleY += dAngleY;
	loc_x = cos(angleY);
	loc_z = sin(angleY);
	cam_x += dX * loc_x;
	cam_z += dZ * loc_z;


	//Camera setup
	gluLookAt(cam_x, 1.0f, cam_z,				//Camera Position  
		cam_x + loc_x, 1.0f, cam_z + loc_z,		//Look at
		0.0f, 10.0f, 0.0f);				//Up vector

	renderMap();
	drawCube();
	drawSphere();
	drawCone();

        // if the display list has not been made yet, create a new one and
        // fill it with scene contents
	if(scene_list == 0) {
		scene_list = glGenLists(1);
		glNewList(scene_list, GL_COMPILE);

		glPushMatrix();
		glRotatef(-3.0 * angle, 0.f, 1.f, 0.f);
		recursive_render(scene, scene->mRootNode->mChildren[1], 1.0);
		glPopMatrix();
		//glPushMatrix();
		//glRotatef(-1.5 * angle, 1.f, 0.f, 0.f);
		recursive_render(scene, scene->mRootNode->mChildren[0], 1.0);
		//glPopMatrix();
	    //scene_list = glGenLists(1);
	    //glNewList(scene_list, GL_COMPILE);
     //       // now begin at the root node of the imported data and traverse
     //       // the scenegraph by multiplying subsequent local transforms
     //       // together on GL's matrix stack.
	    //recursive_render(scene, scene->mRootNode, 1.0);
	    glEndList();
	}


	glCallList(scene_list);

	glutSwapBuffers();

	do_motion();



}

// ----------------------------------------------------------------------------

int loadasset (const char* path)
{
	// we are taking one of the postprocessing presets to avoid
	// writing 20 single postprocessing flags here.
	scene = aiImportFile(path,aiProcessPreset_TargetRealtime_Quality);

	if (scene) {
		get_bounding_box(&scene_min,&scene_max);
		scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
		scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
		scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
		return 0;
	}
	return 1;
}

int LoadGLTextures(const aiScene* scene)
{
	ILboolean success;

	/* Before calling ilInit() version should be checked. */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		ILint test = ilGetInteger(IL_VERSION_NUM);
		/// wrong DevIL version ///
		std::string err_msg = "Wrong DevIL version. Old devil.dll in system32/SysWow64?";
		char* cErr_msg = (char *) err_msg.c_str();
		
		return -1;
	}

	ilInit(); /* Initialization of DevIL */

	//if (scene->HasTextures()) abortGLInit("Support for meshes with embedded textures is not implemented");

	/* getTexture Filenames and Numb of Textures */
	for (unsigned int m=0; m<scene->mNumMaterials; m++)
	{
		int texIndex = 0;
		aiReturn texFound = AI_SUCCESS;

		aiString path;	// filename

		while (texFound == AI_SUCCESS)
		{
			texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
			textureIdMap[path.data] = NULL; //fill map with textures, pointers still NULL yet
			texIndex++;
		}
	}

	int numTextures = textureIdMap.size();

	/* array with DevIL image IDs */
	ILuint* imageIds = NULL;
	imageIds = new ILuint[numTextures];

	/* generate DevIL Image IDs */
	ilGenImages(numTextures, imageIds); /* Generation of numTextures image names */

	/* create and fill array with GL texture ids */
	textureIds = new GLuint[numTextures];
	glGenTextures(numTextures, textureIds); /* Texture name generation */

	/* define texture path */
	//std::string texturepath = "../../../test/models/Obj/";

	/* get iterator */
	std::map<std::string, GLuint*>::iterator itr = textureIdMap.begin();

	for (int i=0; i<numTextures; i++)
	{

		//save IL image ID
		std::string filename = (*itr).first;  // get filename
		(*itr).second =  &textureIds[i];	  // save texture id for filename in map
		itr++;								  // next texture


		ilBindImage(imageIds[i]); /* Binding of DevIL image name */
		std::string fileloc = basepath + filename;	/* Loading of image */
		success = ilLoadImage((const wchar_t *)fileloc.c_str());

		fprintf(stdout,"Loading Image: %s\n", fileloc.data());	

		if (success) /* If no error occured: */
		{
			success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE); /* Convert every colour component into
			unsigned byte. If your image contains alpha channel you can replace IL_RGB with IL_RGBA */
			if (!success)
			{
				/* Error occured */
				fprintf(stderr,"Couldn't convert image");
				return -1;
			}
			//glGenTextures(numTextures, &textureIds[i]); /* Texture name generation */
			glBindTexture(GL_TEXTURE_2D, textureIds[i]); /* Binding of texture name */
			//redefine standard texture values
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear
			interpolation for magnification filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear
			interpolation for minifying filter */
			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
				ilGetData()); /* Texture specification */
		}
		else
		{
			/* Error occured */			
			fprintf(stderr,"Couldn't load Image: %s\n", fileloc.data());
		}
	}
	ilDeleteImages(numTextures, imageIds); /* Because we have already copied image data into texture data
	we can release memory used by image. */

	//Cleanup
	delete [] imageIds;
	imageIds = NULL;

	//return success;
	return TRUE;
}

int InitGL()					 // All Setup For OpenGL goes here
{
	if (!LoadGLTextures(scene))
	{
		return FALSE;
	}

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);		 // Enables Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);				// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);		// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);			// The Type Of Depth Test To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculation

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);    // Uses default lighting parameters
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_NORMALIZE);

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT1);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	return TRUE;					// Initialization Went OK
}
//-----------------------------------------------------------------------------


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
		loc_x = 1.0f;
		loc_z = 0.0f;
		cam_x = 0.0f;
		cam_z = 0.0f;
		break;
	case 'p':


		/*for (unsigned int m = 0; m < scene->mRootNode->mNumChildren; m++) {
			printf("prova: ");
			printf(scene->mRootNode->mChildren[m]->mName.data);
		}*/
		printf(scene->mRootNode->mChildren[0]->mName.data);

		printf("ciaooo\n");
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

void processNKeysReleased(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
	case 's': dX = 0; dZ = 0;break;
	case 'a':
	case 'd': dAngleY = 0.0f;break;
	}
}


// ----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct aiLogStream stream;

	glutInitWindowSize(900,600);
	glutInitWindowPosition(100,100);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInit(&argc, argv);

	glutCreateWindow("Assimp - Very simple OpenGL sample");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutIdleFunc(display);


	// get a handle to the predefined STDOUT log stream and attach
	// it to the logging system. It will be active for all further
	// calls to aiImportFile(Ex) and aiApplyPostProcessing.

	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);

	// ... exactly the same, but this stream will now write the
	// log file to assimp_log.txt
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE,"assimp_log.txt");
	aiAttachLogStream(&stream);

	//load this model
	loadasset("models\\suzanne2.obj");
	// the model name can be specified on the command line. 
	/*if (argc >= 2)
		loadasset( "models/suzanne.blend" );
	else
	{
		fprintf(stderr,"usage: >>SimpleOpenGLTexturedLoader <modelpath\\modelname>");
		exit(1);
	}	*/

	if (!InitGL())
	{
		fprintf(stderr,"Initialization failed");
		return FALSE;
	}

	//------------

	// handle input
	glutKeyboardFunc(processNKeys);
	glutSpecialFunc(processSKeys);
	//glutSpecialUpFunc(processSkeysReleased);
	glutKeyboardUpFunc(processNKeysReleased);


	//------------


	glutGet(GLUT_ELAPSED_TIME);
	glutMainLoop();

	// cleanup - calling 'aiReleaseImport' is important, as the library 
	// keeps internal resources until the scene is freed again. Not 
	// doing so can cause severe resource leaking.
	aiReleaseImport(scene);

	// We added a log stream to the library, it's our job to disable it
	// again. This will definitely release the last resources allocated
	// by Assimp.
	aiDetachAllLogStreams();	

	//--------------



	return 0;
}