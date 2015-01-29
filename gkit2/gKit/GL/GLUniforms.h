
#include <vector>

#include "GL/GLPlatform.h"
#include "Vec.h"

namespace gk {

void ProgramUniform( const GLuint program, const GLint location, const unsigned int x )
{
    glProgramUniform1ui(program, location, x);
}

void ProgramUniform( const GLuint program, const GLint location, const int x )
{
    glProgramUniform1i(program, location, x);
}

//! scalar
void ProgramUniform( const GLuint program, const GLint location, const float x )
{
    glProgramUniform1f(program, location, x);
}

//! scalar array
void ProgramUniform( const GLuint program, const GLint location, const std::vector<float>& v )
{
    glProgramUniform1fv(program, location, v.size(), &v[0]);
}

//! vec2
void ProgramUniform( const GLuint program, const GLint location, const Vec2& v  )
{
    glProgramUniform2fv(program, location, 1, &v.x);
}

//! vec2 array
void ProgramUniform( const GLuint program, const GLint location, const std::vector<Vec2>& v )
{
    glProgramUniform2fv(program, location, v.size(), &v[0].x);
}

//! vec3
void ProgramUniform( const GLuint program, const GLint location, const Vec3& v )
{
    glProgramUniform3fv(program, location, 1, &v.x);
}

//! vec3 array
void ProgramUniform( const GLuint program, const GLint location, const std::vector<Vec3>& v )
{
    glProgramUniform3fv(program, location, v.size(), &v[0].x);
}

//! vec4
void ProgramUniform( const GLuint program, const GLint location, const Vec4& v )
{
    glProgramUniform4fv(program, location, 1, &v.x);
}

//! vec4 array
void ProgramUniform( const GLuint program, const GLint location, const std::vector<Vec4>& v )
{
    glProgramUniform4fv(program, location, v.size(), &v[0].x);
}

//! mat4
void ProgramUniformMatrix( const GLuint program, const GLint location, const Mat4& m )
{
    glProgramUniformMatrix4fv(program, location, 1, GL_TRUE, &m.m[0][0]);
}

//! mat4 array
void ProgramUniformMatrix( const GLuint program, const GLint location, const vector<Mat4>& m )
{
    glProgramUniformMatrix4fv(program, location, m.size(), GL_TRUE, &m[0].m[0][0]);
}

//! scalar, uniform name
void ProgramUniform( const GLuint program, const GLchar *uniform, const unsigned int x )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, x);
}

void ProgramUniform( const GLuint program, const GLchar *uniform, const int x )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, x);
}

//! scalar
void ProgramUniform( const GLuint program, const GLchar *uniform, const float x )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform1f(program, location, x);
}

//! scalar array
void ProgramUniform( const GLuint program, const GLchar *uniform, const std::vector<float>& v )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, v);
}

//! vec2
void ProgramUniform( const GLuint program, const GLchar *uniform, const Vec2& v  )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, v);
}

//! vec2 array
void ProgramUniform( const GLuint program, const GLchar *uniform, const std::vector<Vec2>& v )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, v);
}

//! vec3
void ProgramUniform( const GLuint program, const GLchar *uniform, const Vec3& v )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, v);
}

//! vec3 array
void ProgramUniform( const GLuint program, const GLchar *uniform, const std::vector<Vec3>& v )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, v);
}

//! vec4
void ProgramUniform( const GLuint program, const GLchar *uniform, const Vec4& v )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, v);
}

//! vec4 array
void ProgramUniform( const GLuint program, const GLchar *uniform, const std::vector<Vec4>& v )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniform(program, location, v);
}

//! mat4
void ProgramUniformMatrix( const GLuint program, const GLchar *uniform, const Mat4& m )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniformMatrix(program, location, v);
}

//! mat4 array
void ProgramUniformMatrix( const GLuint program, const GLchar *uniform, const vector<Mat4>& m )
{
    GLint location= glGetUniformLocation(program, uniform);
    ProgramUniformMatrix(program, location, m);
}

}       // namespace
