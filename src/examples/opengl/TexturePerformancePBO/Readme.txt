TexturePerformance PBO Demo
==============

This demo gives examples for different implementation options for repeatedly
downloading texture (streaming model) to the graphics card and reading a
rendered image back (glReadPixel).
The demo is structured by providing the user several modes of operation that
are discussed in the following sections.
For more detailed information turn to the UserGuide.pdf located in the src/docs
folder.

1. Download Mode
----------------

In this mode a 16bit floating point image is repeatedly downloaded to the
graphics card and rendered as a screen-aligned quad after the CPU put a 
"binary frame stamp" in the bottom left corner of the image.
This mode should be rather slow. It can be enabled either using the menu system
(left-clicking on the application brings up the menu) or by hitting '1'.

2. Download Mode (PBO enabled)
------------------------------

This mode performs the same action as mode 1 but uses the Pixel Buffer Extension (PBO)
to speed up the download.

3. Download Mode (PBO, multibuffered)
-------------------------------------

This mode uses several pixel buffer objects to increase performance by reducing
synchronization.

4. Readback Mode
----------------

This mode uploads the image to the graphics card one (glTexImage) and repeatedly 
renders the texture as a screen-aligned quad.
After each render glReadPixel is called and the part of the framebuffer containing
the image is read back to system memory.

5. Roundtrip Mode
-----------------

Roundtrip mode is a combination of mode 2 (PBO enabled download) and 4 (readback).
The image basically makes a roundtrip from system memory to the graphics card, where
it gets rendered. It then gets read back to system memory.


6. Different Shaders
--------------------

Since the image is of high-dynamic range (16bit floating point, OpenEXR format) a 
tone-mapping step is desirable. This is achived using a fragment shader. The user 
of this demo has the choice between two differnt fragment shaders:

* SimpleHDRShader: This shader does a Gamma correction step and can arbitrarily 
                   adjust the exposure.
                   
* FastHDRShader:   This is the minimal shader that simply performs a texture read
                   and returns the texture's color value.
                   
Using the different shaders can greatly influence the performance depending on the
shading capabilities of the hardware.

Other experiments are to resize the window so that only a fraction of the image
is drawn, thus reducing the load on the texture units and shader unit.


7. Contact
----------

For questions contact Frank Jargstorff (fjargstorff@nvidia.com) or (devsupport@nvidia.com)

