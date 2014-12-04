
#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "Mesh.h"
#include "MeshIO.h"
#include <sstream>
#include <iostream>
#include "GL/GLQuery.h"
#include "GL/GLTexture.h"
#include "GL/GLBuffer.h"
#include "GL/GLVertexArray.h"
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
        m_vertex_buffer = gk::createBuffer(GL_ARRAY_BUFFER, mesh->positions);
        // associe le contenu du buffer a la variable 'position' du shader
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //0 à la place de m_program->attribute("position") car a priori le programme n'a qu'un seul attribut à ce moment là
															   // c'est hard-coder, et sale, mais tant pis
       
		glVertexAttribPointer(1,3, GL_FLOAT, GL_TRUE, 3*sizeof(float), 0);//pour le calcul de normales
        // active l'utilisation du buffer 
        glEnableVertexAttribArray(0);
        
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
	gk::VecColor color = gk::VecColor(1, 1, 1);
	
public : 
	
	PimpMyMesh(){}
	PimpMyMesh(const MyMesh *mesh, const gk::Transform mat = gk::Transform()){
		init(mesh, mat);
	}
	
	void init(const MyMesh *mesh, const gk::Transform mat = gk::Transform()){
		m_mesh = mesh;
		matModele = mat;		
	}
	
	void draw(gk::Transform VP,  gk::GLProgram & m_program){
		VP = VP * matModele;
		
		bool boolet = false;
        // dessiner quelquechose
        glUseProgram(m_program.name);
        
        //ICI des uniforms !
        m_program.uniform("mvpMatrix")= VP.matrix();      // transformation model view projection
        m_program.uniform("diffuse_color")=  color;    // couleur des fragments
        
        
        //LA des unicorns !
        
        //ben non, c'est un programme de synthese d'images, pas un zoo...
        
		//idiot...
		
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

//! squelette d'application gKit.
class TP : public gk::App
{
    nv::SdlContext m_widgets;
    
    gk::GLProgram *m_program;
    
	std::vector<PimpMyMesh> m_mesh_instances;
	std::vector<MyMesh> m_array_meshes;
    gk::Point m_camera_position;
    gk::Point light_position;
    gk::VecColor light_color;
    gk::Vector m_view_direction;
    gk::GLCounter *m_time;
    float m_speed_camera;
    float m_mouse_sensitivity;
    float m_dx, m_dy;
    int m_mousex, m_mousey;
	bool m_mouse_pressed;
	gk::Transform m_projection;
    
public:
    // creation du contexte openGL et d'une fenetre
    TP( )
        :
        gk::App(),
        m_camera_position(gk::Point(80.0, 20.0, 50.0)),
        light_position(30.0, 50.0, 30.0),
        light_color(1, 1, 1),
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
        m_program= gk::createProgram("dFnormal.glsl");
        m_projection = gk::Perspective(45.0, (float)windowWidth()/ windowHeight(), 0.1, 1000.0);
        if(m_program == gk::GLProgram::null())
            return -1;
        int nbBigguy = 59;
        m_array_meshes.resize(nbBigguy); //atention au +1 pour le sol si on le veut
        m_mesh_instances.resize(nbBigguy);
        for(int i = 0; i < nbBigguy; i++){
			std::ostringstream oss117;
			oss117<<i;
			m_array_meshes[i].init("./Bigguy/bigguy_"+oss117.str()+".obj");
			m_mesh_instances[i].init(&m_array_meshes[i], gk::Translate(gk::Vector((i%8)*25.0, 0.0, -(i/8)*25.0)));  //lignes de 8
			//m_mesh_instances[i].init(&m_array_meshes[i]); //code animation Yoann trop bien ++
		}
		//m_array_meshes[nbBigguy].init("./Bigguy/ground.obj");
		//m_mesh_instances[nbBigguy].init(&m_array_meshes[nbBigguy], gk::Translate(gk::Vector(0.0, 0.0, 0.0)));  //sol
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
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
     
        gk::Transform VP; //pour voir le bigguy dans ce cas là
        
		VP =  m_projection * gk::LookAt(m_camera_position, m_camera_position + m_view_direction, gk::Vector(0.0, 1.0, 0.0));
       
        //Robo Uniform Attack
        static float time = 0.0;
        time += (float)m_time->cpu64() * 0.0000001; //va trop vite sinon soluce temp
		//light_position = (0.5* gk::Vector(std::cos(time), 0.0, std::sin(time))) * gk::Vector(100.0, -10.0, 100.0);
        m_program->uniform("light_color")= light_color; // couleur de la lumière 
        m_program->uniform("light_position")= light_position; // position de la lumière
        m_program->uniform("camera_position")= m_camera_position;
        
        for(int m = 0; m < m_mesh_instances.size(); m++){
				m_mesh_instances[m].draw(VP, *m_program);
				
		}
		
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

