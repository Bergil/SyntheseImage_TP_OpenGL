#version 330

#ifdef VERTEX_SHADER
    uniform mat4 mvpMatrix;
    layout (location= 0) in vec3 position;
    out vec3 vertex_position;

    void main( )
    {
        gl_Position= mvpMatrix * vec4(position, 1.0);
        vertex_position= position;
    }
#endif

#ifdef FRAGMENT_SHADER
    uniform vec4 diffuse_color;
    in vec3 vertex_position;
    out vec4 fragment_color;
    uniform vec4 light_color;
    uniform vec3 light_position;
    uniform vec3 camera_position;
    uniform vec3 ambiante = vec3(0.1,0.1,0.1);

    void main( )
    {
        vec3 t= normalize(dFdx(vertex_position));
        vec3 b= normalize(dFdy(vertex_position));
        vec3 n= normalize(cross(t, b));
		vec3 Lemise = light_color.rgb;
		//vec3 fr = diffuse_color.rgb;
		int constante = 10;
		vec3 h = (normalize(camera_position - vertex_position) + normalize(light_position- vertex_position)) /2;
		float theta = max(0.0, dot(h,n));
		float fr = (constante+1)/(2*3.1415926)*pow(theta,constante);
        fragment_color.rgb = Lemise*fr*theta + ambiante;
        
        
        //fragment_color.rgb = n;
        //fragment_color.rgb= diffuse_color.rgb * abs(n.z);
        //~ fragment_color.rgb= diffuse_color.rgb;
        //~ fragment_color.rgb= abs(n.zzz);
    }
#endif

