echo off
cd ../../bin/release

rem 100 jacobi iterations, no display, no copies, 
rem quiet mode, benchmark 1000 iters and exit [-b to not exit]

gpgpu_fluid -r  64 -j 100 -d 0 -u 0 -q -bx 1000
gpgpu_fluid -r 128 -j 100 -d 0 -u 0 -q -bx 1000
gpgpu_fluid -r 256 -j 100 -d 0 -u 0 -q -bx 1000

rem 100 jacobi iterations, no display, copies enabled, 
rem quiet mode, benchmark 1000 iters and exit [-b to not exit]

gpgpu_fluid -r  64 -j 100 -d 0 -u 1 -q -bx 1000
gpgpu_fluid -r 128 -j 100 -d 0 -u 1 -q -bx 1000
gpgpu_fluid -r 256 -j 100 -d 0 -u 1 -q -bx 1000

cd ../../src/gpgpu_fluid
pause
