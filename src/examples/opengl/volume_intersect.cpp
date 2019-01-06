#if defined(WIN32)
#  include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#elif defined(UNIX)
#  include <GL/glx.h>
#endif

#include <iostream>
#include <list>
#include <vector>

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_glut.h>

#include <shared/timer.h>

using namespace std;
using namespace glh;

glut_callbacks cb;
glut_simple_mouse_interactor camera, object, object2, light;
glut_perspective_reshaper reshaper;

timer t;

bool b[256];

// glut-ish callbacks
void display();
void key(unsigned char k, int x, int y);
void idle();

void menu(int entry) { key(entry, 0, 0); }

// my functions
void init_opengl();

int mainWindow = 0;

rotationf r;

int main(int argc, char **argv)
{
    b[' '] = true; // animation on

    glutInit(&argc, argv);
    glutInitWindowSize(800, 800);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    mainWindow = glutCreateWindow("intersect(shadow_volume, light_bounds)");

    init_opengl();

    glut_helpers_initialize();

    cb.keyboard_function = key;
    camera.configure_buttons(1);
    camera.set_camera_mode(true);
    object.configure_buttons(1);
    object.trackball.scale *= .4;
    object.trackball.incr = rotationf(vec3f(1,2,1.5), .005);
    object.pan.pan[1] = -0.5;
    object2.configure_buttons(1);
    object2.dolly.dolly[2] = -3.25;
    object2.trackball.scale *= .5;
    object2.trackball.incr = rotationf(vec3f(2,1,.5), .002);
    light.configure_buttons(1);
    light.dolly.dolly[2] = -3;
    light.trackball.scale *= .1;
    light.trackball.incr = rotationf(vec3f(.1,1,-.5), .0025);


    camera.set_parent_rotation(& camera.trackball.r);
    light.set_parent_rotation(& camera.trackball.r);
    object.set_parent_rotation(& r);
    object2.set_parent_rotation(& camera.trackball.r);

    object.enable();
    object2.disable();
    camera.disable();
    light.disable();

    glut_add_interactor(&cb);
    glut_add_interactor(&object);
    glut_add_interactor(&object2);
    glut_add_interactor(&light);
    glut_add_interactor(&camera);
    glut_add_interactor(&reshaper);


    glutCreateMenu(menu);
    glutAddMenuEntry("animation [ ]", ' ');
    glutAddMenuEntry("performance stats [r]", 'r');
    glutAddMenuEntry("move camera [1]", '1');
    glutAddMenuEntry("move light [2]", '2');
    glutAddMenuEntry("move object [3]", '3');
    glutAddMenuEntry("move object parent xform [4]", '4');
    glutAddMenuEntry("quit [esc]", 27);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}

void init_opengl()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_CLAMP_NV);
}

void key(unsigned char k, int x, int y)
{

    b[k] = ! b[k];
    if(k==27 || k=='q') {
        exit(0);
    }
    
    if('1' == k ) {
        camera.enable();
        object.disable();
        object2.disable();
        light.disable();
    }
    if('2' == k ) {
        camera.disable();
        object.disable();
        object2.disable();
        light.enable();
    }
    if('3' == k ) {
        camera.disable();
        object.enable();
        object2.disable();
        light.disable();
    }
    if('4' == k ) {
        camera.disable();
        object.disable();
        object2.enable();
        light.disable();
    }



    if(' '==k)
    {
        if(b[k])
            glutIdleFunc(idle);
        else
            glutIdleFunc(0);
    }
    glutPostRedisplay();
}

void idle()
{
    light.trackball.increment_rotation();
    object.trackball.increment_rotation();
    object2.trackball.increment_rotation();
    glutPostRedisplay();
}

struct poly
{
    vector<int> vi;
    vector<int> ni;
    vec4f plane;
};

struct edge
{
    int vi[2];
    int pi[2];
};

vector<int> four_ints(int a, int b, int c, int d)
{
    vector<int> vi;
    vi.push_back(a);
    vi.push_back(b);
    vi.push_back(c);
    vi.push_back(d);
    return vi;
}

vec3f homogeneous_difference(vec4f a, vec4f b)
{
    vec3f v;
    v[0] = b[0]*a[3] - a[0]*b[3];
    v[1] = b[1]*a[3] - a[1]*b[3];
    v[2] = b[2]*a[3] - a[2]*b[3];
    return v;
}

// handles positive w only
vec4f compute_homogeneous_plane(vec4f a, vec4f b, vec4f c)
{
    vec4f v, t;

    if(a[3] == 0)
    { t = a; a = b; b = c; c = t; }
    if(a[3] == 0)
    { t = a; a = b; b = c; c = t; }

    // can't handle 3 infinite points
    if( a[3] == 0 )
        return v;

    vec3f vb = homogeneous_difference(a, b);
    vec3f vc = homogeneous_difference(a, c);
    
    vec3f n = vb.cross(vc);
    n.normalize();
    
    v[0] = n[0];
    v[1] = n[1];
    v[2] = n[2];

    v[3] = - n.dot(vec3f(a.v)) / a[3] ;

    return v;
}

struct polyhedron
{
    vector<vec4f> v;
    vector<poly>  p;
    vector<edge>  e;

    void add_quad( int va, int vb, int vc, int vd )
    {
        poly pg;
        pg.vi = four_ints(va, vb, vc, vd);
        pg.ni = four_ints(-1, -1, -1, -1);
        pg.plane = compute_homogeneous_plane(v[va], v[vb], v[vc]);
        p.push_back(pg);
    }

    void discard_neighbor_info()
    {
        for(unsigned int i = 0; i < p.size(); i++ )
        {
            vector<int> & ni = p[i].ni;
            for(unsigned int j = 0; j < ni.size(); j++)
                ni[j] = -1;
        }
    }

    void compute_neighbors()
    {
        e = vector<edge>();

        discard_neighbor_info();

        bool found;
        int P = p.size();
        // for each polygon
        for(int i = 0; i < P-1; i++ )
        {
            const vector<int> & vi = p[i].vi;
            vector<int> & ni = p[i].ni;
            int Si = vi.size();

            // for each edge of that polygon
            for(int ii=0; ii < Si; ii++)
            {
                int ii0 = ii;
                int ii1 = (ii+1) % Si;

                // continue if we've already found this neighbor
                if(ni[ii] != -1)
                    continue;
                found = false;
                // check all remaining polygons
                for(int j = i+1; j < P; j++ )
                {
                    const vector<int> & vj = p[j].vi;
                    vector<int> & nj = p[j].ni;
                    int Sj = vj.size();

                    for( int jj = 0; jj < Sj; jj++ )
                    {
                        int jj0 = jj;
                        int jj1 = (jj+1) % Sj;
                        if(vi[ii0] == vj[jj1] && vi[ii1] == vj[jj0])
                        {
                            edge ed;
                            ed.vi[0] = vi[ii0];
                            ed.vi[1] = vi[ii1];
                            ed.pi[0] = i;
                            ed.pi[1] = j;
                            e.push_back(ed);
                            ni[ii] = j;
                            nj[jj] = i;
                            found = true;
                            break;
                        }
                        else if ( vi[ii0] == vj[jj0] && vi[ii1] == vj[jj1] )
                        {
                            fprintf(stderr,"why am I here?\n");
                        }
                    }
                    if( found ) 
                        break;
                }
            }
        }
    }


    void recompute_planes()
    {
        // for each polygon
        for(unsigned int i = 0; i < p.size(); i++ )
        {
            p[i].plane = compute_homogeneous_plane(v[p[i].vi[0]], v[p[i].vi[1]], v[p[i].vi[2]]);
        }
    }

    void transform(const matrix4f & m)
    {
        for(unsigned int i=0; i < v.size(); i++ )
            m.mult_matrix_vec(v[i]);
        recompute_planes();
    }

    void dump()
    {
        for(unsigned int i = 0; i < p.size(); i++)
        {
            {
                fprintf(stderr, "poly %2d:: verts: ", i);
                for(unsigned int j = 0; j < p[i].vi.size(); j++)
                    fprintf(stderr, "%2d ", p[i].vi[j]);
                fprintf(stderr, "    ");
            }
        
            {
                fprintf(stderr, " neighbors: ");
                for(unsigned int j = 0; j < p[i].ni.size(); j++)
                    fprintf(stderr, "%2d ", p[i].ni[j]);
                fprintf(stderr, "\n");
            }
        }
        fprintf(stderr, "edges: ");
        for(unsigned int j = 0; j < e.size(); j++)
        {
            fprintf(stderr, "(%d, %d)", e[j].vi[0], e[j].vi[1]);
            if( j+1 < e.size() )
                fprintf(stderr, ", ");
        }
        fprintf(stderr, "\n");
    }


};

// make a unit cube
polyhedron make_cube()
{

//       3----------2
//       |\        /|
//       | \      / |
//       |   7--6   |
//       |   |  |   |
//       |   4--5   |
//       |  /    \  |
//       | /      \ |
//       0----------1
//

    polyhedron p;

    p.v.push_back(vec4f(-1,-1, 1, 1));
    p.v.push_back(vec4f( 1,-1, 1, 1));
    p.v.push_back(vec4f( 1, 1, 1, 1));
    p.v.push_back(vec4f(-1, 1, 1, 1));
    p.v.push_back(vec4f(-1,-1,-1, 1));
    p.v.push_back(vec4f( 1,-1,-1, 1));
    p.v.push_back(vec4f( 1, 1,-1, 1));
    p.v.push_back(vec4f(-1, 1,-1, 1));

    p.add_quad( 0, 1, 2, 3 );
    p.add_quad( 7, 6, 5, 4 );
    p.add_quad( 1, 0, 4, 5 );
    p.add_quad( 2, 1, 5, 6 );
    p.add_quad( 3, 2, 6, 7 );
    p.add_quad( 0, 3, 7, 4 );

    p.compute_neighbors();
    return p;
}

vec3f operator * ( const matrix4f & m, const vec3f & v )
{
    vec3f rv = v;
    m.mult_matrix_vec(rv);
    return rv;
}

polyhedron make_sv(const polyhedron & oc, vec4f light)
{
    polyhedron ph = oc;

    int V = ph.v.size();
    for( int j = 0; j < V; j++ ) 
    {
        vec3f proj = homogeneous_difference( light, ph.v[j] );
        ph.v.push_back( vec4f(proj[0], proj[1], proj[2], 0) );
    }

    ph.p = vector<poly>();

    {
        for(unsigned int i=0; i < oc.p.size(); i++)
        {
            if( oc.p[i].plane.dot(light) > 0)
            {
                ph.p.push_back(oc.p[i]);
            }
        }
    }

    if(ph.p.size() == 0)
        return polyhedron();

    ph.compute_neighbors();

    {
        vector<poly> vpg;
        int I = ph.p.size();

        for(int i=0; i < I; i++)
        {
            vector<int> & vi = ph.p[i].vi;
            vector<int> & ni = ph.p[i].ni;
            int S = vi.size();

            for(int j = 0; j < S; j++)
            {
                if( ni[j] == -1 )
                {
                    poly pg;
                    int a = vi[(j+1)%S];
                    int b = vi[j];
                    pg.vi = four_ints( a, b, b+V, a+V);
                    pg.ni = four_ints(-1, -1, -1, -1);
                    vpg.push_back(pg);
                }
            }
        }
        {
        for(unsigned int i = 0; i < vpg.size(); i++)
            ph.p.push_back(vpg[i]);
        }
    }

    ph.compute_neighbors();

    // need to compute planes for the shadow volume (sv)
    ph.recompute_planes();

    return ph;
}

void draw_polyhedron(const polyhedron & ph)
{
    for(unsigned int i=0; i < ph.p.size(); i++)
    {
        const vector<int> & vi = ph.p[i].vi;
        glBegin(GL_LINE_LOOP);
        for(unsigned int j = 0; j < vi.size(); j++ )
            glVertex4fv(ph.v[vi[j]].v);
        glEnd();
    }
}

void polyhedron_edges(polyhedron & a, vector<vec4f> & e)
{
    if(a.e.size() == 0 && a.p.size() != 0)
        a.compute_neighbors();

    for(unsigned int i = 0; i < a.e.size(); i++)
    {
        e.push_back(a.v[a.e[i].vi[0]]);
        e.push_back(a.v[a.e[i].vi[1]]);
    }

}

// clip the segments of e by the planes of polyhedron a.
void clip_segments(const polyhedron & ph, vector<vec4f> & is, vector<vec4f> & os)
{
    const vector<poly> & p = ph.p;

    for(unsigned int i = 0; i < is.size(); i+=2 )
    {
        vec4f a = is[i  ];
        vec4f b = is[i+1];
        vec4f c;

        bool discard = false;

        for(unsigned int j = 0; j < p.size(); j++ )
        {
            float da = a.dot(p[j].plane);
            float db = b.dot(p[j].plane);
            float rdw = 1/(da - db);

            int code = 0;
            if( da > 0 )
                code = 2;
            if( db > 0 )
                code |= 1;


            switch ( code ) 
            {
            case 3:
                discard = true;
                break;

            case 2:
                c = -db * rdw * a + da * rdw * b;
                a = c;
                break;

            case 1:
                c = -db * rdw * a + da * rdw * b;
                b = c;
                break;

            case 0:
                break;

            default:
                fprintf(stderr, "bad clip code!\n");
                break;
            }

            if( discard )
                break;
        }

        if( ! discard )
        {
            os.push_back(a);
            os.push_back(b);
        }
    }

}

void draw_segments(const vector<vec4f> & segs)
{
    glBegin(GL_LINES);
    for(unsigned int i=0; i < segs.size(); i++)
        glVertex4fv(segs[i].v);
    glEnd();
}

void compute_and_draw_scissor( vector<vec4f> & all_segs )
{
    vector<vec4f> points = all_segs;
    matrix4f m = get_matrix(GL_PROJECTION_MATRIX) * get_matrix(GL_MODELVIEW_MATRIX);

    float min[2] = {  1000000,  1000000 };
    float max[2] = { -1000000, -1000000 };



    for(unsigned int i=0; i < points.size(); i++ )
    {
        vec4f & p = points[i];
        m.mult_matrix_vec(p);
        // do Blinn thing here?

        // for now, just skip the whole nasty -w business
        if( p[3] <= 0 )
            return;

        p /= p[3];
        for(int j=0; j < 2; j++)
        {
            if( p[j] < min[j])
                min[j] = p[j];
            if( p[j] > max[j] )
                max[j] = p[j];
        }

    }

    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glBegin(GL_LINE_LOOP);
    glVertex2f(min[0], min[1]);
    glVertex2f(max[0], min[1]);
    glVertex2f(max[0], max[1]);
    glVertex2f(min[0], max[1]);
    glEnd();

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

void compute_and_draw_depth_bounds ( vector<vec4f> & all_segs, float y0, float y1)
{
    vector<vec4f> points = all_segs;
    matrix4f m = get_matrix(GL_PROJECTION_MATRIX) * get_matrix(GL_MODELVIEW_MATRIX);

    float min =  1000000;
    float max = -1000000;


    bool all_clipped = true;

    for(unsigned int i=0; i < points.size(); i++ )
    {
        vec4f & p = points[i];
        m.mult_matrix_vec(p);

        if( p[3] <= 0 )
        {
            min = -1;
            continue;
        }


        float z = p[2] / p[3];
        
        if( z >= -1  && z <= 1 ) 
          all_clipped = false;

        if( z < min)
            min = z;
        if( z > max )
            max = z;

    }

    if(min < -1)
        min = -1;
    if(max > 1 || all_clipped)
        max = 1;

    // remap z to see more of the [.9,1] range
    min *=  .5;
    min +=  .5;
    max *=  .5;
    max +=  .5;

    min = pow(min, 10);
    max = pow(max, 10);

    min *= 1.5;
    min -= 0.6;
    max *= 1.5;
    max -= 0.6;

    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glBegin(GL_QUADS);
    glVertex2f(min, y0);
    glVertex2f(max, y0);
    glVertex2f(max, y1);
    glVertex2f(min, y1);
    glEnd();

    glEnable(GL_DEPTH_TEST);

    
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

void draw_text ()
{
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glTranslatef(-.95, .94, 0);
    glScalef(.0003, .0003, .0003);

    string s("depth range");
    for(unsigned int i=0; i < s.size(); i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, s[i]);
        float f = glutStrokeWidth(GLUT_STROKE_ROMAN, s[i]);
        glTranslatef(0.1 * f, 0, 0);
    }
        

    glEnable(GL_DEPTH_TEST);

    
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

// here's where to add the thing to time...
void draw()
{

    // original light volume (and light source location)
    r = object2.trackball.r.inverse() * camera.trackball.r ;
    glColor3f(.6,.6,.6);
    glPushMatrix();
    light.apply_transform();
    glutWireCube(2);
    glColor3f(1,1,1);
    glutSolidCube(.03);
    glPopMatrix();


    polyhedron lvol = make_cube();
    lvol.transform(light.get_transform());



    glColor3f(.8, .8, .1);
    matrix4f s;
    s.set_scale(.1);
    matrix4f m = object2.get_transform() * object.get_transform() * s ;

    polyhedron vol = make_cube();

    vol.transform(m);

    // draw occluder points
    glPointSize(2);
    glBegin(GL_POINTS);
    for(unsigned int i = 0; i < vol.v.size(); i++)
        glVertex3fv(vol.v[i].v);
    glEnd();
    glPointSize(1);


    vec4f lightpos(0,0,0,1);
    light.get_transform().mult_matrix_vec(lightpos);


    polyhedron sv = make_sv(vol, lightpos);

    if(b['d'])
    {
        sv.dump();
        b['d'] = false;
    }

    vector<vec4f> in_segs, out_segs, all_segs;

    // draw the edges of the shadow volume (sv) that intersect the light volume (lvol)
    glColor3f(.8, .8, .1);
    polyhedron_edges(sv, in_segs);
    clip_segments(lvol, in_segs, out_segs);

    glColor3f(.3, .3, .05);
    glDepthRange(.001, 1);
    draw_segments(in_segs);
    glDepthRange(0, 1);
    glColor3f(.8, .8, .1);
    draw_segments(out_segs);

    all_segs = out_segs;

    // render the clipped segments as points
    glColor3f(1,0,1);
    glPointSize(3);
    glBegin(GL_POINTS);
    { for(unsigned int i = 0; i < out_segs.size(); i++) glVertex4fv(out_segs[i].v); }
    glEnd();
    glPointSize(1);


    in_segs = out_segs = vector<vec4f>();
    

    // clip the edges of the light volume (lvol) by the shadow volume (sv) planes
    glColor3f(.8,.8,.8);
    glLineWidth(2);
    polyhedron_edges(lvol, in_segs);
    clip_segments(sv, in_segs, out_segs);
    draw_segments(out_segs);
    glLineWidth(1);

    all_segs.insert(all_segs.end(), out_segs.begin(), out_segs.end());

    // render the clipped segments as points
    glColor3f(0,1,1);
    glPointSize(5);
    glBegin(GL_POINTS);
    { for(unsigned int i = 0; i < out_segs.size(); i++) glVertex4fv(out_segs[i].v); }
    glEnd();
    glPointSize(1);


    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(0);
    glColor4f(.2,.2,.02, .3);
    glPushMatrix();
    object2.apply_transform();
    object.apply_transform();
    glutSolidCube(.2);
    glPopMatrix();
    glDepthMask(1);
    glDisable(GL_BLEND);    

    glColor3f(0,0,1);
    compute_and_draw_scissor(in_segs);
    compute_and_draw_depth_bounds(in_segs, .92, .94);
    glColor3f(0,1,0);
    compute_and_draw_scissor(all_segs);
    compute_and_draw_depth_bounds(all_segs, .95, .97);

    glColor3f(1,1,1);
    draw_text();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    camera.apply_inverse_transform();
    draw();
    glPopMatrix();

    // swap then do FPS calculations
    glutSwapBuffers();

    t.frame(b['r']);
}

