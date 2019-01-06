/*********************************************************************NVMH4****
Path:  SDK\LIBS\src\nv_nvb
File:  nv_factory.cpp

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:





******************************************************************************/

#include "stdafx.h"

nv_factory * nv_factory::_the_factory = 0;

nv_factory::nv_factory()
{

}

nv_factory * nv_factory::get_factory()
{
    if (_the_factory)
        return _the_factory;
    _the_factory = new nv_factory();
    return _the_factory;
}
    
bool nv_factory::shutdown()
{
    if (_the_factory)
    {
        delete _the_factory;
        _the_factory = 0;
        return true;
    }
    return false;
}

nv_drawmodel_visitor * nv_factory::new_drawmodel_visitor()
{
    nv_drawmodel_visitor * visitor = new nv_drawmodel_visitor;
    visitor->add_ref();
    return visitor;
}

nv_scene * nv_factory::new_scene()
{
    nv_scene * scene = new nv_scene;
    scene->add_ref();
    return scene;
}


nv_node * nv_factory::new_node()
{
    nv_node * node = new nv_node;
    node->add_ref();
    return node;
}

nv_model * nv_factory::new_model()
{
    nv_model * model = new nv_model;
    model->add_ref();
    return model;
}

nv_light * nv_factory::new_light()
{
    nv_light * light = new nv_light;
    light->add_ref();
    return light;
}

nv_camera * nv_factory::new_camera()
{
    nv_camera * camera = new nv_camera;
    camera->add_ref();
    return camera;
}
