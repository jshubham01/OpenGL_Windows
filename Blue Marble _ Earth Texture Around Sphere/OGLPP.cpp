/*
Module:
	Orange Book
	Blue Marble
	Simple Texture On Sphere

Abstract:
	Plain Sphere with Lights
*/

/////////////////////////////////////////////////////////////////////
//	H E A D E R S.
/////////////////////////////////////////////////////////////////////
#include<Windows.h>
#include<stdio.h>
#include<math.h>
#include<GL/glew.h>
#include<GL/gl.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "vmath.h"
#include "res.h"

/////////////////////////////////////////////////////////////////////
//	M A C R O S   &   P R A G M A S 
/////////////////////////////////////////////////////////////////////
#pragma comment(lib, "Opengl32.lib")
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define LINE_POINTS 1000

using namespace vmath;

/////////////////////////////////////////////////////
// Global Variables declarations and initializations
/////////////////////////////////////////////////////
HDC g_hdc = NULL;
HWND g_hwnd = NULL;
HGLRC g_hrc = NULL;		//HGLRC is openGL rendering context
bool g_boActiveWindow = false;
bool g_bFullScreen = false;
DWORD g_dwStyle;

GLuint g_uiVertextShaderObject;
GLuint g_uiFragmentShaderObject;
GLuint g_uiShaderProgramObject;

GLuint vao_sphere;
GLuint vbo_sphere_normals;
GLuint vbo_sphere_texture;
GLuint vbo_sphere_position;
GLuint vbo_sphere_elements;

GLuint mvpUniform;
GLuint mvUniform;
GLuint lightPositionUniform;
GLuint textureLocationSamplerUniform;

mat4 perspectiveProjectionMatrix;

GLuint g_texture;
int* iIndices = NULL;
float* ivertices = NULL;
float* iNormals = NULL;
float* itexCords = NULL;
int iNoOfVerticesSphere;
int iIndicesLengthSphere;

int gNumElements;
float* fSpherePositions;
float* fSphereNormals;
float* fSphereTexturesCoords;
int* indices;

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD
};

FILE* g_fLogger;

WINDOWPLACEMENT g_wpPrev = {
	sizeof(WINDOWPLACEMENT)
};

////////////////////////////////////////////////////////////////////
// G l o b a l  F u n c t i o n s
////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void update();

/////////////////////////////////////////////////////////////////////
//	F U N C T I O N  D E F I N I T I O N S.
/////////////////////////////////////////////////////////////////////
int
WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszcmdLine,
	int iCmdShow
)
{
	//
	//Declarations
	//
	MSG msg;
	HWND hwnd;
	int iRet = 0;
	TCHAR szAppName[] = TEXT("sphere");

	WNDCLASSEX wndclass;

	//
	//function declarations
	//
	int Initialize(void);
	void Display(void); // first change for Double Buffer

	//code 
	if (fopen_s(&g_fLogger, "LOG.txt", "w"))
	{
		MessageBox(NULL, TEXT("Log File can not be created"), TEXT("Error"), MB_OK);
		exit(0);
	}

	fprintf(g_fLogger, "Log File SuccessFully Created \r\n");
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;  // Adding owndc means telling OS that dont discard(fixed | discardable | movable)
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,		//top of taskbar even in multi-monitor scenario
		szAppName,
		TEXT("sphere"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	g_hwnd = hwnd;

	iRet = Initialize();
	if (-1 == iRet)
	{
		//Choose PixelFormat 
		fprintf(g_fLogger, "ChoosePixelFormat Failed \r\n");
	}
	else if (-2 == iRet)
	{
		fprintf(g_fLogger, "SetPixelFormat Failed \r\n");
	}
	else if (-3 == iRet)
	{
		fprintf(g_fLogger, "Create Context Failed \r\n"); //createcontxt makecurrent
	}
	else if (-4 == iRet)
	{
		fprintf(g_fLogger, "MakeContext Failed \r\n");
	}
	else
	{
		fprintf(g_fLogger, "Initialization Succedded \r\n");
	}

	ShowWindow(hwnd, iCmdShow);
	//
	// UpdateWindow(hwnd); No call in OpenGL
	//
	SetForegroundWindow(hwnd);					// Keep My Window At The First Position in  Z - ORDER
	SetFocus(hwnd);

	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == msg.message)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (true == g_boActiveWindow)
			{
				// Call UPDATE Function or Display Func
				update();
			}

			Display(); // 2nd change to make Double Buffer
		}

	}

	return ((int)msg.wParam);
}

LRESULT CALLBACK
WndProc(
	HWND hwnd,
	UINT iMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	//function declarations;				
	void Resize(int, int);
	void uninitialization(void);
	void ToggleFullScreen(void);

	switch (iMsg)
	{
	case WM_SETFOCUS:
		g_boActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		g_boActiveWindow = false;
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 'f':
		case 'F':
			ToggleFullScreen();
			break;
		}

		break;
	case WM_SIZE:
		Resize(LOWORD(lParam), HIWORD(lParam));
		break;

	//
	// New Case need to be used to make Double Buffer
	//
	case WM_ERASEBKGND:
		return 0; // it tells that we have Display call

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		uninitialization();
		PostQuitMessage(0);
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

int
Initialize(
	void
)
{
	GLenum result;
	BOOL boLoadedTest = FALSE;

	//
	// Functions Declarations
	//
	void Resize(int, int);
	void uninitialization();
	void mySphereWithRadius(float radius, int slices, int stacks);

	BOOL loadTexture(GLuint *texture, TCHAR imgResource[]);

	//
	// Variable Declarations
	//
	int iPixelFormatIndex;

	PIXELFORMATDESCRIPTOR pfd;

	// Code
	memset((void *)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));
	//ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER; // new flag added for Double Buffer
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	//
	// Change Added for 3D Enabling
	//
	pfd.cDepthBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	g_hdc = GetDC(g_hwnd);

	iPixelFormatIndex = ChoosePixelFormat(g_hdc, &pfd);

	if (!iPixelFormatIndex)
	{
		return -1;
	}

	if (FALSE == SetPixelFormat(g_hdc, iPixelFormatIndex, &pfd))
	{
		return -2;
	}

	g_hrc = wglCreateContext(g_hdc);  //it gives rendering context from device context
	if (!g_hrc)
	{
		return -3;
	}

	if (FALSE == wglMakeCurrent(g_hdc, g_hrc))
	{
		return -4;
	}

	result = glewInit();
	if (GLEW_OK != result)
	{
		fprintf(g_fLogger, "glewInit() failed\n");
		uninitialization();
		DestroyWindow(g_hwnd);
	}

	// define vertex shader object
	g_uiVertextShaderObject = glCreateShader(GL_VERTEX_SHADER);

	const GLchar *pcVertexShaderSourceCode =
			"#version 440 core" \
			"\n" \
			"in vec4 v_position;" \
			"in vec3 v_normal;" \
			"in vec2 v_in_texcoord;" \

			"out vec2 v_texcoord;" \
			"out float light_intensity;" \

			"uniform mat4 u_mvp_matrix;" \
			"uniform mat4 u_mv_matrix;" \
			"uniform vec3 u_light_position;" \

			"const float specular_contribution = 0.1;" \
			"const float diffused_contribution = 1.0 - specular_contribution;" \

			"void main(void)" \
			"{" \
				"float spec;" \
				"vec3 trans_norm;" \
				"vec4 ec_position;" \
				"vec3 view_vector;" \
				"mat3 normal_matrix;" \
				"vec3 reflect_vector;" \
				"vec3 light_direction;" \

				"ec_position = u_mv_matrix * v_position;" \
				"normal_matrix = mat3(u_mv_matrix);" \
				"trans_norm = normal_matrix * v_normal;" \
				"light_direction = u_light_position - ec_position.xyz;" \
				"reflect_vector = reflect(-light_direction, trans_norm);" \
				"view_vector = normalize(-ec_position.xyz);" \
				
				"spec = clamp(dot(reflect_vector, view_vector), 0.0, 1.0);" \
				"spec = pow(spec, 16.0);" \

				"light_intensity = diffused_contribution" \
								"* max(dot(light_direction, trans_norm), 0.0)" \
								"+ specular_contribution * spec;" \

				"vec3 r = reflect(normalize(ec_position.xyz), trans_norm);" \
				"r.z += 1.0;" \

				"float m = 0.5 * inversesqrt(dot(r, r));" \

				"v_texcoord = r.xy * m + 0.5;" \
			
				"gl_Position = u_mvp_matrix * v_position;" \

			"}";

	glShaderSource(g_uiVertextShaderObject, 1,
		(const GLchar**)(&pcVertexShaderSourceCode), NULL);

	// compile the vertext shader
	glCompileShader(g_uiVertextShaderObject);

	// catching shader related errors if there are any
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;
	GLint iShaderCompileStatus = 0;

	// getting compile status code
	glGetShaderiv(g_uiVertextShaderObject,
		GL_COMPILE_STATUS, &iShaderCompileStatus);

	if(GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiVertextShaderObject,
			GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if(NULL != szInfoLog)
			{
				GLsizei written;

				glGetShaderInfoLog(
					g_uiVertextShaderObject,
					iInfoLogLength,
					&written,
					szInfoLog
				);

				fprintf(g_fLogger, "VERTEX SHADER FATAL ERROR: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialization();
				DestroyWindow(g_hwnd);
			}
		}
	}

	// working for fragment shader
	g_uiFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar *pcFragmentShaderSourceCode = 
		"#version 440 core" \
		"\n" \
		"in float light_intensity;" \
		"in vec2 v_texcoord;" \

		"out vec4 vFragColor;" \

		"uniform sampler2D earth_texture;" \

		"void main(void)" \
		"{" \
			"vec3 lightColor = vec3(texture(earth_texture, v_texcoord));" \
			"vFragColor = vec4(lightColor, 1.0);" \
		"}";

	// specify above code of shader to vertext shader object
	glShaderSource(g_uiFragmentShaderObject,
		1,
		(const GLchar**)&pcFragmentShaderSourceCode,
		NULL);

	// compile the vertext shader
	glCompileShader(g_uiFragmentShaderObject);

	// catching shader related errors if there are any
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	// getting compile status code
	glGetShaderiv(g_uiFragmentShaderObject,
		GL_COMPILE_STATUS,
		&iShaderCompileStatus);

	if (GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiFragmentShaderObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
			{
				GLsizei written;

				glGetShaderInfoLog(g_uiFragmentShaderObject,
					iInfoLogLength,
					&written,
					szInfoLog);

				fprintf(g_fLogger, ("FRAGMENT SHADER FATAL ERROR: %s\n"), szInfoLog);
				free(szInfoLog);
				uninitialization();
				DestroyWindow(g_hwnd);
			}
		}
	}

	// create shader program objects
	g_uiShaderProgramObject = glCreateProgram();

	// attach fragment shader to shader program
	glAttachShader(g_uiShaderProgramObject, g_uiVertextShaderObject);
	glAttachShader(g_uiShaderProgramObject, g_uiFragmentShaderObject);

	// here we binded gpu`s variable to cpu`s index
	glBindAttribLocation(g_uiShaderProgramObject,
		AMC_ATTRIBUTE_POSITION, "v_position");

	glBindAttribLocation(g_uiShaderProgramObject,
		AMC_ATTRIBUTE_TEXCOORD, "v_in_texcoord");

	glBindAttribLocation(g_uiShaderProgramObject,
		AMC_ATTRIBUTE_NORMAL, "v_normal");

	// link the shader
	glLinkProgram(g_uiShaderProgramObject);

	// uniforms binding should happen after linking
	mvpUniform = glGetUniformLocation(g_uiShaderProgramObject,  "u_mvp_matrix");
	mvUniform = glGetUniformLocation(g_uiShaderProgramObject,  "u_mv_matrix");
	lightPositionUniform = glGetUniformLocation(g_uiShaderProgramObject,  "u_light_position");
	textureLocationSamplerUniform = glGetUniformLocation(g_uiShaderProgramObject,  "earth_texture");

	// SPHERE
	int slices = 50;
	int stacks = 50;
	mySphereWithRadius(0.6, slices, stacks);

	int vertexCount = (slices + 1) * (stacks + 1);
	fprintf(g_fLogger, "vertexCount %d \n", vertexCount);
	//for (int i = 0; i < vertexCount * 2; i++)
	//{
	//	fprintf(g_fLogger, "%f \n", fSphereTexturesCoords[i]);
	//}


	glGenVertexArrays(1, &vao_sphere);
	glBindVertexArray(vao_sphere);

	glGenBuffers(1, &vbo_sphere_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
	glBufferData(GL_ARRAY_BUFFER,
		3 * vertexCount * sizeof(float),
		fSpherePositions,
		GL_STATIC_DRAW);

	glVertexAttribPointer(
		AMC_ATTRIBUTE_POSITION, 3,
		GL_FLOAT, GL_FALSE,
		0,	 NULL
	);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	

	// normals
	glGenBuffers(1, &vbo_sphere_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normals);
	glBufferData(GL_ARRAY_BUFFER, 3 * vertexCount * sizeof(float),
		fSphereNormals,  GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3,		
		GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// textures vbo_sphere_texture

	glGenBuffers(1, &vbo_sphere_texture);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_texture);
	glBufferData(GL_ARRAY_BUFFER, 2 * vertexCount * sizeof(float),
		fSphereTexturesCoords, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2,
		GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// element vbo
	glGenBuffers(1, &vbo_sphere_elements);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_elements);
	glBufferData(GL_ARRAY_BUFFER,
		gNumElements * sizeof(int),
		indices,
		GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	// ERROR HANDLING
	GLint iProgramCompileLinkStatus;
	iInfoLogLength = 0;
	szInfoLog = NULL;
	glGetProgramiv(g_uiShaderProgramObject,
		GL_LINK_STATUS,
		&iProgramCompileLinkStatus);

	if (GL_FALSE == iProgramCompileLinkStatus)
	{
			glGetProgramiv(g_uiShaderProgramObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
			{
				GLsizei written;
				glGetProgramInfoLog( g_uiShaderProgramObject,
					iInfoLogLength, &written, szInfoLog);
				fprintf(g_fLogger, 
					"FATAL ERROR OF Program of shader Linking: %s", szInfoLog);
				free(szInfoLog);
				uninitialization();
				DestroyWindow(g_hwnd);
			}
		}
	}

	boLoadedTest = loadTexture(&g_texture, MAKEINTRESOURCE(IDBITMAP));
	if (TRUE == boLoadedTest)
	{
		fprintf(g_fLogger, "Texture Is Loaded Successfully\n");
	}
	else if (FALSE == boLoadedTest)
	{
		fprintf(g_fLogger, "Texture Is Not Loaded\n");
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	// #change for perspective	
	perspectiveProjectionMatrix = mat4::identity();

	// Warm Up Call to resize
	Resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;
}

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

	extern GLuint vbo_sphere_elements;
	extern int gNumVertices;
	extern int gNumElements;

	// CUBE
	modelViewMatrix = mat4::identity();
	modelRotationMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix = translate(0.0f, 0.0f, -5.0f);
	//modelRotationMatrix = rotate(90.0f, 1.0f, 0.0f, 0.0f);
	//modelViewMatrix = modelViewMatrix * rotate(-90.0f, 0.0f, 1.0f, 0.0f);
	//modelViewMatrix = modelViewMatrix * modelRotationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	glUniformMatrix4fv(mvUniform, 1, GL_FALSE, modelViewMatrix);
	glUniform3f(lightPositionUniform, 0.0f, 0.0f, 4.0f);
	glUniform1i(textureLocationSamplerUniform, 0); // first texture slot

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture);

	glBindVertexArray(vao_sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
	SwapBuffers(g_hdc); // for Double Buffer
}

void
Resize(int iwidth, int iheight)
{
	if (0 == iheight)
	{
		iheight = 1;
	}

	glViewport(0, 0, (GLsizei)iwidth, (GLsizei)iheight);
	perspectiveProjectionMatrix = perspective(45.0f, (GLfloat)iwidth / (GLfloat)iheight, 0.1f, 100.0f);
}

void
mySphereWithRadius(float radius, int slices, int stacks)
{
	int vertexCount = (slices + 1) * (stacks + 1);
	gNumElements = 2 * slices * stacks * 3;

	fSpherePositions = (float*)malloc(3 * vertexCount * sizeof(float));
	fSphereNormals = (float*)malloc(3 * vertexCount * sizeof(float));
	fSphereTexturesCoords = (float*)malloc(2 * vertexCount * sizeof(float));
	indices = (int*)malloc(gNumElements * sizeof(int));

	float du = 2 * M_PI / slices;
	float dv = M_PI / stacks;

	unsigned int indexV = 0;
	unsigned int indexT = 0;

	float u, v, x, y, z;
	int i, j, k;
	for (i = 0; i <= stacks; i++)
	{
		v = - M_PI / 2 + i * dv;
		for (j = 0; j <= slices; j++)
		{
			u = j * du;
			x = cos(u) * cos(v);
			y = sin(u) * cos(v);
			z = sin(v);
			fSpherePositions[indexV] = radius * x;
			fSphereNormals[indexV++] = x;
			fSpherePositions[indexV] = radius * y;
			fSphereNormals[indexV++] = y;
			fSpherePositions[indexV] = radius * z;
			fSphereNormals[indexV++] = z;
			fSphereTexturesCoords[indexT++] = j / slices;
			fSphereTexturesCoords[indexT++] = i / stacks;
		}
	}

	fprintf(g_fLogger, "indexV: %u\n", indexV);
	fprintf(g_fLogger, "indexV: %u\n", indexT);

	k = 0;
	for (j = 0; j < stacks; j++)
	{
		int row1 = j * (slices + 1);
		int row2 = (j + 1) * (slices + 1);
		for (i = 0; i < slices; i++)
		{
			indices[k++] = row1 + i;
			indices[k++] = row2 + i + 1;
			indices[k++] = row2 + i;
			indices[k++] = row1 + i;
			indices[k++] = row1 + i + 1;
			indices[k++] = row2 + i + 1;
		}
	}
}

void
ToggleFullScreen(void)
{
	MONITORINFO mi;

	//code
	if (false == g_bFullScreen)
	{
		g_dwStyle = GetWindowLong(g_hwnd, GWL_STYLE);

		if (WS_OVERLAPPEDWINDOW == (WS_OVERLAPPEDWINDOW & g_dwStyle))
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(
				g_hwnd, &g_wpPrev)
				&&
				GetMonitorInfo(MonitorFromWindow(g_hwnd, MONITORINFOF_PRIMARY), &mi)
				)
			{
				SetWindowLong(
					g_hwnd,
					GWL_STYLE,
					g_dwStyle & (~WS_OVERLAPPEDWINDOW)
				);
				SetWindowPos(
					g_hwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					(mi.rcMonitor.right - mi.rcMonitor.left),
					(mi.rcMonitor.bottom - mi.rcMonitor.top),
					SWP_NOZORDER |
					SWP_FRAMECHANGED);
			}
		}

		ShowCursor(false);
		g_bFullScreen = true;
	}
	else
	{
		SetWindowLong(
			g_hwnd,
			GWL_STYLE,
			g_dwStyle | WS_OVERLAPPEDWINDOW
		);

		SetWindowPlacement(g_hwnd, &g_wpPrev);

		SetWindowPos(
			g_hwnd,
			HWND_TOP, 0,
			0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED |
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(true);
		g_bFullScreen = FALSE;
	}
}

void
uninitialization(
	void
)
{
	void ToggleFullScreen();

	if (g_bFullScreen)
	{
		ToggleFullScreen();
	}

	if (vbo_sphere_position)
	{
		glDeleteBuffers(1, &vbo_sphere_position);
		vbo_sphere_position = 0;
	}

	if (vbo_sphere_normals)
	{
		glDeleteBuffers(1, &vbo_sphere_normals);
		vbo_sphere_normals = 0;
	}

	if (vbo_sphere_elements)
	{
		glDeleteBuffers(1, &vbo_sphere_elements);
		vbo_sphere_elements = 0;
	}

	if (vao_sphere)
	{
		glDeleteVertexArrays(1, &vao_sphere);
		vao_sphere = 0;
	}

	if (g_uiShaderProgramObject)
	{
		GLsizei shaderCount;
		GLsizei shaderNo;
		GLuint* pShaders;

		glUseProgram(g_uiShaderProgramObject);
		glGetProgramiv(g_uiShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);
		pShaders = (GLuint*)malloc(sizeof(GLuint) * shaderCount);
		if (pShaders)
		{
			glGetAttachedShaders(
				g_uiShaderProgramObject, shaderCount,
				&shaderCount, pShaders);

			for (shaderNo = 0; shaderNo < shaderCount; shaderNo++)
			{
				glDetachShader(g_uiShaderProgramObject, pShaders[shaderNo]);
				glDeleteShader(pShaders[shaderNo]);
				pShaders[shaderNo] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(g_uiFragmentShaderObject);
		g_uiFragmentShaderObject = 0;
		glUseProgram(0);
	}

	if (wglGetCurrentContext() == g_hrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (g_hrc)
	{
		wglDeleteContext(g_hrc);
		g_hrc = NULL;
	}

	if (g_hdc)
	{
		ReleaseDC(g_hwnd, g_hdc);
		g_hdc = NULL;
	}

	fprintf(g_fLogger, "Log File Is Closed Successfully \r\n");
	_fcloseall();
}

void update()
{}

BOOL
loadTexture(GLuint* texture, TCHAR imgResource[])
{
	// initializations
	BITMAP bmp;
	BOOL bStatus = FALSE;
	HBITMAP hBitMap = NULL;

	hBitMap = (HBITMAP)LoadImage(
			GetModuleHandle(NULL), imgResource, IMAGE_BITMAP,
			0, 0, LR_CREATEDIBSECTION
		);
	if (hBitMap)
	{
		bStatus = TRUE;
		GetObject(hBitMap, sizeof(BITMAP), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			bmp.bmWidth, bmp.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		DeleteObject(hBitMap);
	}
	else
	{
		fprintf(g_fLogger, "Possible Fatal Error Occured No. %d\n", GetLastError());
	}

	return bStatus;
}

