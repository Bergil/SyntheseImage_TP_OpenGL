#version 430    // core profile, compute shader

#ifdef COMPUTE_SHADER

uniform mat4 mvpMatrix;
uniform mat4 mvpvInvMatrix;

struct ray 
{
    vec3 o;
    vec3 d;
};

struct triangle
{
    vec3 p;
    vec3 edge1;
    vec3 edge2;
    vec3 color;
};

#define EPSILON 0.001
#define RAY_EPSILON 0.0001

bool intersect( in struct triangle triangle, in const ray ray, in const float htmax, out float rt ) //, out float ru, out float rv ) 
{
    /* begin calculating determinant - also used to calculate U parameter */
    vec3 pvec= cross(ray.d, triangle.edge2);
    
    /* if determinant is near zero, ray lies in plane of triangle */
    const float det= dot(triangle.edge1, pvec);
    //~ if (det > -EPSILON && det < EPSILON)
        //~ return false;
    
    const float inv_det= 1.0 / det;
    
    /* calculate distance from vert0 to ray origin */
    const vec3 tvec= ray.o - triangle.p;
    /* calculate U parameter and test bounds */
    const float u= dot(tvec, pvec) * inv_det;
    /* prepare to test V parameter */
    const vec3 qvec= cross(tvec, triangle.edge1);
    /* calculate V parameter and test bounds */
    const float v= dot(ray.d, qvec) * inv_det;
    
    /* calculate t, ray intersects triangle */
    rt= dot(triangle.edge2, qvec) * inv_det;
    //~ ru= u;
    //~ rv= v;
    
    // ne renvoie vrai que si l'intersection est valide (comprise entre tmin et tmax du rayon)
    return (u >= 0.0 && v >= 0.0 && u + v < 1.0 && rt < htmax && rt > RAY_EPSILON);
    //~ return true;
}

layout( binding= 0 ) readonly buffer triangleData  
{
    triangle triangles[];
};

layout( binding= 0, rgba8 ) writeonly uniform image2D framebuffer;

layout( local_size_x= 32 , local_size_y= 2 ) in;
void main( )
{
    // rayon associe au pixel (local.x, local.y) de la tuile
    vec4 oh= mvpvInvMatrix * vec4(gl_GlobalInvocationID.xy, -1, 1);
    vec4 eh= mvpvInvMatrix * vec4(gl_GlobalInvocationID.xy, 1, 1);
    
    struct ray r;
    r.o= oh.xyz / oh.w;
    r.d= eh.xyz / eh.w - oh.xyz / oh.w;

    float h= 1.0;
    vec3 color;
    
    // intersections avec les triangles
    float t;
    for(int i= 0; i < triangles.length(); i++)
    {
        if(intersect(triangles[i], r, h, t))
        {
            h= t;
            color= triangles[i].color;
        }
    }
    
    imageStore(framebuffer, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1));
}

#endif
