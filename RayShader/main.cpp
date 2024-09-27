#define GL_GLEXT_PROTOTYPES


#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/matrix_projection.hpp>

#include "time.h"
#include "shader.h"
#include "textures.h"
#include "scene.h"
#include "Liquid.h"

using namespace glm;


#define F 1000.0 // floor size

Shader shader;

GLuint window;
int window_wd = 600;
int window_ht = 600;
float aspectRatio = window_wd / window_ht;
float window_xOffset;

int framecount = 0;
float seconds = 0;
float seconds_floor = 0; // floor(seconds)

char window_title[200];

float mouse_x = 0.5;
float mouse_y = 0.5;
float zoom = 1.0f;
int shadowSamples = 0;
bool leftMouse = false;
bool rightMouse = false;

//the centre of the window
float winCentreX = 0.0f;
float winCentreY = 0.0f;
//the distance the mouse has moved from the centre
float mouseDisX = 0.0f;
float mouseDisY = 0.0f;

GLuint framebufferNames[2]; // The frame buffer objects
GLuint renderTextures[2];   // The textures we're going to render to
unsigned int renderSource = 0;
unsigned int renderTarget = 1;
GLuint prevFrame;
bool prevFrame_enabled = true;
int framesWithoutChange = 0;
float prevFrame_ratio = 1.0 / (framesWithoutChange + 1.0);
bool softDiffuse = false;

bool camMove = false;
float camRotY = 0.0f;
float camRotX = 0.0f;

bool sphereMove = false;
vec4 spherePos = vec4(0.0, 0.75, 0.0, 0.0);

enum RenderMode {
    SHADE_TRACE = 0,
    LIQUID_ONLY,
    OPENGL
};

RenderMode renderMode = SHADE_TRACE;

float modelScale = 0.7;



// scene specifications
scene currentScene = SCENE_BEACH;
std::vector<vec4> vertecies;
std::vector<int> triangles;
std::vector<vec4> ball_pos;
std::vector<float> ball_radius;

GLuint skybox;
bool skybox_enabled = true;


bool zPressed = false;
bool vortex = false;

// modified by water simulation
std::vector<vec3> water;
std::vector<vec3> water_normals;
float turbulent_min, turbulent_max;
float water_bottom;
bool water_enabled = false;
bool model_enabled = true;
bool portal_enabled = false;
bool refract_enabled = true;

// lights
// float light_position[]  = {0.4, 0.5, -0.6, 1.0};
float light_position[]  = {0.8, 1.0, -1.2, 1.0};
// float light_position[]  = {0.0, 1.5, -1.0, 1.0};
float light_direction[] = {1.0f, 0.0f, 0.0f};
GLfloat light_ambient[]     = {0.1, 0.1, 0.1, 1.0};
GLfloat light_diffuse[]     = {0.7, 0.7, 0.7, 1.0};
GLfloat light_specular[]    = {1.0, 1.0, 1.0, 1.0};
GLfloat material_emissive[] = {0.0, 0.0, 0.0, 1.0};
GLfloat material_diffuse[]  = {1.0, 0.0, 0.0, 1.0};
GLfloat material_ambient[]  = {1.0, 1.0, 1.0, 1.0};
GLfloat material_specular[] = {8.8, 8.8, 8.8, 1.0};
GLfloat material_shininess[] = {89};



Liquid g_Liquid(
    &water,
    &water_normals,
    &turbulent_min,
    &turbulent_max,
    &water_bottom,
    seconds
);

mat4 cameraTransform = mat4(1.0f);
// mat4 view = mat4(1.0f);
// void applyView(mat4 viewMatrix) {
//     view = viewMatrix;
//     for (unsigned int i=0; i<ball_pos.size(); ++i) {
//         ball_pos[i] = view * ball_pos[i];
//     }
//     for (unsigned int i=0; i<vertecies.size(); ++i) {
//         vertecies[i] = view * vertecies[i];
//     }
// }
// void undoView() {
//     applyView(inverse(view));
// }


void printVec(vec4 *v){
    printf(
        "[%f %f %f]\n",
        v->x,
        v->y,
        v->z
    );
}
void myTranslate(vec4 v) {
    glTranslatef(
        v.x,
        v.y,
        v.z
    );
}


float extremify(float val) {
    return pow(10*(val-0.5), 5);
}
float openglCoords(float val) {
    return 2.0*val - 1.0;
}

//applies lighting
void setupLighting() {
    // glPushMatrix();
    //     glRotatef(60, 0, 0, 1);
    //     glRotatef(30, 0, 1, 0);
    //     glLightfv(GL_LIGHT0, GL_POSITION, light_direction);
    //     glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    //     glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    //     glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    // glPopMatrix();

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
}

// resets the distance the mouse travelled
void zeroMouseDis() {
    mouseDisX = 0.0f;
    mouseDisY = 0.0f;
}

// should be called when the view of scene has changed and needs
// to converge on a new image
void changed() {
    framesWithoutChange = 0;
}


void display() {
    seconds += time_dt();
    framecount += 1;
    if (floor(seconds) > seconds_floor) { // we are in a new second
        seconds_floor = floor(seconds);
        sprintf(window_title, "RayShader  %2dfps", framecount);
        glutSetWindowTitle(window_title);
        framecount = 0;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // //clear colour
    // glClearColor(0.0, 0.0, 0.0, 1.0);


    //if the camera is being moved
    if (leftMouse) {
        sphereMove = false;

        if (!camMove) {
            //hide the cursor
            glutSetCursor(GLUT_CURSOR_NONE);
            //glutWarpPointer(winCentreX, winCentreY);
            zeroMouseDis();
            camMove = true;
        }
        else {
            float mouseSensitivity = 5.0;
            camRotY += mouseSensitivity * (mouseDisX / 15.0f);
            camRotX += mouseSensitivity * (mouseDisY / 15.0f);
            zeroMouseDis();
            //glutWarpPointer(winCentreX, winCentreY);
        }
    }

    // how the camera is transformed
    cameraTransform = mat4(1.0);
    cameraTransform = scale(    cameraTransform, vec3(zoom, zoom, zoom));
    cameraTransform = rotate(   cameraTransform, camRotY, vec3(0.0f, 1.0f, 0.0f));
    cameraTransform = rotate(   cameraTransform, camRotX, vec3(1.0f, 0.0f, 0.0f));
    cameraTransform = translate(cameraTransform, vec3(0.0, 0.0, -2.0));

    if (rightMouse) {
        camMove = false;
        if (!sphereMove) {
            //hide the cursor
            glutSetCursor(GLUT_CURSOR_NONE);
            // glutWarpPointer(winCentreX, winCentreY);
            zeroMouseDis();
            sphereMove = true;
        }
        else {
            float mouseSensitivity = 5.0;
            float sx = mouseSensitivity * -mouseDisX / 3000.0f;
            float sy = mouseSensitivity * -mouseDisY / 3000.0f;

            // float cy = camRotY * DEGREES_TO_RADIANS;
            // float cx = camRotX * DEGREES_TO_RADIANS;

            // spherePosX += (-cos(cy) * sx) + ( sin(cx) * sy);
            // spherePosY += cos(cx) * sy;
            // spherePosZ += (-sin(cy) * sin(cx) * sx) + (-sin(cx) * cos(cy) * sy);
            // printf("before %f\n", sx);
            // printf("after ");
            // vec4 af = ;
            // printVec(&af);
            spherePos += cameraTransform * vec4(1.0, 0.0, 0.0, 0.0) * sx;
            spherePos += cameraTransform * vec4(0.0, 1.0, 0.0, 0.0) * sy;

            zeroMouseDis();
            // glutWarpPointer(winCentreX, winCentreY);
        }
    }
    if (!leftMouse && !rightMouse) {
        //set the cursor back to standard
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
        sphereMove = false;
        camMove = false;
    }

    //set the position of the sphere
    g_Liquid.setSpherePos(spherePos.x, spherePos.y, spherePos.z);

    if (currentScene == SCENE_SURFACE) {
        ball_pos[0].x = spherePos.x;
        ball_pos[0].y = spherePos.y;
        ball_pos[0].z = spherePos.z;
    }




    if (renderMode == SHADE_TRACE) {


        if (water_enabled) framesWithoutChange = 0;
        prevFrame_ratio = 1.0 / (framesWithoutChange + 1.0);
        framesWithoutChange += 1; // technically it should be done at the end of the frame

        // switch render source and target
        renderTarget = 1 - renderTarget;
        renderSource = 1 - renderSource;
        // printf("%d %d\n", rendert[0], rendert[1]);

        // Render to our framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferNames[renderTarget]);
        glViewport(0, 0, window_wd, window_ht);

        // Always check that our framebuffer is ok
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("bad framebuffer\n");
            exit(-1);
        }


        g_Liquid.render(liquid::RAYTRACE);

        // pass texture samplers
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, renderTextures[renderSource]);
        glActiveTexture(GL_TEXTURE0);  // this fixed a bug where the texture behaves as one colour. I don't know why

        shader.bind();

        // // print out our normals
        // printf("w ");
        // for (unsigned int i=0; i<water_normals.size(); ++i) {
        //     printf("%.1f", water_normals[i].y);
        // }
        // printf("\n");

        // pass the data to the shader
        // glUniform4fv(glGetUniformLocation(shader.id(), "triangles"),    numTriangles*3, value_ptr(triangles[0]) );
        glUniform4fv(glGetUniformLocation(shader.id(), "vertecies"),    vertecies.size(), value_ptr(vertecies[0]) );
        glUniform1iv(glGetUniformLocation(shader.id(), "triangles"),    triangles.size(), &triangles[0] );
        glUniform1i( glGetUniformLocation(shader.id(), "numTriangles"), triangles.size()/3);
        glUniform1i( glGetUniformLocation(shader.id(), "numBalls"),    ball_pos.size());
        glUniform4fv(glGetUniformLocation(shader.id(), "ball_pos"),    ball_pos.size(), value_ptr(ball_pos[0]) );
        glUniform1fv(glGetUniformLocation(shader.id(), "ball_radius"), ball_pos.size(), &ball_radius[0]);

        glUniform1i( glGetUniformLocation(shader.id(), "numWater"     ), water.size());
        glUniform3fv(glGetUniformLocation(shader.id(), "water"        ), water.size(), value_ptr(water        [0]));
        glUniform3fv(glGetUniformLocation(shader.id(), "water_normals"), water.size(), value_ptr(water_normals[0]));
        glUniform1f( glGetUniformLocation(shader.id(), "turbulent_min"), turbulent_min);
        glUniform1f( glGetUniformLocation(shader.id(), "turbulent_max"), turbulent_max);
        glUniform1i( glGetUniformLocation(shader.id(), "water_enabled"), water_enabled);
        glUniform1i( glGetUniformLocation(shader.id(), "model_enabled"), model_enabled);
        glUniform1i( glGetUniformLocation(shader.id(), "portal_enabled"), portal_enabled);
        glUniform1i( glGetUniformLocation(shader.id(), "refract_enabled"), refract_enabled);

        glUniform1i( glGetUniformLocation(shader.id(), "skybox"), 0); //Texture unit 0
        glUniform1i( glGetUniformLocation(shader.id(), "skybox_enabled"), skybox_enabled);
        glUniform1i( glGetUniformLocation(shader.id(), "prevFrame"), 1); //Texture unit 1
        glUniform1i( glGetUniformLocation(shader.id(), "prevFrame_enabled"), prevFrame_enabled);
        glUniform1f( glGetUniformLocation(shader.id(), "prevFrame_ratio"), prevFrame_ratio);
        glUniform1f( glGetUniformLocation(shader.id(), "softDiffuse"), softDiffuse);


        glUniform2f( glGetUniformLocation(shader.id(), "mouse"), extremify(mouse_x), extremify(mouse_y));
        glUniform1i( glGetUniformLocation(shader.id(), "shadowSamples"), shadowSamples);
        glUniform1f( glGetUniformLocation(shader.id(), "time"), seconds);
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "cameraTransform"), 1, false, value_ptr(cameraTransform));

        float tv = 1.0;
        glColor3f(1,0,0);
        glBegin(GL_TRIANGLES);
            // two triangles that cover the screen
            glVertex3f(-tv,-tv, 0.0);
            glVertex3f( tv,-tv, 0.0);
            glVertex3f(-tv, tv, 0.0);

            glVertex3f( tv, tv, 0.0);
            glVertex3f(-tv, tv, 0.0);
            glVertex3f( tv,-tv, 0.0);
        glEnd();

        shader.unbind();


        // Render to the screen
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);// Render to the screen
        glViewport(window_xOffset, 0, window_ht/aspectRatio, window_ht);
        // render the most recent renderTextures
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glBindTexture(GL_TEXTURE_2D, renderTextures[renderTarget]);

        glColor4f(1,1,1,1);
        tv = 1.0;
        glBegin(GL_TRIANGLES);
            // two triangles that cover the screen
            glTexCoord2f(0, 0); glVertex3f(-tv,-tv, 0.0);
            glTexCoord2f(1, 0); glVertex3f( tv,-tv, 0.0);
            glTexCoord2f(0, 1); glVertex3f(-tv, tv, 0.0);

            glTexCoord2f(1, 1); glVertex3f( tv, tv, 0.0);
            glTexCoord2f(0, 1); glVertex3f(-tv, tv, 0.0);
            glTexCoord2f(1, 0); glVertex3f( tv,-tv, 0.0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
    }
    else if (renderMode == LIQUID_ONLY) {

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        //set the clear colour
        glClearColor(0.3, 0.3, 0.3, 1.0);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(60.0f, ((float) window_wd) / ((float) window_ht),
            0.1f, 1000.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // glLoadMatrixf(value_ptr(inverse(cameraTransform)));
        // //shift the camera back
        glTranslatef(0.0f, 0.0f, -2.0f);
        glRotatef(camRotX, 1.0f, 0.0f, 0.0f);
        glRotatef(-camRotY, 0.0f, 1.0f, 0.0f);
        glScalef(1.0/zoom, 1.0/zoom, 1.0/zoom);

        //apply lighting
        float direction[]     = {1.0f, 1.0f, 0.0f, 0.0f};
        float diffintensity[] = {0.4f, 0.4f, 0.4f, 1.0f};
        float ambient[]       = {0.4f, 0.4f, 0.4f, 1.0f};

        glLightfv(GL_LIGHT0, GL_POSITION, direction);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffintensity);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient);

        g_Liquid.render(liquid::GRID);

        //reset the projection to the identity
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        //reset the model view matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_LIGHTING);

        //reset lights
        setupLighting();
    }
    else {

        glEnable(GL_LIGHTING);

            g_Liquid.render(liquid::GRID);

            glColor4f(0.9f, 0.5f, 0.5f, 1.0f);

            for (unsigned int i=0; i<ball_pos.size(); ++i) {
                glPushMatrix();
                    myTranslate(ball_pos[i]);
                    glutSolidSphere(ball_radius[i], 32, 32);
                glPopMatrix();
            }

            glBegin(GL_TRIANGLES);
            for (unsigned int i=0; i<triangles.size(); i+=3) {
                glVertex3fv(value_ptr(vertecies[triangles[i  ]]));
                glVertex3fv(value_ptr(vertecies[triangles[i+1]]));
                glVertex3fv(value_ptr(vertecies[triangles[i+2]]));
            }
            glEnd();

        glDisable(GL_LIGHTING);
    }

    // undoView();





    glutSwapBuffers();
    glutPostRedisplay();
}

void idle() {
    g_Liquid.update(seconds);
}

void initShader() {
    shader.init(
        "shaders/vert.glsl",
        "shaders/frag.glsl"
    );
}

void reloadScene() {
    portal_enabled = (currentScene == SCENE_PORTAL);
    loadScene(
        currentScene,
        vertecies,
        triangles,
        ball_pos,
        ball_radius
    );
}

void keyHander(unsigned char key, int, int) {
    switch (key) {
        case 27: // Escape -> exit
            glutDestroyWindow(window);
            exit(0);
            break;
        case 13:  // Enter -> refresh shader
            initShader();
            break;
        case '1': currentScene = SCENE_BEACH;      reloadScene();   break;
        case '2': currentScene = SCENE_SURFACE;    reloadScene();   break;
        case '3': currentScene = SCENE_WATER;      reloadScene();   break;
        case '4': currentScene = SCENE_OBJ;        reloadScene();   break;
        case '5': currentScene = SCENE_PORTAL;     reloadScene();   break;
        case '6': currentScene = SCENE_SOFT;       reloadScene();   break;
        case 'a': renderMode = OPENGL;                              break;
        case 's': renderMode = SHADE_TRACE;                         break;
        case 'd': renderMode = LIQUID_ONLY;                         break;
        case 'l': skybox_enabled = !skybox_enabled;                 break;
        case ']': shadowSamples += 1;                               break;
        case '[': shadowSamples -= 1;                               break;
        case 'p': shadowSamples  = 0;                               break;
        case 'q': softDiffuse = !softDiffuse;                       break;
        case 'w': water_enabled = !water_enabled;                   break;
        case 'e': model_enabled = !model_enabled;                   break;
        case 'o': portal_enabled = !portal_enabled;                 break;
        case 'r': refract_enabled = !refract_enabled;               break;
        case 'v': vortex = !vortex; g_Liquid.setVortex(vortex);     break;
        case 'f': g_Liquid.fill();                                  break;
    }

    if (key == 'z' && !zPressed) {

        glm::vec2 rPos(
            ((rand() % 200) / 100.0f) - 1.0f,
            ((rand() % 200) / 100.0f) - 1.0f);

        if (renderMode == SHADE_TRACE) {

            rPos.y = 0.0f;
        }

        g_Liquid.addRipple(new RipplePoint(rPos,
            0.02, -5.0f, 20.0f, 0.4f));
        zPressed = true;
    }

    changed();
    //glutPostRedisplay();
}

void keyUp(unsigned char key, int, int) {

    if (key == 'z') {

        zPressed = false;
    }
}

void initFrameBuffers() {
    for (int i=0; i<2; ++i) {
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &renderTextures[i]);
        glBindTexture(GL_TEXTURE_2D, renderTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_wd, window_ht, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glDisable(GL_TEXTURE_2D);


        //create framebufferNames[i] and attach texture A to it
        glGenFramebuffers(1, &framebufferNames[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferNames[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextures[i], 0);
    }
    changed();
}

void reshapeHandler(int wd, int ht) {
    window_wd = wd;
    window_ht = ht;

    winCentreX = wd / 2.0f;
    winCentreY = ht / 2.0f;
    window_xOffset = (wd - (ht/aspectRatio)) / 2.0f;

    glViewport(window_xOffset, 0, window_ht/aspectRatio, window_ht);

    initFrameBuffers();
}

float mouse_x_prev = 0.0;
float mouse_y_prev = 0.0;
void mouseMoveHander(int x, int y){

    // mouse_x = x/(float)window_wd;
    // mouse_y = y/(float)window_ht;
    if (
        (leftMouse || rightMouse) &&
        (mouse_x != x || mouse_y != y)
    ) {
        changed();
    }

    mouse_x_prev = mouse_x;
    mouse_y_prev = mouse_y;
    mouse_x = x;
    mouse_y = y;



    // mouseDisX -= mouse_x - winCentreX;
    // mouseDisY += winCentreY - mouse_y; // WTF!
    mouseDisX -= mouse_x - mouse_x_prev;
    mouseDisY += mouse_y - mouse_y_prev; // WTF!
    //glutPostRedisplay();

}

void mouseButtonHandler(int button, int dir, int x, int y) {

    if (button == 0 && dir == 0) {
        leftMouse = true;
        mouseMoveHander(x, y);
    }
    else if (button == 0 && dir == 1) {
        leftMouse = false;
    }
    if (button == 2 && dir == 0) {
        rightMouse = true;
    }
    else if (button == 2 && dir == 1) {
        rightMouse = false;
    }

    switch(button) {
        case 3:   zoom -= 0.03f;    changed();  break;
        case 4:   zoom += 0.03f;    changed();  break;
    }

    //clamp
    if (zoom < 0.5f) {
        zoom = 0.5f;
    }
    else if (zoom > 1.7f) {
        zoom = 1.7f;
    }
}


int main(int argc, char** argv) {
    time_init();

    glutInit(&argc, argv);


    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(window_wd, window_ht);
    window = glutCreateWindow("RayShader");

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshapeHandler);
    glutKeyboardFunc(keyHander);
    glutKeyboardUpFunc(keyUp);
    glutMotionFunc(mouseMoveHander);
    glutPassiveMotionFunc(mouseMoveHander);
    glutMouseFunc(mouseButtonHandler);

    // initialize the framebuffer
    initFrameBuffers();

    // // initialize triangles
    // for (int i=0; i<numTriangles; ++i) {
    //     float theta = i/float(numTriangles) * 2.0*PI;
    //     triangles[i*3 +2].y = cos(theta);
    //     triangles[i*3 +2].z = sin(theta);
    // printf("A: %f %f\n", triangles[i*3 +2].y, triangles[i*3 +2].z);
    // }


    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepth(1000);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_LIGHT0);

    // skybox = png_texture_load("sky.png", &skybox_wd, &skybox_ht);
    reloadScene();
    skybox = png_cubemap_load("resources/beach/");
    initShader();
    setupLighting();

    srand(time(0));

    winCentreX = window_wd / 2.0f;
    winCentreY = window_ht / 2.0f;

    // glutWarpPointer(winCentreX, winCentreY);

    glutMainLoop();

    return 0;
}
