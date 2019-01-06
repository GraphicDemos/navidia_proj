#if defined(WIN32)
#  include <windows.h>
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#include <string>

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>
#include <shared/array_texture.h>
#include <shared/data_path.h>
#include <nv_png.h>

#include <shared/quitapp.h>

using namespace std;
using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper;

string title_l = "Lit w/o Separate Specular\nOut = (diffuse+specular) * tex";
string title_r = "Lit with Separate Specular\nOut = diffuse * tex + specular";

bool b[256];

// Texture object for decal texture
tex_object_2D decaltex;


void init_texture()
{
    // Load the decal texture.
    decaltex.bind();
    array2<vec3ub> decal_img;
    read_png_rgb("checker.png", decal_img);
    make_rgb_texture( decal_img, true );
    decaltex.parameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    decaltex.parameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void init_lighting()
{
    GLfloat mat_specular[] = { 0.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 1.0 };

    glShadeModel( GL_SMOOTH );

    glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
    glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );
    glLightfv( GL_LIGHT0, GL_POSITION, light_position );
    glEnable( GL_LIGHT0 );
}

void init_opengl()
{

    // Verify that the GL_EXT_separate_specular_color extension is supported.
    if(!glh_init_extensions("GL_VERSION_1_2"))
    {
        cerr << "Necessary extensions were not supported:" << endl
             << glh_get_unsupported_extensions() << endl;
        quitapp( -1 );
    }

    init_texture();
    init_lighting();
    glEnable(GL_DEPTH_TEST);
}

void key(unsigned char k, int x, int y)
{
    b[k] = ! b[k];
    if(k==27 || k=='q') exit(0);
}

void idle()
{
    object.trackball.increment_rotation();
    glutPostRedisplay();
}

void displayModel()
{
    decaltex.bind();
    decaltex.enable();

    glPushMatrix();
    camera.apply_inverse_transform();
    object.apply_transform();
    glColor3f( 0.5, 0.5, 0.5 );
    glutSolidTeapot(.28f);

    glPopMatrix();

    decaltex.disable();
}

// left
void display_lit_no_separate()
{
    glEnable( GL_LIGHTING );
    glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR );
    displayModel();
    glDisable( GL_LIGHTING );
}

// right
void display_lit_separate()
{
    glEnable( GL_LIGHTING );
    glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR );
    displayModel();
    glDisable( GL_LIGHTING );
}


void render_string(const char * str)
{
    glColor3f( 1.0, 1.0, 1.0 );
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-.9f, .8f, 0);
    glScalef(.0006f, .0006f, 1);
    int count = 0;
    while(*str)
    {
        if ( *str == '\n' )
            {
            count++;
            glLoadIdentity();
            glTranslatef(-.9f, .8f - (count*0.1f), 0);
            glScalef(.0006f, .0006f, 1);
            }
        else
            {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *str);
            }
        str++;
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    GLint w = vp[2]/2;
    GLint h = vp[3];

    // left
    glViewport(0, 0, w, h);
    display_lit_no_separate();
    render_string(title_l.c_str());

    // right
    glViewport(w, 0, w, h);
    display_lit_separate();
    render_string(title_r.c_str());

    glViewport(vp[0], vp[1], vp[2], vp[3]);

    glutSwapBuffers();
}


int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowSize( 800, 400 );
    glutInitWindowPosition( 0, 0 );
    glutCreateWindow("Separate Specular Demo");

    init_opengl();

    glut_helpers_initialize();

    cb.keyboard_function = key;
    cb.idle_function = idle;
    camera.configure_buttons(1);
    camera.set_camera_mode(true);
    object.configure_buttons(1);
    object.dolly.dolly[2] = -1;
    reshaper.aspect_factor = .5f;

    glut_add_interactor(&cb);
    glut_add_interactor(&object);
    glut_add_interactor(&reshaper);

    glut_idle(1);

    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}

