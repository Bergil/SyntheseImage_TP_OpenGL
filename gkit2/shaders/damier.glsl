#version 330

#ifdef VERTEX_SHADER

out vec3 position;

void main( )
{
    vec3 quad[4]= vec3[4]( vec3(-1, -1, 0), vec3( 1, -1, 0), 
                           vec3(-1,  1, 0), vec3( 1,  1, 0) );
    
    position= quad[gl_VertexID];
    gl_Position= vec4(quad[gl_VertexID], 1.0);
}

#endif

#ifdef FRAGMENT_SHADER
uniform int taille;
in vec3 position;

out vec4 fragment_color;

void main( )
{
    int ligne= int(gl_FragCoord.y) / taille;
    int colonne= int(gl_FragCoord.x) / taille;
    int couleur= (colonne % 2) ^ (ligne % 2);
    
    fragment_color= vec4(couleur > 0 ? 1.0 : 0.0);
}

#endif
