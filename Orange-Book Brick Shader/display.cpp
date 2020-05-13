#include "main.h"

extern GLuint vao;
extern HDC g_hdc;
extern FILE* g_fLogger;
extern GLuint mvpUniform;
extern GLuint g_uiShaderProgramObject;
extern mat4 perspectiveProjectionMatrix;

extern GLuint brickColorUniform;
extern GLuint morterColorUniform;
extern GLuint brickSizeUniform;
extern GLuint brickPctUniform;

extern GLuint mvUniform;
extern GLuint lightPosition;
extern GLuint uiKeyOfLightsIsPressed;

extern BOOL boKeyOfLightsIsPressed;

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
	modelViewMatrix = translate(0.0f, 0.0f, -3.0f);
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glUniform3f(brickColorUniform, 1.0, 0.3, 0.2);
	glUniform3f(morterColorUniform, 0.85, 0.86, 0.84);
	glUniform2f(brickSizeUniform, 0.30, 0.15);
	glUniform2f(brickPctUniform, 0.90, 0.85);

	if (TRUE == boKeyOfLightsIsPressed)
	{
		glUniform1i(uiKeyOfLightsIsPressed, 1);
		glUniformMatrix4fv(mvUniform, 1, GL_FALSE, modelViewMatrix);
		glUniform4f(lightPosition, 0.0f, 0.0f, 4.0f, 1.0f);
	}
	else
	{
		glUniform1i(uiKeyOfLightsIsPressed, 0);
	}

	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLE_FAN, 	0,	4);

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


