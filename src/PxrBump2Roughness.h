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
/*   Pxr Standard Bump to Roughness Functions                               */
/****************************************************************************/


#ifndef PXR_BUMP2ROUGHNESS_H
#define PXR_BUMP2ROUGHNESS_H



struct Struct_Bump2RoughnessBundle
{
    float l1;
    float l2;
    vector v1;
    vector v2;
    vector bumpnorm;
};


struct Struct_Eigen2D
{
    float l1;
    float l2;
    vector v1;
    vector v2;
};


void partialDeriv(point Q, float a, float b,
                  output vector dQda, output vector dQdb)
{
    vector dQdx = Dx(Q);
    vector dQdy = Dy(Q);

    float A = Dx(a);
    float B = Dx(b);
    float C = Dy(a);
    float D = Dy(b);

    float invdet = 1.0/(A*D - B*C);
    
    dQda = (dQdx*D-dQdy*B)*invdet;
    dQdb = (dQdy*A-dQdx*C)*invdet;
}


void partialDeriv(float Q, float a, float b,
                  output float dQda, output float dQdb)
{
    float dQdx = Dx(Q);
    float dQdy = Dy(Q);

    float A = Dx(a);
    float B = Dx(b);
    float C = Dy(a);
    float D = Dy(b);

    float invdet = 1.0/(A*D - B*C);
    
    dQda = (dQdx*D-dQdy*B)*invdet;
    dQdb = (dQdy*A-dQdx*C)*invdet;
}

void covarToEigen2D(float a, float b, float c,
                    output float l1, output float l2,
                    output vector v1, output vector v2)
{
    //2x2 covariance matrix to eigen space transformation
    
    //solve for this => (a b)*(v1_x v2_x) = (l1)*(v1_x v2_x)
    //                  (b c) (v1_y v2_y) = (l2)*(v1_y v2_y)

    //del is the discriminate
    float del = sqrt(a*a+4*b*b-2*a*c+c*c);

    //solve for eigenvalues
    l1 = .5*(a+c+del);
    l2 = .5*(a+c-del);

    //two eigenvectors v1, v2, corresponding to l1, l2, respectively
    v1=vector(0);
    v1[1] = 1;
    v1[0] = (l1-c)/b;
    v1 = normalize(v1);

    v2=vector(0);
    v2[1] = 1;
    v2[0] = (l2-c)/b;
    v2 = normalize(v2);
}

/*
void SymetricEigenVecVal2D(float a, float b, float c,
                    output float l1, output float l2,
                    output vector v1, output vector v2)
{
    //del is the discriminate
    float del = sqrt(a*a+4*b*b-2*a*c+c*c);
    
    //solve for eigenvalues
    l1 = .5*(a+c+del);
    l2 = .5*(a+c-del);

    // two eigenvectors v1 , v2
    v1=vector (0);
    v1[0] = 1;
    v1[1] = (l1-a)/b;
    v1 = normalize(v1);

    v2=vector (0);
    v2[0] = 1;
    v2[1] = (l2-a)/b;
    v2 = normalize(v2);
}
*/


void covarToEigen2D(float a, float b, float c,
                    output Struct_Eigen2D Eigens)
{
    covarToEigen2D(a, b, c, Eigens.l1, Eigens.l2, Eigens.v1, Eigens.v2);
}



void bumpRoughMapping(color D1, color D2,
                      output float l1, output float l2,
                      output vector v1, output vector v2)
{
    float dx=D1[0];
    float dy=D1[1];
    float dxdx=D2[0];
    float dydy=D2[1];
    float dxdy=D2[2];

    float sxx = 2*(dxdx - dx*dx);
    float sxy = 2*(dxdy - dx*dy);
    float syy = 2*(dydy - dy*dy);

    covarToEigen2D(sxx, sxy, syy, l1, l2, v1, v2);
    v1 = normalize(v1);
    v2 = normalize(v2);
}

void bumpRoughMapping(color D1, color D2,
                      output Struct_Eigen2D Eigens)
{
    bumpRoughMapping(D1, D2, Eigens.l1, Eigens.l2, Eigens.v1, Eigens.v2);
}

/*
    private function use the function below to conform to standard
 */
void _combineEigens2D(float al1, float al2, vector av1, vector av2,
                     float bl1, float bl2, vector bv1, vector bv2,
                     output float ol1, output float ol2,
                     output vector ov1, output vector ov2)
{
    
    vector _av1 = normalize(av1);
    vector _av2 = normalize(av2);
    vector _bv1 = normalize(bv1);
    vector _bv2 = normalize(bv2);
    
    if (dot(av1,bv1)<0) {                   // make sure that both eigen vectors are facing the same hemisphere
        _bv1=-_bv1;
        _bv2=-_bv2;
    }
    
    ol2 = al2+bl2;                          // this accounts for the isotropic sum of the two eigen spaces
    
    vector bv_delta = bl1*_bv1- bl2*_bv1;   // this measures how elliptical b eigenspace is
    
    ov1 = al1*_av1+bv_delta;                // now figure out the anisotropic part of the new eigenvectors
    ov2 = normalize(cross(N, ov1));         // then orthogonalize
    ol1 = length(ov1);
    ov1 = normalize(ov1);    
}
                                                                                                                                                                                                    
/*
    private function use the function below to conform to standard
 */
void _combineEigens2D_v1(float al1, float al2, vector av1, vector av2,
                     float bl1, float bl2, vector bv1, vector bv2,
                     output float ol1, output float ol2,
                     output vector ov1, output vector ov2)
{
    
    vector _av1 = normalize(av1);
    vector _av2 = normalize(av2);
    vector _bv1 = normalize(bv1);
    vector _bv2 = normalize(bv2);
    
    if (dot(av1,bv1)<0) {                   // make sure that both eigen vectors are facing the same hemisphere
        _bv1=-_bv1;
        _bv2=-_bv2;
    }
    
    ol2 = al2+bl2;                          // this accounts for the isotropic sum of the two eigen spaces
    
    vector bv_delta = bl1*_bv1- bl2*_bv1;   // this measures how elliptical b eigenspace is
    
    ov1 = al1*_av1+bv_delta;                // now figure out the anisotropic part of the new eigenvectors
    ov2 = normalize(cross(N, ov1));         // then orthogonalize
    ol1 = length(ov1);
    ov1 = normalize(ov1);    
}

/* this swaps the major and minor eigens */
void _swapEigen2D(float al1, float al2, vector av1, vector av2,
                  output float ol1, output float ol2, output vector ov1, output vector ov2)
{
    ol1 = al2;
    ov1 = av2;
    ol2 = al1;
    ov2 = av1;
}

/* this normalizes both the major and minor eigens */
void _normalizeEigen2D(float al1, float al2, vector av1, vector av2,
                       output float ol1, output float ol2, output vector ov1, output vector ov2)
{
    float w1 = length(av1);
    float w2 = length(av2);
    ov1 = normalize(av1);
    ov2 = normalize(av2);
    ol1 = al1*w1;
    ol2 = al2*w2;
}

/* we would always want the eigenvalues to be right-handed, this function does not change the
   handedness of the major compoent, but reverse the minor component to enforce the rule */
void _forceRhandEigen2D(float al1, float al2, vector av1, vector av2,
                       output float ol1, output float ol2, output vector ov1, output vector ov2)
{
    
    ol1 = al1;
    ol2 = al2;
    ov1 = av1;
    vector Nn = vector(normalize(N));
    vector Ncross = vector(cross(av1, av2));
    if(dot(Nn,Ncross)<.0){
        ov2 = -av2;
    } else {
        ov2 = av2;
    }
}

/* if al1 and bl1 are pointed in opposite directions, flip bl1 and bl2 so they are aligned with al1 and al2 */
void _alignEigenB2D(float al1, float al2, vector av1, vector av2,
                    float bl1, float bl2, vector bv1, vector bv2,
                    output float ol1, output float ol2, output vector ov1, output vector ov2)
{
    if (dot(av1,bv1)>=.0) {
        ol1 = bl1;
        ol2 = bl2;
        ov1 = bv1;
        ov2 = bv2;
    } else {
        ol1 = bl1;
        ol2 = bl2;
        ov1 = -bv1;
        ov2 = -bv2;
    }
}


/*
    private function use the function below to conform to standard
 */
// void _combineEigens2D(float al1, float al2, vector av1, vector av2,
//                       float bl1, float bl2, vector bv1, vector bv2,
//                       output float ol1, output float ol2, 
//                       output vector ov1, output vector ov2)
// {
    
//     _normalizeEigen2D(al1, al2, av1, av2, al1, al2, av1, av2);
//     _normalizeEigen2D(bl1, bl2, bv1, bv2, bl1, bl2, bv1, bv2);
//     if(al1<al2){
//         _swapEigen2D(al1, al2, av1, av2, al1, al2, av1, av2);
//     }
//     if(bl1<bl2){
//         _swapEigen2D(bl1, bl2, bv1, bv2, bl1, bl2, bv1, bv2);
//     }
//     _forceRhandEigen2D(al1, al2, av1, av2, al1, al2, av1, av2);
//     _forceRhandEigen2D(bl1, bl2, bv1, bv2, bl1, bl2, bv1, bv2);
//     _alignEigenB2D(al1, al2, av1, av2,
//                    bl1, bl2, bv1, bv2,
//                    bl1, bl2, bv1, bv2);
        
//     vector sumv = al1*av1+bl1*bv1;
//     //proof that sin(theta) is the same whether measured from av1 or av2
//     //printf("test.av1=%f,    test.bv1=%f\n", length(cross(normalize(sumv),al1*av1)), length(cross(normalize(sumv),bl1*bv1)));
    
//     ol1 = al1+bl1;
//     ol2 = al2+bl2;
//     ov1 = normalize(sumv);

//     //project to tangent plane
//     ov2 = normalize(cross(N, ov1));
//     ov1 = normalize(cross(ov2, N));
    
// }


/*
    simple function to test input signals for combining eigens
 */
/*
void combineEigens2D_privateTest(float al1, float al2, vector av1, vector av2,
                     float bl1, float bl2, vector bv1, vector bv2,
                     output float ol1, output float ol2, output vector ov1, output vector ov2)
{
    
    ol1 = al1+bl1;
    ol2 = al2+bl2;
    ov1 = av1;
    ov2 = av2;
}
*/
        
/*
    public function, this is called by outside functions not in this .h file
 */

void combineEigens2D(Struct_Bump2RoughnessBundle a, Struct_Bump2RoughnessBundle b,
                     output Struct_Bump2RoughnessBundle c)
{
    
    if(a.l1==0 && a.l2==0){
        c = b;
    } else if (b.l1==0 && b.l2==0){
        c = a;
    } else {
        _combineEigens2D(a.l1, a.l2, a.v1, a.v2,
                        b.l1, b.l2, b.v1, b.v2,
                        c.l1, c.l2, c.v1, c.v2);
    }
}


/*
    In contrast to ColorOpacity, BumpRoughness has no equivalence concept to
    color premult. And is at full roughness and anistropy whatever the opacity value.
    Therefore, when we Over-Composite BumpRoughness A over B -> C, C is then
    assumed to be fully "opaque" in Eigenspace, even when the opacity is < 1.
    For the resulting Opacity of the A over B -> C operation, its math remains
    the same as coloropacity over-composite operations.
 */

void EigenBlend_Over(Struct_Bump2RoughnessBundle Eover,       float Oover,
                     Struct_Bump2RoughnessBundle Eunder,      float Ounder,
                     output Struct_Bump2RoughnessBundle Eout, output float Oout)
{
    Struct_Bump2RoughnessBundle Eover_writeable = Eover;
    Struct_Bump2RoughnessBundle Eunder_writeable = Eunder;

    float Ototal = Oover+Ounder;
    
    if(Ototal<=0){
        Eout.l1 = 0;
        Eout.l2 = 0;
        Eout.v1 = vector(1,0,0);
        Eout.v2 = vector(0,1,0);
        Eout.bumpnorm = normalize(N);
        Oout = 0;
        return;
    } else if (Oover<=0){
        Eout = Eunder;
        Oout = Ounder;
        return;
    }
    
    /*
    Is this the smartest way to correct for over-1 or under-1 total opacities?
    - jling: come up with a smarter over, or maybe we don't need this?
    */
    float divOtotal = 1/Ototal; 
    
    Eunder_writeable.l1 = (1.-Oover)*Eunder_writeable.l1*divOtotal;
    Eunder_writeable.l2 = (1.-Oover)*Eunder_writeable.l2*divOtotal;
    
    Eover_writeable.l1 = Oover*Eover_writeable.l1*divOtotal;
    Eover_writeable.l2 = Oover*Eover_writeable.l2*divOtotal;
    
    combineEigens2D(Eover_writeable, Eunder_writeable, Eout); //full intensity combine of the eigenvalues

    Eout.bumpnorm = Oover*Eover_writeable.bumpnorm + (1.-Oover)*Eunder_writeable.bumpnorm;
    Eout.bumpnorm = normalize(Eout.bumpnorm);
    Oout = Oover + (1.-Oover)*Ounder;     //plain-vennila over for the opacity values
}

void EigenBlend_Add(Struct_Bump2RoughnessBundle Eover,       float Oover,
                    Struct_Bump2RoughnessBundle Eunder,      float Ounder,
                    output Struct_Bump2RoughnessBundle Eout, output float Oout)
{
    Struct_Bump2RoughnessBundle Eover_writeable = Eover;
    Struct_Bump2RoughnessBundle Eunder_writeable = Eunder;

    if(Oover<=0){
        Eout = Eunder;
        Oout = Ounder;
        return;
    }
    
    Eover_writeable.l1 = Oover*Eover_writeable.l1;    
    Eover_writeable.l2 = Oover*Eover_writeable.l2;
    combineEigens2D(Eover_writeable, Eunder_writeable, Eout);
    Eout.bumpnorm = Oover*Eover_writeable.bumpnorm + Eunder_writeable.bumpnorm;
    Eout.bumpnorm = normalize(Eout.bumpnorm);
    Oout = Oover + (1.-Oover)*Ounder;     //plain-vennila over for the opacity values
}

#endif //enddef PXR_BUMP2ROUGHNESS_H
