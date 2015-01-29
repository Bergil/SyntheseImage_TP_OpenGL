
#include "App.h"

#include "Mesh.h"
#include "MeshIO.h"

#include "GL/GLBasicMesh.h"
#include "ProgramManager.h"

//! squelette d'application gKit.
class TP : public gk::App
{
    gk::GLProgram *m_program;
    gk::GLBasicMesh *m_mesh;
    
public:
    // creation du contexte openGL et d'une fenetre
    TP( ) 
        : 
        gk::App()
    {
        // specifie le type de contexte openGL a creer :
        gk::AppSettings settings;
        settings.setGLVersion(3,3);     // version 3.3
        settings.setGLCoreProfile();      // core profile
        settings.setGLDebugContext();     // version debug pour obtenir les messages d'erreur en cas de probleme
        
        // cree le contexte et une fenetre
        if(createWindow(512, 512, settings) < 0)
            closeWindow();
    }
    
    ~TP( ) {}
    
    int init( )
    {
        // compilation simplifiee d'un shader program
        gk::programPath("shaders");
        m_program= gk::createProgram("mini", "mini_vertex.glsl", "mini_fragment.glsl");
        if(m_program == gk::GLProgram::null())
            return -1;
        
        // charge un mesh
        gk::Mesh *mesh= gk::MeshIO::readOBJ("bbox.obj");
        if(mesh == NULL)
            return -1;
        
        // cree le vertex array objet, description des attributs / associations aux variables du shader
        // utilise un gk::GLBasicMesh qui permet de creer et de configurer les buffers directement
        m_mesh= new gk::GLBasicMesh(GL_TRIANGLES, mesh->indices.size());
        // cree le buffer de position et l'associe a l'attribut 'position' declare dans le vertex shader, cf core.glsl
        m_mesh->createBuffer(m_program->attribute("position").location, mesh->positions);
        // cree le buffer d'indices et l'associe au vertex array
        m_mesh->createIndexBuffer(mesh->indices);
        
        // mesh n'est plus necessaire, les donnees sont transferees dans les buffers sur la carte graphique
        delete mesh;
        
        // nettoyage de l'etat opengl
        glBindVertexArray(0);                       // desactive le vertex array, cree par gk::GLBasicMesh
        glBindBuffer(GL_ARRAY_BUFFER, 0);           // desactive le buffer de positions
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);   // desactive le buffer d'indices
        
        // ok, tout c'est bien passe
        return 0;
    }
    
    int quit( )
    {
        delete m_mesh;
        return 0;
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
        
        glViewport(0, 0, windowWidth(), windowHeight());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        /* configuration minimale du pipeline :
            glBindVertexArray() // association du contenu des buffers aux attributs declares dans le shader
            glUseProgram()  // indique quelle paire de shaders utiliser
            { ... } // parametrer les uniforms des shaders

            glDraw()    // execution du pipeline
         */

        glUseProgram(m_program->name);
        
        // parametrer le shader
        m_program->uniform("mvpMatrix")= gk::Transform().matrix();      // transformation model view projection
        m_program->uniform("color")= gk::Vec3(1, 1, 0);     // couleur des fragments

        // utilise l'utilitaire draw de gk::GLBasicMesh
        m_mesh->draw();

        // nettoyage
        glUseProgram(0);
        glBindVertexArray(0);
        
        // afficher le dessin
        present();
        return 1;
    }
};


int main( void )
{
    TP app;
    app.run();
    
    return 0;
}

