/*
  CSCI 420 Computer Graphics, USC
  Assignment 2: Roller Coaster
  C++ starter code

  Student username: <katiepar>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <cstring>
#include "openGLHeader.h"
#include "imageIO.h"

#include <vector>
#include "glutHeader.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"
#include "openGLMatrix.h"

#ifdef WIN32
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#ifdef WIN32
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };
float cameraTranslate[3] = {0.0f, 0.0f, 0.0f};


int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

int currentPos = 0;

ImageIO * heightmapImage;

GLenum gLMode = GL_LINES;

BasicPipelineProgram* pipelineProgram;

GLuint buffer;
GLuint groundBuffer;
GLuint skyBackBuffer;
GLuint skyTopBuffer;
GLuint skyRightBuffer;
GLuint skyLeftBuffer;
GLuint skyFrontBuffer;

GLuint program;

OpenGLMatrix* matrix;

GLuint vao;
GLuint groundVao;
GLuint skyBackVao;
GLuint skyTopVao;
GLuint skyRightVao;
GLuint skyLeftVao;
GLuint skyFrontVao;

GLuint TrackTexHandle;
GLuint groundTexHandle;
GLuint SkyBackTexHandle;
GLuint SkyTopTexHandle;
GLuint SkyRightTexHandle;
GLuint SkyLeftTexHandle;
GLuint SkyFrontTexHandle;


int Screen=0;

/*dimensions for ground and sky*/
float ground[6][3] = {{-100.0f, -100.0f, 10.0f}, {-100.0f, 100.0f, -10.0f}, {100.0f, -100.0f, -10.0f}, 
                      {-100.0f, 100.0f, -10.0f}, {100.0f, 100.0f, -10.0f}, {100.0f, -100.0f, -10.0f}};
float skyBack[6][3] = {{-100.0f, -100.0f, -100.0f}, {-100.0f, -100.0f, 100.0f}, {100.0f, -100.0f, -100.0f},
                        {-100.0f, -100.0f, 100.0f}, {100.0f, -100.0f, 100.0f}, {100.0f, -100.0f, -100.0f}};
float skyTop[6][3] = {{-100.0f, -100.0f, 100.0f}, {100.0f, -100.0f, 100.0f}, {100.0f, 100.0f, 100.0f},
                        {-100.0f, -100.0f, 100.0f}, {-100.0f, 100.0f, 100.0f}, {100.0f, 100.0f, 100.0f}};
float skyRight[6][3] = {{-100.0f, -100.0f, -100.0f}, {-100.0f, 100.0f, -100.0f}, {-100.0f, -100.0f, 100.0f},
                        {-100.0f, 100.0f, -100.0f}, {-100.0f, 100.0f, 100.0f}, {-100.0f, -100.0f, 100.0f}};
float skyLeft[6][3] = {{100.0f, -100.0f, -100.0f}, {100.0f, 100.0f, -100.0f}, {100.0f, -100.0f, 100.0f},
                        {100.0f, 100.0f, -100.0f}, {100.0f, 100.0f, 100.0f}, {100.0f, -100.0f, 100.0f}};
float skyFront[6][3] = {{-100.0f, 100.0f, -100.0f}, {-100.0f, 100.0f, 100.0f}, {100.0f, 100.0f, -100.0f},
                        {-100.0f, 100.0f, 100.0f}, {100.0f, 100.0f, 100.0f}, {100.0f, 100.0f, -100.0f}};


/*UVs for ground and skybox*/
float grounduvs[6][2] = {{0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, 
                          {1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}};
float skyBackUvs[6][2] = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f},
                          {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}};
float skyTopUvs[6][2] = {{0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f},
                          {1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}};
float skyRightUvs[6][2] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f},
                            {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
float skyLeftUvs[6][2] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f},
                            {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
float skyFrontUvs[6][2] = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f},
                          {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}};

// represents one control point along the spline 
struct Point 
{
  double x;
  double y;
  double z;
};

// spline struct 
// contains how many control points the spline has, and an array of control points 
struct Spline 
{
  int numControlPoints;
  Point * points;
};

// the spline array 
Spline * splines;
// total number of splines 
int numSplines;

//positions, tangents, normals, binormals
vector<Point> positions;
vector<Point> tangents;
vector<Point> normals;
vector<Point> binormals;

/*the actualy spline track triangles that will
  go into the VBO*/
vector<float> track;
vector<float> track_uvs;


/*Math used for calculating the normals and binormals*/

Point Normalize(Point n) {
  float length = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
  n.x /= length;
  n.y /= length;
  n.z /= length;
  return n;
}

Point CrossProduct(Point a, Point b) {
  Point c;
  c.x = a.y*b.z - a.z*b.y;
  c.y = a.z*b.x - a.x*b.z;
  c.z = a.x*b.y - a.y*b.x;
  return c;
}

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void initVBO() {

  //VBO for track
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * track.size()) + (sizeof(float) * track_uvs.size()), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof(float) * track.size()), track.data());
  glBufferSubData(GL_ARRAY_BUFFER, (sizeof(float) * track.size()), (sizeof(float) * track_uvs.size()), track_uvs.data());

  //VBO for ground
  glGenBuffers(1, &groundBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, groundBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ground) + sizeof(grounduvs), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ground), ground);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(ground), sizeof(grounduvs), grounduvs);

  //VBO for sky back
  glGenBuffers(1, &skyBackBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, skyBackBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyBack) + sizeof(skyBackUvs), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skyBack), skyBack);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(skyBack), sizeof(skyBackUvs), skyBackUvs);

  //VBO for sky top
  glGenBuffers(1, &skyTopBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, skyTopBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyTop) + sizeof(skyTopUvs), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skyTop), skyTop);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(skyTop), sizeof(skyTopUvs), skyTopUvs);

  //VBO for sky right
  glGenBuffers(1, &skyRightBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, skyRightBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyRight) + sizeof(skyRightUvs), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skyRight), skyRight);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(skyRight), sizeof(skyRightUvs), skyRightUvs);

  //VBO for sky left
  glGenBuffers(1, &skyLeftBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, skyLeftBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyLeft) + sizeof(skyLeftUvs), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skyLeft), skyLeft);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(skyLeft), sizeof(skyLeftUvs), skyLeftUvs);

  //VBO for sky front
  glGenBuffers(1, &skyFrontBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, skyFrontBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyFront) + sizeof(skyFrontUvs), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skyFront), skyFront);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(skyFront), sizeof(skyFrontUvs), skyFrontUvs);
}

void initVAO() {
  //VAO for track
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  GLuint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  void* offset = (void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  GLuint loc2 = glGetAttribLocation(program, "texCoord");
  glEnableVertexAttribArray(loc2);
  void* offset1 = (void*)(sizeof(float) * track.size());
  glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, 0, offset1); 

  glBindVertexArray(0);

  //VAO for ground
  glBindVertexArray(groundVao);
  glBindBuffer(GL_ARRAY_BUFFER, groundBuffer);

  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  offset = (void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  loc2 = glGetAttribLocation(program, "texCoord");
  glEnableVertexAttribArray(loc2);
  offset1 = (void*)(sizeof(ground));
  glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, 0, offset1); 

  glBindVertexArray(0);

  //VAO for sky back
  glBindVertexArray(skyBackVao);
  glBindBuffer(GL_ARRAY_BUFFER, skyBackBuffer);

  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  offset = (void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  loc2 = glGetAttribLocation(program, "texCoord");
  glEnableVertexAttribArray(loc2);
  offset1 = (void*)(sizeof(ground));
  glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, 0, offset1); 

  glBindVertexArray(0);

  //VAO for sky top
  glBindVertexArray(skyTopVao);
  glBindBuffer(GL_ARRAY_BUFFER, skyTopBuffer);

  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  offset = (void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  loc2 = glGetAttribLocation(program, "texCoord");
  glEnableVertexAttribArray(loc2);
  offset1 = (void*)(sizeof(ground));
  glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, 0, offset1); 

  glBindVertexArray(0);

  //VAO for sky right
  glBindVertexArray(skyRightVao);
  glBindBuffer(GL_ARRAY_BUFFER, skyRightBuffer);

  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  offset = (void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  loc2 = glGetAttribLocation(program, "texCoord");
  glEnableVertexAttribArray(loc2);
  offset1 = (void*)(sizeof(ground));
  glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, 0, offset1); 

  glBindVertexArray(0);

  //VAO for sky left
  glBindVertexArray(skyLeftVao);
  glBindBuffer(GL_ARRAY_BUFFER, skyLeftBuffer);

  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  offset = (void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  loc2 = glGetAttribLocation(program, "texCoord");
  glEnableVertexAttribArray(loc2);
  offset1 = (void*)(sizeof(ground));
  glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, 0, offset1); 

  glBindVertexArray(0);

  //VAO for sky front
  glBindVertexArray(skyFrontVao);
  glBindBuffer(GL_ARRAY_BUFFER, skyFrontBuffer);

  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  offset = (void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  loc2 = glGetAttribLocation(program, "texCoord");
  glEnableVertexAttribArray(loc2);
  offset1 = (void*)(sizeof(ground));
  glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, 0, offset1); 

  glBindVertexArray(0);
}

void bindProgram() {

  //write projection and modelview matrix to shader 
  GLint h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  float m[16];
  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
  matrix->GetMatrix(m);
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

  //Projection Matrix
  GLint h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  float p[16];
  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  matrix->GetMatrix(p);
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);

}

int loadSplines(char * argv) 
{
  char * cName = (char *) malloc(128 * sizeof(char));
  FILE * fileList;
  FILE * fileSpline;
  int iType, i = 0, j, iLength;

  // load the track file 
  fileList = fopen(argv, "r");
  if (fileList == NULL) 
  {
    printf ("can't open file\n");
    exit(1);
  }
  
  // stores the number of splines in a global variable 
  fscanf(fileList, "%d", &numSplines);

  splines = (Spline*) malloc(numSplines * sizeof(Spline));

  // reads through the spline files 
  for (j = 0; j < numSplines; j++) 
  {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) 
    {
      printf ("can't open file\n");
      exit(1);
    }

    // gets length for spline file
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    // allocate memory for all the points
    splines[j].points = (Point *)malloc(iLength * sizeof(Point));
    splines[j].numControlPoints = iLength;

    // saves the data to the struct
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &splines[j].points[i].x, 
	   &splines[j].points[i].y, 
	   &splines[j].points[i].z) != EOF) 
    {
      i++;
    }
  }

  free(cName);

  return 0;
}

void setTextureUnit(GLint unit)
{
  glActiveTexture(unit); // select the active texture unit
  // get a handle to the “textureImage” shader variable
  GLint h_textureImage = glGetUniformLocation(program, "textureImage");
  // deem the shader variable “textureImage” to read from texture unit “unit”
  glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}

int initTexture(const char * imageFilename, GLuint textureHandle)
{
  // read the texture image
  ImageIO img;
  ImageIO::fileFormatType imgFormat;
  ImageIO::errorType err = img.load(imageFilename, &imgFormat);

  if (err != ImageIO::OK) 
  {
    printf("Loading texture from %s failed.\n", imageFilename);
    return -1;
  }

  // check that the number of bytes is a multiple of 4
  if (img.getWidth() * img.getBytesPerPixel() % 4) 
  {
    printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
    return -1;
  }

  // allocate space for an array of pixels
  int width = img.getWidth();
  int height = img.getHeight();
  unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++)
    for (int w = 0; w < width; w++) 
    {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
      pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

      // set the RGBA channels, based on the loaded image
      int numChannels = img.getBytesPerPixel();
      for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
        pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
    }

  // bind the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  // initialize the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

  // generate the mipmaps for this texture
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // query support for anisotropic texture filtering
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  printf("Max available anisotropic samples: %f\n", fLargest);
  // set anisotropic texture filtering
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

  // query for any errors
  GLenum errCode = glGetError();
  if (errCode != 0) 
  {
    printf("Texture initialization error. Error code: %d.\n", errCode);
    return -1;
  }
  
  // de-allocate the pixel array -- it is no longer needed
  delete [] pixelsRGBA;

  return 0;
}

void displayFunc()
{
  // render some stuff...
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
  matrix->LoadIdentity();

  //camera movement implemented using Sloan's method
  float eyeX = positions[currentPos].x + (binormals[currentPos].x * (-0.035));
  float eyeY = positions[currentPos].y + (binormals[currentPos].y * (-0.035));
  float eyeZ = positions[currentPos].z + (binormals[currentPos].z * (-0.035));

  matrix->LookAt(eyeX, eyeY, eyeZ, 
              eyeX + tangents[currentPos].x, eyeY + tangents[currentPos].y, eyeZ + tangents[currentPos].z, 
              -binormals[currentPos].x, -binormals[currentPos].y, -binormals[currentPos].z);
  matrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix->Rotate(landRotate[0], 1.0, 0.0, 0.0);
  matrix->Rotate(landRotate[1], 0.0, 1.0, 0.0);
  matrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);
  matrix->Scale(landScale[0], landScale[1], landScale[2]);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  bindProgram();
  initVAO();
  initVBO();

  //texture stuff
  //draw sky, spline, and ground
  setTextureUnit(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, groundTexHandle);
  glBindVertexArray(groundVao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, SkyTopTexHandle);
  glBindVertexArray(skyTopVao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, SkyBackTexHandle);
  glBindVertexArray(skyBackVao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, SkyRightTexHandle);
  glBindVertexArray(skyRightVao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, SkyLeftTexHandle);
  glBindVertexArray(skyLeftVao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, SkyFrontTexHandle);
  glBindVertexArray(skyFrontVao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindTexture(GL_TEXTURE_2D, TrackTexHandle);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, track.size());


  glBindVertexArray(0);

  //renderTriangle(); 
  glutSwapBuffers();
}

void idleFunc()
{
  // do some stuff... 
  currentPos += 50.0f;
  if (currentPos >= positions.size()) {
    currentPos = 0;
  }

  // for example, here, you can save the screenshots to disk (to make the animation)
  //save series of screenshots
  string pic = "ScreenShots/screenshot";
  string intStr = to_string(Screen);
  pic += intStr;
  pic += ".jpg";
  saveScreenshot(pic.c_str());

  // make the screen update 
  glutPostRedisplay();
  Screen++;
}

void reshapeFunc(int w, int h)
{
  GLfloat aspect = (GLfloat) w / (GLfloat) h;
  glViewport(0, 0, w, h);

  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  matrix->LoadIdentity();
  matrix->Perspective(130.0f, aspect, 0.01f, 1000.0f);
  //matrix->Ortho(-2.0, 2.0, -2.0/aspect, 2.0/aspect, 0.0, 10.0);
  //matrix->SetMatrixMode(OpenGLMatrix::ModelView);

  // setup perspective matrix...
}

void calculateSplineTriangles() {

  //compute the normals and binormals for cross section
  Point init;
  init.x = 0.0f;
  init.y = 0.0f;
  init.z = 1.0f;
  //camera movement implemented using Sloan's method
  normals.push_back(Normalize(CrossProduct(tangents[0], init)));
  binormals.push_back(Normalize(CrossProduct(tangents[0], normals[0])));
  for (int i=1; i<positions.size(); i++) {
    normals.push_back(Normalize(CrossProduct(binormals[i-1], tangents[i])));
    binormals.push_back(Normalize(CrossProduct(tangents[i],normals[i])));
  }

  //formula for rail cross section
  for (int i=0; i<positions.size()-1; i++) {
    const float alpha = 0.015f;

    //FRONT FACE
    // V3
    track.push_back(positions[i].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z - binormals[i].z));
    // V2
    track.push_back(positions[i].x + alpha * (normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (normals[i].z - binormals[i].z));
    // V1
    track.push_back(positions[i].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y + binormals[i].y)); 
    track.push_back(positions[i].z + alpha * (normals[i].z + binormals[i].z));
    // V1
    track.push_back(positions[i].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i].z + alpha * (normals[i].z + binormals[i].z));
    // V0
    track.push_back(positions[i].x + alpha * (-normals[i].x + binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y + binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z + binormals[i].z));
    // V3
    track.push_back(positions[i].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z - binormals[i].z));

    //BACK FACE
    // V7
    track.push_back(positions[i+1].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z - binormals[i].z));
    // V6
    track.push_back(positions[i+1].x + alpha * (normals[i].x - binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (normals[i].y - binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (normals[i].z - binormals[i].z));
    // V5
    track.push_back(positions[i+1].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (normals[i].z + binormals[i].z));
    // V5
    track.push_back(positions[i+1].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (normals[i].z + binormals[i].z));
    // V4
    track.push_back(positions[i+1].x + alpha * (-normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (-normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z + binormals[i].z));
    // V7
    track.push_back(positions[i+1].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z - binormals[i].z));

    //RIGHT FACE
    // V0
    track.push_back(positions[i].x + alpha * (-normals[i].x + binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y + binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z + binormals[i].z));
    // V1
    track.push_back(positions[i].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i].z + alpha * (normals[i].z + binormals[i].z));
    // V5
    track.push_back(positions[i+1].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (normals[i].z + binormals[i].z));
    // V5
    track.push_back(positions[i+1].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (normals[i].z + binormals[i].z));
    // V4
    track.push_back(positions[i+1].x + alpha * (-normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (-normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z + binormals[i].z));
    // V0
    track.push_back(positions[i].x + alpha * (-normals[i].x + binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y + binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z + binormals[i].z));

    //LEFT FACE
    //V7
    track.push_back(positions[i+1].x + alpha * (-normals[i].x - binormals[i].x)); 
    track.push_back(positions[i+1].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z - binormals[i].z));
    //V6
    track.push_back(positions[i+1].x + alpha * (normals[i].x - binormals[i].x)); 
    track.push_back(positions[i+1].y + alpha * (normals[i].y - binormals[i].y)); 
    track.push_back(positions[i+1].z + alpha * (normals[i].z - binormals[i].z));
    //V2
    track.push_back(positions[i].x + alpha * (normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (normals[i].z - binormals[i].z));
    //V2
    track.push_back(positions[i].x + alpha * (normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (normals[i].z - binormals[i].z));
    //V3
    track.push_back(positions[i].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z - binormals[i].z));
    // V7
    track.push_back(positions[i+1].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z - binormals[i].z));

    //TOP FACE
    // V2
    track.push_back(positions[i].x + alpha * (normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (normals[i].z - binormals[i].z));
    // V6
    track.push_back(positions[i+1].x + alpha * (normals[i].x - binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (normals[i].y - binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (normals[i].z - binormals[i].z));
    // V5
    track.push_back(positions[i+1].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (normals[i].z + binormals[i].z));
    // V5
    track.push_back(positions[i+1].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (normals[i].z + binormals[i].z));
    // V1
    track.push_back(positions[i].x + alpha * (normals[i].x + binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y + binormals[i].y));
    track.push_back(positions[i].z + alpha * (normals[i].z + binormals[i].z));
    // V2
    track.push_back(positions[i].x + alpha * (normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (normals[i].z - binormals[i].z));

    //BOTTOM FACE
    // V3
    track.push_back(positions[i].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z - binormals[i].z));
    // V7
    track.push_back(positions[i+1].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z - binormals[i].z));
    // V4
    track.push_back(positions[i+1].x + alpha * (-normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (-normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z + binormals[i].z));
    // V4
    track.push_back(positions[i+1].x + alpha * (-normals[i].x + binormals[i].x));
    track.push_back(positions[i+1].y + alpha * (-normals[i].y + binormals[i].y));
    track.push_back(positions[i+1].z + alpha * (-normals[i].z + binormals[i].z));
    // V0
    track.push_back(positions[i].x + alpha * (-normals[i].x + binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y + binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z + binormals[i].z));
    // V3
    track.push_back(positions[i].x + alpha * (-normals[i].x - binormals[i].x));
    track.push_back(positions[i].y + alpha * (-normals[i].y - binormals[i].y));
    track.push_back(positions[i].z + alpha * (-normals[i].z - binormals[i].z));


    for (int j=0; j<6; j++) {
      track_uvs.push_back(0.0f);
      track_uvs.push_back(0.0f);

      track_uvs.push_back(0.0f);
      track_uvs.push_back(1.0f);

      track_uvs.push_back(1.0f);
      track_uvs.push_back(1.0f);

      track_uvs.push_back(1.0f);
      track_uvs.push_back(1.0f);

      track_uvs.push_back(0.0f);
      track_uvs.push_back(1.0f);

      track_uvs.push_back(0.0f);
      track_uvs.push_back(0.0f);
    }

  }
}

void calculateSplinePoints(float u, float s) {

  for (int i=0; i<numSplines; i++) {
    for (int j=0; j<splines[i].numControlPoints-3; j++) {
      for (float k=0; k<=1.0; k+=u) {

        //cout << "splines[" << i << "].numControlPoints-3: "  << splines[i].numControlPoints-3 << endl;

        float u2 = k*k;
        float u3 = k*k*k;

        //calculate the points using catmull rom
        Point temp;
        temp.x = (u3*-1*s + u2*2*s - k*s) * splines[i].points[j].x 
                    + (u3*(2-s) + u2*(s-3)+1) * splines[i].points[j+1].x 
                    + (u3*(s-2) + u2*(3-2*s) + k*s) * splines[i].points[j+2].x 
                    + (u3*s - u2*s) * splines[i].points[j+3].x;
        temp.y = (u3*-1*s + u2*2*s - k*s) * splines[i].points[j].y 
                    + (u3*(2-s) + u2*(s-3)+1) * splines[i].points[j+1].y 
                    + (u3*(s-2) + u2*(3-2*s) + k*s) * splines[i].points[j+2].y 
                    + (u3*s - u2*s) * splines[i].points[j+3].y;
        temp.z = (u3*-1*s + u2*2*s - k*s) * splines[i].points[j].z 
                    + (u3*(2-s) + u2*(s-3)+1) * splines[i].points[j+1].z 
                    + (u3*(s-2) + u2*(3-2*s) + k*s) * splines[i].points[j+2].z 
                    + (u3*s - u2*s) * splines[i].points[j+3].z;
        positions.push_back(temp);



        //calculate the tanget points by taking the derivative
        u2 = 2 * k;
        u3 = 3 * k*k;

        temp.x = (u3*-1*s + u2*2*s - s) * splines[i].points[j].x 
                          + (u3*(2-s) + u2*(s-3)) * splines[i].points[j+1].x 
                          + (u3*(s-2) + u2*(3-2*s) + s) * splines[i].points[j+2].x 
                          + (u3*s - u2*s) * splines[i].points[j+3].x;
        temp.y = (u3*-1*s + u2*2*s - s) * splines[i].points[j].y 
                          + (u3*(2-s) + u2*(s-3)) * splines[i].points[j+1].y 
                          + (u3*(s-2) + u2*(3-2*s) + s) * splines[i].points[j+2].y 
                          + (u3*s - u2*s) * splines[i].points[j+3].y;
        temp.z = (u3*-1*s + u2*2*s - s) * splines[i].points[j].z 
                          + (u3*(2-s) + u2*(s-3)) * splines[i].points[j+1].z 
                          + (u3*(s-2) + u2*(3-2*s) + s) * splines[i].points[j+2].z 
                          + (u3*s - u2*s) * splines[i].points[j+3].z;
        tangents.push_back(Normalize(temp));

      }
    }
  }

  //triangles for the cross section of spline
  calculateSplineTriangles();

}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_ALT:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;


  }
}


void initPipelineProgram() {
  //initialize shader pipeline program
  pipelineProgram = new BasicPipelineProgram();
  pipelineProgram->Init("../openGLHelper-starterCode");
  pipelineProgram->Bind();
  program = pipelineProgram->GetProgramHandle();
}

void initScene(int argc, char *argv[])
{
  // load the splines from the provided filename
  loadSplines(argv[1]);

  printf("Loaded %d spline(s).\n", numSplines);
  for(int i=0; i<numSplines; i++)
    printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  matrix = new OpenGLMatrix();

  calculateSplinePoints(0.001f, 0.5f);
  
  initPipelineProgram();


  glGenVertexArrays(1, &vao);
  glGenVertexArrays(1, &groundVao);
  glGenVertexArrays(1, &skyBackVao);
  glGenVertexArrays(1, &skyTopVao);
  glGenVertexArrays(1, &skyRightVao);
  glGenVertexArrays(1, &skyLeftVao);
  glGenVertexArrays(1, &skyFrontVao);

  // do additional initialization here...

  //do initialization of texture images
  glGenTextures(1, &groundTexHandle);
  int file = initTexture("ground.jpg", groundTexHandle);
  if (file != 0){
    printf("Error loading the texture image.\n");
    exit(EXIT_FAILURE);
  }

  glGenTextures(1, &SkyBackTexHandle);
  file = initTexture("sky.jpg", SkyBackTexHandle);
  if (file != 0){
    printf("Error loading the texture image.\n");
    exit(EXIT_FAILURE);
  }

  glGenTextures(1, &SkyTopTexHandle);
  file = initTexture("sky.jpg", SkyTopTexHandle);
  if (file != 0){
    printf("Error loading the texture image.\n");
    exit(EXIT_FAILURE);
  }

  glGenTextures(1, &SkyRightTexHandle);
  file = initTexture("sky.jpg", SkyRightTexHandle);
  if (file != 0){
    printf("Error loading the texture image.\n");
    exit(EXIT_FAILURE);
  }

  glGenTextures(1, &SkyLeftTexHandle);
  file = initTexture("sky.jpg", SkyLeftTexHandle);
  if (file != 0){
    printf("Error loading the texture image.\n");
    exit(EXIT_FAILURE);
  }

  glGenTextures(1, &SkyFrontTexHandle);
  file = initTexture("sky.jpg", SkyFrontTexHandle);
  if (file != 0){
    printf("Error loading the texture image.\n");
    exit(EXIT_FAILURE);
  }

  glGenTextures(1, &TrackTexHandle);
  file = initTexture("track.jpg", TrackTexHandle);
  if (file != 0){
    printf("Error loading the texture image.\n");
    exit(EXIT_FAILURE);
  }
}

// Note: You should combine this file
// with the solution of homework 1.

// Note for Windows/MS Visual Studio:
// You should set argv[1] to track.txt.
// To do this, on the "Solution Explorer",
// right click your project, choose "Properties",
// go to "Configuration Properties", click "Debug",
// then type your track file name for the "Command Arguments".
// You can also repeat this process for the "Release" configuration.

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
    printf ("usage: %s <trackfile>\n", argv[0]);
    exit(0);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();

  return 0;
}

