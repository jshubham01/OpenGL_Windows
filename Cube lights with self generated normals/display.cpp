#include "main.h"

extern HDC g_hdc;
extern GLuint vao_cube;
extern FILE* g_fLogger;
extern GLuint g_uiShaderProgramObject;
extern mat4 perspectiveProjectionMatrix;

extern GLuint ldUniform;
extern GLuint kdUniform;
extern GLuint uiModelViewUniform;
extern GLuint uiProjectionUniform;
extern BOOL boKeyOfLightsIsPressed;
extern GLuint uiKeyOfLightsIsPressed;
extern GLuint lightPositionVectorUniform;

float fAngleRotate = 0.0f;

void
Display(void)
{

	// Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(g_uiShaderProgramObject);

	// Declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelRotationMatrix;
	mat4 modelViewProjectionMatrix;

	// CUBE
	modelViewMatrix = mat4::identity();
	modelRotationMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix = translate(0.0f, 0.0f, -5.5f);
	modelRotationMatrix = rotate(fAngleRotate, 0.0f, 1.0f, 0.0f);
	modelViewMatrix = modelViewMatrix * modelRotationMatrix;
	// modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(uiModelViewUniform,
		1,
		GL_FALSE,
		modelViewMatrix);

	glUniformMatrix4fv(uiProjectionUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);

	if(TRUE == boKeyOfLightsIsPressed)
	{
		glUniform1i(uiKeyOfLightsIsPressed, 1);
		glUniform3f(ldUniform, 1.0, 1.0, 1.0); // white light
		glUniform3f(kdUniform, 0.5, 0.5, 0.5); // grey material
		glUniform4f(lightPositionVectorUniform,
			0.0f, 0.0f, 2.0f, 1.0f);
	}
	else
	{
		glUniform1i(uiKeyOfLightsIsPressed, 0);
	}

	glBindVertexArray(vao_cube);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);

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

void update()
{
	static BOOL face = TRUE;

	fAngleRotate += 0.05f;
	if (fAngleRotate > 360.0f)
	{
	
		fAngleRotate = 0.0f;
		/*if (TRUE == face)
		{	
			glFrontFace(GL_CW);
			face = FALSE;
		}
		else
		{
			glFrontFace(GL_CCW);
			face = TRUE;
		}*/
	}

	
	
}
