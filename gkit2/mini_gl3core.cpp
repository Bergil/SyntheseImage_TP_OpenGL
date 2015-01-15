
#include <set>
#include <string>
#include <fstream>
#include <sstream>

#include "GL/GLPlatform.h"
#include "SDLPlatform.h"

#ifndef _MSC_VER
  #define GK_CALLBACK
#else
  #define GK_CALLBACK __stdcall
#endif

//! utilitaires.

//! charge un fichier texte.
std::string read( const char *filename )
{
    std::stringbuf source;
    std::ifstream in(filename);
    if(in.good() == false)
        printf("error reading '%s'\n", filename);
    
    in.get(source, 0);        // lire tout le fichier, le caractere '\0' ne peut pas se trouver dans le source de shader
    return source.str();
}

//! affiche les erreurs de compilation de shader.
bool shader_errors( const GLuint shader )
{
    GLint status; glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_TRUE) return false;
    
    char errors[4096];
    glGetShaderInfoLog(shader, sizeof(errors) -1, NULL, errors);
    printf("errors:\n%s\n", errors);
    return true;
}

//! affiche les erreurs d'edition de liens du programme.
bool program_errors( const GLuint program )
{
    GLint status; glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_TRUE) return false;
    
    char errors[4096];
    glGetProgramInfoLog(program, sizeof(errors) -1, NULL, errors);
    printf("errors:\n%s\n", errors);
    return true;
}

//! charge les sources des shaders, les compile, cree un shader program, verifie les erreurs et renvoie le program.
GLuint make_program( const char *vertex, const char *fragment )
{
    // gl core profile : compiler un shader program
    GLuint program= glCreateProgram();
    
    // creer un vertex shader
    GLuint vertex_shader= glCreateShader(GL_VERTEX_SHADER);
    {
        std::string source= read(vertex);
        if(source.empty()) return 0;
        
        const char *sources[]= { source.c_str() };
        glShaderSource(vertex_shader, 1, sources, NULL);
        glCompileShader(vertex_shader);
        if(shader_errors(vertex_shader))
        {
            printf("vertex source:\n%s\n", sources[0]);
            return 0;
        }
    }
    
    // creer un fragment shader
    GLuint fragment_shader= glCreateShader(GL_FRAGMENT_SHADER);
    {
        std::string source= read(fragment);
        if(source.empty()) return 0;
        
        const char *sources[]= { source.c_str() };
        glShaderSource(fragment_shader, 1, sources, NULL);
        glCompileShader(fragment_shader);
        if(shader_errors(fragment_shader))
        {
            printf("fragment source:\n%s\n", sources[0]);
            return 0;
        }
    }
    
    // construire le shader program
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    // verifier les erreurs
    if(program_errors(program)) return 0;
    
    // plus besoin des shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  
    return program;
}


//! boucle de gestion des evenements de l'application.
int draw( );
int run( SDL_Window *window )
{
    SDL_Event event;
    int width;
    int height;
    int stop= 0;
    
    while(stop == 0)
    {
        // gestion des evenements 
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_WINDOWEVENT:
                    if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        width= event.window.data1;
                        height= event.window.data2;
                        SDL_SetWindowSize(window, width, height);
                        
                        glViewport(0, 0, width, height);
                    }
                    break;
                    
                case SDL_QUIT:
                    stop= 1;
                    break;
            }
        }
        
        // dessiner
        draw();
        SDL_GL_SwapWindow(window);
    }

    return 0;
}

//! creation d'une fenetre pour l'application.
SDL_Window *create_window( const int width= 512, const int height= 512 )
{
    // init sdl
    if(SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE ) < 0)
    {
        printf("SDL_Init() failed:\n%s\n", SDL_GetError());
        return NULL;
    }
    
    // enregistre le destructeur de sdl
    atexit(SDL_Quit);
    
    // creer la fenetre et le contexte openGL
    SDL_Window *window= SDL_CreateWindow("gKit", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(window == NULL)
    {
        printf("error creating sdl2 window.\n");
        return NULL;
    }
    
    SDL_SetWindowDisplayMode(window, NULL);
    return window;
}


//! affiche les messages d'erreur opengl. (contexte debug core profile necessaire).
void GK_CALLBACK debug( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, void *userParam )
{
    static std::set<std::string> log;
    if(log.insert(message).second == false) 
        // le message a deja ete affiche, pas la peine de recommencer 60 fois par seconde.
        return;    
    
    if(severity == GL_DEBUG_SEVERITY_HIGH)
        printf("openGL error:\n%s\n", message);
    else if(severity == GL_DEBUG_SEVERITY_MEDIUM)
        printf("openGL warning:\n%s\n", message);
    else
        printf("openGL message:\n%s\n", message);
}

//! cree et configure un contexte opengl
SDL_GLContext create_context( SDL_Window *window, const int major= 3, const int minor= 3 )
{
    if(window == NULL) return NULL;
    
    // configure la creation du contexte opengl core profile, debug profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    SDL_GLContext context= SDL_GL_CreateContext(window);
    if(context == NULL)
    {
        printf("error creating openGL context.\n");
        return NULL;
    }
    
    SDL_GL_SetSwapInterval(1);
    
    // initialise les extensions opengl
    glewExperimental= 1;
    GLenum err= glewInit();
    if(err != GLEW_OK)
    {
        printf("error loading extensions: %s\n", glewGetErrorString(err));
        SDL_GL_DeleteContext(context);
        return NULL;
    }
    
    // purge les erreurs opengl generees par glew !
    while(glGetError() != GL_NO_ERROR) {;}
    
    // configure l'affichage des messages d'erreurs opengl
    if(GLEW_ARB_debug_output)
    {
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glDebugMessageCallbackARB(debug, NULL);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    }
    
    return context;
}


//! application

GLuint program;
GLint mvp_location;
GLint color_location;

GLuint vao;
GLuint vertex_buffer;
GLuint index_buffer;


//! compile les shaders et construit le programme + les buffers + le vertex array.
//! renvoie -1 en cas d'erreur.
int init( )
{
    // charger et creer le shader program
    program= make_program("mini_vertex.glsl", "mini_fragment.glsl");
    if(program == 0) return -1;
    
    // recupere les identifiants des uniforms
    mvp_location= glGetUniformLocation(program, "mvpMatrix");
    color_location= glGetUniformLocation(program, "color");
    
    // creer les buffers : decrire un cube indexe.
    float positions[][3] = { 
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},  // face arriere z= 0
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}  // face avant z= 1
    }; 
    
    unsigned int indices[] = {
        4, 5, 6,     // face avant, manque le 2ieme triangle
        1, 2, 6,  
        0, 1, 5,
        0, 3, 2,     // face arriere 
        0, 4, 7, 
        2, 3, 7,
    };
    
    // gl core profile : configurer un vertex array
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // creer le vertex buffer
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    
    // configurer le vertex array : associer le contenu du vertex buffer avec un attribut utilise par le vertex shader
    // l'indice 0 est impose dans le source du shader, cf layout(location= 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    // creer l'index buffer
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    // l'index buffer est automatiquement associe au vertex array lors de sa creation (au moment du BindBuffer(GL_ELEMENT_ARRAY_BUFFER))
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // nettoyage
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return 0;
}

int quit( )
{
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &index_buffer);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    return 0;
}

int draw( )
{
    // effacer l'image
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    /* configuration minimale du pipeline :
        glUseProgram()  // indique quel shader program utiliser pour dessiner 
        glBindVertexArray() // association du contenu des buffers aux attributs declares dans le vertex shader
        { ... } // parametrer les uniforms des shaders

        glDraw()    // execution du pipeline
     */
    glUseProgram(program);
    glBindVertexArray(vao);

    // initialiser les transformations
    float mvp[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    glUniformMatrix4fv(mvp_location, 1, GL_TRUE, mvp);
    
    // donner une couleur a l'objet
    float color[] = {1, 1, 0};
    glUniform3fv(color_location, 1, color);
    
    // draw
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    
    return 1;
}


int main( void )
{
    SDL_Window *window= create_window();
    if(window == NULL) return 1;
    SDL_GLContext context= create_context(window);        // cree un contexte opengl 3.3, par defaut
    if(context == NULL) return 1;
    
    // creation des objets opengl
    if(init() < 0)
    {
        printf("init failed.\n");
        return 1;
    }
    
    // affichage de l'application
    run(window);

    // nettoyage
    quit();
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    
    return 0;
}
