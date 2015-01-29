#version 330

#ifdef VERTEX_SHADER
    uniform mat4 mvpMatrix = mat4(1.0);
    layout (location= 0) in vec3 position;
    layout (location= 1) in vec3 normal;
    layout (location= 2) in vec3 text_coord;
    
    void main( )
    {
        gl_Position = mvpMatrix * vec4(position, 1.0);
    }
#endif

#ifdef FRAGMENT_SHADER
	out vec4 color;
    void main( )
    {
		color = vec4(1.0);
    }
#endif

