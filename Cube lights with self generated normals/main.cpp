/*
Module Name:
	2D Colored Animated objects at one time

Abstract:
	This App demostrates the Black Screen having Perspective Projection
	This App displays plain B&B cube

Revision History:
	Date:	July 28, 2019.
	Desc:	Started

	Date:	July 28, 2019.
	Desc:	Done
*/

#include "main.h"
/////////////////////////////////////////////////////
// Global Variables declarations and initializations
/////////////////////////////////////////////////////
HDC g_hdc = NULL;
HWND g_hwnd = NULL;
HGLRC g_hrc = NULL;		//HGLRC is openGL rendering context
bool g_boActiveWindow = false;
bool g_bFullScreen = false;
DWORD g_dwStyle;

mat4 perspectiveProjectionMatrix;

GLuint g_uiVertextShaderObject;
GLuint g_uiFragmentShaderObject;
GLuint g_uiShaderProgramObject;

GLuint vao_cube;
GLuint vbo_position_cube;
GLuint vbo_normals_cube;

GLuint uiModelViewUniform;
GLuint uiProjectionUniform;

GLuint uiKeyOfLightsIsPressed;
GLuint ldUniform;
GLuint kdUniform;
GLuint lightPositionVectorUniform;

BOOL boKeyOfLightsIsPressed = FALSE;
float *normalsCalculated = NULL;

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

//////////////////////////////f
//global function declarations
//////////////////////////////
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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
	TCHAR szAppName[] = TEXT("OGL");

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
		TEXT("OGL"),
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
Initialize(
	void
)
{
	GLenum result;

	//
	// Functions Declarations
	//
	void Resize(int, int);
	void uninitialization();
	float* getNormals(float arr[72]);

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
		"in vec3 v_normals;" \
		"uniform mat4 u_model_view_mat;" \
		"uniform mat4 u_model_projection_mat;" \
		"uniform int ui_is_lighting_key_pressed;" \
		"uniform vec3 u_ld;" \
		"uniform vec3 u_kd;" \
		"uniform vec4 u_light_position;" \
		"out vec3 diffused_color;" \
		"void main(void)" \
		"{" \

		"if(ui_is_lighting_key_pressed == 1){" \
			"vec4 eye_coordinates = u_model_view_mat * v_position;" \
			"mat3 normal_matrix = mat3(transpose(inverse(u_model_view_mat)));" \
			"vec3 t_norm = normalize(normal_matrix * v_normals);" \
			"vec3 source = vec3(u_light_position - eye_coordinates);" \
			"diffused_color = u_ld *u_kd * dot(source, t_norm);" \
		"}" \

			"gl_Position = u_model_projection_mat * u_model_view_mat * v_position;" \
		"}";

	glShaderSource(g_uiVertextShaderObject,
		1,
		(const GLchar**)(&pcVertexShaderSourceCode),
		NULL);

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

	if (GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiVertextShaderObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
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

	// fragment shader
	g_uiFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar *pcFragmentShaderSourceCode =
		"#version 440 core" \
		"\n" \
		"in vec3 diffused_color;" \
		"out vec4 v_frag_color;" \
		"uniform int ui_is_lighting_key_pressed;" \
		"void main(void)" \
		"{" \
			"if(ui_is_lighting_key_pressed == 1){ " \
				"v_frag_color = vec4(diffused_color, 1.0);" \
			"}" \
			"else{" \
				"v_frag_color = vec4(1.0, 1.0, 1.0, 1.0);" \
			"}" \
		"}";


	// specify above code of shader to vertext shader object
	glShaderSource(g_uiFragmentShaderObject, 1, (const GLchar**)&pcFragmentShaderSourceCode, NULL);

	// compile the vertext shader
	glCompileShader(g_uiFragmentShaderObject);

	// catching shader related errors if there are any
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

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

	// Before Prelinking bind binding our no to vertex attribute
	// change for Ortho
	// here we binded gpu`s variable to cpu`s index
	glBindAttribLocation(g_uiShaderProgramObject,
		AMC_ATTRIBUTE_POSITION,
		"v_position");

	glBindAttribLocation(g_uiShaderProgramObject,
		AMC_ATTRIBUTE_NORMAL,
		"v_normals");

	// link the shader
	glLinkProgram(g_uiShaderProgramObject);

	// after linking now its time to retrive uniform locations

	// now this is rule: attribute binding should happen before linking program and
	// uniforms binding should happen after linking
	uiModelViewUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_model_view_mat"
	);

	uiProjectionUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_model_projection_mat"
	);

	uiKeyOfLightsIsPressed = glGetUniformLocation(
		g_uiShaderProgramObject,
		"ui_is_lighting_key_pressed"
	);

	ldUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_ld"
	);

	kdUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_kd"
	);

	lightPositionVectorUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_light_position"
	);

	// RECTANGLE
	GLfloat fCubePositions[] = {
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
			-1.0f, -1.0f, -1.0f };

	const GLfloat fCubeNormals[] = {
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

	normalsCalculated = getNormals(fCubePositions);
	//for (int i = 0; i < 72; i += 12)
	//{
		//fprintf(g_fLogger,
		//		"%f %f %f\n",
		//		normalsCalculated[i + 0], normalsCalculated[i + 1], normalsCalculated[i + 2]);
	//}

	glGenVertexArrays(1, &vao_cube);
	glBindVertexArray(vao_cube);

	// working on position of rectangles
	glGenBuffers(1, &vbo_position_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(fCubePositions),
		fCubePositions,
		GL_STATIC_DRAW);

	glVertexAttribPointer(
		AMC_ATTRIBUTE_POSITION,
		3,									// how many co-ordinates in vertice
		GL_FLOAT,							// type of above data
		GL_FALSE,							// no normalization is desired
		0,									// (dangha)
		NULL								// offset to start in above attrib position
	);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_normals_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals_cube);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(fCubeNormals), fCubeNormals, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fCubeNormals), normalsCalculated, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL );

	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//
	// ERROR HANDLING
	//

	// steps for catching program related error
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
				glGetProgramInfoLog(
					g_uiShaderProgramObject,
					iInfoLogLength,
					&written,
					szInfoLog);
				fprintf(g_fLogger,
					"FATAL ERROR OF Program of shader Linking: %s", szInfoLog);
				free(szInfoLog);
				uninitialization();
				DestroyWindow(g_hwnd);
			}
		}
	}

	glEnable(GL_DEPTH_TEST);

	//
	// Tell OpenGL engine that which test has to be performed
	//
	glDepthFunc(GL_LEQUAL);

	// #New line
	// glEnable(GL_CULL_FACE);
	//glEnable(GL_FRONT_FACE);
	//glEnable(GL_FRONT_FACE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	// #change for perspective	
	perspectiveProjectionMatrix = mat4::identity();

	glFrontFace(GL_CW);
	//glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);

	//
	// Warm Up Call to resize
	// 
	Resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;
}

void
uninitialization(
	void
)
{
	//
	// Check whether FullScreen Or Not, If it is there restore to normal size &
	// then proceed for uninitization 
	// We do safe release here
	//
	void ToggleFullScreen();

	free(normalsCalculated);
	normalsCalculated = NULL;

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
				g_uiShaderProgramObject,
				shaderCount,
				&shaderCount,
				pShaders);

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
ToggleFullScreen(
	void
)
{
	//
	// Declarations 
	//
	MONITORINFO mi;

	//code
	if (false == g_bFullScreen)
	{
		g_dwStyle = GetWindowLong(
			g_hwnd,
			GWL_STYLE
		);

		if (WS_OVERLAPPEDWINDOW == (WS_OVERLAPPEDWINDOW & g_dwStyle))
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(
				g_hwnd,
				&g_wpPrev)
				&&
				GetMonitorInfo(
					MonitorFromWindow(g_hwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(g_hwnd, GWL_STYLE, g_dwStyle & (~WS_OVERLAPPEDWINDOW));
				SetWindowPos(g_hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, (mi.rcMonitor.right -
						mi.rcMonitor.left),
						(mi.rcMonitor.bottom -
							mi.rcMonitor.top),
					SWP_NOZORDER | SWP_FRAMECHANGED);
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

		SetWindowPlacement(
			g_hwnd,
			&g_wpPrev
		);

		SetWindowPos(g_hwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(true);
		g_bFullScreen = FALSE;
	}
}

float*
getNormals(float arr[72])
{
	static int i = 0;
	vec3 a2, a3, a4;
	float *data = NULL;
	int size = 0;
	vec3 getNormalForChord(float a[]);

	for(int i = 0; i < 72; i+=12)
	{
		/*float newarr[12];
		if (0 == i)
		{
			for (int j = 0; j < 12; j++)
			{
				newarr[j] = arr[i + j];
			}

			i = 1;
		}
		else
		{
			int k = 0;
			for (int j = 11; j >= 0; j--, k++)
			{
				newarr[j] = arr[i + k];
			}

			i = 0;
		}*/

		vec3 nor = getNormalForChord(&arr[i]);

		if (NULL == data)
		{
			size = 12 * sizeof(float);
			data = (float*)malloc(size);
			memcpy_s(data,     3 * sizeof(float),  nor,  3 * sizeof(float));
			memcpy_s(data + 3, 3 * sizeof(float),  nor,  3 * sizeof(float));
			memcpy_s(data + 6, 3 * sizeof(float),  nor,  3 * sizeof(float));
			memcpy_s(data + 9, 3 * sizeof(float),  nor, 3 * sizeof(float));
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

	/*
		float vec1[] = {a[0], a[1], a[2]};
		float vec2[] = {a[3], a[4], a[5]};
		float vec3[] = {a[6], a[7], a[8]};
	*/

	static int flag = 0;

	if (0 == flag)
	{
		float vec1[] = { a[0], a[1], a[2] };
		float vec2[] = { a[3], a[4], a[5] };
		float vec3[] = { a[6], a[7], a[8] };

		nor = normalVector(vec1, vec2, vec3);
		flag = 1;
	}
	else
	{
		float vec1[] = { a[6], a[7], a[8] };
		float vec2[] = { a[3], a[4], a[5] };
		float vec3[] = { a[0], a[1], a[2] };

		nor = normalVector(vec1, vec2, vec3);
		flag = 0;
	}

	//if (vec1[0] < 0 && vec2[0] < 0 && vec3[0] < 0)
	//{
	//	nor = -nor;
	//}
	//else if (vec1[1] < 0 && vec2[1] < 0 && vec3[1] < 0)
	//{
	//	nor = -nor;
	//}
	//else if (vec1[2] < 0 && vec2[2] < 0 && vec3[2] < 0)
	//{
	//	nor = -nor;
	//}

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

