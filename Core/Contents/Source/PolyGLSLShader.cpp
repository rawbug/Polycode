/*
Copyright (C) 2011 by Ivan Safrin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#include "PolyGLSLShader.h"

#include "PolyLogger.h"
#include "PolyShader.h"
#include "PolyGLSLProgram.h"
#include "PolyTexture.h"
#include "PolyCubemap.h"

#ifdef _WINDOWS
#include <windows.h>
#endif

#include "PolyGLHeaders.h"

using std::vector;

#ifdef _WINDOWS
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
#ifndef _MINGW
extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation;
#endif
#endif

using namespace Polycode;

GLSLShaderBinding::GLSLShaderBinding(GLSLShader *shader) : ShaderBinding(shader) {
	glslShader = shader;
}

GLSLShaderBinding::~GLSLShaderBinding() {
	
}

Texture *GLSLShaderBinding::getTexture(const String& name) {
	for(int i=0; i < textures.size(); i++) {
		if(textures[i].name == name) {			
			return textures[i].texture;
		}
	}
	return NULL;
}
			
void GLSLShaderBinding::addTexture(const String& name, Texture *texture) {
	GLSLTextureBinding binding;
	binding.name = name;
	binding.texture = texture;
//	binding.vpParam = GLSLGetNamedParameter(glslShader->fp->program, name.c_str());
	textures.push_back(binding);
}

void GLSLShaderBinding::addCubemap(const String& name, Cubemap *cubemap) {
	GLSLCubemapBinding binding;
	binding.cubemap = cubemap;
	binding.name = name;
//	binding.vpParam = GLSLGetNamedParameter(GLSLShader->fp->program, name.c_str());
	cubemaps.push_back(binding);
}

void GLSLShaderBinding::clearTexture(const String& name) {
	for(int i=0; i < textures.size(); i++) {
		if(textures[i].name == name) {
			textures.erase(textures.begin()+i);
			return;
		}
	}
}


void GLSLShaderBinding::addParam(const String& type, const String& name, const String& value) {
	int paramType;
	void *defaultData;
	void *minData;
	void *maxData;		
	ProgramParam::createParamData(&paramType, type, value, "", "", &defaultData,&minData, &maxData);
	LocalShaderParam *newParam = new LocalShaderParam;
	newParam->data = defaultData;
	newParam->name = name;
	localParams.push_back(newParam);
}

void GLSLShader::linkProgram() {
	shader_id = glCreateProgram();
    glAttachShader(shader_id, ((GLSLProgram*)fp)->program);
    glAttachShader(shader_id, ((GLSLProgram*)vp)->program);	
	glBindAttribLocation(shader_id, 6, "vTangent");	
    glLinkProgram(shader_id);
	if(vp) {
		vp->addEventListener(this, Event::RESOURCE_RELOAD_EVENT);
	}
	if(fp) {
		fp->addEventListener(this, Event::RESOURCE_RELOAD_EVENT);
	}	
}

void GLSLShader::unlinkProgram() {
	if(vp) {
		vp->removeAllHandlersForListener(this);
	}
	if(fp) {
		fp->removeAllHandlersForListener(this);
	}
	glDetachShader(shader_id, ((GLSLProgram*)fp)->program);
    glDetachShader(shader_id, ((GLSLProgram*)vp)->program);
	glDeleteProgram(shader_id);	
}

void GLSLShader::handleEvent(Event *event) {
	if(event->getEventCode() == Event::RESOURCE_RELOAD_EVENT && (event->getDispatcher() == vp || event->getDispatcher() == fp)) {
		unlinkProgram();
		linkProgram();
	}
}

void GLSLShader::setVertexProgram(ShaderProgram *vp) {
	unlinkProgram();
	this->vp = vp;
	linkProgram();
}

void GLSLShader::setFragmentProgram(ShaderProgram *fp) {
	unlinkProgram();
	this->fp = fp;
	linkProgram();
}


GLSLShader::GLSLShader(GLSLProgram *vp, GLSLProgram *fp) : Shader(Shader::MODULE_SHADER) {
	this->vp = vp;
	this->fp = fp;
	
	linkProgram();
}

void GLSLShader::reload() {
	glDeleteProgram(shader_id);
	linkProgram();
}

GLSLShader::~GLSLShader() {
	unlinkProgram();
}

ShaderBinding *GLSLShader::createBinding() {
	return new GLSLShaderBinding(this);
}
