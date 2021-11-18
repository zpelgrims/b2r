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
/*   Pxr Standard Manifold Functions                                        */
/****************************************************************************/


#ifndef PXR_MANIFOLD_H
#define PXR_MANIFOLD_H



struct Manifold {
    point Q;
    normal QN;
    float Qradius;
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
Manifold_MultFrequency(output Manifold ioManifold, float frequency)
{
    ioManifold.Q *= frequency;
    ioManifold.Qradius = length(filterwidth(ioManifold.Q));
}


#define MANIFOLD_ZERO { point(0,0,0), normal(0,0,1), 0.0 }


//
// If you use MANIFOLD_STD_INPUT() it creates a port named inputManifold
//   use MANIFOLD_GET_STD_INPUT()
//   to take in a writable inputManifoldRw that contains either
//   the attached inputManifold or (P,N) if no inputManifold exists
//
#define MANIFOLD_STD_INPUT() \
    STRUCT("", Manifold, inputManifold, MANIFOLD_ZERO, W_NULL)


#define MANIFOLD_GET_STD_INPUT(inputManifoldRw) \
    if (!isconnected(inputManifold)) { \
        Manifold_Construct(P,N,inputManifoldRw); \
        Manifold_Transform(inputManifoldRw,"object"); \
    } else Manifold_Copy(inputManifold,inputManifoldRw); \

#define MANIFOLD_STD_INPUT_ISCONNECTED() isconnected(inputManifold)

#define MANIFOLD_STD_OUTPUT() \
    OUT_STRUCT(Manifold, resultManifold, MANIFOLD_ZERO), \
    OUT_POINT(resultPoint, point(0,0,0)), \
    OUT_FLOAT(resultS,0), \
    OUT_FLOAT(resultT,0), \
    OUT_FLOAT(resultRadius,0), \
    OUT_NORMAL(resultNormal, normal(0,0,0)), \
    OUT_FLOAT(resultPointX, 0), \
    OUT_FLOAT(resultPointY, 0), \
    OUT_FLOAT(resultPointZ, 0)

void
Manifold_Set_Std_Outputs(Manifold resultManifold, 
                         output float resultS, 
                         output float resultT,
                         output point resultPoint,
                         output normal resultNormal,
                         output float resultRadius,
                         output float resultPointX,
                         output float resultPointY,
                         output float resultPointZ)
{
    resultS = resultManifold.Q[0];
    resultT = resultManifold.Q[1];
    resultPoint  =  resultManifold.Q;
    resultNormal =  resultManifold.QN;
    resultRadius =  resultManifold.Qradius;
    resultPointX =  resultManifold.Q[0];
    resultPointY =  resultManifold.Q[1];
    resultPointZ =  resultManifold.Q[2];  
}


#define MANIFOLD_SET_STD_OUTPUTS() \
    Manifold_Set_Std_Outputs(resultManifold, resultS, resultT, \
            resultPoint,resultNormal,resultRadius, \
            resultPointX,resultPointY,resultPointZ)


#define MANIFOLD_STD_XFORM_PARAMS() \
    POINT("Transform", origin, point(0,0,0),HELP("")), \
    FLOAT("Transform", frequency, 1,HELP("")), \
    VEC("Transform", scale, vector(1,1,1),HELP("")), \
    FLOAT("Transform", offset, 0,HELP("")), \
    VEC("Transform", offsetVector, vector(0,0,0),HELP("")), \
    POINT("Transform", rotation, point(0,0,0),HELP(""))


void 
Manifold_Std_Xform(output Manifold manifold, 
                   point origin, 
                   float frequency, 
                   vector scale, 
                   float offset, vector offsetVector, 
                   point rotation)
{   
    manifold.Q -= origin;

    if (abs(rotation[0]) > 0) {
        manifold.Q = rotate(manifold.Q, radians(rotation[0]), 
                            point(1,0,0), point(0,0,0));
        manifold.QN = rotate(manifold.QN, radians(rotation[0]), 
                            point(1,0,0), point(0,0,0));
    }
    if (abs(rotation[1]) > 0) {
        manifold.Q = rotate(manifold.Q, radians(rotation[1]), 
                            point(0,1,0), point(0,0,0));
        manifold.QN = rotate(manifold.QN, radians(rotation[1]), 
                            point(0,1,0), point(0,0,0));
    }
    if (abs(rotation[2]) > 0) {
        manifold.Q = rotate(manifold.Q, radians(rotation[2]), 
                            point(0,0,1), point(0,0,0));
        manifold.QN = rotate(manifold.QN, radians(rotation[2]), 
                            point(0,0,1), point(0,0,0));
    }

    manifold.QN = normalize(manifold.QN);

    vector scaleFactor = vector(frequency) / scale;
    manifold.Q *= scaleFactor;

    manifold.Q += origin;
    manifold.Q += offsetVector*offset;

    manifold.Qradius = length(filterwidth(manifold.Q));
}





#endif // PXR_MANIFOLD_H
