#version 430    // core profile, compute shader

#ifdef COMPUTE_SHADER

uniform mat4 mvpMatrix;
uniform int positionSize;

layout( binding= 0 ) readonly   // nom du buffer pour l'application, 
buffer positionData
// cf glGetProgramResourceIndex et glBindBufferBase()
{
    vec3 position[];
};

layout( binding= 1)  writeonly   // buffer 1: tableau de positions transformees
buffer transformedData
{
    vec4 transformed[];
};

layout( local_size_x= 32 ) in;
void main( )
{
    uint i= gl_GlobalInvocationID.x;
    if(i < positionSize)
        transformed[i]= mvpMatrix * vec4(position[i], 1.0);
}

#endif
