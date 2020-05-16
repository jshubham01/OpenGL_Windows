/*
Module:
	Lights From Orange Book
	9th Chapther

Programmer:
	Shubham G. Jangam

Abstract:
	Use Of Lights - Directional Lights, Point Light and Spot Light
*/

#include "main.h"

extern GLuint vao_pyramid; // vertex array object
extern GLuint vao_cube;
extern HDC g_hdc;
extern FILE* g_fLogger;
extern GLuint mvpUniform;
extern GLuint g_uiShaderProgramObject;
extern mat4 perspectiveProjectionMatrix;

GLfloat fanglePyramid = 0.0f;
GLfloat fangleCube = 0.0f;

extern GLuint	mv_matrix;
extern BOOL		bo_lightKeyIsEnabled;
extern GLuint	lighting_key_pressed;
extern GLuint lightTypeToggle;
extern int LightPressed;

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

	// RECTANGLE
	modelViewMatrix = mat4::identity();
	modelRotationMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix      = translate(0.0f, 0.0f, -4.0f);
	modelRotationMatrix  = rotate(fangleCube, 0.0f, 1.0f, 0.0f);
	modelRotationMatrix *= rotate(fangleCube, 1.0f, 0.0f, 0.0f);
	modelRotationMatrix *= rotate(fangleCube, 0.0f, 0.0f, 1.0f);

	modelViewMatrix = modelViewMatrix * modelRotationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	glUniformMatrix4fv(mv_matrix, 1, GL_FALSE, modelViewMatrix);

	if (TRUE == bo_lightKeyIsEnabled)
	{
		glUniform1i(lighting_key_pressed, 1);
		glUniform1i(lightTypeToggle, LightPressed);
	}
	else
	{
		glUniform1i(lighting_key_pressed, 0);
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

void
update()
{
	fanglePyramid += 0.01f;
	if (fanglePyramid > 360.0f)
	{
		fanglePyramid = 0.0f;
	}

	fangleCube += 0.05f;
	if (fangleCube > 360.0f)
	{
		fangleCube = 0.0f;
	}
}
