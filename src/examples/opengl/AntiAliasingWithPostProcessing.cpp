/*
    Post-processing effects with multisample anti-aliasing.

    This example demonstrates how to combine post-processing effects with multisample
    anti-aliasing in OpenGL. Current texture hardware is not capable of reading directly
    from a multisampled buffer, so it is not possible to use render-to-texture in this case.
    Instead, we render to the back buffer, and then use glCopyTexImage to copy from the
    back buffer to a texture. The copy performs the necessary downsampling automatically.

    This example also shows how to use the "alpha to coverage" mode of the ARB multisample
    extension to provide higher quality, order indepenent transparency for geometry
    such as trees. This mode converts the alpha value of the pixel into a 16-level (dithered)
    coverage mask, which is logically ANDed with the raster-generated coverage mask.
*/

#if defined(WIN32)
#  include <windows.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include <shared/data_path.h>
#include <shared/array_texture.h>
#include <png/nv_png.h>
#include <nvparse/include/nvparse.h>

using namespace glh;

bool b[256];
glut_simple_mouse_interactor object;
GLuint tree_tex, ground_tex;
GLuint scene_tex = 0, scene_tex2 = 0;
float alpha_ref = 0.5;
int win_w = 512, win_h = 512;
int blur_passes=4;
int post_effect = 1;
GLuint sum_combiners, lerp_combiners;
float lerp = 0.6;

GLuint create_scene_texture(int w, int h);
GLuint load_texture(char *filename, bool alpha);
void init_combiners();

void init_opengl()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.8, 0.9, 1.0, 1.0);

    tree_tex = load_texture("Palm.png", true);
    ground_tex = load_texture("rock.png", false);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    scene_tex = create_scene_texture(win_w, win_h);
    scene_tex2 = create_scene_texture(win_w, win_h);

    init_combiners();
}

void init_exts()
{
    if (!glh_init_extensions(
                             "GL_ARB_multitexture "                            
                             "GL_ARB_multisample "
                             "GL_NV_texture_env_combine4 "
                             )) 
    {
        fprintf(stderr, "Error - required extensions were not supported: %s", glh_get_unsupported_extensions());
        exit(-1);
    }
}

// initialize register combiners using NVParse
void init_combiners()
{
    // sum 4 textures
    sum_combiners = glGenLists(1);
    glNewList(sum_combiners, GL_COMPILE);
    nvparse(   
    "!!RC1.0\n"
    "const0 = ( 0.25, 0.25, 0.25, 1 );\n"
    "{\n"
    "	rgb {\n"
    "		spare0 = tex0 * const0;\n"
    "	}\n"
    "}\n"
    "{\n"
    "	rgb {\n"
    "		discard = tex1 * const0;\n"
    "       discard = spare0;\n"
    "       spare0 = sum();\n"
    "	}\n"
    "}\n"
    "{\n"
    "	rgb {\n"
    "		discard = tex2 * const0;\n"
    "       discard = spare0;\n"
    "       spare0 = sum();\n"
    "	}\n"
    "}\n"
    "{\n"
    "	rgb {\n"
    "		discard = tex3 * const0;\n"
    "       discard = spare0;\n"
    "       spare0 = sum();\n"
    "	}\n"
    "}\n"

    "out.rgb = spare0;\n"
    "out.a = spare0;\n"
    );
    glEndList();
    nvparse_print_errors(stderr);

    // lerp between two textures
    lerp_combiners = glGenLists(1);
    glNewList(lerp_combiners, GL_COMPILE);
    nvparse(   
    "!!RC1.0\n"
    "const0 = ( 0.6, 0.6, 0.6, 0.0 );\n"
    "{\n"
    "	rgb {\n"
    "		discard = tex0 * unsigned_invert(const0);\n"
    "		discard = tex1 * const0;\n"
//    "		discard = tex0;\n"
//    "		discard = tex1;\n"
    "       spare0 = sum();\n"
    "	}\n"
    "}\n"
    "out.rgb = spare0;\n"
    "out.a = spare0;\n"
    );
    glEndList();
    nvparse_print_errors(stderr);

}

GLuint load_texture(char *filename, bool alpha)
{
    // load tree texture
    array2<vec4ub> img;
    array2<vec3ub> img2;
    if (alpha)
        read_png_rgba(filename, img);
    else
        read_png_rgb(filename, img2);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (alpha)
        make_rgba_texture(img, true);
    else
        make_rgb_texture(img2, true);
    return tex;
}

GLuint create_scene_texture(int w, int h)
{
    GLenum target = GL_TEXTURE_RECTANGLE_NV;
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(target, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    return tex;
}

void texCoord2f(float s, float t)
{
    // send texture coordinate to 4 texture units
    for(int i=0; i<4; i++) {
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB + i, s, t);
    }
}

void draw_quad(int tw, int th)
{
    glBegin(GL_QUADS);
    texCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
    texCoord2f(tw, 0.0); glVertex2f(1.0, 0.0);
    texCoord2f(tw, th); glVertex2f(1.0, 1.0);
    texCoord2f(0.0, th); glVertex2f(0.0, 1.0);
    glEnd();
}

// draw 2 intersecting billboards for tree
void draw_tree()
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 0.0, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.5, 0.0, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.5, 1.0, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 1.0, 0.0);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, -0.5);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 0.0, 0.5);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.0, 1.0, 0.5);
    glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 1.0, -0.5);
    glEnd();
}

float frand()
{
    return rand() / (float) RAND_MAX;
}

// draw some trees in random positions
void draw_trees(int n)
{
    srand(42);
    for(int i=0; i<n; i++) {
        glPushMatrix();
        glTranslatef(frand()*10.0-5.0, 0.0, frand()*10.0-5.0);
        draw_tree();
        glPopMatrix();
    }
}

void render_scene()
{
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, ground_tex);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(-5.0, 0.0, -5.0);
    glTexCoord2f(10.0, 0.0); glVertex3f(5.0, 0.0, -5.0);
    glTexCoord2f(10.0, 10.0); glVertex3f(5.0, 0.0, 5.0);
    glTexCoord2f(0.0, 10.0); glVertex3f(-5.0, 0.0, 5.0);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, tree_tex);
    glColor3f(1.0, 1.0, 1.0);
    draw_trees(100);

    glDisable(GL_TEXTURE_2D);
}

// copy from back buffer to texture
// the AA downsampling is performed here
void copy_scene_to_texture(GLuint tex)
{
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex);
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, win_w, win_h);
}

void draw_fullscreen_quad()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    draw_quad(win_w, win_h);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void display_scene_texture(GLuint tex)
{
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glDisable(GL_DEPTH_TEST);

    draw_fullscreen_quad();

    glDisable(GL_TEXTURE_RECTANGLE_NV);
    glEnable(GL_DEPTH_TEST);
}

// simple PS1.1 blur effect
void display_scene_texture_blur(float scale)
{
    // texture coordinate offsets to perform 3x3 Gaussian blur using 4 bilinear samples
    float offset[4][2] = {
        -0.5, -0.5,
        0.5, -0.5,
        0.5, 0.5,
        -0.5, 0.5,
    };

    for(int i=0; i<4; i++) {
        glActiveTextureARB(GL_TEXTURE0_ARB + i);
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, scene_tex);
        glEnable(GL_TEXTURE_RECTANGLE_NV);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        // scale distance between samples for each pass
        glTranslatef(offset[i][0]*scale, offset[i][1]*scale, 0.0);
    }

    glDisable(GL_DEPTH_TEST);
    glCallList(sum_combiners);
    glEnable(GL_REGISTER_COMBINERS_NV);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    draw_fullscreen_quad();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    for(int i=0; i<4; i++) {
        glActiveTextureARB(GL_TEXTURE0_ARB + i);
        glDisable(GL_TEXTURE_RECTANGLE_NV);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_REGISTER_COMBINERS_NV);
}

// radial blur effect
void display_scene_texture_radial_blur(float blur_amount)
{
    for(int i=0; i<4; i++) {
        glActiveTextureARB(GL_TEXTURE0_ARB + i);
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, scene_tex);
        glEnable(GL_TEXTURE_RECTANGLE_NV);

        // scale image around centre
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslatef(win_w/2.0, win_h/2.0, 0.0);
        glScalef(1.0 + (i/3.0)*blur_amount, 1.0 + (i/3.0)*blur_amount, 1.0);
        glTranslatef(-win_w/2.0, -win_h/2.0, 0.0);
    }
    glDisable(GL_DEPTH_TEST);
    glCallList(sum_combiners);
    glEnable(GL_REGISTER_COMBINERS_NV);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.3, 0.3, 0.3);
    draw_fullscreen_quad();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    for(int i=0; i<4; i++) {
        glActiveTextureARB(GL_TEXTURE0_ARB + i);
        glDisable(GL_TEXTURE_RECTANGLE_NV);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_REGISTER_COMBINERS_NV);
}

void display_scene_texture_combined()
{
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, scene_tex2);
    glEnable(GL_TEXTURE_RECTANGLE_NV);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, scene_tex);
    glEnable(GL_TEXTURE_RECTANGLE_NV);

    glCallList(lerp_combiners);
    glEnable(GL_REGISTER_COMBINERS_NV);
    GLfloat lerp_col[] = { lerp, lerp, lerp, 0.0 };
    glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, (GLfloat *) lerp_col);
    glDisable(GL_DEPTH_TEST);

    draw_fullscreen_quad();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_REGISTER_COMBINERS_NV);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_NV);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_RECTANGLE_NV);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.apply_transform();

    if (b['m']) glEnable(GL_MULTISAMPLE_ARB);
    if (b['c']) {
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
    }
    if (b['a']) {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, alpha_ref);
    }
    if (b['b']) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    render_scene();

    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_MULTISAMPLE_ARB);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
    glEnable(GL_DEPTH_TEST);

    if (b['p']) {
        switch(post_effect) {
            case 0:
                copy_scene_to_texture(scene_tex);
                display_scene_texture(scene_tex);
                break;

            case 1:
                // make a copy of original scene image
                copy_scene_to_texture(scene_tex2);

                // blur scene image
                for(int i=0; i<blur_passes; i++) {
                    copy_scene_to_texture(scene_tex);
                    display_scene_texture_blur(i+1);
                }

                // display original and blurred image combined
                display_scene_texture_combined();
                break;

            case 2:
                copy_scene_to_texture(scene_tex);
                display_scene_texture_radial_blur(-0.1);
                break;
        }
    }

    glutSwapBuffers();
}

void idle()
{
    if (b[' '])
        object.trackball.increment_rotation();
    
    glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
    switch(k) {
    case 27:
    case 'q':
        exit(0);

    case '=':
        alpha_ref += 0.01;
        if (alpha_ref > 1.0) alpha_ref = 1.0;
        break;
    case '-':
        alpha_ref -= 0.01;
        if (alpha_ref < 0.0) alpha_ref = 0.0;
        break;

    case ']':
        blur_passes++;
        break;
    case '[':
        if (blur_passes > 0) blur_passes--;
        break;

    case '.':
        lerp+=0.1;
        if (lerp > 1.0) lerp = 1.0;
        break;
    case ',':
        lerp-=0.1;
        if (lerp < 0.0) lerp = 0.0;
        break;

    case '0':
    case '1':
    case '2':
        post_effect = k - '0';
        break;
    }

    printf("alpha = %f, blur_passes = %d, lerp = %f\n", alpha_ref, blur_passes, lerp);
    object.keyboard(k, x, y);
    
	glutPostRedisplay();
}

void resize(int w, int h)
{
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    object.reshape(w, h);

    win_w = w; win_h = h;

    // reallocate scene textures
    if (scene_tex) {
        glDeleteTextures(1, &scene_tex);
    }
    scene_tex = create_scene_texture(w, h);

    if (scene_tex2) {
        glDeleteTextures(1, &scene_tex2);
    }
    scene_tex2 = create_scene_texture(w, h);
}

void mouse(int button, int state, int x, int y)
{
    object.mouse(button, state, x, y);
}

void motion(int x, int y)
{
    object.motion(x, y);
}

void main_menu(int i)
{
    key((unsigned char) i, 0, 0);
}

void init_menu()
{
    glutCreateMenu(main_menu);
    glutAddMenuEntry("Toggle multisample [m]", 'm');
    glutAddMenuEntry("Toggle alpha to converage [c]", 'c');
    glutAddMenuEntry("Toggle alpha test [a]", 'a');
    glutAddMenuEntry("Toggle blending [b]", 'b');
    glutAddMenuEntry("Toggle post-processing [p]", 'p');
    glutAddMenuEntry("Effect: pass-through [0]", '0');
    glutAddMenuEntry("Effect: blur [1]", '1');
    glutAddMenuEntry("Effect: radial blur [2]", '2');
    glutAddMenuEntry("Increment alpha reference [=]", '=');
    glutAddMenuEntry("Decrement alpha reference [-]", '-');
    glutAddMenuEntry("Increment blur passes []]", ']');
    glutAddMenuEntry("Decrement blur passes [[]", '[');
    glutAddMenuEntry("Increment lerp value [.]", '.');
    glutAddMenuEntry("Decrement lerp value [,]", ',');
    glutAddMenuEntry("Quit (esc)", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(win_w, win_h);

    // create a window with multisample antialiasing
    int samples = 4;
    char displayString[256];
    sprintf(displayString, "double rgb~8 depth~16 samples~%d", samples);
    glutInitDisplayString(displayString);

	glutCreateWindow("Anti-Aliasing with Post Processing");

    init_menu();
    init_exts();
    init_opengl();

    object.configure_buttons(1);
    object.dolly.dolly[1] = -0.5;
    object.dolly.dolly[2] = -2;

	glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(resize);

    b['m'] = true;
    b['c'] = true;
    b['e'] = true;
    b['p'] = true;

	glutMainLoop();

	return 0;
}