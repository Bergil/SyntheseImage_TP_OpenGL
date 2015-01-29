#version 330

#ifdef VERTEX_SHADER

in vec4 transformed;

void main( )
{
    gl_Position= transformed;
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 fragment_color;

void main( )
{
    fragment_color= vec4(1, 1, 0, 1);
}

#endif
