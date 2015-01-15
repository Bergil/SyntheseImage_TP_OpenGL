
// charge un shader program et l'execute sur tout les pixels de la fenetre.

#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "ProgramManager.h"
#include "GL/GLQuery.h"
#include "GL/GLTexture.h"
#include "GL/GLVertexArray.h"


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



class TP : public gk::App
{
    nv::SdlContext m_widgets;
    
    gk::GLProgram *m_program;
    gk::GLVertexArray *m_vao;
    gk::GLCounter *m_time;
    
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
        
        m_widgets.init();
        m_widgets.reshape(windowWidth(), windowHeight());
    }
    
    ~TP( ) {}
    
    int init( )
    {
        // compilation simplifiee
        m_program= gk::createProgram("tp1sphere.glsl");
        if(m_program == gk::GLProgram::null())
            return -1;
        
        m_vao= gk::createVertexArray();
        m_time= gk::createTimer();
        
        return 0;
    }
    
    int quit( )
    {
        return 0;
    }

    // a redefinir pour utiliser les widgets.
    void processWindowResize( SDL_WindowEvent& event )
    {
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
        m_widgets.reshape(windowWidth(), windowHeight());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // mesurer le temps d'execution
        m_time->start();
        
        /* configuration minimale du pipeline :
            glBindVertexArray() // association du contenu des buffers aux attributs declares dans le shader
            glUseProgram()  // indique quelle paire de shaders utiliser
            { ... } // parametrer les uniforms des shaders

            glDraw()    // execution du pipeline
         */

        glBindVertexArray(m_vao->name);
        glUseProgram(m_program->name);
        // dessiner quelquechose
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        // nettoyage
        glUseProgram(0);
        glBindVertexArray(0);
        
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

