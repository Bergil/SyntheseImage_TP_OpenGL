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
        vertex_normal = mat3(mMatrix) * normal; //mat3 car comme ça on retrouve la matrice de rotation 
        vertex_text_coord = text_coord;
    }
#endif

#ifdef FRAGMENT_SHADER
	uniform int nb_lights;
	uniform vec3 light_position[10];
	uniform vec4 light_color[10];
	uniform vec4 lightshadow_color;
	uniform vec3 lightshadow_pos;
	uniform vec3 camera_position = vec3(0.0);
    uniform vec3 ambiante = vec3(0.1,0.1,0.1);
	uniform int isTextured;
    uniform vec4 diffuse_color = vec4(1.0);
    uniform mat4 vpMatrixLight;
	uniform sampler2D color;
    uniform sampler2D shadow;
    
    in vec3 vertex_position;
    in vec3 vertex_normal;
    in vec3 vertex_text_coord;
    out vec4 fragment_color;
   
	float speculaire = 100.0;
	vec4 texture_color;
	vec4 texture_shadow;
    vec4 proj_lumiere;
    vec3 phongLight (vec3 colorlight, vec3 colorobjet, vec3 positionlight, vec3 positionobjet, vec3 normale){
		vec3 Lemise = 20.0 * colorlight/(length(positionlight- positionobjet));
		vec3 h = (normalize(camera_position - positionobjet) + normalize(positionlight- positionobjet)) /2;
		float NdotL = max(0.0, dot(normale, normalize(positionlight - positionobjet))); 
		float RdotL = max(0.0, dot(reflect(normalize(positionobjet - camera_position),normale), normalize(positionlight - positionobjet)));	
		
		return (Lemise*NdotL*colorobjet.rgb  + pow(RdotL,speculaire)  * Lemise);
		
	}
		
    void main()
    {
		vec4 shadowcoordtemp;
		vec4 colortemp;
		if(isTextured != 0.0){
			texture_color.rgb = texture(color, vertex_text_coord.xy).rgb;
			proj_lumiere = (vpMatrixLight * vec4(vertex_position, 1.0));
			shadowcoordtemp = (proj_lumiere+1.0)/2.0; 
			texture_shadow = textureProj(shadow,shadowcoordtemp.xyw); 
			
			vec3 n = normalize(vertex_normal);
			if(texture_shadow.z < shadowcoordtemp.z/proj_lumiere.w && 
			  (shadowcoordtemp.x/proj_lumiere.w<0 || shadowcoordtemp.y/proj_lumiere.w<0 || shadowcoordtemp.x/proj_lumiere.w>1|| shadowcoordtemp.y/proj_lumiere.w>1)){
				fragment_color.rgb = vec3(0.0); //dans l'ombre
			}else{
				fragment_color.rgb = phongLight(lightshadow_color.rgb, texture_color.rgb, lightshadow_pos.xyz, vertex_position.xyz, n);
			}
			
			fragment_color.rgb += ambiante * texture_color.rgb;
			for(int i = 0; i < nb_lights; i++){
				fragment_color.rgb += phongLight(light_color[i].rgb, texture_color.rgb,light_position[i].xyz,  vertex_position.xyz, n)*0.5;
			}
		}else{
			fragment_color.rgb = vec3(1.0,1.0,1.0);
		}
		fragment_color.a = 1.0;
		
    }
#endif

