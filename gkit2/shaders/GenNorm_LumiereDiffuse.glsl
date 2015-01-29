#version 330

#ifdef VERTEX_SHADER
    uniform mat4 mvpMatrix = mat4(1.0);
    uniform mat4 mMatrix = mat4(1.0);
    layout (location= 0) in vec3 position;

    void main( )
    {
        gl_Position = vec4(position, 1.0);
    }
#endif

#ifdef GEOMETRY_SHADER
    uniform mat4 mvpMatrix = mat4(1.0);
    uniform mat4 mMatrix = mat4(1.0);
	layout(triangles) in;
	layout(triangle_strip, max_vertices = 3) out;
	
	out vec3 vertex_position;
	out vec3 vertex_normal;
	
	void main()
	{
		vec3 a = gl_in[0].gl_Position.xyz;
		vec3 b = gl_in[1].gl_Position.xyz;
		vec3 c = gl_in[2].gl_Position.xyz;
		
		vec3 ab = normalize(b-a);
		vec3 ac = normalize(c-a);
		vec3 n = normalize(cross(ab, ac));
		
		vertex_position = (mMatrix * gl_in[0].gl_Position).xyz;
		vertex_normal = n;
		gl_Position = mvpMatrix * gl_in[0].gl_Position;
		EmitVertex();
		
		vertex_position = (mMatrix * gl_in[1].gl_Position).xyz;
		vertex_normal = n;
		gl_Position = mvpMatrix * gl_in[1].gl_Position;
		EmitVertex();
		
		vertex_position = (mMatrix * gl_in[2].gl_Position).xyz;
		vertex_normal = n;
		gl_Position = mvpMatrix * gl_in[2].gl_Position;
		EmitVertex();
		EndPrimitive();
	}
#endif

#ifdef FRAGMENT_SHADER
    uniform vec4 diffuse_color = vec4(1.0);
    in vec3 vertex_position;
    in vec3 vertex_normal;
    out vec4 fragment_color;
    uniform vec4 light_color = vec4(1.0);
    uniform vec3 light_position = vec3(0.0);
    uniform vec3 camera_position = vec3(0.0);
    uniform vec3 ambiante = vec3(0.1,0.1,0.1);

    void main( )
    {
        
        vec3 n = normalize(vertex_normal);
		vec3 Lemise = light_color.rgb;
		int constante = 10;
		float theta = max(0.0, dot(n, normalize(light_position - vertex_position)));
		float fr = (constante+1)/(2*3.1415926)*pow(theta,10.0f);
        fragment_color.rgb = Lemise*theta*diffuse_color.rgb + ambiante;
        
    }
#endif

