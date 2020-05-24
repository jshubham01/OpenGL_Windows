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

GLuint textureDayUniform;
GLuint textureNightUniform;
GLuint textureCloudsUniform;

mat4 perspectiveProjectionMatrix;

GLuint g_textureDay;
GLuint g_textureNight;
GLuint g_textureClouds;

int* iIndices = NULL;
float* ivertices = NULL;
float* iNormals = NULL;
float* itexCords = NULL;
int iNoOfVerticesSphere;
int iIndicesLengthSphere;

int*   indices;
int    gNumElements;
float* fSphereNormals;
float* fSpherePositions;
float* fSphereTexturesCoords;



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
Initialize(void)
{
	GLenum result;
	BOOL boLoadedTest = FALSE;

	// Functions Declarations
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

	// Change Added for 3D Enabling

	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

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

			"out float diffuse;" \
			"out vec3 specular;" \
			"out vec2 v_texcoord;" \

			"uniform mat4 u_mvp_matrix;" \
			"uniform mat4 u_mv_matrix;" \
			"uniform vec3 u_light_position;" \

			"void main(void)" \
			"{" \
				"vec4 ec_position;" \
				"vec3 trans_norm;" \
				"mat3 normal_matrix;" \
				"vec3 light_direction;" \
				"vec3 reflect_vector;" \
				"vec3 view_vector;" \

				"float spec;" \

				"ec_position = u_mv_matrix * v_position;" \
				"normal_matrix = mat3(u_mv_matrix);" \
				"trans_norm = normal_matrix * v_normal;" \
				"light_direction = normalize(u_light_position - ec_position.xyz);" \
				"reflect_vector = reflect(-light_direction, trans_norm);" \
				"view_vector = normalize(-ec_position.xyz);" \
				
				"spec = clamp(dot(reflect_vector, view_vector), 0.0, 1.0);" \
				"spec = pow(spec, 16.0);" \

				"specular = spec * vec3(1.0, 0.941, 0.898) * 2;" \

				"diffuse  = max(dot(light_direction, trans_norm), 0.0);" \
				
				// calculations for texcoods
				//"vec3 r = reflect(normalize(ec_position.xyz), trans_norm);" \
				//"r.z += 1.0;" \
				//"float m = 0.5 * inversesqrt(dot(r, r));" \
				//"v_texcoord = r.xy * m + 0.5;" 
				"v_texcoord = v_in_texcoord;" \

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

		"uniform sampler2D day_texture;" \
		"uniform sampler2D night_texture;" \
		"uniform sampler2D clouds_gloss_texture;" \

		"in float diffuse;" \
		"in vec3 specular;" \
		"in vec2 v_texcoord;" \

		"out vec4 vFragColor;" \

		"void main(void)" \
		"{" \
			"vec2 clouds;" \
			"vec3 daytime;" \
			"vec3 nighttime;" \
			"vec3 color;" \

			"clouds = texture(clouds_gloss_texture, v_texcoord).rg ;" \
			
			"daytime = (texture(day_texture, v_texcoord).rgb * diffuse + " \
							"specular * clouds.g) * (1.0 - clouds.r) + clouds.r * diffuse;" \

			"nighttime = texture(night_texture, v_texcoord).rgb * (1.0 - clouds.r) * 2.0;" \
			"color = daytime;" \

			"if(diffuse < 0.2)" \
			"{" \
				"color = mix(nighttime, daytime, (diffuse + 0.1) * 5.0);" \
				 // "color = texture(day_texture, v_texcoord).rgb;"
			"}" \
			
			"vFragColor = vec4(color, 1.0);" \
		"}";

	//"vFragColor = texture(night_texture, v_texcoord);" 
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
	
	// clouds_gloss_texture;" 
	textureNightUniform  =  glGetUniformLocation(g_uiShaderProgramObject, "night_texture");
	textureDayUniform    =  glGetUniformLocation(g_uiShaderProgramObject, "day_texture");
	textureCloudsUniform =  glGetUniformLocation(g_uiShaderProgramObject, "clouds_gloss_texture");

	// SPHERE
	int slices = 50;
	int stacks = 50;
	mySphereWithRadius(1.0, slices, stacks);

	int vertexCount = (slices + 1) * (stacks + 1);
	fprintf(g_fLogger, "vertexCount %d \n", vertexCount);

	glGenVertexArrays(1, &vao_sphere);
	glBindVertexArray(vao_sphere);

	glGenBuffers(1, &vbo_sphere_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
	glBufferData(GL_ARRAY_BUFFER,
		3 * vertexCount * sizeof(float),
		fSpherePositions, GL_STATIC_DRAW);

	glVertexAttribPointer(
		AMC_ATTRIBUTE_POSITION, 3,
		GL_FLOAT, GL_FALSE,
		0, NULL);

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

	boLoadedTest = loadTexture(&g_textureNight, MAKEINTRESOURCE(IDBITMAP));
	if (TRUE == boLoadedTest)
	{
		fprintf(g_fLogger, "Night Texture Is Loaded Successfully\n");
	}
	else if (FALSE == boLoadedTest)
	{
		fprintf(g_fLogger, "Night Texture Is Not Loaded\n");
	}

	boLoadedTest = loadTexture(&g_textureDay, MAKEINTRESOURCE(IDBITMAP1));
	if (TRUE == boLoadedTest)
	{
		fprintf(g_fLogger, "Day Texture Is Loaded Successfully\n");
	}
	else if (FALSE == boLoadedTest)
	{
		fprintf(g_fLogger, "Day Texture Is Not Loaded\n");
	}

	boLoadedTest = loadTexture(&g_textureClouds, MAKEINTRESOURCE(IDBITMAP2));
	if (TRUE == boLoadedTest)
	{
		fprintf(g_fLogger, "Cloud Texture Is Loaded Successfully\n");
	}
	else if (FALSE == boLoadedTest)
	{
		fprintf(g_fLogger, "Cloud Texture Is Not Loaded\n");
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
	static float fangleY = 0.0f;

	// CUBE
	modelViewMatrix = mat4::identity();
	modelRotationMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix		= translate(0.0f, 0.0f, -3.0f);
	//modelViewMatrix = modelViewMatrix * rotate(-90.0f, 0.0f, 1.0f, 0.0f);
	modelRotationMatrix = rotate(-18.5f, 0.0f, 0.0f, 1.0f);
	modelRotationMatrix = modelRotationMatrix * rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	modelRotationMatrix = modelRotationMatrix * rotate(-fangleY, 0.0f, 0.0f, 1.0f);
	modelViewMatrix = modelViewMatrix * modelRotationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniform3f(lightPositionUniform, 100.0f, 0.0f, 30.0f);
	glUniformMatrix4fv(mvUniform, 1, GL_FALSE, modelViewMatrix);
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glUniform1i(textureNightUniform,  0);  // first texture slot
	glUniform1i(textureDayUniform,    1);  // first texture slot
	glUniform1i(textureCloudsUniform, 2);  // first texture slot

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_textureNight);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_textureDay);	
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_textureClouds);

	glBindVertexArray(vao_sphere);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	glUseProgram(0);
	SwapBuffers(g_hdc); // for Double Buffer
	
	fangleY = fangleY + 0.05f;
	if (fangleY >= 360.0f)
	{
		fangleY = 0.0f;
	}
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
			fSphereTexturesCoords[indexT++] = (float) j / slices;
			fSphereTexturesCoords[indexT++] = (float) i  / stacks;
		}
	}

	// fprintf(g_fLogger, "indexV: %u\n", indexV);
	// fprintf(g_fLogger, "indexV: %u\n", indexT);

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

