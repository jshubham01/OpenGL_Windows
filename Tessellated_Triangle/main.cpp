/*
Module Name:
	Tesselation Shader Tryout
	This code builds the template for Tesselation Shader of Programmable Pipeline

Abstract:
	Plain Template of pipeline programming for upcoming Programmable Programs

Revision History:
	Date:	December 23, 2019.
	Desc:	Started

	Date:	December 23, 2019.
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

GLuint g_uiVertextShaderObject;
GLuint g_uiFragmentShaderObject;
GLuint g_uiShaderProgramObject;

GLuint g_uiTesselationControlShaderObject;
GLuint g_uiTesselationEvaluationShaderObject;

GLuint g_uiNumberOfSegmentUniform;
GLuint g_uiNumberOfStripUniform;
GLuint g_uiLineColorUniform;

// added changes for Ortho-Triangle.
GLuint vao; // vertex array object
GLuint vbo; // vertex buffer object
GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;

unsigned int g_uiNumberOfLineSegments;
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

//////////////////////////////
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
	TCHAR szAppName[] = TEXT("OpenGLPP");

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
		TEXT("Bezier"),
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
				//update();
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

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_DOWN:
			g_uiNumberOfLineSegments--;
			if (g_uiNumberOfLineSegments <= 0)
			{
				g_uiNumberOfLineSegments = 1;
			}

			break;

		case VK_UP:
			g_uiNumberOfLineSegments++;
			if (g_uiNumberOfLineSegments >= 50)
			{
				g_uiNumberOfLineSegments = 50;
			}

			break;
		}

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

	//
	// Functions Declarations
	//
	void Resize(int, int);
	void uninitialization();

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

	//const GLchar *pcVertexShaderSourceCode =
	//		"#version 450 core" \
	//		"\n" \
	//		"in vec2 vPosition;" \

	//		"void main(void)" \
	//		"{" \
	//			"gl_Position = vec4(vPosition, 0.0, 1.0);" \
	//		"}";

	const GLchar *pcVertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		//"in vec2 vPosition;" 

		"void main(void)" \
		"{" \
		"const vec4 vertices[] =   vec4[](vec4(0.25, -0.25, 0.5, 1.0)," \
										"vec4(-0.25, -0.25, 0.5, 1.0), " \
										"vec4(0.25, 0.25, 0.5, 1.0)); " \

		"	gl_Position = vertices[gl_VertexID];" \
	"}";

	// "gl_Position = u_mvp_matrix * vPosition;"
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

	if(GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiVertextShaderObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);
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
	
	//
	// Change Code for TesselationControlShaderObject
	//
	g_uiTesselationControlShaderObject = glCreateShader(GL_TESS_CONTROL_SHADER);
	const GLchar *tessellationControlShaderSourceCode =
		"#version 450" \
		"\n" \
		"layout(vertices=3)out;" \
		// "uniform int numberOfSegments;" \
		"uniform int numberOfStrips;" 

		"void main(void)" \
		"{" \
			"if (gl_InvocationID == 0)" \
			"{" \
				"gl_TessLevelInner[0] = 5.0; " \
				"gl_TessLevelOuter[0] = 5.0; " \
				"gl_TessLevelOuter[1] = 5.0; " \
				"gl_TessLevelOuter[2] = 5.0; " \
			"}" \

			"gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;" \

		"}";

	glShaderSource(g_uiTesselationControlShaderObject, 1, (const GLchar**)&tessellationControlShaderSourceCode, NULL);

	// compile shader
	glCompileShader(g_uiTesselationControlShaderObject);

	// catching shader related errors if there are any
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	// getting compile status code
	glGetShaderiv(g_uiTesselationControlShaderObject,
		GL_COMPILE_STATUS,
		&iShaderCompileStatus);

	if (GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiTesselationControlShaderObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
			{
				GLsizei written;

				glGetShaderInfoLog(g_uiTesselationControlShaderObject,
					iInfoLogLength,
					&written,
					szInfoLog);

				fprintf(g_fLogger, ("Tessalation control FATAL ERROR: %s\n"), szInfoLog);
				free(szInfoLog);
				uninitialization();
				DestroyWindow(g_hwnd);
			}
		}
	}


	// catching shader related errors if there are any
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	g_uiTesselationEvaluationShaderObject = glCreateShader(GL_TESS_EVALUATION_SHADER);
	const GLchar* tessellationEvaluationShaderSourceCode =
		"#version 450" \
		"\n" \
		"layout(triangles, equal_spacing, cw) in;" \
		//"uniform mat4 mvpMatrix;" 
		"void main(void)" \
		"{" \

		"gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) + " \
				"(gl_TessCoord.y * gl_in[1].gl_Position) +	" \
				"(gl_TessCoord.z * gl_in[2].gl_Position);" \
		"}";



		//"#version 450" \
		//"\n" \
		//"layout(isolines)in;" \
		//"uniform mat4 u_mvp_matrix;" \

		//"void main(void)" \
		//"{" \
		//	"float u = gl_TessCoord.x;" \
		//	"vec3 p0 = gl_in[0].gl_Position.xyz;" \
		//	"vec3 p1 = gl_in[1].gl_Position.xyz;" \
		//	"vec3 p2 = gl_in[2].gl_Position.xyz;" \
		//	"vec3 p3 = gl_in[3].gl_Position.xyz;" \

		//	"float u1 = (1.0 - u);" \
		//	"float u2 = u * u;" \
		//	"float b3 = u2 * u;" \
		//	"float b2 = 3.0 * u2 * u1;" \
		//	"float b1 = 3.0 * u * u1 * u1;" \
		//	"float b0 = u1 * u1 * u1;" \

		//	"vec3 p = b0 * p0 + b1 * p1 + b2 * p2 + b3 * p3;" \
		//	"gl_Position = u_mvp_matrix * vec4(p, 1.0);" \

		//"}";

	glShaderSource(g_uiTesselationEvaluationShaderObject, 1, (const GLchar**)&tessellationEvaluationShaderSourceCode, NULL);

	// compile shader
	glCompileShader(g_uiTesselationEvaluationShaderObject);

	// catching shader related errors if there are any
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	// getting compile status code
	glGetShaderiv(g_uiTesselationEvaluationShaderObject,
		GL_COMPILE_STATUS,
		&iShaderCompileStatus);

	if (GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiTesselationEvaluationShaderObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
			{
				GLsizei written;

				glGetShaderInfoLog(g_uiTesselationEvaluationShaderObject,
					iInfoLogLength,
					&written,
					szInfoLog);

				fprintf(g_fLogger, ("Tessalation Evalution SHADER FATAL ERROR: %s\n"), szInfoLog);
				free(szInfoLog);
				uninitialization();
				DestroyWindow(g_hwnd);
			}
		}
	}

	// working for fragment shader
	g_uiFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar *pcFragmentShaderSourceCode = 
		"#version 450 core" \
		"\n" \
		"uniform vec4 lineColor;" \
		"out vec4 vFragColor;" \
		"void main(void)" \
		"{" \
		// "vFragColor = lineColor;" 
		"vFragColor = vec4(0.0, 0.8, 1.0, 1.0);" \
		"}";

	//"#version 450 core" \
	//	"\n" \
	//	"uniform vec4 lineColor;" \
	//	"out vec4 vFragColor;" \
	//	"void main(void)" \
	//	"{" \
	//	"vFragColor = lineColor;" \
	//	"}";

	// specify above code of shader to vertext shader object
	glShaderSource(g_uiFragmentShaderObject, 1, (const GLchar**)&pcFragmentShaderSourceCode, NULL);

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
	glAttachShader(g_uiShaderProgramObject, g_uiTesselationControlShaderObject);
	glAttachShader(g_uiShaderProgramObject, g_uiTesselationEvaluationShaderObject);

	// link the shader
	glLinkProgram(g_uiShaderProgramObject);

	// after linking now its time to retrive uniform locations
	// Before Prelinking bind binding our no to vertex attribute
	// change for Ortho
	// here we binded gpu`s variable to cpu`s index
	//glBindAttribLocation(g_uiShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");


	// now this is rule: attribute binding should happen before linking program and
	// uniforms binding should happen after linking
	mvpUniform = glGetUniformLocation(g_uiShaderProgramObject, "u_mvp_matrix");

	// g_uiNumberOfSegmentUniform = glGetUniformLocation(g_uiShaderProgramObject, "numberOfSegments");
	// g_uiNumberOfStripUniform = glGetUniformLocation(g_uiShaderProgramObject, "numberOfStrips");
	// g_uiLineColorUniform = glGetUniformLocation(g_uiShaderProgramObject, "lineColor");

	const GLfloat fVertices[] = {-1.0f, -1.0f, -0.5f, 1.0f, 0.5f, -1.0f, 1.0f, 1.0f};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
			8 * sizeof(float),
			fVertices,
			GL_STATIC_DRAW);

	glVertexAttribPointer(
			AMC_ATTRIBUTE_POSITION,
			2,									// how many co-ordinates in vertice
			GL_FLOAT,							// type of above data
			GL_FALSE,							// no normalization is desired
			0,									// (dangha)
			NULL								// offset to start in above attrib position
		);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	
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
	glEnable(GL_CULL_FACE);

	//
	// Tell OpenGL engine that which test has to be performed
	//
	glDepthFunc(GL_LEQUAL);

	// CHANGE
	// glLineWidth(3.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	// #change for perspective
	perspectiveProjectionMatrix = mat4::identity();

	//g_uiNumberOfLineSegments = 1;
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

	if (g_bFullScreen)
	{
		ToggleFullScreen();
	}

	if(vbo)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
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
ToggleFullScreen(void)

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
					MonitorFromWindow(
						g_hwnd,
						MONITORINFOF_PRIMARY),
					&mi)
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
					(mi.rcMonitor.right -
						mi.rcMonitor.left),
						(mi.rcMonitor.bottom -
							mi.rcMonitor.top),
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

		SetWindowPlacement(
			g_hwnd,
			&g_wpPrev
		);

		SetWindowPos(
			g_hwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER |
			SWP_FRAMECHANGED |
			SWP_NOMOVE |
			SWP_NOSIZE |
			SWP_NOOWNERZORDER
		);

		ShowCursor(true);
		g_bFullScreen = FALSE;
	}
}

