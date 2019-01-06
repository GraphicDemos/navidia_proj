/*********************************************************************NVMH4****
Path:  SDK\LIBS\src\nv_nvb
File:  nv_drawmodel_visitor.cpp

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

nv_drawmodel_visitor::nv_drawmodel_visitor()
{

}

void nv_drawmodel_visitor::visit_model(const nv_model * model)
{
    _models.push_back(model);
}

unsigned int nv_drawmodel_visitor::get_num_models() const
{
    return _models.size();
}

const nv_model * nv_drawmodel_visitor::get_model(unsigned int idx) const
{
    return _models[idx];
}

const nv_model * nv_drawmodel_visitor::operator[](unsigned int idx) const
{
    return _models[idx];
}
