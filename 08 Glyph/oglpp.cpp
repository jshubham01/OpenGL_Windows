/*
	Cube
	Glyph Texture Bombing
	Orange Book

	Code By: Shubham G Jangam

*/

#pragma once
/////////////////////////////////////////////////////////////////////
//	H E A D E R S.
/////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>
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

#define CHECK_IMAGE_HEIGHT 1024
#define CHECK_IMAGE_WEIGHT 1024

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

GLuint vao_cube;
GLuint vbo_position_cube;
GLuint vbo_texture_cube;
GLuint vbo_normals_cube;

GLuint mvUniform;
GLuint mvpUniform;
GLuint lightPositionUniform;
mat4 perspectiveProjectionMatrix;

GLuint glyphSamplerUniform;
GLuint randSamplerUniform;
GLuint g_uiGlyph;
GLuint g_randImage;

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

GLfloat g_RandImageData[CHECK_IMAGE_WEIGHT][CHECK_IMAGE_HEIGHT];

/////////////////////////////////////////////////////////////////////
// G l o b a l  F u n c t i o n  D e c la r a t i o n s
/////////////////////////////////////////////////////////////////////
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
	TCHAR szAppName[] = TEXT("OGLPP");

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
		TEXT("OGLPP"),
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
	HWND hwnd, 	UINT iMsg,
	WPARAM wParam, 	LPARAM lParam
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

	// Functions Declarations
	void Resize(int, int);
	void uninitialization();

	BOOL boLoadTexture(GLuint * texture, TCHAR imgResource[]);
	void boLoadProceduralTexture();

	// Variable Declarations
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
			"in vec2 v_tex_coords;" \
			"in vec3 v_normals;" \

			"uniform mat4 u_mvp_matrix;" \
			"uniform mat4 u_mv_matrix;" \
			"uniform vec3 u_light_position;" \

			"float specular_contribution = 0.2;" \
			"float scale_factor = 10.0f;" \
			
			"out vec2 v_out_tex_coords;" \
			"out float light_intensity;" \
			"out vec3 v_out_normals;" \

			"void main(void)" \
			"{" \
				"vec3 ec_position;" \
				"mat3 normal_matrix;" \
				"vec3 trans_norm;" \
				"vec3 light_direction;" \
				"vec3 reflection_vector;" \
				"vec3 viewer_vector;" \
				"float diffuse;" \
				"float diffuse_contribution;" \
				"float spec = 0.0;" \

				"ec_position = vec3(u_mv_matrix * v_position);" \
				"normal_matrix = mat3(u_mv_matrix);" \
				"trans_norm = normalize(normal_matrix * v_normals);" \
				"light_direction = normalize(u_light_position - ec_position);" \
				"reflection_vector = reflect(-light_direction, trans_norm);" \
				"viewer_vector = normalize(-ec_position);" \
				"diffuse = max(dot(reflection_vector, trans_norm), 0.0);" \
				"if(diffuse > 0.0)" \
				"{" \
					"spec = max(dot(reflection_vector, viewer_vector), 0.0);" \
					"spec = pow(spec, 16.0);" \
				"}" \

				"diffuse_contribution = 1.0 - specular_contribution;" \
				"light_intensity =  diffuse_contribution * diffuse * 2.0 + specular_contribution * spec;" \

				"v_out_tex_coords = v_tex_coords.st * scale_factor;" \
				"gl_Position = u_mvp_matrix * v_position;" \
			"}";

	glShaderSource(g_uiVertextShaderObject, 1, (const GLchar**)(&pcVertexShaderSourceCode), NULL);

	// compile the vertext shader
	glCompileShader(g_uiVertextShaderObject);

	// catching shader related errors if there are any
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	// getting compile status code
	glGetShaderiv(g_uiVertextShaderObject,
		GL_COMPILE_STATUS,
		&iShaderCompileStatus);

	if(GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiVertextShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if(NULL != szInfoLog)
			{
				GLsizei written;

				glGetShaderInfoLog(g_uiVertextShaderObject,
					iInfoLogLength,  &written,
					szInfoLog);

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

		"float TWO_PI  = 6.28318;" \

		"in float light_intensity;" \
		"in vec2 v_out_tex_coords;" \
		
		"uniform sampler2D u_sampler_glyph;" \
		"uniform sampler2D u_sampler_rand;"	\

		"vec4 model_color = vec4(1.0, 1.0, 1.0, 1.0);" \
		"float col_adjust = 0.75;" \
		"float scale_factor = 10.0;" \
		"float percentage = 1.0;" \
		"float samples_per_cell = 1.0;" \
		"float RO1 = 0.29;" \

		"bool random_scale = true;" \
		"bool random_rotate = false;" \

		"out vec4 vFragColor;" \
		"void main(void)" \
		"{" \
			"vec4 color = model_color;" \
			"vec2 cell = floor(v_out_tex_coords);" \
			"vec2 offset = fract(v_out_tex_coords);" \

			"vec2 dPdx = dFdx(v_out_tex_coords) / scale_factor;" \
			"vec2 dPdy = dFdy(v_out_tex_coords) / scale_factor;" \

			"vec2 randomUV;" \
			"vec2 current_cell;" \
			"vec2 current_offset;" \
			"vec4 random;" \

			"for(int i= -1; i <= int(random_rotate); i++)" \
			"{" \
				"for(int j= -1; j <= int(random_rotate); j++)" \
				"{" \
					"current_cell = cell + vec2(float(i), float(j));" \
					"current_offset = offset - vec2(float(i), float(j));" \
					"randomUV = current_cell * vec2(RO1);" \

					"for(int k = 0; k < int(samples_per_cell); k++)" \
					"{" \
						"random = textureGrad(u_sampler_rand, randomUV, dPdx, dPdy);" \
						"randomUV += random.ba;" \

						"if(random.r < percentage)" \
						"{"	\
							"vec2 glyph_index;" \
							"mat2 rotater;" \
							"vec2 index;" \
							"float rotation_angle, cos_rot, sin_rot;" \

							"index.s = floor(random.b * 10);" \
							"index.t = floor(col_adjust * 10);" \

							"if(random_rotate)" \
							"{" \
								"rotation_angle = TWO_PI * random.g;" \
								"cos_rot = cos(rotation_angle);" \
								"sin_rot = sin(rotation_angle);" \
								"rotater[0] = vec2(cos_rot, sin_rot);" \
								"rotater[1] = vec2(-sin_rot, cos_rot);" \
								"glyph_index = -rotater * (current_offset - random.rg);" \
							"}" \
							"else" \
							"{" \
								"glyph_index = current_offset -random.rg;" \
							"}" \

							"if(random_scale)" \
							"{" \
								"glyph_index /= vec2(0.5 * random.r + 0.5);" \
							"}" \

							"glyph_index = (clamp(glyph_index, 0.0, 1.0) + index) * 0.1;" \

							"vec4 image = textureGrad(u_sampler_glyph, glyph_index, dPdx, dPdy);" \

							"if(image.r != 1.0)" \
							"{" \
								"color.rgb = mix(random.rgb * 0.7, color.rgb, image.r);" \
							"}" \
						"}" \
					"}" \
				"}" \
			"}" \

			"vFragColor = color * light_intensity;"  \
		"}";

	// specify above code of shader to vertext shader object
	glShaderSource(g_uiFragmentShaderObject, 1, (const GLchar**)&pcFragmentShaderSourceCode, NULL);

	glCompileShader(g_uiFragmentShaderObject);

	szInfoLog = NULL;
	iInfoLogLength = 0;
	iShaderCompileStatus = 0;

	// getting compile status code
	glGetShaderiv(g_uiFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

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
	glBindAttribLocation(g_uiShaderProgramObject, AMC_ATTRIBUTE_POSITION, "v_position");
	glBindAttribLocation(g_uiShaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "v_tex_coords");
	glBindAttribLocation(g_uiShaderProgramObject, AMC_ATTRIBUTE_NORMAL, "v_normals");

	// link the shader
	glLinkProgram(g_uiShaderProgramObject);

	mvUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_mv_matrix");
	mvpUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_mvp_matrix");
	glyphSamplerUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_sampler_glyph");
	randSamplerUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_sampler_rand");
	lightPositionUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_light_position");

	// RECTANGLE
	const GLfloat frectangleVertices[] = {
			1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,

			1.0f, -1.0f, -1.0f ,
			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,

			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,

			1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, -1.0f
	};

	const GLfloat fCubeNormals[] =
	{
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f
	};

	GLfloat ftextCooards_cube[] = 
	{
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,

		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	glGenVertexArrays(1, &vao_cube);
	glBindVertexArray(vao_cube);

	// position of rectangles
	glGenBuffers(1, &vbo_position_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(frectangleVertices), frectangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Textures
	glGenBuffers(1, &vbo_texture_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ftextCooards_cube), ftextCooards_cube, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Normals
	glGenBuffers(1, &vbo_normals_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fCubeNormals), fCubeNormals, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	// ERROR HANDLING
	// steps for catching program related error
	szInfoLog = NULL;
	iInfoLogLength = 0;
	GLint iProgramCompileLinkStatus;
	glGetProgramiv(g_uiShaderProgramObject, GL_LINK_STATUS, &iProgramCompileLinkStatus);

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
				glGetProgramInfoLog(
					g_uiShaderProgramObject, iInfoLogLength,
					&written, szInfoLog);
				fprintf(g_fLogger, 
					"FATAL ERROR OF Program of shader Linking: %s", szInfoLog);
				free(szInfoLog);
				uninitialization();
				DestroyWindow(g_hwnd);
			}
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	BOOL boLoadedTest = FALSE;
	boLoadedTest = boLoadTexture(&g_uiGlyph, MAKEINTRESOURCE(ID_BITMAP_GLYPH));
	if (TRUE == boLoadedTest)
	{
		fprintf(g_fLogger, "Glyph Texture Is Loaded Successfully\n");
	}
	else
	{
		fprintf(g_fLogger, "Glyph Texture Load Failed\n");
	}

	boLoadProceduralTexture();

	perspectiveProjectionMatrix = mat4::identity();

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
	static GLfloat fangleCube = 0.0f;

	mat4 modelViewMatrix;
	mat4 modelRotationMatrix;
	mat4 modelViewProjectionMatrix;

	// CUBE
	modelViewMatrix = mat4::identity();
	modelRotationMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix = translate(0.0f, 0.0f, -5.5f);
	modelRotationMatrix = rotate(fangleCube, 0.0f, 1.0f, 0.0f);
	modelRotationMatrix = modelRotationMatrix * rotate(fangleCube, 1.0f, 0.0f, 0.0f);
	modelRotationMatrix = modelRotationMatrix * rotate(fangleCube, 0.0f, 0.0f, 1.0f);
	modelViewMatrix = modelViewMatrix * modelRotationMatrix;

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	glUniformMatrix4fv(mvUniform, 1, GL_FALSE, modelViewMatrix);
	glUniform3f(lightPositionUniform, 10.0f, 10.0f, 10.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_uiGlyph);
	glUniform1i(glyphSamplerUniform, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_randImage);
	glUniform1i(randSamplerUniform, 1);

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
	fangleCube += 0.05f;
	if (fangleCube > 360.0f)
	{
		fangleCube = 0.0f;
	}
}

void update(){}

void
Resize(int iwidth,int iheight)
{
	if (0 == iheight)
	{
		iheight = 1;
	}

	glViewport(0, 0, (GLsizei)iwidth, (GLsizei)iheight);
	perspectiveProjectionMatrix = perspective(45.0f, (GLfloat)iwidth / (GLfloat)iheight, 0.1f, 100.0f);
}

BOOL
boLoadTexture(GLuint* texture, TCHAR imgResource[])
{
	// Initializations
	BITMAP bmp;
	BOOL boStatus = FALSE;
	HBITMAP hBitmap = NULL;

	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		imgResource,
		IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBitmap)
	{
		boStatus = TRUE;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0,
						GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		DeleteObject(hBitmap);
	}
	else
	{
		fprintf(g_fLogger, "Possible Fatal Error Occured No. %d\n", GetLastError());
	}

	return boStatus;
}

void
boLoadProceduralTexture()
{
	void MakeRandImage(void);

	// code
	MakeRandImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &g_randImage);
	glBindTexture(GL_TEXTURE_2D, g_randImage);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGBA, CHECK_IMAGE_WEIGHT,
		CHECK_IMAGE_HEIGHT, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, g_RandImageData);
	
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void
uninitialization(void)
{
	void ToggleFullScreen();

	if (g_bFullScreen)
	{
		ToggleFullScreen();
	}

	if (vbo_position_cube)
	{
		glDeleteBuffers(1, &vbo_position_cube);
		vbo_position_cube = 0;
	}

	if (vao_cube)
	{
		glDeleteVertexArrays(1, &vao_cube);
		vao_cube = 0;
	}

	if (g_uiShaderProgramObject)
	{
		GLsizei shaderCount;
		GLsizei shaderNo;
		GLuint *pShaders;

		glUseProgram(g_uiShaderProgramObject);
		glGetProgramiv(g_uiShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);
		pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);
		if (pShaders)
		{
			glGetAttachedShaders(
				g_uiShaderProgramObject, shaderCount,
				&shaderCount, pShaders
			);

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

void
ToggleFullScreen(void)
{
	// Declarations 
	MONITORINFO mi;

	//code
	if (false == g_bFullScreen)
	{
		g_dwStyle = GetWindowLong(g_hwnd, GWL_STYLE);

		if (WS_OVERLAPPEDWINDOW == (WS_OVERLAPPEDWINDOW & g_dwStyle))
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(
				g_hwnd,
				&g_wpPrev)
				&&
				GetMonitorInfo(
					MonitorFromWindow(g_hwnd, MONITORINFOF_PRIMARY), &mi)
				)
			{
				SetWindowLong(g_hwnd, GWL_STYLE, g_dwStyle & (~WS_OVERLAPPEDWINDOW));
				SetWindowPos(
					g_hwnd, HWND_TOP,
					mi.rcMonitor.left,  mi.rcMonitor.top,
					(mi.rcMonitor.right - mi.rcMonitor.left),
						(mi.rcMonitor.bottom - mi.rcMonitor.top),
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}

		ShowCursor(false);
		g_bFullScreen = true;
	}
	else
	{
		SetWindowLong(g_hwnd, GWL_STYLE, g_dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(g_hwnd, &g_wpPrev);
		SetWindowPos(
			g_hwnd, HWND_TOP,
			0, 0,
			0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED |
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER );

		ShowCursor(true);
		g_bFullScreen = FALSE;
	}
}

void
MakeRandImage(void)
{
	// initialization

	int i;
	int j;
	float c;

	float fScale = 1.0f / (float)RAND_MAX;;
	for (i = 0; i < CHECK_IMAGE_HEIGHT; i++)
	{
		for (j = 0; j < CHECK_IMAGE_WEIGHT; j++)
		{
			c = fScale * rand();
			g_RandImageData[i][j] = c;
		}
	}
}
