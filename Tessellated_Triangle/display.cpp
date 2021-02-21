#include "main.h"

extern GLuint vao;
extern HDC g_hdc;
extern FILE* g_fLogger;
extern GLuint mvpUniform;
extern GLuint g_uiShaderProgramObject;
extern mat4 perspectiveProjectionMatrix;


extern GLuint g_uiNumberOfSegmentUniform;
extern GLuint g_uiNumberOfStripUniform;
extern GLuint g_uiLineColorUniform;
extern unsigned int g_uiNumberOfLineSegments;

extern HWND g_hwnd;

void
Display(void)
{
	// Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(g_uiShaderProgramObject);

	// Declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

	// initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	// do necessary transformation
	// ...

	modelViewMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	// uniforms are given to m_uv_matrix (i.e. model view matrix)
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// glUniform1i(g_uiNumberOfSegmentUniform, g_uiNumberOfLineSegments);

	// TCHAR str[255];
	// wsprintf(str, TEXT("OpenGL Programming Pipeline Window : [Segments = %d]"), g_uiNumberOfLineSegments);
	// SetWindowText(g_hwnd, str);

	// glUniform1i(g_uiNumberOfStripUniform, 1);
	// glUniform4fv(g_uiLineColorUniform, 1, vmath::vec4(1.0f, 1.0f, 0.0f, 1.0f));

	//glBindVertexArray(vao);

	//glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawArrays(GL_PATCHES, 0, 3);
	//glBindVertexArray(0);
	glUseProgram(0);
	SwapBuffers(g_hdc); // for Double Buffer
}

void
Resize(
	int iwidth,
	int iheight
)
{
	if (0 == iheight)
	{
		iheight = 1;
	}

	glViewport(0, 0, (GLsizei)iwidth, (GLsizei)iheight);
	perspectiveProjectionMatrix = perspective(45.0f, (GLfloat)iwidth/(GLfloat)iheight, 0.1f, 100.0f);
}
