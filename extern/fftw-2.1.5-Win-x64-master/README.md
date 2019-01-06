Description
FFTW 2.1.5, a collection of fast C routines to compute the Discrete
Fourier Transform in one or more dimensions.

AUTHOR of Solution File: ~ Sweet ~

Visual Studio 2013 Solution File: 'build/fftw_rfftw-VS2013.sln'

The solution file was created in Visual Studio 2013
using the 'Configuration Properties>General>Platform Toolset: Visual Studio 2010(v100)

The solution is designed to build both lib files for Panda3D.
'fftw.lib' & 'rfftw.lib'

This repo contains the source files needs to build the FFTW third-party
libraries for using in the Panda3D-?-x64 source.

I designed the project to output the built lib files into the proper
folder structure for the Panda3D thirdparty folder integration.
After building the lib files, simply merge the created thirdparty
folder into the thirdparty folder that's found in the root of the
Panda3D source and build the Panda3D-?-x64.

The directory structure in the Panda3D source should be the following:
Panda3D-?-x64/thirdparty/win-libs-vc10-x64/fftw/include/config.h
Panda3D-?-x64/thirdparty/win-libs-vc10-x64/fftw/include/fftw.h
Panda3D-?-x64/thirdparty/win-libs-vc10-x64/fftw/include/rfftw.h
Panda3D-?-x64/thirdparty/win-libs-vc10-x64/fftw/lib/fftw.lib
Panda3D-?-x64/thirdparty/win-libs-vc10-x64/fftw/lib/rfftw.lib

Author Disclaimer:
THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

CONTACTS:
Authors of FFTW reachable at fftw@fftw.org:
Matteo Frigo athena@fftw.org
Stevenj G. Johnson (stevenj@alum.mit.edu)
FFTW Homepage: http://www.fftw.org

