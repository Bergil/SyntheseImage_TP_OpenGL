#version 430    // core profile, compute shader

#ifdef COMPUTE_SHADER

uniform mat4 mvpMatrix;
uniform mat4 mvpvInvMatrix;
uniform int triangleCount;

#define EPSILON 0.001
#define RAY_EPSILON 0.0001

bool intersect( in vec3 p, in vec3 edge1, in vec3 edge2, in vec3 o, in vec3 d, in float htmax, out float rt ) //, out float ru, out float rv ) 
{
#if 1
    /* begin calculating determinant - also used to calculate U parameter */
    vec3 pvec= cross(d, edge2);
    
    /* if determinant is near zero, ray lies in plane of triangle */
    const float det= dot(edge1, pvec);
    //~ if (det > -EPSILON && det < EPSILON)
        //~ return false;
    
    const float inv_det= 1.0 / det;
    /* calculate distance from vert0 to ray origin */
    const vec3 tvec= o - p;
    /* calculate U parameter and test bounds */
    const float u= dot(tvec, pvec) * inv_det;
    /* prepare to test V parameter */
    const vec3 qvec= cross(tvec, edge1);
    /* calculate V parameter and test bounds */
    const float v= dot(d, qvec) * inv_det;
    
    /* calculate t, ray intersects triangle */
    rt= dot(edge2, qvec) * inv_det;
    //~ ru= u;
    //~ rv= v;
    
    // ne renvoie vrai que si l'intersection est valide (comprise entre tmin et tmax du rayon)
    return (u >= 0.0 && v >= 0.0 && u + v < 1.0 && rt < htmax && rt > RAY_EPSILON);
    
#else
    return true;
#endif
}

layout( local_size_x= 64, local_size_y= 16 ) in;

struct triangle_batch
{
    vec3 p[256];  
    vec3 edge1[256];  
    vec3 edge2[256];  
    vec3 color[256];  
};

layout( binding= 0 ) readonly buffer triangleData  
{
    triangle_batch triangles[];
};

layout( binding= 0, rgba8 ) uniform image2D framebuffer;

shared vec3 local_p[256];
shared vec3 local_edge1[256];
shared vec3 local_edge2[256];
shared vec3 local_color[256];

void main( )
{
    // resultat
    float h= 1.0;
    vec3 color= vec3(1, 0, 0);

    #if 1
    // rayon associe au pixel (local.x, local.y) de la tuile
    vec4 oh= mvpvInvMatrix * vec4(gl_GlobalInvocationID.xy, -1, 1);
    vec4 eh= mvpvInvMatrix * vec4(gl_GlobalInvocationID.xy, 1, 1);
    vec3 o= oh.xyz / oh.w;
    vec3 d= eh.xyz / eh.w - oh.xyz / oh.w;
    
    // position dans la tuile 
    uint index= gl_LocalInvocationID.y * gl_WorkGroupSize.x + gl_LocalInvocationID.x; 

    for(uint batch= 0; batch < triangles.length(); batch++)
    {
        barrier();

        // copie en parallele d'un groupe de triangles dans la memoire partagee
        if(256 * batch + index  < triangleCount)
        {
            local_p[index]= triangles[batch].p[index];
            local_edge1[index]= triangles[batch].edge1[index];
            local_edge2[index]= triangles[batch].edge2[index];
            local_color[index]= triangles[batch].color[index];
        }
        
        barrier();
        
        // intersections avec les triangles
        float t;
        uint n= min(256, triangleCount - batch * 256);
        for(uint i= 0; i < n; i++)
        {
            uint local_index= i;
            if(intersect(local_p[local_index], local_edge1[local_index], local_edge2[local_index], o, d, h, t))
            {
                h= t;
                color= local_color[local_index];
            }
        }
    }
    #endif
    
    imageStore(framebuffer, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1));
}

#endif
