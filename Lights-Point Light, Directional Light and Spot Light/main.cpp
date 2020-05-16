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

GLuint vao_pyramid; // vertex array object
GLuint vao_cube;

GLuint vbo_position_pyramid; //vertex buffer object
GLuint vbo_position_cube;
GLuint vbo_normal_cube;

GLuint vbo_color_pyramid;
GLuint vbo_color_cube;

GLuint lighting_key_pressed, mv_matrix;

GLuint lightTypeToggle;

BOOL bo_lightKeyIsEnabled = FALSE;
int LightPressed = 0;

GLuint mvpUniform;
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
	//
	//Declarations
	//
	MSG msg;
	HWND hwnd;
	int iRet = 0;
	TCHAR szAppName[] = TEXT("OGL-PP");

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
		TEXT("OGL-PP"),
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

		case 'l':
		case 'L':
			if (FALSE == bo_lightKeyIsEnabled)
			{
				bo_lightKeyIsEnabled = TRUE;
			}
			else
			{
				bo_lightKeyIsEnabled = FALSE;
			}

			break;

		case 's':
		case 'S':
			LightPressed = 2;
			break;
		
		case 'd':
		case 'D':
			LightPressed = 0;
			break;

		case 'p':
		case 'P':
			LightPressed = 1;
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

			"uniform mat4   u_mvp_matrix;" \
			"uniform mat4	u_mv_matrix;" \
			"uniform int	u_is_lighting_key_pressed;" \

			"out vec3	tnorm;" \
			"out vec3	ec_position;" \
			"out vec3	light_direction_directed; " \
			"out vec3	view_vector;" \

			"void main(void)" \
			"{" \
				"mat3	normal_matrix;" \
				"vec3 light_pos_directional_light = vec3(0.0, 0.0, 100.0);" \

				"if(u_is_lighting_key_pressed == 1)" \
				"{"
					"ec_position = vec3(u_mv_matrix * v_position);" \
					"normal_matrix = mat3(u_mv_matrix);" \
					"light_direction_directed =light_pos_directional_light  - ec_position;" \
					"tnorm = normalize(normal_matrix * v_normal);" \
					"view_vector = ec_position;" \
				"}"

				"gl_Position = u_mvp_matrix * v_position;" \
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

	// working for fragment shader
	g_uiFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar *pcFragmentShaderSourceCode = 
		"#version 440 core" \
		"\n" \

		"uniform int u_is_lighting_key_pressed;" \
		"uniform int light_type_toggle;" \

		"in	vec3 tnorm;" \
		"in	vec3 ec_position;" \
		"in	vec3 view_vector;" \
		"in	vec3 light_direction_directed;" \

		"vec3 halfVector_directed_light;" \

		"out vec4 vFragColor;" \

		"vec4 amb = vec4(0.0);" \
		"vec4 diff = vec4(0.0);" \
		"vec4 spec = vec4(0.0);" \

		"vec4 point_light_position = vec4(0.0, 0.0, 1.0, 1.0);" \
		"vec3 spot_direction = vec3(0.0, 0.0, -1.0);" \
		"float spotCosCutOff = 0.02;" \
		"float spot_exponent = 4.0;" \

		"float constant_attenuation = 1.0; " \
		"float linear_attenuation = 0.5;" \
		"float quadratic_attenuation = 0.2;" \

		"void SpotLight(const in int i, const vec3 eye, const vec3 ecPosition3, const in vec3 normal," \
							"inout vec4 ambient, inout vec4 diffuse, inout vec4 specular) " \
		"{" \
			"float nDotVP;" \
			"float nDotHV;" \
			"float pf;" \
			"float spotDot;" \
			"float spotAttenuation;" \
			"float attenuation;" \
			"float d;" \
			"vec3 VP;" \
			"vec3 halfVector;" \

			"vec4 ambientP  = vec4(0.3, 0.0, 0.0, 1.0);" \
			"vec4 diffuseP  = vec4(0.5, 0.0, 0.0, 1.0);" \
			"vec4 specularP = vec4(1.0, 1.0, 1.0, 1.0);" \

			"VP = vec3(point_light_position) - ecPosition3;" \
			"d = length(VP);" \
			"VP = normalize(VP);" \

			"attenuation = 1.0 / (constant_attenuation + linear_attenuation * d" \
								"+ quadratic_attenuation * d * d);" \
			
			"spotDot = dot(-VP, normalize(spot_direction));" \
			"if(spotDot < spotCosCutOff)" \
			"{"
				"spotAttenuation = 0.0;" \
			"}" \
			"else" \
			"{" \
				"spotAttenuation = pow(spotDot, spot_exponent);" \
			"}" \

			"attenuation = attenuation * spotAttenuation;" \

			"halfVector = normalize(VP + eye);" \

			"nDotVP = max(0.0, dot(normal, VP));" \
			"nDotHV = max(0.0, dot(normal, halfVector));" \

			"if(nDotVP == 0.0)" \
			"{" \
				"pf = 0.0;" \
			"}" \
			"else" \
			"{" \
				"pf = pow(nDotHV, 100.0);" \
			"}" \

			"ambient =  ambient + ambientP * attenuation;" \
			"diffuse = diffuse + diffuseP * nDotVP * attenuation;" \
			"specular = specular + specularP * pf * attenuation; " \
			
		"}" \

		"void PointLight(const in int i, const vec3 eye, const vec3 ecPosition3, const in vec3 normal," \
							"inout vec4 ambient, inout vec4 diffuse, inout vec4 specular) " \
		"{" \
			"float nDotVP;" \
			"float nDotHV;" \
			"float pf;" \
			"float attenuation;" \
			// d = distance from surface to light source
			"float d;" \
			// VP is direction from surface to normal
			"vec3 VP;" \
			// halfVector gives direction of maximum hightlight
			"vec3 halfVector;" \
		
			"vec4 ambientP  = vec4(0.0, 0.0, 0.0, 1.0);" \
			"vec4 diffuseP  = vec4(1.0, 0.0, 1.0, 1.0);" \
			"vec4 specularP = vec4(1.0, 1.0, 1.0, 1.0);" \

			"VP = vec3(point_light_position) - ecPosition3;" \
			"d = length(VP);" \
			"VP = normalize(VP);" \

			// Compute Attenuation
			"attenuation = 1.0 / (constant_attenuation + linear_attenuation * d" \
								"+ quadratic_attenuation * d * d);" \
			"halfVector = normalize(VP + eye);" \
			
			"nDotVP = max(0.0, dot(normal, VP));" \
			"nDotHV = max(0.0, dot(normal, halfVector));" \

			"if(nDotVP == 0.0)" \
			"{" \
				"pf = 0.0;" \
			"}" \
			"else" \
			"{" \
				"pf = pow(nDotHV, 100.0);" \
			"}" \

			"ambient =  ambient + ambientP * attenuation;" \
			"diffuse = diffuse + diffuseP * nDotVP * attenuation;" \
			"specular = specular + specularP * pf * attenuation; " \

		"}" \

		"void DirectionalLight(const in int i, const in vec3 normal, inout vec4 ambient," \
							"inout vec4 diffuse, inout vec4 specular) " \
		"{" \
			"float pf;" \
			"float nDotVP;" \
			"float nDotHV;" \

			"vec4 ambientD = vec4(0.1, 0.1, 0.1, 1.0);" \
			"vec4 diffuseD  = vec4(1.0, 1.0, 0.0, 1.0);" \
			"vec4 specularD = vec4(1.0, 1.0, 1.0, 1.0);" \

			"nDotVP = max(0.0, dot(normal, normalize(vec3(light_direction_directed))));" \
			"nDotHV = max(0.0, dot(normal, vec3(halfVector_directed_light)));" \

			"if (nDotVP == 0.0)" \
			"{" \
				"pf = 0.0;" \
			"}" \
			"else" \
			"{" \
				"pf = pow(nDotHV, 100.0);" \
			"}" \

			"ambient =  ambient + ambientD;" \
			"diffuse = diffuse + diffuseD * nDotVP;" \
			"specular = specular + specularD * pf; " \
		"}" \

		"void main(void)" \
		"{" \
			"vec3	l_view_vector;" \
			"vec3	l_light_directD;" \
			"vec4	light_final;" \

			"if(u_is_lighting_key_pressed == 1)" \
			"{" \
				"halfVector_directed_light = normalize(light_direction_directed -view_vector);" \
				"l_view_vector = normalize(-view_vector);" \
				"l_light_directD = normalize(light_direction_directed);" \
				
				"if (light_type_toggle == 0)" \
				"{"
					"DirectionalLight(1, tnorm, amb, diff, spec);" \
				"}" \
				"else if (light_type_toggle == 1) " \
				"{" \
					"PointLight(2, -view_vector, ec_position, tnorm, amb, diff, spec);" \
				"}" \
				"else if (light_type_toggle == 2) " \
				"{" \
					"SpotLight(2, -view_vector, ec_position, tnorm, amb, diff, spec);" \
				"}" \

				"SpotLight(2, -view_vector, ec_position, tnorm, amb, diff, spec);" \
				"light_final = amb + diff + spec;" \
				"vFragColor = light_final;"
			"}" \
			"else" \
			"{" \
				"vFragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
			"} " \
		"}";

	//"SpotLight(3, -view_vector, ec_position, tnorm, amb, diff, spec);" \
	// "PointLight(2, -view_vector, ec_position, tnorm, amb, diff, spec);" 
	

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

	// Before Prelinking bind binding our no to vertex attribute
	// here we binded gpu`s variable to cpu`s index
	glBindAttribLocation(g_uiShaderProgramObject,AMC_ATTRIBUTE_POSITION, "v_position");	
	
	glBindAttribLocation(g_uiShaderProgramObject, AMC_ATTRIBUTE_NORMAL,  "v_normal"); 

	// link the shader
	glLinkProgram(g_uiShaderProgramObject);

	mvpUniform = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_mvp_matrix"
	);

	lighting_key_pressed  = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_is_lighting_key_pressed"
	);
	
	lightTypeToggle = glGetUniformLocation(
		g_uiShaderProgramObject,
		"light_type_toggle"
	);

	mv_matrix = glGetUniformLocation(
		g_uiShaderProgramObject,
		"u_mv_matrix"
	);

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
			-1.0f, -1.0f, -1.0f};

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

	glGenVertexArrays(1, &vao_cube);
	glBindVertexArray(vao_cube);

	//
	// working on position of rectangles
	//
	glGenBuffers(1, &vbo_position_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);

	glBufferData(GL_ARRAY_BUFFER, sizeof(frectangleVertices), frectangleVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3,	GL_FLOAT, GL_FALSE, 0,	NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_normal_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fCubeNormals), fCubeNormals, GL_STATIC_DRAW);
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
	
	// #New line
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	// #change for perspective	
	perspectiveProjectionMatrix = mat4::identity();

	// Warm Up Call to resize
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
	//
	void ToggleFullScreen();

	if (g_bFullScreen)
	{
		ToggleFullScreen();
	}

	if (vbo_color_cube)
	{
		glDeleteBuffers(1, &vbo_color_cube);
		vbo_color_cube = 0;
	}

	if (vbo_color_pyramid)
	{
		glDeleteBuffers(1, &vbo_color_pyramid);
		vbo_color_pyramid = 0;
	}

	if (vbo_position_pyramid)
	{
		glDeleteBuffers(1, &vbo_position_pyramid);
		vbo_position_pyramid = 0;
	}

	if (vbo_position_cube)
	{
		glDeleteBuffers(1, &vbo_position_cube);
		vbo_position_cube = 0;
	}

	if (vao_pyramid)
	{
		glDeleteVertexArrays(1, &vao_pyramid);
		vao_pyramid = 0;
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

