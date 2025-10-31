#include <GL/glut.h>
#include <cmath>
#include <string>
#include <map>
#include <iostream>

// Window size
int width = 800, height = 600;

// Angle for orbit animation
float angle = 0.0f;

// Planet info
struct Planet
{
    std::string name;
    float distance;
    float size;
    float color[3];
    std::string info;
};

Planet planets[] = {
    {"Mercury", 5.0f, 0.5f, {0.7f, 0.7f, 0.7f}, "Mercury is the smallest planet."},
    {"Venus", 8.0f, 0.9f, {1.0f, 0.7f, 0.0f}, "Venus has a thick, toxic atmosphere."},
    {"Earth", 11.0f, 1.0f, {0.0f, 0.6f, 1.0f}, "Earth is our home planet."},
    {"Mars", 14.0f, 0.7f, {1.0f, 0.2f, 0.0f}, "Mars is known as the Red Planet."},
    {"Jupiter", 18.0f, 2.0f, {1.0f, 0.8f, 0.6f}, "Jupiter is the largest planet."},
    {"Saturn", 23.0f, 1.7f, {1.0f, 1.0f, 0.6f}, "Saturn has beautiful rings."},
    {"Uranus", 28.0f, 1.2f, {0.4f, 1.0f, 1.0f}, "Uranus rotates on its side."},
    {"Neptune", 33.0f, 1.2f, {0.0f, 0.0f, 1.0f}, "Neptune is very windy and cold."}};

int numPlanets = sizeof(planets) / sizeof(Planet);

// Selected planet info
std::string selectedInfo = "";

// Draw a sphere representing a planet
void drawPlanet(const Planet &p, float orbitAngle)
{
    glPushMatrix();
    float x = p.distance * cos(orbitAngle);
    float z = p.distance * sin(orbitAngle);
    glTranslatef(x, 0.0f, z);
    glColor3fv(p.color);
    glutSolidSphere(p.size, 30, 30);
    glPopMatrix();
}

// Display callback
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Camera
    gluLookAt(0, 20, 50, 0, 0, 0, 0, 1, 0);

    // Draw Sun
    glColor3f(1.0f, 1.0f, 0.0f);
    glutSolidSphere(3.0f, 50, 50);

    // Draw planets
    for (int i = 0; i < numPlanets; i++)
        drawPlanet(planets[i], angle / (i + 1));

    // Show selected info
    if (!selectedInfo.empty())
    {
        glColor3f(1, 1, 1);
        glRasterPos2f(-1.0f, -0.9f); // bottom left
        for (char c : selectedInfo)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glutSwapBuffers();
}

// Timer for animation
void timer(int v)
{
    angle += 0.02f;
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// Right-click menu
void menu(int id)
{
    if (id >= 0 && id < numPlanets)
        selectedInfo = planets[id].name + ": " + planets[id].info;
    else
        selectedInfo = "";
    glutPostRedisplay();
}

// Setup OpenGL
void setup()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    // Create menu
    int menuID = glutCreateMenu(menu);
    for (int i = 0; i < numPlanets; i++)
        glutAddMenuEntry(planets[i].name.c_str(), i);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Reshape callback
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / h, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

// Main
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Interactive Solar System");

    setup();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}