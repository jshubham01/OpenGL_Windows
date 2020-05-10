#pragma once
/////////////////////////////////////////////////////////////////////
//	H E A D E R S.
/////////////////////////////////////////////////////////////////////
#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<GL/glew.h>
#include<GL/gl.h>
#include<stdlib.h>
//#define _USE_MATH_DEFINES
//#include <cmath>
#include "vmath.h"
#include "res.h" 

/////////////////////////////////////////////////////////////////////
//	M A C R O S   &   P R A G M A S 
/////////////////////////////////////////////////////////////////////
#pragma comment(lib, "Opengl32.lib")
//#pragma comment(lib, "glew32.lib")
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define LINE_POINTS 1000

using namespace vmath;

/////////////////////////////////////////////////////////////////////
// Global Variables declarations and initializations
/////////////////////////////////////////////////////////////////////
HDC g_hdc = NULL;
HWND g_hwnd = NULL;
HGLRC g_hrc = NULL;		//HGLRC is openGL rendering context
bool g_boActiveWindow = false;
bool g_bFullScreen = false;
DWORD g_dwStyle;

GLuint g_uiVertextShaderObject;
GLuint g_uiFragmentShaderObject;
GLuint g_uiShaderProgramObject;

GLuint vao;
GLuint vbo_vertex;
GLuint vbo_normals;
GLuint vbo_textcoords;

GLuint samplerUniform;
mat4 perspectiveProjectionMatrix;

BOOL boKeyOfLightsIsPressed = FALSE;

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

/////////////////////////////////////////////////////////////////////
//global function declarations
/////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// DS Related Changes
typedef struct _vertex
{
	GLfloat xUp;
	GLfloat yUp;
	GLfloat xLeft;
	GLfloat yLeft;
	GLfloat xRight;
	GLfloat yRight;
	GLfloat z;

	GLfloat xUp_back;
	GLfloat yUp_back;
	GLfloat xLeft_back;
	GLfloat yLeft_back;
	GLfloat xRight_back;
	GLfloat yRight_back;
	GLfloat z_back;

} Vertex;

typedef struct _tree
{
	void* data;
	struct _tree* left;
	struct _tree* right;
	int level;

} Treenode;

typedef struct
_queue {
	int front;
	int rear;
	Treenode* array[50];

} Queue;

Treenode* insertNode(Treenode** node);
int height(Treenode* root);
Treenode* front(Queue* queue);
void DeleteDeepest(Treenode** root);
void DeleteLastLevel(Treenode** node, int level);

Treenode* root = NULL;
Treenode* lastNode = NULL;

GLfloat pfVertexArray[4000];
GLfloat pfNormalsArray[4000];
GLfloat* pfTexCoordsmal = NULL;

int size_vertex;
size_t size_texture;

Queue* createQueue();
void enQueue(Queue* queue, Treenode* value);
int isEmpty(Queue* queue);
void deQueue(Queue* queue);

bool g_EnableRotation = false;
GLfloat fAngle = 10.0f; // to rotate the structure about Y axis

GLuint g_textLeeAsher;
GLuint g_textBackside;

GLuint uiModelMatrixUniform;
GLuint uiViewMatrixUniform;
GLuint uiProjectionUniform;

GLuint laUniform_z;
GLuint ldUniform_z;
GLuint lsUniform_z;

GLuint lightPositionVectorUniform_x;
GLuint lightPositionVectorUniform_y;
GLuint lightPositionVectorUniform_z;

GLuint laUniform_x;
GLuint ldUniform_x;
GLuint lsUniform_x;

GLuint laUniform_y;
GLuint ldUniform_y;
GLuint lsUniform_y;

GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;

GLuint uiKeyOfLightsIsPressed;
GLuint shineynessUniform;

int flag_for_invert_normal = 0;

float light_ambient_red[4] = { 0.1f, 0.0f, 0.0f, 1.0f };
float light_diffused_red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float light_specular_red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

float light_ambient_blue[4] = { 0.0f, 0.0f, 0.1f, 1.0f };
float light_diffused_blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
float light_specular_blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

float light_ambient_green[4] = { 0.0f, 0.1f, 0.0f, 1.0f };
float light_diffused_green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
float light_specular_green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

float material_ambient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
float material_diffused[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float material_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float material_shineyness = 128.0f;

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
	void update();
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
		szAppName, TEXT("OpenGL PP"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		100, 100, WIN_WIDTH, WIN_HEIGHT,
		NULL, NULL,
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

		case 'I':
		case 'i':
			fprintf(g_fLogger, "Entered in I key \n");
			lastNode = insertNode(&root);
			break;

		case 'D':
		case 'd':
			DeleteDeepest(&root);
			break;

		case 'y':
		case 'Y':
			if (!g_EnableRotation)
			{
				g_EnableRotation = true;
			}
			else
			{
				g_EnableRotation = false;
			}

			break;

		case 'l':
		case 'L':
			if (TRUE == boKeyOfLightsIsPressed)
			{
				boKeyOfLightsIsPressed = FALSE;
			}
			else
			{
				boKeyOfLightsIsPressed = TRUE;
			}

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
	int iPixelFormatIndex;
	PIXELFORMATDESCRIPTOR pfd;

	// Functions Declarations
	void Resize(int, int);
	void uninitialization();
	BOOL loadTexture(GLuint * texture, TCHAR imgResource[]);

	// Code
	memset((void*)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER; // new flag added for Double Buffer
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

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

	g_hrc = wglCreateContext(g_hdc);
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

	const GLchar* pcVertexShaderSourceCode =
		"#version 440 core" \
		"\n" \

		"in vec4 vPosition;" \
		"in vec2 vtextCoord;" \
		"in vec3 v_normals;" \

		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int ui_is_lighting_key_pressed;" \

		"uniform vec4 u_light_position_z;" \
		"uniform vec4 u_light_position_x;" \
		"uniform vec4 u_light_position_y;" \

		"out vec3 light_direction_z;" \
		"out vec3 light_direction_x;" \
		"out vec3 light_direction_y;" \

		"out vec3 t_norm;"
		"out vec3 viewer_vector;" \
		"out vec2 voutTextCoord;" \

		"void main(void)" \
		"{" \

		"vec4 eye_coordinates;" \

		"if(ui_is_lighting_key_pressed >= 1)" \
		"{" \
		"eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"t_norm = mat3(u_view_matrix * u_model_matrix) * v_normals;" \
		"viewer_vector = vec3(-eye_coordinates);" \

		"light_direction_z = vec3(u_light_position_z - eye_coordinates);" \
		"light_direction_x = vec3(u_light_position_x - eye_coordinates);" \
		"light_direction_y = vec3(u_light_position_y - eye_coordinates);" \
		"}" \

		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"voutTextCoord = vtextCoord;" \
		"}";

	glShaderSource(g_uiVertextShaderObject, 1, (const GLchar**)(&pcVertexShaderSourceCode), NULL);

	glCompileShader(g_uiVertextShaderObject);

	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar* szInfoLog = NULL;

	glGetShaderiv(g_uiVertextShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiVertextShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
			{
				GLsizei written;

				glGetShaderInfoLog(g_uiVertextShaderObject, iInfoLogLength, &written, szInfoLog);

				fprintf(g_fLogger, "VERTEX SHADER FATAL ERROR: %s\n", szInfoLog);
				free(szInfoLog);
				uninitialization();
				DestroyWindow(g_hwnd);
			}
		}
	}

	// working for fragment shader
	g_uiFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* pcFragmentShaderSourceCode =
		"#version 440 core" \
		"\n" \
		"in vec2 voutTextCoord;" \

		"in vec3 light_direction_x;" \
		"in vec3 light_direction_y;" \
		"in vec3 light_direction_z;" \

		"in vec3 t_norm;" \
		"in vec3 viewer_vector;" \

		"uniform sampler2D u_sampler;" \

		"uniform vec3 u_la_z;" \
		"uniform vec3 u_ld_z;" \
		"uniform vec3 u_ls_z;" \

		"uniform vec3 u_la_x;" \
		"uniform vec3 u_ld_x;" \
		"uniform vec3 u_ls_x;" \

		"uniform vec3 u_la_y;" \
		"uniform vec3 u_ld_y;" \
		"uniform vec3 u_ls_y;" \

		"uniform vec3 u_ka;" \
		"uniform vec3 u_kd;" \
		"uniform vec3 u_ks;" \

		"uniform float u_material_shiney_ness;" \
		"uniform int ui_is_lighting_key_pressed;" \

		"out vec4 vfrag_color;" \
		"void main(void)" \
		"{" \

		"vec4 vFragColor;" \

		"vec3 ambient;" \
		"vec3 diffused;" \
		"vec3 specular;" \
		"vec3 l_t_norm;" \
		"float tn_dot_ld;" \
		"vec3 l_viewer_vector;" \
		"vec3 l_light_direction;" \
		"vec3 reflection_vector;" \
		"vec3 phong_ads_light;" \

		"if(ui_is_lighting_key_pressed == 1)" \
		"{" \
				"l_t_norm = normalize(t_norm);" \
				"l_viewer_vector = normalize(viewer_vector);" \

				"l_light_direction	= normalize(light_direction_z);" \
				"tn_dot_ld = max(dot(l_light_direction, l_t_norm), 0.0);"
				"reflection_vector = reflect(-l_light_direction, l_t_norm);" \

				"ambient = u_la_z * u_ka;" \
				"diffused = u_ld_z * u_kd * tn_dot_ld;" \
				"specular = u_ls_z * u_ks * " \
				"pow(max(dot(reflection_vector," \
				"l_viewer_vector), 0.0), u_material_shiney_ness);" \

				"phong_ads_light = ambient + diffused + specular;" \

				"l_light_direction	= normalize(light_direction_x);" \
				"tn_dot_ld = max(dot(l_light_direction, l_t_norm), 0.0); " \
				"reflection_vector = reflect(-l_light_direction, l_t_norm);" \

				"ambient = u_la_x * u_ka;" \
				"diffused = u_ld_x * u_kd * tn_dot_ld;" \
				"specular = u_ls_x * u_ks * " \
				"pow(max(dot(reflection_vector," \
				"l_viewer_vector), 0.0), u_material_shiney_ness);" \

				"phong_ads_light = phong_ads_light + ambient + diffused + specular;" \

				"l_light_direction	= normalize(light_direction_y);" \
				"tn_dot_ld = max(dot(l_light_direction, l_t_norm), 0.0); " \
				"reflection_vector = reflect(-l_light_direction, l_t_norm);" \

				"ambient = u_la_y * u_ka;" \
				"diffused = u_ld_y * u_kd * tn_dot_ld;" \
				"specular = u_ls_y * u_ks * " \
				"pow(max(dot(reflection_vector," \
				"l_viewer_vector), 0.0), u_material_shiney_ness);" \

				"phong_ads_light = phong_ads_light + ambient + diffused + specular;" \
			
				"vFragColor = vec4(phong_ads_light, 1.0);" \
				//"vfrag_color = vec4(0.0, 1.0, 1.0, 1.0);" 
			"}" \
			"else" \
			"{" \
				"vFragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
			"}" \

			"vfrag_color = texture(u_sampler, voutTextCoord) * vFragColor;" \
		"}";

	// "vFragColor = tex_color * voutColor * v_form_color; vfrag_color = texture(u_sampler, voutTextCoord) * vFragColor " 
	glShaderSource(g_uiFragmentShaderObject, 1, (const GLchar**)&pcFragmentShaderSourceCode, NULL);
	glCompileShader(g_uiFragmentShaderObject);

	// catching shader related errors if there are any
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	// getting compile status code
	glGetShaderiv(g_uiFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
			{
				GLsizei written;
				glGetShaderInfoLog(g_uiFragmentShaderObject, iInfoLogLength, &written, szInfoLog);

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
	glBindAttribLocation(g_uiShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(g_uiShaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "vtextCoord");
	glBindAttribLocation(g_uiShaderProgramObject, AMC_ATTRIBUTE_NORMAL, "v_normals");

	glLinkProgram(g_uiShaderProgramObject);

	uiModelMatrixUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_model_matrix");
	uiViewMatrixUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_view_matrix");
	uiProjectionUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_projection_matrix");

	laUniform_z = glGetUniformLocation(g_uiShaderProgramObject, "u_la_z");
	ldUniform_z = glGetUniformLocation(g_uiShaderProgramObject, "u_ld_z");
	lsUniform_z = glGetUniformLocation(g_uiShaderProgramObject, "u_ls_z");

	laUniform_x = glGetUniformLocation(g_uiShaderProgramObject, "u_la_x");
	ldUniform_x = glGetUniformLocation(g_uiShaderProgramObject, "u_ld_x");
	lsUniform_x = glGetUniformLocation(g_uiShaderProgramObject, "u_ls_x");

	laUniform_y = glGetUniformLocation(g_uiShaderProgramObject, "u_la_y");
	ldUniform_y = glGetUniformLocation(g_uiShaderProgramObject, "u_ld_y");
	lsUniform_y = glGetUniformLocation(g_uiShaderProgramObject, "u_ls_y");

	lightPositionVectorUniform_x = glGetUniformLocation(g_uiShaderProgramObject, "u_light_position_x");
	lightPositionVectorUniform_y = glGetUniformLocation(g_uiShaderProgramObject, "u_light_position_y");
	lightPositionVectorUniform_z = glGetUniformLocation(g_uiShaderProgramObject, "u_light_position_z");

	kaUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_ka");
	kdUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_kd");
	ksUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_ks");

	shineynessUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_material_shiney_ness");
	uiKeyOfLightsIsPressed = glGetUniformLocation(g_uiShaderProgramObject, "ui_is_lighting_key_pressed");

	samplerUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_sampler");

	// Vertices
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
	glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3,
		GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// TEXCOORDS
	glGenBuffers(1, &vbo_textcoords);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_textcoords);
	glBufferData(GL_ARRAY_BUFFER, 40 * sizeof(float), 0, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// NORMALS
	glGenBuffers(1, &vbo_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	BOOL boLoadedTest = FALSE;
	boLoadedTest = loadTexture(&g_textBackside, MAKEINTRESOURCE(ID_BIT_IMG_BACK_CARD));
	if (TRUE == boLoadedTest)
	{
		fprintf(g_fLogger, "Back Texture Is Loaded Successfully\n");
	}
	else if (FALSE == boLoadedTest)
	{
		fprintf(g_fLogger, "Back Texture Is Not Loaded\n");
	}

	boLoadedTest = loadTexture(&g_textLeeAsher, MAKEINTRESOURCE(ID_BIT_IMG_INSIDE));
	if (TRUE == boLoadedTest)
	{
		fprintf(g_fLogger, "IN Texture Is Loaded Successfully\n");
	}
	else if (FALSE == boLoadedTest)
	{
		fprintf(g_fLogger, "IN Texture Is Not Loaded\n");
	}

	// ERROR HANDLING
	GLint iProgramCompileLinkStatus;
	iInfoLogLength = 0;
	szInfoLog = NULL;
	glGetProgramiv(g_uiShaderProgramObject, GL_LINK_STATUS, &iProgramCompileLinkStatus);
	if (GL_FALSE == iProgramCompileLinkStatus)
	{
		glGetProgramiv(g_uiShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
			{
				GLsizei written;
				glGetProgramInfoLog(g_uiShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(g_fLogger, "FATAL ERROR OF Program of shader Linking: %s", szInfoLog);
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

	perspectiveProjectionMatrix = mat4::identity();

	// Warm Up Call to resize
	Resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;
}

void
Display(void)
{
	// Declaration of matrices
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 rotationMatrix;
	mat4 modelViewProjectionMatrix;

	void fillVertexData(Treenode * root, int level);

	int myTreeHeight;
	float fCirclePositions[3];
	static int ind = 0;

	// Code
	float angle = 2.0f * M_PI * ind / 2000;
	fCirclePositions[0] = cos(angle) * 40.0f;
	fCirclePositions[1] = sin(angle) * 40.0f;
	fCirclePositions[2] = -5.0f;

	myTreeHeight = height(root);
	size_vertex = 0;
	size_texture = 0;
	for (int lev = 1; lev <= myTreeHeight; lev++)
	{
		fillVertexData(root, lev);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(g_uiShaderProgramObject);

	// initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	// do necessary transformation
	modelMatrix = translate(0.0f, 2.0f, -7.0f);
	rotationMatrix = rotate(fAngle, 0.0f, 1.0f, 0.0f);
	modelMatrix = modelMatrix * rotationMatrix;

	// mvp
	glUniformMatrix4fv(uiModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(uiViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(uiProjectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	// 
	if (TRUE == boKeyOfLightsIsPressed)
	{
		glUniform1i(uiKeyOfLightsIsPressed, 1);

		glUniform4f(lightPositionVectorUniform_z, fCirclePositions[0], fCirclePositions[1], fCirclePositions[2], 1.0f);

		glUniform3fv(laUniform_z, 1, light_ambient_red);
		glUniform3fv(lsUniform_z, 1, light_specular_red);
		glUniform3fv(ldUniform_z, 1, light_diffused_red);

		glUniform4f(lightPositionVectorUniform_x, 0.0f, fCirclePositions[0], fCirclePositions[1] - 4.0f, 1.0f);

		glUniform3fv(laUniform_x, 1, light_ambient_blue);
		glUniform3fv(lsUniform_x, 1, light_specular_blue);
		glUniform3fv(ldUniform_x, 1, light_diffused_blue);

		glUniform4f(lightPositionVectorUniform_y, fCirclePositions[0], 0.0f, fCirclePositions[1] - 4.0f, 1.0f);

		glUniform3fv(laUniform_y, 1, light_ambient_green);
		glUniform3fv(lsUniform_y, 1, light_specular_green);
		glUniform3fv(ldUniform_y, 1, light_diffused_green);

		glUniform3fv(kaUniform, 1, material_ambient);
		glUniform3fv(kdUniform, 1, material_diffused);
		glUniform3fv(ksUniform, 1, material_specular);
		glUniform1f(shineynessUniform, material_shineyness);
	}
	else
	{
		glUniform1i(uiKeyOfLightsIsPressed,	0);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_textBackside);
	glUniform1i(samplerUniform, 0);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
	glBufferData(GL_ARRAY_BUFFER, size_vertex * sizeof(float), pfVertexArray, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, size_vertex * sizeof(float), pfNormalsArray, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_textcoords);
	glBufferData(GL_ARRAY_BUFFER, size_texture * sizeof(float), pfTexCoordsmal, GL_DYNAMIC_DRAW);

	// draw necessary scene
	for (int i = 0; i < size_vertex / 3; i += 20)
	{
		glBindTexture(GL_TEXTURE_2D, g_textLeeAsher);
		glDrawArrays(GL_TRIANGLE_FAN, i + 0, 4);

		glBindTexture(GL_TEXTURE_2D, g_textBackside);
		glDrawArrays(GL_TRIANGLE_FAN, i + 4, 4);

		glBindTexture(GL_TEXTURE_2D, g_textLeeAsher);
		glDrawArrays(GL_TRIANGLE_FAN, i + 8, 4);

		glBindTexture(GL_TEXTURE_2D, g_textBackside);
		glDrawArrays(GL_TRIANGLE_FAN, i + 12, 4);

		glDrawArrays(GL_TRIANGLE_FAN, i + 16, 4);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
	glUseProgram(0);
	SwapBuffers(g_hdc);

	ind = ind + 1;
	if (ind > 2000)
	{
		ind = 0;
	}
}

void
fillVertexData(Treenode* root, int level)
{
	// initialization and declarations
	Vertex* ver;

	float* getNormals(float arr[72]);
	vec3 getNormalForChord(float a[]);

	// code
	if (NULL == root)
	{
		return;
	}
	if (1 == level)
	{
		ver = (Vertex*)root->data;

		// inside card
		pfVertexArray[size_vertex] = ver->xUp_back;
		pfVertexArray[size_vertex + 1] = ver->yUp_back;
		pfVertexArray[size_vertex + 2] = ver->z_back;

		pfVertexArray[size_vertex + 3] = ver->xLeft_back;
		pfVertexArray[size_vertex + 4] = ver->yLeft_back;
		pfVertexArray[size_vertex + 5] = ver->z_back;

		pfVertexArray[size_vertex + 6] = ver->xLeft;
		pfVertexArray[size_vertex + 7] = ver->yLeft;
		pfVertexArray[size_vertex + 8] = ver->z;

		pfVertexArray[size_vertex + 9] = ver->xUp;
		pfVertexArray[size_vertex + 10] = ver->yUp;
		pfVertexArray[size_vertex + 11] = ver->z;

		flag_for_invert_normal = 1;
		vec3 nor = getNormalForChord(&pfVertexArray[size_vertex]);
		//pfNormalsArray
		memcpy_s(&pfNormalsArray[size_vertex], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 3], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 6], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 9], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));

		// back card
		pfVertexArray[size_vertex + 12] = ver->xUp_back;
		pfVertexArray[size_vertex + 13] = ver->yUp_back + 0.01f;
		pfVertexArray[size_vertex + 14] = ver->z_back;

		pfVertexArray[size_vertex + 15] = ver->xLeft_back;
		pfVertexArray[size_vertex + 16] = ver->yLeft_back + 0.01f;
		pfVertexArray[size_vertex + 17] = ver->z_back;

		pfVertexArray[size_vertex + 18] = ver->xLeft;
		pfVertexArray[size_vertex + 19] = ver->yLeft + 0.01f;
		pfVertexArray[size_vertex + 20] = ver->z;

		pfVertexArray[size_vertex + 21] = ver->xUp;
		pfVertexArray[size_vertex + 22] = ver->yUp + 0.01f;
		pfVertexArray[size_vertex + 23] = ver->z;

		nor = getNormalForChord(&pfVertexArray[size_vertex + 12]);

		memcpy_s(&pfNormalsArray[size_vertex + 12], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 15], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 18], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 21], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));

		flag_for_invert_normal = 1;

		// again inside card // Right side
		pfVertexArray[size_vertex + 24] = ver->xUp_back;
		pfVertexArray[size_vertex + 25] = ver->yUp_back;
		pfVertexArray[size_vertex + 26] = ver->z_back;

		pfVertexArray[size_vertex + 27] = ver->xRight_back;
		pfVertexArray[size_vertex + 28] = ver->yRight_back;
		pfVertexArray[size_vertex + 29] = ver->z_back;

		pfVertexArray[size_vertex + 30] = ver->xRight;
		pfVertexArray[size_vertex + 31] = ver->yRight;
		pfVertexArray[size_vertex + 32] = ver->z;

		pfVertexArray[size_vertex + 33] = ver->xUp;
		pfVertexArray[size_vertex + 34] = ver->yUp;
		pfVertexArray[size_vertex + 35] = ver->z;

		nor = getNormalForChord(&pfVertexArray[size_vertex + 24]);

		memcpy_s(&pfNormalsArray[size_vertex + 24], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 27], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 30], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 33], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));

		flag_for_invert_normal = 0;

		// again back side // Right Side
		pfVertexArray[size_vertex + 36] = ver->xUp_back;
		pfVertexArray[size_vertex + 37] = ver->yUp_back + 0.01f;
		pfVertexArray[size_vertex + 38] = ver->z_back;

		pfVertexArray[size_vertex + 39] = ver->xRight_back;
		pfVertexArray[size_vertex + 40] = ver->yRight_back + 0.01f;
		pfVertexArray[size_vertex + 41] = ver->z_back;

		pfVertexArray[size_vertex + 42] = ver->xRight;
		pfVertexArray[size_vertex + 43] = ver->yRight + 0.01f;
		pfVertexArray[size_vertex + 44] = ver->z;

		pfVertexArray[size_vertex + 45] = ver->xUp;
		pfVertexArray[size_vertex + 46] = ver->yUp;
		pfVertexArray[size_vertex + 47] = ver->z;

		nor = getNormalForChord(&pfVertexArray[size_vertex + 36]);

		memcpy_s(&pfNormalsArray[size_vertex + 36], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 39], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 42], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 45], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));

		flag_for_invert_normal = 0;

		// floor with back text
		pfVertexArray[size_vertex + 48] = ver->xLeft_back;
		pfVertexArray[size_vertex + 49] = ver->yLeft_back;
		pfVertexArray[size_vertex + 50] = ver->z_back;

		pfVertexArray[size_vertex + 51] = ver->xLeft;
		pfVertexArray[size_vertex + 52] = ver->yLeft;
		pfVertexArray[size_vertex + 53] = ver->z;

		pfVertexArray[size_vertex + 54] = ver->xRight;
		pfVertexArray[size_vertex + 55] = ver->yRight;
		pfVertexArray[size_vertex + 56] = ver->z;

		pfVertexArray[size_vertex + 57] = ver->xRight_back;
		pfVertexArray[size_vertex + 58] = ver->yRight_back;
		pfVertexArray[size_vertex + 59] = ver->z_back;

		nor = getNormalForChord(&pfVertexArray[size_vertex + 48]);

		memcpy_s(&pfNormalsArray[size_vertex + 48], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 51], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 54], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));
		memcpy_s(&pfNormalsArray[size_vertex + 57], 3 * sizeof(GLfloat), nor, 3 * sizeof(GLfloat));

		size_vertex = size_vertex + 60;

		// TEXCOORDS
		const GLfloat fTexCoords[] = {
				0.0f, 1.0f,
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,

				0.0f, 1.0f,
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,

				0.0f, 1.0f,
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,

				0.0f, 1.0f,
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,

				0.0f, 1.0f,
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f
		};

		if (0 == size_texture)
		{
			pfTexCoordsmal = (GLfloat*)malloc(40 * sizeof(GLfloat));
			memset(pfTexCoordsmal, 0, 40 * sizeof(GLfloat));
			memcpy_s(pfTexCoordsmal, 40 * sizeof(GLfloat), fTexCoords, 40 * sizeof(GLfloat));
			size_texture = size_texture + 40;
		}
		else
		{
			if (NULL == (pfTexCoordsmal = (GLfloat*)realloc(
				pfTexCoordsmal, (size_texture + 40) * sizeof(GLfloat))))
			{
				fprintf(g_fLogger, "Realloc For Texture Failed\n");
				exit(1);
			}

			memcpy_s((void*)(pfTexCoordsmal + size_texture),
				(40) * sizeof(GLfloat),
				fTexCoords,
				sizeof(fTexCoords)
			);

			size_texture = size_texture + 40;
		}
	}
	else
	{
		fillVertexData(root->left, level - 1);
		fillVertexData(root->right, level - 1);
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

//
// Tree Functions
//
Treenode*
insertNode(Treenode** node)
{
	// initialization
	bool checkIfPowerOf2(int);

	int height(Treenode * root);

	Vertex* vold = NULL;
	Vertex* vdata = NULL;
	Treenode* newNode = NULL;
	Treenode* tempNode = NULL;

	// code
	newNode = (Treenode*)malloc(sizeof(Treenode));
	newNode->data = NULL;
	newNode->left = NULL;
	newNode->right = NULL;

	if (NULL == *node)
	{
		*node = newNode;
		vdata = (Vertex*)malloc(sizeof(Vertex));

		vdata->xUp = 0.0f;
		vdata->yUp = (GLfloat)sqrt(3) / 2;
		vdata->xLeft = -0.5f;
		vdata->yLeft = 0.0f;
		vdata->xRight = 0.5f;
		vdata->yRight = 0.0f;
		vdata->z = 0.3f;

		vdata->xUp_back = 0.0f;
		vdata->yUp_back = (GLfloat)sqrt(3) / 2;
		vdata->xLeft_back = -0.5f;
		vdata->yLeft_back = 0.0f;
		vdata->xRight_back = 0.5f;
		vdata->yRight_back = 0.0f;
		vdata->z_back = -0.3f;

		(*node)->level = 1;
		(*node)->data = (Vertex*)vdata;
		return newNode;
	}

	Queue* queue1 = createQueue();
	enQueue(queue1, *node);

	int n = 1;
	int iter = 0;
	int myLevel = 0;
	int lastLevel = 0;

	while (!isEmpty(queue1))
	{
		tempNode = front(queue1);
		myLevel = tempNode->level;
		deQueue(queue1);
		if (!tempNode->left)
		{
			if (myLevel > lastLevel)
			{
				vold = (Vertex*)tempNode->data;
				vdata = (Vertex*)malloc(sizeof(Vertex));

				vdata->xUp = vold->xLeft;
				vdata->yUp = vold->yLeft;
				vdata->xLeft = vold->xLeft - 0.5f;
				vdata->yLeft = vold->yLeft - (GLfloat)sqrt(3) / 2;
				vdata->xRight = vold->xLeft + 0.5f;
				vdata->yRight = vdata->yLeft;
				vdata->z = 0.3f;

				vdata->xUp_back = vold->xLeft;
				vdata->yUp_back = vold->yLeft;
				vdata->xLeft_back = vold->xLeft - 0.5f;
				vdata->yLeft_back = vold->yLeft - (GLfloat)sqrt(3) / 2;
				vdata->xRight_back = vold->xLeft + 0.5f;
				vdata->yRight_back = vdata->yLeft;
				vdata->z_back = -0.3f;

				newNode->data = vdata;
				newNode->level = myLevel + 1;
				tempNode->left = newNode;
				lastLevel = myLevel;
				return newNode;
			}
		}
		else
		{
			enQueue(queue1, tempNode->left);
		}

		if (!tempNode->right)
		{
			vold = (Vertex*)tempNode->data;
			vdata = (Vertex*)malloc(sizeof(Vertex));

			vdata->xUp = vold->xRight;
			vdata->yUp = vold->yRight;
			vdata->xLeft = vold->xRight - 0.5f;
			vdata->yLeft = vold->yRight - (GLfloat)sqrt(3) / 2;
			vdata->xRight = vold->xRight + 0.5f;
			vdata->yRight = vdata->yLeft;
			vdata->z = 0.3f;

			vdata->xUp_back = vold->xRight;
			vdata->yUp_back = vold->yRight;
			vdata->xLeft_back = vold->xRight - 0.5f;
			vdata->yLeft_back = vold->yRight - (GLfloat)sqrt(3) / 2;
			vdata->xRight_back = vold->xRight + 0.5f;
			vdata->yRight_back = vdata->yLeft;
			vdata->z_back = -0.3f;

			newNode->data = vdata;
			newNode->level = myLevel + 1;
			tempNode->right = newNode;
			lastLevel = myLevel;
			return newNode;
		}
		else
		{
			enQueue(queue1, tempNode->right);
			fprintf(g_fLogger, "3. Value N:%d\n", n);
			n += 1;
		}

		lastLevel = myLevel;
	}

	free(queue1);
}

int height(Treenode* root)
{
	if (NULL == root)
	{
		return 0;
	}
	else
	{
		int lheight = height(root->left);
		int rheight = height(root->right);

		if (lheight > rheight)
		{
			return (lheight + 1);
		}
		else
		{
			return (rheight + 1);
		}
	}
}

Treenode*
front(Queue* queue)
{
	return (queue->array[queue->front]);
}

void
DeleteDeepest(Treenode** root)
{
	// initializations & declarations
	int ind = 0;
	Queue* queue;
	Queue* queue2;
	Treenode* temp;
	Treenode* quTemp;
	Treenode* arr[50];
	Treenode* front1;
	bool flagDone = true;
	bool leftLast = false;

	// code
	if (NULL == (*root))
	{
		fprintf(g_fLogger, "Failed as tried to delete empty tree");
		return;
	}

	if ((NULL == (*root)->left) && NULL == (*root)->right)
	{
		free((*root)->data);
		(*root) = NULL;
		return;
	}

	queue = createQueue();
	enQueue(queue, *root);
	while (1)
	{
		leftLast = false;
		flagDone = true;
		temp = front(queue);
		deQueue(queue);
		if (NULL != temp->left)
		{
			enQueue(queue, temp->left);
			leftLast = true;
			flagDone = false;
		}

		if (NULL != temp->right)
		{
			enQueue(queue, temp->right);
			flagDone = false;
		}

		if (true == flagDone)
		{
			break;
		}
	}

	if (isEmpty(queue))
	{
		//free(temp->data);
		free((*root)->left->data);
		free((*root)->left);
		(*root)->left = NULL;
		return;
	}
	while (!isEmpty(queue))
	{
		arr[ind] = front(queue);
		deQueue(queue);
		ind++;
	}

	ind--;
	quTemp = arr[ind];

	queue2 = createQueue();
	enQueue(queue2, *root);
	while (1)
	{
		front1 = front(queue2);
		deQueue(queue2);

		if (quTemp == front1->left)
		{
			front1->left = NULL;
			free(quTemp->data);
			quTemp = NULL;
			break;
		}
		else
		{
			if (NULL != front1->left)
			{
				enQueue(queue2, front1->left);
			}
		}

		if (quTemp == front1->right)
		{
			front1->right = NULL;
			free(quTemp->data);
			quTemp = NULL;
			break;
		}
		else
		{
			if (NULL != front1->right)
			{
				enQueue(queue2, front1->right);
			}
		}
	}

	free(queue);
	free(queue2);
	return;
}

//
// Queue Functions
//
Queue*
createQueue()
{
	Queue* queue = (Queue*)malloc(sizeof(Queue));
	queue->front = queue->rear = -1;
	return queue;
}

void
enQueue(Queue* queue, Treenode* value)
{
	if (queue->rear == sizeof(queue->array))
	{
		return;
	}
	else if (isEmpty(queue))
	{
		queue->rear = queue->front = 0;
		queue->array[queue->rear] = value;
	}
	else
	{
		queue->rear += 1;
		queue->array[queue->rear] = value;
	}
}

void
deQueue(Queue* queue)
{
	if (isEmpty(queue))
	{
		return;
	}
	else if (queue->front == queue->rear)
	{
		queue->front = queue->rear = -1;
	}
	else
	{
		queue->front += 1;
	}
}

int
isEmpty(Queue* queue)
{
	if (-1 == queue->front && -1 == queue->rear)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void
update()
{
	if (g_EnableRotation)
	{
		fAngle = fAngle + 0.02f;
	}
}

void
uninitialization(void)
{
	void ToggleFullScreen();

	if (NULL != pfTexCoordsmal)
	{
		free(pfTexCoordsmal);
		pfTexCoordsmal = NULL;
	}

	if (g_bFullScreen)
	{
		ToggleFullScreen();
	}

	int iheight = height(root);
	for (int c = iheight; c >= 1; c--)
	{
		DeleteLastLevel(&root, c);
	}

	if (vbo_vertex)
	{
		glDeleteBuffers(1, &vbo_vertex);
		glDeleteTextures(1, &g_textLeeAsher);
		glDeleteTextures(1, &g_textBackside);
		vbo_vertex = 0;
	}

	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
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
			glGetAttachedShaders(g_uiShaderProgramObject, shaderCount, &shaderCount, pShaders);

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
			if (GetWindowPlacement(g_hwnd, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(g_hwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(g_hwnd, GWL_STYLE, g_dwStyle & (~WS_OVERLAPPEDWINDOW));
				SetWindowPos(
					g_hwnd, HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
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
		SetWindowPos(g_hwnd, HWND_TOP,
			0, 0, 0, 0, SWP_NOZORDER |
			SWP_FRAMECHANGED | SWP_NOMOVE |
			SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(true);
		g_bFullScreen = FALSE;
	}
}

BOOL
loadTexture(GLuint* texture, TCHAR imgResource[])
{
	// initializations
	HBITMAP hBitMap = NULL;
	BITMAP bmp;
	BOOL bStatus = FALSE;

	// code
	hBitMap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imgResource, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBitMap)
	{
		bStatus = TRUE;
		GetObject(hBitMap, sizeof(BITMAP), &bmp);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0); // explicitly unbinding image
		DeleteObject(hBitMap);
	}
	else
	{
		fprintf(g_fLogger, "Possible Fatal Error Occured No. %d\n", GetLastError());
	}

	return bStatus;
}

void
DeleteLastLevel(Treenode** node, int level)
{
	if (NULL == (*node))
	{
		return;
	}

	if (1 == level)
	{
		free((*node)->data);
		(*node)->data = NULL;
		free((*node));
		*node = NULL;
	}

	else if (level > 1)
	{
		DeleteLastLevel(&(*node)->left, level - 1);
		DeleteLastLevel(&(*node)->right, level - 1);
	}
}

// Normals related Functions
float*
getNormals(float arr[72])
{
	static int i = 0;
	vec3 a2, a3, a4;
	float* data = NULL;
	int size = 0;
	vec3 getNormalForChord(float a[]);

	for (int i = 0; i < 72; i += 12)
	{
		vec3 nor = getNormalForChord(&arr[i]);

		if (NULL == data)
		{
			size = 12 * sizeof(float);
			data = (float*)malloc(size);
			memcpy_s(data, 3 * sizeof(float), nor, 3 * sizeof(float));
			memcpy_s(data + 3, 3 * sizeof(float), nor, 3 * sizeof(float));
			memcpy_s(data + 6, 3 * sizeof(float), nor, 3 * sizeof(float));
			memcpy_s(data + 9, 3 * sizeof(float), nor, 3 * sizeof(float));
		}
		else
		{
			size += 12 * sizeof(float);
			data = (float*)realloc(data, size);
			memcpy_s(data + i, 3 * sizeof(float), nor, 3 * sizeof(float));
			memcpy_s(data + i + 3, 3 * sizeof(float), nor, 3 * sizeof(float));
			memcpy_s(data + i + 6, 3 * sizeof(float), nor, 3 * sizeof(float));
			memcpy_s(data + i + 9, 3 * sizeof(float), nor, 3 * sizeof(float));
		}
	}

	return data;
}

vec3
getNormalForChord(float a[12])
{
	vec3 nor;

	vec3 normalVector(float v1[3], float v2[3], float v3[3]);

	if (0 == flag_for_invert_normal)
	{
		float vec1[] = { a[0], a[1], a[2] };
		float vec2[] = { a[3], a[4], a[5] };
		float vec3[] = { a[6], a[7], a[8] };

		nor = normalVector(vec1, vec2, vec3);
		flag_for_invert_normal = 1;
	}
	else
	{
		float vec1[] = { a[6], a[7], a[8] };
		float vec2[] = { a[3], a[4], a[5] };
		float vec3[] = { a[0], a[1], a[2] };

		nor = normalVector(vec1, vec2, vec3);
		flag_for_invert_normal = 0;
	}

	return nor;
}

vec3
normalVector(float v1[3], float v2[3], float v3[3])
{
	vec3 normcrossprod(vec3, vec3);

	vec3 d1;
	vec3 d2;
	vec3 norm;

	d1 = vec3(v1[0], v1[1], v1[2]) - vec3(v2[0], v2[1], v2[2]);
	d2 = vec3(v2[0], v2[1], v2[2]) - vec3(v3[0], v3[1], v3[2]);

	norm = normcrossprod(d1, d2);
	return norm;
}

vec3
normcrossprod(vec3 v1, vec3 v2)
{
	vec3 normalize(vec3);
	vec3 out;

	out[0] = v1[1] * v2[2] - v1[2] * v2[1];
	out[1] = v1[2] * v2[0] - v1[0] * v2[2];
	out[2] = v1[0] * v2[1] - v1[1] * v2[0];

	return normalize(out);
}

vec3
normalize(vec3 v)
{
	GLfloat d;

	d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (0 == d)
	{
		return vec3(0.0f, 0.0f, 0.0f);
	}

	v[0] /= d;
	v[1] /= d;
	v[2] /= d;

	return v;
}


