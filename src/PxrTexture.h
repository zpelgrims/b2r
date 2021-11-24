/*
# Copyright 2017 Pixar
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.
*/

/****************************************************************************/
/*   Pxr Standard Texturing Functions                                       */
/****************************************************************************/


#ifndef PXR_TEXTURE_H
#define PXR_TEXTURE_H

#include "PxrManifold.h"



struct ColorOpacity
{
    color c;
    float a;
#ifndef COLOROPACITY_ALWAYS_PREMULT
    float isPremultiplied;
#endif
};


void 
Manifold_Copy(Manifold src, output Manifold dest) 
{
    dest.Q = src.Q;
    dest.QN = src.QN;
    dest.Qradius = src.Qradius;
}

void
Manifold_Clear(output Manifold manifold)
{
    manifold.Q = point(0,0,0);
    manifold.QN = normal(0,0,0);
    manifold.Qradius = 0;
}

void
Manifold_Construct(point inP, normal inN, output Manifold m)
{
    m.Q = inP;
    m.QN = inN;
    m.Qradius = length(filterwidth(inP));
}

void
Manifold_Transform(output Manifold ioManifold, 
                    string fromSpace, string toSpace)
{
    ioManifold.Q = transform(fromSpace, toSpace, ioManifold.Q);
    ioManifold.QN = transform(fromSpace, toSpace, ioManifold.QN);
    ioManifold.Qradius = length(filterwidth(ioManifold.Q));
}


void
Manifold_Transform(output Manifold ioManifold, string toSpace)
{
    Manifold_Transform(ioManifold, "common", toSpace);
}

void
Texture_GetSTWithManifold(Manifold manifold, 
                          float inputS, float inputT,
                          output float outputS, output float outputT)
{
    if (isconnected(manifold)) {
        outputS = manifold.Q[0];
        outputT = manifold.Q[1];
    } else {
        outputS = inputS;
        outputT = inputT;
    }
}

void 
Texture_Std_SetupInputs(Manifold manifold, float inputS, float inputT,
                             float filterScale, 
                             output float outputS, output float outputT,
                             output float textureFilterScale)
{
    Texture_GetSTWithManifold(manifold, inputS, inputT, outputS, outputT);

    textureFilterScale = filterScale;
}


///
/// subimage is the face for Ptex
///
void 
Texture_GetColor(string path,
                float s,
                float t,
                string filterType,
                float filterScale,
                float blur,
                color missingColor,
                float missingAlpha,
                int firstChannel,
                int subimage,
                int checkMissing,
                // 0 == no check, let texture call detect and report error for missing textures
                // 1 == check with warning on missing
                // 2 == silent check for missing, handy for UDIM where there might be gaps

                output ColorOpacity result)
{
    int texExists = 1;
    if (path == "") {
        texExists = 0;
    }
    else {
        if (checkMissing > 0) {
            // this might be expensive per ray - probably optimized in osl though
            //
            // Seems to be way faster than calling texture on missing textures - mccoy 7/12/2016
            gettextureinfo(path, "exists", texExists);
        } 
    }
    if (texExists == 0) {
        if (checkMissing == 1 && path != "") {
            warning("texture not found \'%s\'", path);
        }
        result.c = missingColor;
        result.a = missingAlpha;
    } 
    else {
        result.c = texture(path, s, 1.0 - t, 
                "alpha", result.a,
                "interp", filterType, 
                "wrap", "periodic",
                "blur", blur,
                "missingcolor", missingColor,
                "missingalpha", missingAlpha,
                // There is special code to handle the R -> RGB fill
                // so this should fill with 1.0 in other cases,
                // like RGB -> RGBA or R -> RRRA
                // (RA -> RRRA needs to be handled down in side the texture sys.)
                "fill", 1.0,
                "firstchannel", firstChannel,
                "width", filterScale,
                "subimage", max(subimage, 0)
            );
        // all attempts at catching nan's here have been unsuccessful.
    }
}


// For calls with no subimage param
void 
Texture_GetColor(string path,
                float s,
                float t,
                string filterType,
                float filterScale,
                float blur,
                color missingColor,
                float missingAlpha,
                int firstChannel,
                output ColorOpacity result)
{
    Texture_GetColor(path, 
                    s, 
                    t, 
                    filterType,
                    filterScale,
                    blur,
                    missingColor,
                    missingAlpha,
                    firstChannel,
                    0, // subimage
                    0, // let texture() find any errors
                    result);
}


#endif // PXR_TEXTURE_H

