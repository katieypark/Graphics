/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: <katiepar>
 * *************************
*/

#ifdef WIN32
  #include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
  #include <GL/gl.h>
  #include <GL/glut.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
  #define strcasecmp _stricmp
#endif

#include <imageIO.h>
#include <math.h>
#include <iostream>

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0

#define PI 3.14159265

double triangletrace = 1000;
double spheretrace = 1000;

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

int counter = 0;

struct Triangle
{
  Vertex v[3];
};

struct Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
};

struct Light
{
  double position[3];
  double color[3];
};

struct Vector
{
  double x;
  double y;
  double z;

  double length() 
  {
    return sqrt(x*x + y*y + z*z);
  }
};

Vector Normalize(Vector x)
{
  double length = x.length();

  Vector newx;
  newx.x = x.x / length;
  newx.y = x.y / length;
  newx.z = x.z / length;

  return newx;
}

Vector CrossProduct(Vector a, Vector b) {
  Vector c;
  c.x = a.y*b.z - a.z*b.y;
  c.y = a.z*b.x - a.x*b.z;
  c.z = a.x*b.y - a.y*b.x;
  return c;
}

double DotProduct(Vector a, Vector b) 
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct Ray
{
  Vector direction;
  Vector position;
};

Triangle collisionTriangle;
Sphere collisionSphere;
Vector finalColor;

double Clamp(double max, double min, double x)
{
  return std::min(std::max(x, min), max);
}

//ray trace helper function
struct Ray RayTrace(int pointx, int pointy, Vector camPosition)
{
  double aspectRatio = 4.0 / 3.0;

  Vector direction;
  direction.x = (-aspectRatio * tan(fov * PI / 180 / 2.0)) + 
              (-((-aspectRatio * tan(fov * PI / 180 / 2.0)) / 640.0) * 2.0) * pointx;
  direction.y = -tan(fov * PI / 180 / 2.0) + 
              (-((-tan(fov * PI / 180 / 2.0)) / 480.0) * 2.0) * pointy;
  direction.z = -1;
  direction = Normalize(direction);
  struct Ray ray;
  ray.position = camPosition;
  ray.direction = direction;

  return ray;
}

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

//implementing ambient light into lighting
Vector AmbientLight(Vector color)
{
  color.x += ambient_light[0] * 255;
  color.y += ambient_light[1] * 255;
  color.z += ambient_light[2] * 255;
  color.x = Clamp(255.0, 0.0, color.x); color.y = Clamp(255.0, 0.0, color.y); color.z = Clamp(255.0, 0.0, color.z);
  return color;
}

//ray sphere intersection
//equations from Ray-Sphere Intersction, slide 16, page 6
bool SphereCollision(Ray ray, Sphere& m_sphere)
{
  bool collision = false;
  Sphere collsphere = m_sphere;
  int i=0;

  while (i<num_spheres) 
  {
    Sphere currSphere = spheres[i];

    double det = pow((2 * (ray.direction.x * (ray.position.x - currSphere.position[0]) + 
      ray.direction.y * (ray.position.y - currSphere.position[1]) + 
      ray.direction.z * (ray.position.z - currSphere.position[2]))), 2) - 
      4 * (pow(ray.position.x - currSphere.position[0], 2) + 
      pow(ray.position.y - currSphere.position[1], 2) + pow(ray.position.z - currSphere.position[2], 2) -
      pow(currSphere.radius, 2));

    if (det < 0.000001) {
      i++;
      continue;
    }

    double t0 = -(2 * (ray.direction.x * (ray.position.x - currSphere.position[0]) + 
      ray.direction.y * (ray.position.y - currSphere.position[1]) + 
      ray.direction.z * (ray.position.z - currSphere.position[2]))) + sqrt(det);
    double t1 = -(2 * (ray.direction.x * (ray.position.x - currSphere.position[0]) + 
      ray.direction.y * (ray.position.y - currSphere.position[1]) + 
      ray.direction.z * (ray.position.z - currSphere.position[2]))) - sqrt(det);

    t0 /= 2;
    t1 /= 2;

    if (t0<0.000001 || t1 <0.000001) {
      i++;
      continue;
    }

    if (t0>0 && t1>0) {
      collision = true;
      if (fmin(t0, t1) < 1000)
      {
        spheretrace = fmin(t0, t1);
        collsphere = currSphere;
      } 
    }
    i++;
  }

  m_sphere = collsphere;
  return collision;
}

//Phong color calculations using I = lightColor * (kd * (L dot N) + ks * (R dot V) ^ sh) 
Vector PhongSphereColor(Sphere m_sphere, Vector pos, Light light)
{
  Vector lightDir, normal, camVec;
  lightDir.x = light.position[0] - pos.x; lightDir.y = light.position[1] - pos.y; lightDir.z = light.position[2] - pos.z;
  normal.x = pos.x - m_sphere.position[0]; normal.y = pos.y - m_sphere.position[1]; normal.z = pos.z - m_sphere.position[2];
  normal.x *= (1.0 / m_sphere.radius); normal.y *= (1.0 / m_sphere.radius); normal.z *= (1.0 / m_sphere.radius);
  camVec = pos;
  lightDir = Normalize(lightDir); normal = Normalize(normal); camVec = Normalize(camVec);

  double LN = fmax(DotProduct(lightDir, normal), 0);

  Vector reflect;
  reflect.x = lightDir.x - 2.0 * LN * normal.x; reflect.y = lightDir.y - 2.0 * LN * normal.y; reflect.z = lightDir.z - 2.0 * LN * normal.z;
  reflect = Normalize(reflect);

  double RV = fmax(DotProduct(reflect, camVec), 0);

  Vector newColor;
  newColor.x = light.color[0] * 255.0 * (m_sphere.color_diffuse[0] * LN + m_sphere.color_specular[0] * pow(RV, m_sphere.shininess));
  newColor.y = light.color[1] * 255.0 * (m_sphere.color_diffuse[1] * LN + m_sphere.color_specular[1] * pow(RV, m_sphere.shininess));
  newColor.z = light.color[2] * 255.0 * (m_sphere.color_diffuse[2] * LN + m_sphere.color_specular[2] * pow(RV, m_sphere.shininess));

  newColor.x = Clamp(255.0, 0.0, newColor.x); newColor.y = Clamp(255.0, 0.0, newColor.y); newColor.z = Clamp(255.0, 0.0, newColor.z);

  newColor = AmbientLight(newColor);

  return newColor;
}

//points and edges of triangles
void TriangleCalculations(Triangle& currTriangle, Vector& pointA, Vector& pointB, Vector& pointC, Vector& edgeA, Vector& edgeB)
{
    pointA.x = currTriangle.v[0].position[0]; pointA.y = currTriangle.v[0].position[1]; pointA.z = currTriangle.v[0].position[2];
    pointB.x = currTriangle.v[1].position[0]; pointB.y = currTriangle.v[1].position[1]; pointB.z = currTriangle.v[1].position[2];
    pointC.x = currTriangle.v[2].position[0]; pointC.y = currTriangle.v[2].position[1]; pointC.z = currTriangle.v[2].position[2];

    edgeA.x = pointB.x - pointA.x; edgeA.y = pointB.y - pointA.y; edgeA.z = pointB.z - pointA.z;
    edgeB.x = pointC.x - pointA.x; edgeB.y = pointC.y - pointA.y; edgeB.z = pointC.z - pointA.z;
}

bool TriangleCollision(Ray ray, Triangle& m_triangle)
{
  bool collision = false;
  Triangle collTriangle = m_triangle;
  int i=0;

  while (i<num_triangles) 
  {
    Vector pointA, pointB, pointC, edgeA, edgeB;
    TriangleCalculations(triangles[i], pointA, pointB, pointC, edgeA, edgeB);
    
    Vector T;
    T.x = ray.position.x - pointA.x; T.y = ray.position.y - pointA.y; T.z = ray.position.z - pointA.z;
    double u = DotProduct(T, CrossProduct(ray.direction, edgeB)) * (1.0 / (DotProduct(edgeA, CrossProduct(ray.direction, edgeB))));

    if (u<0 || u>1) {
      i++;
      continue;
    }

    double v = DotProduct(ray.direction, CrossProduct(T, edgeA)) * (1.0 / (DotProduct(edgeA, CrossProduct(ray.direction, edgeB))));

    if (v<0 || u + v > 1) {
      i++;
      continue;
    }

    double t = DotProduct(edgeB, CrossProduct(T, edgeA)) * (1.0 / (DotProduct(edgeA, CrossProduct(ray.direction, edgeB))));

    if (t > 0.000001 && t < 1000)
    {
      triangletrace = t;
      collTriangle = triangles[i];
      collision = true;
    }
    i++;

  }

  m_triangle = collTriangle;
  return collision;
}

//check if sphere or triangle in shadow
bool ShadowCheck(Vector pos, Light light)
{
  bool coll = false;

  Vector lightdir;
  lightdir.x = light.position[0] - pos.x; lightdir.y = light.position[1] - pos.y; lightdir.z = light.position[2] - pos.z;
  lightdir = Normalize(lightdir);

  Ray shadow;
  shadow.position.x = pos.x; shadow.position.y = pos.y; shadow.position.z = pos.z;
  shadow.direction.x = lightdir.x; shadow.direction.y = lightdir.y; shadow.direction.z = lightdir.z;

  Sphere collSphere;
  Triangle collTriangle;
  double dist;

  bool spherec = SphereCollision(shadow, collSphere);
  bool trianglec = TriangleCollision(shadow, collTriangle);

  if (spherec || trianglec) 
  {
    if (spherec)
    {
      dist = spheretrace;
    } else {
      dist = triangletrace;
    }
    if (dist > 0.000001)
    {
      Vector lightDist;
      lightDist.x = light.position[0] - pos.x; lightDist.y = light.position[1] - pos.y; lightDist.z = light.position[2] - pos.z;
      Vector shadowLength, lightToShadow;
      shadowLength.x = shadow.position.x + (shadow.direction.x * dist);
      shadowLength.y = shadow.position.y + (shadow.direction.y * dist);
      shadowLength.z = shadow.position.z + (shadow.direction.z * dist);
      lightToShadow.x = light.position[0] - shadowLength.x; lightToShadow.y = light.position[1] - shadowLength.y; lightToShadow.z = light.position[2] - shadowLength.z;

      if (lightToShadow.length() < lightDist.length() - 0.000001)
      {
        coll = true;
      }
    }
  }
  return coll;
}



//Phong color calculations using I = lightColor * (kd * (L dot N) + ks * (R dot V) ^ sh) 
Vector PhongTriangleColor(Triangle m_Triangle, Ray ray, Vector pos, Light light)
{
  Vector PointA, PointB, PointC, EdgeA, EdgeB;
  TriangleCalculations(m_Triangle, PointA, PointB, PointC, EdgeA, EdgeB);

  Vector T;
  T.x = ray.position.x - PointA.x; T.y = ray.position.y - PointA.y; T.z = ray.position.z - PointA.z;
  double u = DotProduct(T, CrossProduct(ray.direction, EdgeB)) * (1 / (DotProduct(EdgeA, CrossProduct(ray.direction, EdgeB))));
  double v = DotProduct(ray.direction, CrossProduct(T, EdgeA)) * (1 / (DotProduct(EdgeA, CrossProduct(ray.direction, EdgeB))));

  Vector PointANormal, PointBNormal, PointCNormal;
  PointANormal.x = m_Triangle.v[0].normal[0]; PointANormal.y = m_Triangle.v[0].normal[1]; PointANormal.z = m_Triangle.v[0].normal[2];
  PointBNormal.x = m_Triangle.v[1].normal[0]; PointBNormal.y = m_Triangle.v[1].normal[1]; PointBNormal.z = m_Triangle.v[1].normal[2];
  PointCNormal.x = m_Triangle.v[2].normal[0]; PointCNormal.y = m_Triangle.v[2].normal[1]; PointCNormal.z = m_Triangle.v[2].normal[2];
  PointANormal = Normalize(PointANormal); PointBNormal = Normalize(PointBNormal); PointCNormal = Normalize(PointCNormal);

  Vector normal;
  normal.x = ((1.0 - u - v) * PointANormal.x) + (u * PointBNormal.x) + (v * PointCNormal.x);
  normal.y = ((1.0 - u - v) * PointANormal.y) + (u * PointBNormal.y) + (v * PointCNormal.y);
  normal.z = ((1.0 - u - v) * PointANormal.z) + (u * PointBNormal.z) + (v * PointCNormal.z);
  normal = Normalize(normal);

  Vector lightDir, camVec;
  camVec = pos;
  lightDir.x = light.position[0] - pos.x; lightDir.y = light.position[1] - pos.y; lightDir.z = light.position[2] - pos.z;
  lightDir = Normalize(lightDir); camVec = Normalize(camVec);

  double LN = fmax(DotProduct(lightDir, normal), 0);

  Vector reflect;
  reflect.x = lightDir.x - 2.0 * LN * normal.x;
  reflect.y = lightDir.y - 2.0 * LN * normal.y;
  reflect.z = lightDir.z - 2.0 * LN * normal.z;
  reflect = Normalize(reflect);

  double RV = fmax(DotProduct(reflect, camVec), 0);

  Vector specular;
  specular.x = (1.0 - u - v) * m_Triangle.v[0].color_specular[0] + u * m_Triangle.v[1].color_specular[0] + v * m_Triangle.v[2].color_specular[0];
  specular.y = (1.0 - u - v) * m_Triangle.v[0].color_specular[1] + u * m_Triangle.v[1].color_specular[1] + v * m_Triangle.v[2].color_specular[1];
  specular.z = (1.0 - u - v) * m_Triangle.v[0].color_specular[2] + u * m_Triangle.v[1].color_specular[2] + v * m_Triangle.v[2].color_specular[2];

  Vector diffuse;
  diffuse.x = (1.0 - u - v) * m_Triangle.v[0].color_diffuse[0] + u * m_Triangle.v[1].color_diffuse[0] + v * m_Triangle.v[2].color_diffuse[0];
  diffuse.y = (1.0 - u - v) * m_Triangle.v[0].color_diffuse[1] + u * m_Triangle.v[1].color_diffuse[1] + v * m_Triangle.v[2].color_diffuse[1];
  diffuse.z = (1.0 - u - v) * m_Triangle.v[0].color_diffuse[2] + u * m_Triangle.v[1].color_diffuse[2] + v * m_Triangle.v[2].color_diffuse[2];

  double alpha = (1.0 - u - v) * m_Triangle.v[0].shininess + u * m_Triangle.v[1].shininess + v * m_Triangle.v[2].shininess;

  Vector newColor;
  newColor.x = light.color[0] * 255.0 * (diffuse.x * LN + specular.x * pow(RV, alpha));
  newColor.y = light.color[1] * 255.0 * (diffuse.y * LN + specular.y * pow(RV, alpha));
  newColor.z = light.color[2] * 255.0 * (diffuse.z * LN + specular.z * pow(RV, alpha));
  newColor.x = Clamp(255.0, 0.0, newColor.x); newColor.y = Clamp(255.0, 0.0, newColor.y); newColor.z = Clamp(255.0, 0.0, newColor.z);

  newColor = AmbientLight(newColor);

  return newColor;
}

//calculate appropriate color rgb values
Vector DoColor(bool trianglecoll, Ray ray, Sphere m_sphere, Triangle m_Triangle, double dist)
{
  Vector collpos;
  collpos.x = ray.position.x + ray.direction.x * dist;
  collpos.y = ray.position.y + ray.direction.y * dist;
  collpos.z = ray.position.z + ray.direction.z * dist;

  Vector col;
  int i=0;
  while (i<num_lights)
  {
    if (!ShadowCheck(collpos, lights[i]))
    {
      if (trianglecoll) {
        col = PhongSphereColor(m_sphere, collpos, lights[i]);
      } else {
        col = PhongTriangleColor(m_Triangle, ray, collpos, lights[i]);
      }
    }
    else {
      col.x = 0; col.y = 0; col.z = 0;
    }
    i++;
  }

  return col;
}

//MODIFY THIS FUNCTION
void draw_scene()
{
  Vector camPosition, backGroundColor;
  camPosition.x = 0.0; camPosition.y = 0.0; camPosition.z = 0.0;
  backGroundColor.x = 255; backGroundColor.y = 255; backGroundColor.z = 255;

  //a simple test output
  for(unsigned int x=0; x<WIDTH; x++)
  {
    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(unsigned int y=0; y<HEIGHT; y++)
    {
      //create rays
      Ray currRay = RayTrace(x, y, camPosition);

      //check if ray hits triangle or sphere
      bool trianglecoll = TriangleCollision(currRay, collisionTriangle);
      bool spherecoll = SphereCollision(currRay, collisionSphere);

      //if triangle or sphere, calculate appropriate color
      if (trianglecoll || spherecoll)
      {
        if ((spherecoll && !trianglecoll) || ((trianglecoll && spherecoll) && spheretrace < triangletrace))
        {
          
          trianglecoll = true;
          finalColor = DoColor(trianglecoll, currRay, collisionSphere, collisionTriangle, spheretrace);

        }
        else if ((trianglecoll && !spherecoll) || ((trianglecoll && spherecoll) && triangletrace < spheretrace))
        {
          trianglecoll = false;
          finalColor = DoColor(trianglecoll, currRay, collisionSphere, collisionTriangle, triangletrace);

        }

        plot_pixel(x, y, finalColor.x, finalColor.y, finalColor.z);

      }
      //else use background color (white)
      else 
      {
        plot_pixel(x, y, backGroundColor.x, backGroundColor.y, backGroundColor.z);
      }

    }
    glEnd();
    glFlush();
  }
  printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  buffer[y][x][0] = r;
  buffer[y][x][1] = g;
  buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
    plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  printf("Saving JPEG file: %s\n", filename);

  ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
  if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
    printf("Error in Saving\n");
  else 
    printf("File saved Successfully\n");
}

void parse_check(const char *expected, char *found)
{
  if(strcasecmp(expected,found))
  {
    printf("Expected '%s ' found '%s '\n", expected, found);
    printf("Parse error, abnormal abortion\n");
    exit(0);
  }
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE * file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i", &number_of_objects);

  printf("number of objects: %i\n",number_of_objects);

  parse_doubles(file,"amb:",ambient_light);

  for(int i=0; i<number_of_objects; i++)
  {
    fscanf(file,"%s\n",type);
    printf("%s\n",type);
    if(strcasecmp(type,"triangle")==0)
    {
      printf("found triangle\n");
      for(int j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

      if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
      triangles[num_triangles++] = t;
    }
    else if(strcasecmp(type,"sphere")==0)
    {
      printf("found sphere\n");

      parse_doubles(file,"pos:",s.position);
      parse_rad(file,&s.radius);
      parse_doubles(file,"dif:",s.color_diffuse);
      parse_doubles(file,"spe:",s.color_specular);
      parse_shi(file,&s.shininess);

      if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
      spheres[num_spheres++] = s;
    }
    else if(strcasecmp(type,"light")==0)
    {
      printf("found light\n");
      parse_doubles(file,"pos:",l.position);
      parse_doubles(file,"col:",l.color);

      if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
      lights[num_lights++] = l;
    }
    else
    {
      printf("unknown type in scene description:\n%s\n",type);
      exit(0);
    }
  }
  return 0;
}

void display()
{
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
      save_jpg();
  }
  once=1;
}

int main(int argc, char ** argv)
{
  if ((argc < 2) || (argc > 3))
  {  
    printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
  {
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}

