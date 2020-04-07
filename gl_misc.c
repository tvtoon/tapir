// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "gl_misc.h"

GLuint compileShaders(const char *vsh_source, const char *fsh_source)
{
 GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER), shader = 0, vsh = glCreateShader(GL_VERTEX_SHADER);
 char zalog[512] = "\0";
 int testistatus = 0;

 glShaderSource(vsh, 1, &vsh_source, NULL);

 glCompileShader(vsh);
{
  glGetShaderiv(vsh, GL_COMPILE_STATUS, &testistatus);

  if ( testistatus == 0)
{
   glGetShaderInfoLog(vsh, sizeof(zalog), NULL, zalog);
   fprintf(stderr, "vertex shader compile error:\n%s\n", zalog);
   return(0);
}

}

 glShaderSource(fsh, 1, &fsh_source, NULL);

 glCompileShader(fsh);
{
  glGetShaderiv(fsh, GL_COMPILE_STATUS, &testistatus );

  if ( testistatus == 0 )
{
   glGetShaderInfoLog(fsh, sizeof(zalog), NULL, zalog);
   fprintf(stderr, "fragment shader compile error:\n%s\n", zalog);
   return(0);
}

}

 shader = glCreateProgram();
// 0 => glDeleteProgram();
 glAttachShader(shader, vsh);
 glAttachShader(shader, fsh);
 glLinkProgram(shader);
{
  glGetProgramiv(shader, GL_LINK_STATUS, &testistatus );

  if ( testistatus == 0 )
{
   glGetProgramInfoLog(shader, sizeof(zalog), NULL, zalog);
   glDeleteProgram(shader);
   fprintf(stderr, "shader link error:\n%s\n", zalog);
   return(0);
}

}

 glDeleteShader(vsh);
 glDeleteShader(fsh);
 return shader;
}

void gl_draw_rect( double x0, double y0, double x1, double y1, double tx0, double ty0, double tx1, double ty1)
{
 if(x1 < x0 || y1 < y0) return;

 glBegin(GL_TRIANGLE_STRIP);
 glTexCoord2f(tx0, ty0);
 glVertex3f(x0, y0, 0.0f);
 glTexCoord2f(tx1, ty0);
 glVertex3f(x1, y0, 0.0f);
 glTexCoord2f(tx0, ty1);
 glVertex3f(x0, y1, 0.0f);
 glTexCoord2f(tx1, ty1);
 glVertex3f(x1, y1, 0.0f);
 glEnd();
}
