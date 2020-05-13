/*
Module Name:
	This App demostrates the just Black Screen having Orthographic Projection Having Triangle To Display of Programmable Pipeline
	This code builds the template for shaders of Programmable Pipeline

Abstract:
	Plain Template of pipeline programming for upcoming Programmable Programs

Revision History:
	Date:	July 21, 2019.
	Desc:	Started

	Date:	July 30, 2019.
	Desc:	Done

	Date:	May 11, 2020
	Desc:   Adding Lights
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

// added changes for Ortho-Triangle.
GLuint vao; 
GLuint vbo; 
GLuint vbo_normal;
GLuint mvpUniform;

GLuint brickPctUniform;
GLuint brickSizeUniform;
GLuint brickColorUniform;
GLuint morterColorUniform;

GLuint mvUniform;
GLuint lightPosition;
GLuint uiKeyOfLightsIsPressed;

BOOL boKeyOfLightsIsPressed = FALSE;

mat4 perspectiveProjectionMatrix;

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
	//Declarations
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

			"uniform mat4	u_mv_matrix;" \
			"uniform mat4	u_mvp_matrix;" \
			"uniform vec4	u_light_position;" \
			"uniform int	u_is_lighting_key_pressed;" \

			"const float specular_contribution = 0.3;" \
			"const float diffused_contribution = 1.0 - specular_contribution;" \

			"out vec2	mc_position;" \
			"out float	light_intensity;" \

			"void main(void)" \
			"{" \
				"vec3		tnorm;" \
				"float		diffuse;" \
				"vec3		ec_position;" \
				"vec3		view_vector;" \
				"float		spec = 0.0;" \
				"mat3		normal_matrix;" \
				"vec3		reflect_vector;" \
				"vec3		light_direction;" \
				
				"if(u_is_lighting_key_pressed == 1)" \
				"{" \
					"ec_position = vec3(u_mv_matrix * v_position);" \
					"normal_matrix = mat3(u_mv_matrix);" \
					"tnorm = normalize(normal_matrix * v_normal);" \
					"light_direction = normalize(u_light_position.xyz - ec_position);" \
					"reflect_vector = reflect(-light_direction, tnorm);" \
					"view_vector = normalize(-ec_position);" \

					"diffuse = max(dot(light_direction, tnorm), 0.0);" \
					"if(diffuse > 0.0)" \
					"{" \
						"spec = max(dot(reflect_vector, view_vector), 0.0);" \
						"spec = pow(spec, 16.0);" \
					"}" \

					"light_intensity = diffused_contribution * diffuse + specular_contribution * spec;" \
				 "}" \

				"mc_position = v_position.xy;" \
				"gl_Position = u_mvp_matrix * v_position;" \
			"}";

	glShaderSource(g_uiVertextShaderObject, 1, (const GLchar**)(&pcVertexShaderSourceCode), NULL);

	glCompileShader(g_uiVertextShaderObject);

	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	// getting compile status code
	glGetShaderiv(g_uiVertextShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

	if(GL_FALSE == iShaderCompileStatus)
	{
		glGetShaderiv(g_uiVertextShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if(iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if(NULL != szInfoLog)
			{
				GLsizei written;

				glGetShaderInfoLog(g_uiVertextShaderObject, iInfoLogLength,
					&written, szInfoLog);

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
		"out vec4 vFragColor;" \
		"in vec2 mc_position;" \

		"uniform vec3 brick_color, morter_color;" \
		"uniform vec2 brick_size;" \
		"uniform vec2 brick_pct;" \
		"uniform int u_is_lighting_key_pressed;" \

		"in float light_intensity;" \

		"void main(void)" \
		"{" \
			"vec3 color;" \
			"vec2 position, useBrick;" \
			"position = mc_position / brick_size;" \
			"if (fract(position.y * 0.5) > 0.5)" \
			"{" \
				"position.x += 0.5;" \
			"}" \

			"position = fract(position);" \
			"useBrick = step(position, brick_pct);" \
			"color = mix(morter_color, brick_color, useBrick.x * useBrick.y);" \
			"if(1 == u_is_lighting_key_pressed)" \
			"{" \
				"color = color * light_intensity;" \
			"}" \

			"vFragColor = vec4(color, 1.0);" \
		"}";

	/*
	"if(0 == color * light_intensity)" \
		"{" \
		" color = vec3(1.0, 0.0, 0.0);" \
		"}" 
		\ */
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
		glGetShaderiv(g_uiFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
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
		"v_normal");

	// link the shader
	glLinkProgram(g_uiShaderProgramObject);

	// after linking now its time to retrive uniform locations

	// now this is rule: attribute binding should happen before linking program and
	// uniforms binding should happen after linking
	mvpUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_mvp_matrix"
	);

	// new Uniforms;
	brickColorUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"brick_color"
	);

	morterColorUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"morter_color"
	);

	brickSizeUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"brick_size"
	);

	brickPctUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"brick_pct"
	);

	uiKeyOfLightsIsPressed = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_is_lighting_key_pressed"
	);

	mvUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_mv_matrix"
	);

	lightPosition = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_light_position"
	);

	const GLfloat frectangleVertices[] = 
		{
			1.0f, 1.0f,   0.0f,
			-1.0f, 1.0f,  0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f,  0.0f,
		};

	const GLfloat frectangleNormal[] =
	{

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};

	//vbo_normal

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(frectangleVertices), frectangleVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_normal);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(frectangleNormal), frectangleNormal, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	// #change for perspective
	perspectiveProjectionMatrix = mat4::identity();

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

