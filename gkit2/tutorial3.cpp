
#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "Mesh.h"
#include "MeshIO.h"
#include <sstream>
#include <iostream>
#include "GL/GLQuery.h"
#include "GL/GLTexture.h"
#include "GL/GLBuffer.h"
#include "Image.h"
#include "ImageIO.h"
#include "ImageManager.h"
#include "GL/GLVertexArray.h"
#include "GL/GLFramebuffer.h"
#include "ProgramManager.h"


//! classe utilitaire : permet de construire une chaine de caracteres formatee. cf sprintf.
struct Format
{
    char text[1024];
    
    Format( const char *_format, ... )
    {
        text[0]= 0;     // chaine vide
        
        // recupere la liste d'arguments supplementaires
        va_list args;
        va_start(args, _format);
        vsnprintf(text, sizeof(text), _format, args);
        va_end(args);
    }
    
    ~Format( ) {}
    
    // conversion implicite de l'objet en chaine de caracteres stantard
    operator const char *( )
    {
        return text;
    }
};

class MyMesh {
	gk::GLVertexArray *m_vao;
    gk::GLBuffer *m_vertex_buffer;
    gk::GLBuffer *m_index_buffer;
    gk::GLBuffer *m_normal_vertex_buffer;
    gk::GLBuffer *m_texture_coord_buffer;
    gk::Point m_pBox[8];
    int m_indices_size;
    
public :
    MyMesh(){}
	MyMesh(const std::string& lien){
		init(lien);
		
	}
	void init(const std::string& lien){
		 // charge un mesh
        gk::Mesh *mesh= gk::MeshIO::readOBJ(lien);
        if(mesh == NULL)
            std::cerr << "Error : Loading Mesh, ou un truc comme ça" << std::endl;
        for(int i= 0; i< 8; i++){
			m_pBox[i] = mesh->box.pMin;
			if(i & 1)
				m_pBox[i].x = mesh->box.pMax.x;
			if(i & 2)
				m_pBox[i].y = mesh->box.pMax.y;
			if(i & 4)
				m_pBox[i].z = mesh->box.pMax.z;
		}
        m_vao = gk::createVertexArray(); // cree le vertex array objet, description des attributs / associations aux variables du shader
        if(m_vao == NULL)
			std::cerr << "Error creating VAO" << std::endl;
		bind();
        
        // cree le buffer de position
        //les vertexq
        m_vertex_buffer = gk::createBuffer(GL_ARRAY_BUFFER, mesh->positions);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //0 à la place de m_program->attribute("position") car a priori le programme n'a qu'un seul attribut à ce moment là
															   // c'est hard-coder, et sale, mais tant pis
		glEnableVertexAttribArray(0);
       //les normals
        m_normal_vertex_buffer = gk::createBuffer(GL_ARRAY_BUFFER, mesh->normals);
        // associe le contenu du buffer a la variable 'position' du shader
       
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);//pour le calcul de normales
        // active l'utilisation du buffer 
        
        glEnableVertexAttribArray(1);
        //les coords de texture
        m_texture_coord_buffer = gk::createBuffer(GL_ARRAY_BUFFER, mesh->texcoords);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
        // cree le buffer d'indices et l'associe au vertex array
        m_index_buffer = gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indices);
        // conserve le nombre d'indices (necessaire pour utiliser glDrawElements)
        m_indices_size = mesh->indices.size();
        
        delete mesh;
        // nettoyage de l'etat opengl
        glBindVertexArray(0);   // desactive le vertex array
        glBindBuffer(GL_ARRAY_BUFFER, 0);       // desactive le buffer de positions
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);       // desactive le buffer d'indices
	}
	int getIndices_size()const{
		return m_indices_size;
	}
	
	const gk::Point* getBbox()const{
		return m_pBox;
	}
		
	void bind()const{
		// selectionner un ensemble de buffers et d'attributs de sommets
		assert(m_vao != nullptr);
		glBindVertexArray(m_vao->name);
   }
};
class PimpMyMesh{
	const MyMesh * m_mesh;
	gk::Transform matModele; //gk::Translate(gk::Vector(20.0, 0.0, 10.0));
	gk::Color color;
	int m_isTextured;
	
public : 
	
	PimpMyMesh(){}
	PimpMyMesh(const MyMesh *mesh, const gk::Transform mat = gk::Transform(), gk::Color colormesh = gk::Color(1, 1, 1), int istext = 1.0){
		init(mesh, mat, colormesh, istext);
	}
	
	void init (const MyMesh *mesh, const gk::Transform mat = gk::Transform(), gk::Color colormesh = gk::Color(1, 1, 1), int istext = 1.0){
		m_mesh = mesh;
		matModele = mat;
		color = colormesh;
		m_isTextured = istext;		
	}
	void setMatModele(gk::Transform model){
		matModele = model;
	}
	
	void draw(gk::Transform VP,  gk::GLProgram & program, bool depth = false){
		
		VP = VP * matModele; //ici important !!
		bool boolet = false;
        // dessiner quelquechose
        glUseProgram(program.name);
        
        //ICI des uniforms !
        program.uniform("mvpMatrix")= VP.matrix();      // transformation model view projection
        if (!depth){
			program.uniform("mMatrix")= matModele.matrix();
			program.uniform("diffuse_color")=  color;    // couleur des fragments
			program.uniform("isTextured")= m_isTextured;
		}
        for(int i = 0; i< 8; i++){
			gk::Point point = VP(m_mesh->getBbox()[i]);
			if(-1 < point.x && point.x< 1 && -1 < point.y && point.y< 1 ){
				boolet = true;
				break;
			}
		}
		if(boolet){
			m_mesh->bind();
			// dessiner un maillage indexe
			glDrawElements(GL_TRIANGLES, m_mesh->getIndices_size(), GL_UNSIGNED_INT, 0);
			//clean
			glBindVertexArray(0);
			
			// nettoyage
			glUseProgram(0);
		}
	}
		
};
struct light{
	gk::Point light_position;
    gk::Color light_color;
};
	

//! squelette d'application gKit.
class TP : public gk::App
{
    nv::SdlContext m_widgets;
    
    gk::GLProgram *m_program; 
    gk::GLProgram *m_program_depth; 
      
	std::vector<PimpMyMesh> m_mesh_instances;
	std::vector<MyMesh> m_array_meshes;
    gk::Point m_camera_position;
    std::vector<light> m_lights; 
    gk::Vector m_view_direction;
    int m_isTextured;
    gk::GLCounter *m_time;
    gk::GLTexture *m_texture;
    float m_speed_camera;
    float m_mouse_sensitivity;
    float m_dx, m_dy;
    int m_mousex, m_mousey;
	bool m_mouse_pressed;
	gk::Transform m_projection;
	gk::Transform m_projection_depth;
	int nbBigguy = 59;
	const int nbLight = 1;  // ICI les lumières sans shadowmap4
	light shadowlight; // ICI la lumière avec shadowmap
	
	GLuint framebuffer;
	GLuint m_depth_texture;
    
public:
    // creation du contexte openGL et d'une fenetre
    TP( )
        :
        gk::App(),
        m_camera_position(gk::Point(80.0, 20.0, 50.0)),
        
        m_view_direction(gk::Vector(0.0, -0.5, -1.0)),
        m_speed_camera(0.5),
		
        m_mouse_sensitivity(0.005),
        m_dx(3.0/2.0),
        m_dy(0.0),
        m_mouse_pressed(false)
    {
        // specifie le type de contexte openGL a creer :
        gk::AppSettings settings;
        settings.setGLVersion(3,3);     // version 3.3
        settings.setGLCoreProfile();      // core profile
        settings.setGLDebugContext();     // version debug pour obtenir les messages d'erreur en cas de probleme
        
        // cree le contexte et une fenetre
        if(createWindow(512, 512, settings) < 0)
            closeWindow();
        
        m_widgets.init();
        m_widgets.reshape(windowWidth(), windowHeight());
    }
    
    ~TP( ) {}
    
    int init( )
    {
		// compilation simplifiee d'un shader program
        gk::programPath("shaders");
        m_program= gk::createProgram("smoothGuysShadowmap.glsl");
        m_program_depth= gk::createProgram("depth.glsl");
        glGenTextures(1, &m_depth_texture);
        glBindTexture(GL_TEXTURE_2D, m_depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D,0);
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
		glFramebufferTexture(GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , m_depth_texture , 0);
		
		glDrawBuffer(GL_NONE);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER,0); //unbind
		
		m_projection_depth = gk::Perspective(45.0, 1.0, 0.1, 1000.0);
        m_projection = gk::Perspective(45.0, (float)windowWidth()/ windowHeight(), 0.1, 1000.0);
        
        if(m_program == gk::GLProgram::null())
            return -1;
            
        m_array_meshes.resize(nbBigguy+1); 
        m_mesh_instances.resize(nbBigguy+nbLight);
        m_lights.resize(nbLight);
        
        for(int i = 0; i < nbBigguy; i++){
			std::ostringstream oss117;
			oss117<<i;
			m_array_meshes[i].init("./obj/Bigguy/bigguy_"+oss117.str()+".obj");
			m_mesh_instances[i].init(&m_array_meshes[i], gk::Translate(gk::Vector((i%8)*25.0, 0.0, -(i/8)*25.0)));  //lignes de 8
			//m_mesh_instances[i].init(&m_array_meshes[i]); //code animation Yoann trop bien ++
		}
		m_array_meshes[nbBigguy].init("./obj/sphere/sphere.obj");
		for(int i = 0; i <nbLight; i++){
			m_lights[i].light_position = gk::Point(50.0, 30.0, 50.0);
			m_lights[i].light_color = gk::Color(1, 1, 1);
			
			m_mesh_instances[nbBigguy+i].init(&m_array_meshes[nbBigguy], gk::Translate(gk::Vector(m_lights[i].light_position)), m_lights[i].light_color, 0.0);  //lumière
        }
        shadowlight.light_position = gk::Point(80.0, 20.0, 100.0);
        shadowlight.light_color = gk::Color(1, 1, 1);
        
        //texture Bigguy
        gk::Image *img = gk::readImage("./obj/Bigguy/bigguy_ambient.png");
        m_texture = gk::createTexture2D(0, img);
        glBindTexture(GL_TEXTURE_2D,0);
		m_program->sampler("color")= 0;
		m_program->sampler("shadow")= 1;
        // mesure du temps de dessin
        m_time= gk::createTimer();
        
        // ok, tout s'est bien passe
        return 0;
    }
    
    int quit( )
    {
        return 0;
    }

    // a redefinir pour utiliser les widgets.
    void processWindowResize( SDL_WindowEvent& event )
    {
		m_projection = gk::Perspective(45.0, (float)windowWidth()/ windowHeight(), 0.1, 1000.0);
		//glViewport(0,0, windowWidth(), windowHeight());
        m_widgets.reshape(event.data1, event.data2);
    }
    
    // a redefinir pour utiliser les widgets.
    void processMouseButtonEvent( SDL_MouseButtonEvent& event )
    {
        m_widgets.processMouseButtonEvent(event);
    }
    
    // a redefinir pour utiliser les widgets.
    void processMouseMotionEvent( SDL_MouseMotionEvent& event )
    {
        m_widgets.processMouseMotionEvent(event);
        
        if( event.state ==  SDL_PRESSED)
        {
			if(!m_mouse_pressed)
			{
				m_mouse_pressed = true;
				m_mousex = event.x;
				m_mousey = event.y;
			}
			m_dx += (event.x-m_mousex) * m_mouse_sensitivity ;
			m_dy += -(event.y-m_mousey) * m_mouse_sensitivity;
			if(m_dy > M_PI) m_dy = M_PI;
			else if (m_dy < -M_PI) m_dy = -M_PI;
			double r_temp = cos(m_dy*M_PI);
			m_view_direction.x = r_temp*cos(m_dx*M_PI);
			m_view_direction.y = sin(m_dy*M_PI);
			m_view_direction.z = r_temp*sin(m_dx*M_PI);
			m_mousex = event.x;
			m_mousey = event.y;
		} else {			
			m_mouse_pressed = false;
		}
    }
    
    // a redefinir pour utiliser les widgets.
    void processKeyboardEvent( SDL_KeyboardEvent& event )
    {
        m_widgets.processKeyboardEvent(event);
    }
    
    int draw( )
    {
        if(key(SDLK_ESCAPE))
            // fermer l'application si l'utilisateur appuie sur ESCAPE
            closeWindow();
        
        if(key('r'))
        {
            key('r')= 0;
            // recharge et recompile les shaders
            gk::reloadPrograms();
        }
        
        if(key('c'))
        {
            key('c')= 0;
            // enregistre l'image opengl
            gk::writeFramebuffer("screenshot.png");
        }
        
        //
        glViewport(0, 0, windowWidth(), windowHeight());
        
        
        // mesurer le temps d'execution
        m_time->start();
        
        
        // déplacement caméra
        gk::Vector Right = gk::Cross(m_view_direction, gk::Vector(0.0, 1.0, 0.0));
        
        if( key(SDLK_LEFT))
			m_camera_position = -m_speed_camera * Right + gk::Vector(m_camera_position);
		if( key(SDLK_RIGHT))
			m_camera_position = m_speed_camera * Right + gk::Vector(m_camera_position);
		if( key(SDLK_z))
			m_camera_position = m_speed_camera * m_view_direction + gk::Vector(m_camera_position);
		if( key(SDLK_s))
			m_camera_position = -m_speed_camera * m_view_direction + gk::Vector(m_camera_position);
		if( key(SDLK_UP))
			m_camera_position = m_speed_camera * gk::Vector(0.0, 1.0, 0.0) + gk::Vector(m_camera_position);
		if( key(SDLK_DOWN))
			m_camera_position = -m_speed_camera * gk::Vector(0.0, 1.0, 0.0) + gk::Vector(m_camera_position);
		
       
        // parametrer le shader
     
        gk::Transform VP; 
        gk::Transform VP_Light;
		VP =  m_projection * gk::LookAt(m_camera_position, m_camera_position + m_view_direction, gk::Vector(0.0, 1.0, 0.0));
		// WARNING : Prévu pour une lumière et une shadowmap !
		VP_Light = m_projection_depth * gk::LookAt(shadowlight.light_position, gk::Point(50.0,0.0,-20.0), gk::Vector(0.0, 1.0, 0.0));
		
        static float time = 0.0;
        time += (float)m_time->cpu64() * 0.000000001; 
        
        for(int i = 0; i <nbLight; i++){
			m_lights[i].light_position = (50 * gk::Vector(std::cos(time*0.5+i*(2*3.1415926)/nbLight), 0.0, std::sin(time*0.5+i*(2*3.1415926)/nbLight/*pour ralentir la boule*/))) + gk::Vector(100.0, 30.0, -40.0);
			m_mesh_instances[nbBigguy+i].setMatModele(gk::Translate(gk::Vector(m_lights[i].light_position)));
			std::ostringstream oss117;
			oss117<<i;
			glProgramUniform3f(m_program->name, glGetUniformLocation(m_program->name, ("light_position["+oss117.str()+"]").c_str()), m_lights[i].light_position.x, m_lights[i].light_position.y, m_lights[i].light_position.z);
			glProgramUniform4f(m_program->name, glGetUniformLocation(m_program->name, ("light_color["+oss117.str()+"]").c_str()), m_lights[i].light_color.r, m_lights[i].light_color.g, m_lights[i].light_color.b, m_lights[i].light_color.a);
		
		}
		glUseProgram(m_program->name);
		m_program->uniform("nb_lights") = nbLight;
		m_program->uniform("lightshadow_color") = shadowlight.light_color;
		m_program->uniform("lightshadow_pos") = shadowlight.light_position;
		
        m_program->uniform("camera_position")= m_camera_position;
        m_program->uniform("vpMatrixLight")= VP_Light.matrix();
        //première pass
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); //bind pour shadowmap
        glViewport(0,0,1024,1024);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        for(unsigned int m = 0; m < m_mesh_instances.size(); m++){
				m_mesh_instances[m].draw(VP_Light, *m_program_depth, true);
		}
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  //unbind, pour dessiner ensuite sur l'écran
		glViewport(0, 0, windowWidth(), windowHeight());

        //seconde pass
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture->name);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_depth_texture);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glProgramUniform1i(m_program->name, glGetUniformLocation(m_program->name, "shadow"), 1);
        glProgramUniform1i(m_program->name, glGetUniformLocation(m_program->name, "color"), 0);
        for(unsigned int m = 0; m < m_mesh_instances.size(); m++){
				m_mesh_instances[m].draw(VP, *m_program);
				
		}
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,0);
		/* code animation Yoann trop bien ++
		static int m = 0;
		m = (m + 1) % 59;
		m_mesh_instances[m].draw(VP, *m_program);
        */
        // mesurer le temps d'execution
        m_time->stop();
        
        // afficher le temps d'execution
        {
            m_widgets.begin();
            m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);
            
            m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());
            
            m_widgets.endGroup();
            m_widgets.end();
        }
        
        // afficher le dessin
        present();
        // continuer
        return 1;
    }
};


int main( int argc, char **argv )
{
    TP app;
    app.run();
    
    return 0;
}

