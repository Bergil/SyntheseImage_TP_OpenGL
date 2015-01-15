#version 330

#ifdef VERTEX_SHADER
    uniform mat4 mvpMatrix = mat4(1.0);
    uniform mat4 mMatrix = mat4(1.0);
    layout (location= 0) in vec3 position;
    layout (location= 1) in vec3 normal;
    layout (location= 2) in vec3 text_coord;
	out vec3 vertex_position;
	out vec3 vertex_normal;
	out vec3 vertex_text_coord;
    
    void main( )
    {
        gl_Position = mvpMatrix * vec4(position, 1.0);
        vertex_position = vec3(mMatrix * vec4(position, 1.0));
        vertex_normal = mat3(mMatrix) * normal; //mat3 car comme ça on retrouve la matrie de rotation 
        vertex_text_coord = text_coord;
    }
#endif

#ifdef FRAGMENT_SHADER
	uniform int nb_lights;
	uniform vec3 light_position[10];
	uniform vec4 light_color[10];
	
    uniform vec4 diffuse_color = vec4(1.0);
    in vec3 vertex_position;
    in vec3 vertex_normal;
    in vec3 vertex_text_coord;
    out vec4 fragment_color;
    uniform vec3 camera_position = vec3(0.0);
    uniform vec3 ambiante = vec3(0.1,0.1,0.1);
    uniform sampler2D color;
	float speculaire = 100.0;
	uniform int isTextured;
	vec4 texture_color;
	
    void main( )
    {
		fragment_color.rgb = vec3(0.0);
        vec3 n = normalize(vertex_normal);
        if(isTextured != 0.0){
			texture_color = texture(color, vertex_text_coord.xy);
			fragment_color.rgb = ambiante * texture_color.rgb;
			for(int i = 0; i < nb_lights; i++){
				vec3 Lemise = 20.0 * light_color[i].rgb/(length(light_position[i].xyz- vertex_position));
				vec3 h = (normalize(camera_position - vertex_position) + normalize(light_position[i].xyz- vertex_position)) /2;
				float NdotL = max(0.0, dot(n, normalize(light_position[i].xyz - vertex_position))); 
				float RdotL = max(0.0, dot(reflect(normalize(vertex_position - camera_position),n), normalize(light_position[i].xyz - vertex_position)));	
				
				fragment_color.rgb += Lemise*NdotL*texture_color.rgb  + pow(RdotL,speculaire)  * Lemise ;
			}
		}else{
			fragment_color.rgb = vec3(1.0,1.0,1.0);//pour avoir boule de lumière, sale mais pas le temps de tt reprendre.
		}
		fragment_color.a = 1.0;

    }
#endif

