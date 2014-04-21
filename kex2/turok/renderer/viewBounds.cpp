// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION: View bounds
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "renderBackend.h"
#include "camera.h"
#include "viewBounds.h"

//
// kexViewBounds::kexViewBounds
//

kexViewBounds::kexViewBounds(void) {
    Clear();
}


//
// kexViewBounds::Clear
//

void kexViewBounds::Clear(void) {
    max[0]  = -M_INFINITY;
    max[1]  = -M_INFINITY;
    min[0]  =  M_INFINITY;
    min[1]  =  M_INFINITY;
    zmin    = 0.0f;
    zfar    = 1.0f;
}

//
// kexViewBounds::AddBox
//

void kexViewBounds::AddBox(kexCamera *camera, kexBBox &box) {
    kexVec3 points[8];
    kexVec3 pmin;
    kexFrustum frustum;
    int i;
    int bits;
    float d;

    frustum = camera->Frustum();
    
    points[0][0]    = box[1][0];
    points[0][1]    = box[0][1];
    points[0][2]    = box[0][2];
    points[1][0]    = box[1][0];
    points[1][1]    = box[0][1];
    points[1][2]    = box[1][2];
    points[2][0]    = box[0][0];
    points[2][1]    = box[0][1];
    points[2][2]    = box[1][2];
    points[3]       = box[0];
    points[4][0]    = box[1][0];
    points[4][1]    = box[1][1];
    points[4][2]    = box[0][2];
    points[5]       = box[1];
    points[6][0]    = box[0][0];
    points[6][1]    = box[1][1];
    points[6][2]    = box[1][2];
    points[7][0]    = box[0][0];
    points[7][1]    = box[1][1];
    points[7][2]    = box[0][2];

    bits = 0;
    
    kexPlane nearPlane = frustum.Near();
    kexVec3 n = nearPlane.Normal().Normalize();
    
    for(i = 0; i < 8; i++) {
        
        bits = 0;

        d = frustum.Near().Distance(points[i]) + nearPlane.d;
        if(d < 0) {
            points[i] += (n * -d);
        }

        d = frustum.Left().Distance(points[i]) + frustum.Left().d;
        bits |= (FLOATSIGNBIT(d)) << 0;
        d = frustum.Top().Distance(points[i]) + frustum.Top().d;
        bits |= (FLOATSIGNBIT(d)) << 1;
        d = frustum.Right().Distance(points[i]) + frustum.Right().d;
        bits |= (FLOATSIGNBIT(d)) << 2;
        d = frustum.Bottom().Distance(points[i]) + frustum.Bottom().d;
        bits |= (FLOATSIGNBIT(d)) << 3;

        pmin = camera->ProjectPoint(points[i], 0, 0);

        if(bits & 1) {
            pmin[0] = 0;
        }

        if(bits & 2) {
            pmin[1] = 0;
        }

        if(bits & 4) {
            pmin[0] = (float)sysMain.VideoWidth();
        }

        if(bits & 8) {
            pmin[1] = (float)sysMain.VideoHeight();
        }
        
        if(pmin[0] < min[0]) {
            min[0] = pmin[0];
        }
        if(pmin[1] < min[1]) {
            min[1] = pmin[1];
        }
        if(pmin[0] > max[0]) {
            max[0] = pmin[0];
        }
        if(pmin[1] > max[1]) {
            max[1] = pmin[1];
        }
        
        // get closest z-clip
        if(pmin[2] < zfar) zfar = pmin[2];
    }
}

//
// kexViewBounds::ViewBoundInside
//

bool kexViewBounds::ViewBoundInside(const kexViewBounds &viewBounds) {
    if((viewBounds.min[0] < min[0] || viewBounds.min[1] < min[1] ||
        viewBounds.min[0] > max[0] || viewBounds.min[1] > max[1])) {
        return false;
    }
    if((viewBounds.max[0] < min[0] || viewBounds.max[1] < min[1] ||
        viewBounds.max[0] > max[0] || viewBounds.max[1] > max[1])) {
        return false;
    }
    
    if(viewBounds.zfar < zfar) {
        return false;
    }
    
    return true;
}

//
// kexViewBounds::operator=
//

kexViewBounds &kexViewBounds::operator=(const kexViewBounds &viewBounds) {
    min[0] = viewBounds.min[0];
    min[1] = viewBounds.min[1];
    max[0] = viewBounds.max[0];
    max[1] = viewBounds.max[1];

    zmin = viewBounds.zmin;
    zfar = viewBounds.zfar;

    return *this;
}

//
// kexViewBounds::DebugDraw
//

void kexViewBounds::DebugDraw(void) {
    renderBackend.SetState(GLSTATE_TEXTURE0, false);
    renderBackend.SetState(GLSTATE_CULL, false);
    renderBackend.SetState(GLSTATE_BLEND, true);
    renderBackend.SetState(GLSTATE_LIGHTING, false);
    
    renderBackend.DisableShaders();
    renderer.BindDrawPointers();
    renderer.AddLine(min[0], min[1], 0, min[0], max[1], 0, 255, 0, 255, 255);
    renderer.AddLine(min[0], max[1], 0, max[0], max[1], 0, 255, 0, 255, 255);
    renderer.AddLine(max[0], max[1], 0, max[0], min[1], 0, 255, 0, 255, 255);
    renderer.AddLine(max[0], min[1], 0, min[0], min[1], 0, 255, 0, 255, 255);
    renderer.DrawLineElements();
    
    renderBackend.SetState(GLSTATE_TEXTURE0, true);
}
