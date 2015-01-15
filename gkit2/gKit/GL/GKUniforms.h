
#include "GLUniforms.h"
#include "Geometry.h"

namespace gk {
    
//! point
void ProgramUniform( const GLuint program, const GLint location, const Point& v )
{
    glProgramUniform3fv(program, location, 1, &v.x);
}

//! point array
void ProgramUniform( const GLuint program, const GLint location, const std::vector<Point>& v )
{
    glProgramUniform3fv(program, location, v.size(), &v[0].x);
}

//! vector
void ProgramUniform( const GLuint program, const GLint location, const Vector& v )
{
    glProgramUniform3fv(program, location, 1, &v.x);
}

//! vector array
void ProgramUniform( const GLuint program, const GLint location, const std::vector<Vector>& v )
{
    glProgramUniform3fv(program, location, v.size(), &v[0].x);
}

//! normal
void ProgramUniform( const GLuint program, const GLint location, const Normal& v )
{
    glProgramUniform3fv(program, location, 1, &v.x);
}

//! normal array
void ProgramUniform( const GLuint program, const GLint location, const std::vector<Normal>& v )
{
    glProgramUniform3fv(program, location, v.size(), &v[0].x);
}

//! color
void ProgramUniform( const GLuint program, const GLint location, const Color& v )
{
    glProgramUniform4fv(program, location, 1, &v.x);
}

//! color array
void ProgramUniform( const GLuint program, const GLint location, const std::vector<Color>& v )
{
    glProgramUniform4fv(program, location, v.size(), &v[0].x);
}

//! transform
void ProgramUniformMatrix( const GLuint program, const GLint location, const Transform& m )
{
    glProgramUniformMatrix4fv(program, location, 1, GL_TRUE, &m.m[0][0]);
}


//! vec3, uniform name
void ProgramUniform( const GLuint program, const GLchar *uniform, const Point& v )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniform(program, location, v);
}

//! vec3 array
void ProgramUniform( const GLuint program, const GLchar *uniform, const std::vector<Point>& v )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniform(program, location, v);
}

//! vec3
void ProgramUniform( const GLuint program, const GLchar *uniform, const Vector& v )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniform(program, location, v);
}

//! vec3 array
void ProgramUniform( const GLuint program, const GLchar *uniform, const std::vector<Vector>& v )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniform(program, location, v);
}

//! vec3
void ProgramUniform( const GLuint program, const GLchar *uniform, const Normal& v )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniform(program, location, v);
}

//! vec3 array
void ProgramUniform( const GLuint program, const GLchar *uniform, const std::vector<Normal>& v )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniform(program, location, v);
}

//! vec4
void ProgramUniform( const GLuint program, const GLchar *uniform, const Color& v )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniform(program, location, v);
}

//! vec4 array
void ProgramUniform( const GLuint program, const GLchar *uniform, const std::vector<Color>& v )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniform(program, location, v);
}

//! mat4
void ProgramUniformMatrix( const GLuint program, const GLchar *uniform, const Transform& m )
{
    GLint location= glGetUniformLocation(program, uniform);    
    ProgramUniformMatrix(program, location, m);
}

}       // namespace
