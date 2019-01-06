/*
  Stable fluid solver
  from "A Simple Fluid Solver based on the FFT" by Jos Stam
*/

#include <rfftw/rfftw.h>
#include <math.h>

static rfftwnd_plan plan_rc, plan_cr;

void init_FFT ( int n )
{
	plan_rc = rfftw2d_create_plan ( n, n, FFTW_REAL_TO_COMPLEX, FFTW_IN_PLACE );
	plan_cr = rfftw2d_create_plan ( n, n, FFTW_COMPLEX_TO_REAL, FFTW_IN_PLACE );
}

#define FFT(s,u)\
if (s==1) rfftwnd_one_real_to_complex ( plan_rc, (fftw_real *)u, (fftw_complex *)u );\
else rfftwnd_one_complex_to_real ( plan_cr, (fftw_complex *)u, (fftw_real *)u )

#define floor(x) ((x)>=0.0?((int)(x)):(-((int)(1-(x)))))

void stable_solve ( int n, float * u, float * v, float * u0, float * v0, float visc, float dt )
{
	float x, y, x0, y0, f, r, U[2], V[2], s, t;
	int i, j, i0, j0, i1, j1;
	
    // add force
	for ( i=0 ; i<n*n ; i++ ) {
		u[i] += dt*u0[i]; u0[i] = u[i];
		v[i] += dt*v0[i]; v0[i] = v[i];
	}

    // self-advection
	for ( x=0.5f/n,i=0 ; i<n ; i++,x+=1.0f/n ) {
		for ( y=0.5f/n,j=0 ; j<n ; j++,y+=1.0f/n) {
			x0 = n*(x-dt*u0[i+n*j])-0.5f; y0 = n*(y-dt*v0[i+n*j])-0.5f;
			i0 = floor(x0); s = x0-i0; i0 = (n+(i0%n))%n; i1 = (i0+1)%n;
			j0 = floor(y0); t = y0-j0; j0 = (n+(j0%n))%n; j1 = (j0+1)%n;
			u[i+n*j] = (1-s)*((1-t)*u0[i0+n*j0]+t*u0[i0+n*j1])+
				          s *((1-t)*u0[i1+n*j0]+t*u0[i1+n*j1]);
			v[i+n*j] = (1-s)*((1-t)*v0[i0+n*j0]+t*v0[i0+n*j1])+
		    			  s *((1-t)*v0[i1+n*j0]+t*v0[i1+n*j1]);
		}
	}

	for ( i=0 ; i<n ; i++ )
		for ( j=0 ; j<n ; j++ )
			{ u0[i+(n+2)*j] = u[i+n*j]; v0[i+(n+2)*j] = v[i+n*j]; }

    // transform velocity to fourier domain
	FFT(1,u0); FFT(1,v0);

    // viscosity / conservation of mass
	for ( i=0 ; i<=n ; i+=2 ) {
		x = 0.5f*i;
		for ( j=0 ; j<n ; j++ ) {
			y = (float)((j<=n/2) ? j : j-n);
			r = x*x+y*y;
			if ( r==0.0 ) continue;
			f = (float)exp(-r*dt*visc);
			U[0] = u0[i +(n+2)*j]; V[0] = v0[i +(n+2)*j];
			U[1] = u0[i+1+(n+2)*j]; V[1] = v0[i+1+(n+2)*j];
			u0[i +(n+2)*j] = f*( (1-x*x/r)*U[0] -x*y/r *V[0] );
			u0[i+1+(n+2)*j] = f*( (1-x*x/r)*U[1] -x*y/r *V[1] );
			v0[i+ (n+2)*j] = f*( -y*x/r *U[0] + (1-y*y/r)*V[0] );
			v0[i+1+(n+2)*j] = f*( -y*x/r *U[1] + (1-y*y/r)*V[1] );
		}
	}

    // transform back to spatial domain
	FFT(-1,u0); FFT(-1,v0);

    // normalize
	f = 1.0f/(n*n);
	for ( i=0 ; i<n ; i++ )
		for ( j=0 ; j<n ; j++ )
			{ u[i+n*j] = f*u0[i+(n+2)*j]; v[i+n*j] = f*v0[i+(n+2)*j]; }

}
