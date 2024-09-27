#version 330 compatibility
// ====== varyings ======

varying vec4 vertex_light_position;
varying vec4 v;

// ====== uniforms ======

uniform vec2 mouse;
uniform float time;
uniform int numTriangles;
uniform int numBalls;
uniform int numWater;
uniform vec4 vertecies[100];
uniform int triangles[100];
uniform vec4 ball_pos[20];     // positions
uniform float ball_radius[20]; // radii
uniform mat4 cameraTransform;

uniform samplerCube skybox;
uniform bool        skybox_enabled;

uniform sampler2D   prevFrame;
uniform bool        prevFrame_enabled;
uniform float       prevFrame_ratio;
uniform bool        softDiffuse;

uniform vec3 water        [80];
uniform vec3 water_normals[80];
uniform float turbulent_min;
uniform float turbulent_max;
uniform bool water_enabled;
uniform bool model_enabled;
uniform bool portal_enabled;
uniform bool refract_enabled;
uniform int shadowSamples;

// ====== local variables ======

int refractThing = 1;

float shadowPerSample = pow(0.50, 1.0/float(shadowSamples));

// enum hitType
int HIT_TYPE_NO_HIT   = 0;
int HIT_TYPE_SPHERE   = 1;
int HIT_TYPE_TRIANGLE = 2;
int HIT_TYPE_INITIAL  = 3;
int HIT_TYPE_WATER    = 4;
int HIT_TYPE_PORTAL   = 5;

// float root2 = sqrt(2.0);
float PI  = acos(0.0)*2.0; // 3.14...
float PI2 = acos(0.0);
vec2 leftFront = normalize(vec2(1.0, 1.0));

bool debug = false;

bool portalled = false;
vec2 portalUV = vec2(0.0);

vec4 toLight; // vector to the light + a small offset
float lightSize = 0.1;

// ====== ret ======

// re-tracing information
struct ret {
    vec4 normal;
    vec4 eye;
    vec4 dir;
    float t; // intersect = eye + t*dir
    int hit;   // what hitType it is
    int thing; // the index of what we collided with
};


// returns a new ret saying that there was no hit
ret noHit() {
    return ret(
        vec4(0.0),
        vec4(0.0),
        vec4(0.0),
        9001.0, // something far away
        HIT_TYPE_NO_HIT,
        0
    );
}


// if B is closer than A, the contents of B are put in A
ret min_ret(ret a, ret b) {
    if (a.hit == HIT_TYPE_NO_HIT || b.t < a.t)
        return b;
    return a;
}

// ====== functions ======

vec2 randomV = v.xy * sin(time);
float rand() { // returns a random value within the range -1.0 to 1.0
    float random = fract(sin(dot(randomV.xy, vec2(12.9898, 78.233)))* 43758.5453)  *2.0 - 1.0;
    randomV = vec2(random, randomV.y*0.6364+randomV.x*0.2412+1.3);
    return random;
}

vec4 rand3D() {
    return vec4(rand(), rand(), rand(), 0.0);
}

// gets called if a ray does not hit any objects
vec4 specular(vec4 dir) {
    if (skybox_enabled)
        return textureCube(skybox, dir.xyz);

    return clamp((
        1.0 *
        pow(max(dot(dir, vertex_light_position), 0.5), 50.0) *
        vec4(1.0, 1.0, 1.0, 0.0)
    ), 0.0, 1.0);
}

float diffuse (vec4 normal, vec4 toLight) { return clamp(dot(normal, toLight), 0.0, 1.0); }
float diffuse (vec4 normal) {  return clamp(dot(normal, vertex_light_position), 0.0, 1.0); } // assumes directional light
float diffuse2(vec4 normal) {  return   abs(dot(normal, vertex_light_position)          ); } // a two-sided version.

bool floatZero(float f) {
    return (-0.0001 < f && f < 0.0001);
}

// takes only the x and y component
vec3 twoD(vec4 v) { return vec3(v.xy, 0.0); }
vec3 twoD(vec3 v) { return vec3(v.xy, 0.0); }

// ====== trace functions ======

// returns whether the given ray intersects the given Axis Aligned Bounding Box
// lb (left bottom) is the corner of AABB with minimal coordinates, rt (right top) is the maximal corner
bool hitAABB(vec4 eye, vec4 dir, vec3 lb, vec3 rt) {
    // r.dir is unit direction vector of ray
    // dirfrac.x = 1.0f / r.dir.x;
    // dirfrac.y = 1.0f / r.dir.y;
    // dirfrac.z = 1.0f / r.dir.z;
    dir = vec4(1.0) / dir; // don't use a new vector

    // r.org is origin of ray
    float t1 = (lb.x - eye.x)*dir.x;
    float t2 = (rt.x - eye.x)*dir.x;
    float t3 = (lb.y - eye.y)*dir.y;
    float t4 = (rt.y - eye.y)*dir.y;
    float t5 = (lb.z - eye.z)*dir.z;
    float t6 = (rt.z - eye.z)*dir.z;

    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    // if tmin > tmax, ray doesn't intersect AABB
    if (tmax < 0.0 || tmin > tmax) {
        return false;
    }

    return true;
}

// ret trace_water(vec4 eye, vec4 dir) {
//     ret r = noHit();

//     // AABB optimization
//     if (! hitAABB(
//         eye,
//         dir,
//         vec3(water[0         ].x, turbulent_min, -1.0),
//         vec3(water[numWater-1].x, turbulent_max,  1.0)
//     )) {
//         return r;
//     }

//     vec3 v = twoD(dir);
//     vec3 p = twoD(eye);
//     for (int i=0; i<numWater-1; ++i) {
//         // ray segment intersection from: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
//         vec3 q  = twoD(water[i  ]);
//         vec3 q1 = twoD(water[i+1]);
//         vec3 s = q1 - q;

//         float c = cross(v, s).z;
//         if (floatZero(c)) continue; // parallel case
//         float t = cross(q - p, s).z / c;
//         float u = cross(q - p, v).z / c;

//         if (u < 0.0 || u > 1.0) continue; // not in segment range

//         if (t > 0.0001 && t < r.t || r.t==0.0) {
//             vec4 intersect = eye + t*dir;
//             if (intersect.z < -1.0 || intersect.z > 1.0) continue; // z bounds
//             r.t = t;
//             r.eye = intersect;
//             r.hit = HIT_TYPE_WATER;
//             // debug = true;
//             r.normal = vec4(normalize(
//                 // water_normals[i]
//                 mix( // interpolate the normals smoothly
//                     water_normals[i  ],
//                     water_normals[i+1],
//                     smoothstep(0.0, 1.0, u)
//                 )
//                 + 0.03*vec3( // add a fake ripples
//                     cos((intersect.x+intersect.z*0.3)*13.0 + time*-3.0),
//                     0.0,
//                     cos((intersect.x)*7.0 + time*4.0) + sin(intersect.z*10.0 + time*1.0)
//                 )
//             ), 0.0);
//         }
//     }


//     return r;
// }

bool xor(bool a, bool b) {
    return (a && !b) || (b && !a);
}

ret trace_triangles(vec4 eye, vec4 dir) {
    ret r = noHit();
    for (int i=0; i<numTriangles*3; i+=3) {

        // Möller Trumbore method
        vec3 edge1 = vertecies[triangles[i+1]].xyz - vertecies[triangles[i  ]].xyz;
        vec3 edge2 = vertecies[triangles[i+2]].xyz - vertecies[triangles[i  ]].xyz;
        vec3 pvec = cross(dir.xyz, edge2);
        float det = dot(edge1, pvec);
        if (det != 0.0) {
            float invDet = 1.0 / det;
            vec3 tvec = vec3(eye - vertecies[triangles[i  ]]);
            vec2 isectData; // UV vector (xy=uv)
            isectData.x = dot(tvec, pvec) * invDet;
            if (!(isectData.x < 0.0 || isectData.x > 1.0)) {

                vec3 qvec = cross(tvec, edge1);
                isectData.y = dot(dir.xyz, qvec) * invDet;
                if (!(isectData.y < 0.0 || isectData.x + isectData.y > 1.0)) {
                    float tt = dot(edge2, qvec) * invDet;
                    vec4 intersect = eye + tt * dir; // the intersect point
                    float t = dot(dir, intersect-eye);
                    if (t > 0.001 && (t < r.t || r.hit==HIT_TYPE_NO_HIT)) {
                        r.t = t;
                        r.hit = HIT_TYPE_TRIANGLE;
                        r.eye = intersect;
                        r.normal = vec4(normalize(cross(edge1, edge2)), 0.0);
                        r.thing = i / 3;



                        if (portal_enabled) {
                            portalUV = isectData;
                            r.hit = HIT_TYPE_PORTAL;
                        }

                        // if (!portal_enabled) continue;
                        // portalUV = isectData;
                        // r.hit = HIT_TYPE_PORTAL;
                    }
                }
            }

        }


    }  // endfor
    return r;
}

ret trace_spheres(vec4 eye, vec4 dir) {
    ret r = noHit();

    // find the best ball
    for (int i=0; i<numBalls; ++i) {
        vec4 pos = ball_pos[i];
        float radius = ball_radius[i];

        vec4 projection = (eye-pos) - dot(eye-pos, dir)*dir;
        float minDist = length(projection);
        if (minDist <= radius) {
            vec4 intersect = (pos+projection) - sqrt((radius*radius)-(minDist*minDist)) * dir;
            float t = dot(dir, intersect-eye);
            vec4 orig = intersect;
            if (refract_enabled && t<0.001 && i==refractThing) {
                intersect = (pos+projection) + sqrt((radius*radius)-(minDist*minDist)) * dir;
                t = dot(dir, intersect-eye);
            }
            if (t > 0.001 && (t < r.t || r.hit==HIT_TYPE_NO_HIT)) {
                r.t = t;
                r.hit = HIT_TYPE_SPHERE;
                r.eye = intersect;
                r.thing = i;
                r.normal = normalize(orig - pos);
            }
        }
    }

    return r;
}

bool hitWater = false;

// the returned r.dir is NOT reflected
ret trace(vec4 eye, vec4 dir) {
    dir = normalize(dir);

    ret r = noHit();
    if (model_enabled){
        r = min_ret(
            trace_spheres(eye, dir),
            trace_triangles(eye, dir)
        );
    }

    // if (water_enabled && !hitWater) {
    //     r = min_ret(
    //         r,
    //         trace_water(eye, dir)
    //     );
    //     hitWater = (r.hit == HIT_TYPE_WATER);
    // }




    r.dir = dir;
    return r;
}


// rotate about y axis
vec4 rotate(vec4 v, float theta) {
    // 2D rotation
    float v_x = v.x; // temp variable
    v.x = cos(theta) * v.x + sin(theta) * v.z;
    v.z = -sin(theta) * v_x + cos(theta) * v.z;
    return v;
}

vec4 transformToOtherPortal(int portal, vec4 dir) {
    return rotate(dir, (portal==0)? -PI2 : PI2);
}


// http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(
        oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
        oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
        oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

// adapted from http://madebyevan.com/webgl-path-tracing/
vec3 biasedDirection(vec3 normal) {
    float u = abs(rand());
    float r = 1.0-sqrt(u);
    float angle = 6.283185307179586 * rand();

    // sdir, tdir and normal are all orthogonal to each others
    vec3 sdir;
    if (abs(normal.x) < 0.5) {
        sdir = cross(normal, vec3(1,0,0));
    }
    else {
        sdir = cross(normal, vec3(0,1,0));
    }
    vec3 tdir = cross(normal, sdir);
    return r*cos(angle)*sdir + r*sin(angle)*tdir + sqrt(1.0-u)*normal;
}

// ====== main ======

void main() {
    // v.x += rand() * 0.002;
    // v.y += rand() * 0.002;


    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    ret r = ret(
        vec4(0.0),
        cameraTransform * vec4(0.0, 0.0, 0.0, 1.0),
        cameraTransform * normalize(vec4(
            v.xy + 0.001 * vec2(rand(), rand()),
            1.5, // decrease this for more FoV
            0.0
        )),
        0.0,
        HIT_TYPE_INITIAL,
        0
    );


    float shadow = 1.0;
    float bounce;
    float maxBounces = 7.0;
    if (softDiffuse) {
        maxBounces = 3.0;
    }
    for (bounce=0.0; r.hit>HIT_TYPE_NO_HIT && bounce<maxBounces; bounce+=1.0) {
        r = trace(r.eye, r.dir);
        // return;

        if (debug) {
            gl_FragColor = vec4(1.0);
            return;
        }


        if (r.hit == HIT_TYPE_WATER) {
            // this refraction is a bit of a hack
            r.normal.y *= r.dir.y;
            r.dir = refract(r.dir, r.normal, 0.93);
            gl_FragColor += vec4(0.01, 0.04, 0.09, 0.0); // water is blue-ish right?

        }
        else if (r.hit == HIT_TYPE_PORTAL) {

            // // calculate r.dir
            // r.dir = transformToOtherPortal(r.thing, r.dir);
            // // reflect(
            // //     transformToOtherPortal(r.thing, r.dir),
            // //     vec4(cross(edge1.xyz, edge2.xyz), 0.0)
            // // );

            // calculate r.eye
            // int o = 1 - r.thing; // other portal
            // vec4 edge1 = vertecies[triangles[o+1]] - vertecies[triangles[o  ]];
            // vec4 edge2 = vertecies[triangles[o+2]] - vertecies[triangles[o  ]];
            // r.eye = vertecies[triangles[o]];
            // r.eye += portalUV.y * (edge1);
            // r.eye += portalUV.x * (edge2);

            bool left = r.thing!=0;
            mat4 portal2portal = rotationMatrix(vec3(0.0, 1.0, 0.0), (left)? -PI2 : PI2);
            r.eye = portal2portal * r.eye;
            r.dir = portal2portal * r.dir;
            r.dir = reflect(r.dir, vec4(1.0, 0.0, 0.0, 0.0));
            r.dir = reflect(r.dir, vec4(0.0, 0.0, 1.0, 0.0));
            if (left)   r.eye = reflect(r.eye, vec4(1.0, 0.0, 0.0, 0.0));
            else        r.eye = reflect(r.eye, vec4(0.0, 0.0, 1.0, 0.0));

            // gl_FragColor = vec4((r.dir.xyz + vec3(1.0, 1.0, 1.0))*0.5, 1.0);
            // return;

            gl_FragColor *= 0.8;
            if (left)   gl_FragColor += vec4(0.2, 0.1, 0.0, 0.0);
            else        gl_FragColor += vec4(0.0, 0.1, 0.2, 0.0);
        }
        else if(refract_enabled && r.hit == HIT_TYPE_SPHERE && r.thing==refractThing) {
            r.dir = refract(r.dir, r.normal, 0.93);
            // gl_FragColor = vec4(vec3(1.0), 1.0);
            // return;
        }
        else {
            r.dir = reflect(r.dir, r.normal);
        }



        if (r.hit > HIT_TYPE_NO_HIT) {
            // shadow
            for (int i=0; i<shadowSamples; ++i) {
                if (trace(r.eye, vertex_light_position + 0.09*rand3D()).hit > HIT_TYPE_NO_HIT) {
                    shadow *= shadowPerSample;
                }
            }

            // diffuse only if we don't have a skybox or we are in diffuse mode
            if (softDiffuse || !skybox_enabled) {
                vec4 colour = vec4(0.5, 0.5, 0.5, 1.0);
                if (softDiffuse) {

                    // colour the left wall red
                    // and the right wall yellow
                    switch (r.thing) {
                        // case 0:
                        //     if (r.hit==HIT_TYPE_SPHERE) colour = vec4(0.0, 1.0, 0.0, 1.0);
                        //     break;
                        case 2:
                        case 3:
                            colour = vec4(1.0, 0.2, 0.2, 1.0);
                            break;
                        case 6:
                        case 7:
                            colour = vec4(0.9, 0.9, 0.0, 1.0);
                            break;
                    }
                    // r.normal += 0.5*normalize(rand3D());
                    // r.normal = normalize(r.normal);
                    // r.dir += 0.5*normalize(rand3D());
                    // r.dir = normalize(r.dir);
                    r.dir = vec4(biasedDirection(r.normal.xyz), 0.0);
                    // if (trace(r.eye, vertex_light_position + 0.09*rand3D()).hit > HIT_TYPE_NO_HIT) {
                    //     continue;
                    // }
                    toLight = vertex_light_position + 0.09*rand3D() - r.eye;
                    colour *= diffuse(r.normal, normalize(toLight));
                    colour *= 2.0 / (length(toLight) * length(toLight));

                }
                else {
                    colour *= diffuse(r.normal);
                }
                // diffuse += vec4(0.0, 0.3, 0.0, 0.0); // ambient
                // gl_FragColor += colour * pow(0.4, bounce+0.0);
                gl_FragColor += colour;
            }

        }

    }

    if (!softDiffuse) {
        gl_FragColor += specular(r.dir) * pow(0.85, bounce);
        gl_FragColor *= shadow;
    }
    gl_FragColor.a = 1.0;


    gl_FragColor = mix(
        texture2D(prevFrame, (v.xy*0.5) + vec2(0.5, 0.5)), // screenspace to UV
        gl_FragColor,
        prevFrame_ratio
    );

    // gl_FragColor = vec4(rand3D(), 0.0);
    // gl_FragColor *= 1.5;
    // gl_FragColor *= 0.5;

}
