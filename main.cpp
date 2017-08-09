/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| Virtual Camera - create and modify VIEW and PROJECTION matrices              |
| keyboard controls: W,S,A,D,left and right arrows                             |
\******************************************************************************/
#include "gl_utils.h"
#include "maths_funcs.h"
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "tools.h"
#define GL_LOG_FILE "gl.log"

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 1920;
int g_gl_height = 1080;
GLFWwindow *g_window = NULL;

int main(int argc, char *argv[]) {
	if(argc != 3){ 
		fprintf(stderr, "ejecutar ./prog x y\n"); 
		exit(EXIT_FAILURE);
	}
	restart_gl_log();
	/*------------------------------start GL
	 * context------------------------------*/
	start_gl();
	glfwSwapInterval(0);
int cant_tri = atoi(argv[1]);
    int cant_li = atoi(argv[2]);
    int nvertx = (cant_tri+1);
    int nverty = (cant_li+1);
    int nvert = nvertx*nverty;
    int ntri = (cant_li)*(cant_tri);
    GLfloat *points = (GLfloat*)malloc(sizeof(GLfloat)*nvert*3);
    GLuint *indices = (GLuint*)malloc(sizeof(GLuint)*ntri*3);
    GLfloat *colours = (GLfloat*)malloc(sizeof(GLfloat)*nvert*3);

    genvertices(points, nvertx, nverty);
    genindices(indices, nvertx, nverty);
    gencolors(colours, nvert);

	/*------------------------------create
	 * geometry-------------------------------*/
	//GLfloat points[]= { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
	//nvert = 3;
	//GLfloat colours[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	GLuint points_vbo;
	glGenBuffers( 1, &points_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*nvert*3, points, GL_STATIC_DRAW );
	glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 0, 0);

    GLuint indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*ntri*3, indices, GL_STATIC_DRAW);

	GLuint colours_vbo;
	glGenBuffers( 1, &colours_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, colours_vbo );
	glBufferData( GL_ARRAY_BUFFER, nvert*3* sizeof( GLfloat ), colours, GL_STATIC_DRAW );

	GLuint vao;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	glBindBuffer( GL_ARRAY_BUFFER, colours_vbo );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	//glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBufferID );
	//glVertexAttribPointer( 2, 3, GL_UNSIGNED_INT, GL_FALSE, 0, NULL );

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*ntri*3, indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	//glEnableVertexAttribArray( 2 );

	


	/*------------------------------create
	 * shaders--------------------------------*/
	char vertex_shader[1024 * 256];
	char fragment_shader[1024 * 256];
	parse_file_into_str( "test_vs.glsl", vertex_shader, 1024 * 256 );
	parse_file_into_str( "test_fs.glsl", fragment_shader, 1024 * 256 );

	GLuint vs = glCreateShader( GL_VERTEX_SHADER );
	const GLchar *p = (const GLchar *)vertex_shader;
	glShaderSource( vs, 1, &p, NULL );
	glCompileShader( vs );

	// check for compile errors
	int params = -1;
	glGetShaderiv( vs, GL_COMPILE_STATUS, &params );
	if ( GL_TRUE != params ) {
		fprintf( stderr, "ERROR: GL shader index %i did not compile\n", vs );
		print_shader_info_log( vs );
		return 1; // or exit or something
	}

	GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
	p = (const GLchar *)fragment_shader;
	glShaderSource( fs, 1, &p, NULL );
	glCompileShader( fs );

	// check for compile errors
	glGetShaderiv( fs, GL_COMPILE_STATUS, &params );
	if ( GL_TRUE != params ) {
		fprintf( stderr, "ERROR: GL shader index %i did not compile\n", fs );
		print_shader_info_log( fs );
		return 1; // or exit or something
	}

	GLuint shader_programme = glCreateProgram();
	glAttachShader( shader_programme, fs );
	glAttachShader( shader_programme, vs );
	glLinkProgram( shader_programme );

	glGetProgramiv( shader_programme, GL_LINK_STATUS, &params );
	if ( GL_TRUE != params ) {
		fprintf( stderr, "ERROR: could not link shader programme GL index %i\n",
						 shader_programme );
		print_programme_info_log( shader_programme );
		return false;
	}

	/*--------------------------create camera
	 * matrices----------------------------*/
	/* create PROJECTION MATRIX */
	// input variables
	float near = 0.1f;									// clipping plane
	float far = 100.0f;									// clipping plane
	float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 degrees to radians
	float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
	// matrix components
	float range = tan( fov * 0.5f ) * near;
	float Sx = ( 2.0f * near ) / ( range * aspect + range * aspect );
	float Sy = near / range;
	float Sz = -( far + near ) / ( far - near );
	float Pz = -( 2.0f * far * near ) / ( far - near );
	GLfloat proj_mat[] = { Sx,	 0.0f, 0.0f, 0.0f,	0.0f, Sy,		0.0f, 0.0f,
												 0.0f, 0.0f, Sz,	 -1.0f, 0.0f, 0.0f, Pz,		0.0f };

	/* create VIEW MATRIX */
	float cam_speed = 1.0f;			 // 1 unit per second
	float cam_yaw_speed = 10.0f; // 10 degrees per second
	float cam_pos[] = { 0.0f, 0.0f,
											2.0f }; // don't start at zero, or we will be too close
	float cam_yaw = 0.0f;				// y-rotation in degrees
	mat4 T =
		translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
	mat4 R = rotate_y_deg( identity_mat4(), -cam_yaw );
	mat4 view_mat = R * T;

	/* get location numbers of matrices in shader programme */
	GLint view_mat_location = glGetUniformLocation( shader_programme, "view" );
	GLint proj_mat_location = glGetUniformLocation( shader_programme, "proj" );
	/* use program (make current in state machine) and set default matrix values*/
	glUseProgram( shader_programme );
	glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
	glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat );

	/*------------------------------rendering
	 * loop--------------------------------*/
	/* some rendering defaults */
	glEnable( GL_CULL_FACE ); // cull face
	glCullFace( GL_BACK );		// cull back face
	glFrontFace( GL_CW );			// GL_CCW for counter clock-wise
	double t_inicial = glfwGetTime();
	int cont_fps = 0;
	float fps_final = 0;
	while ( !glfwWindowShouldClose( g_window ) ) {
		static double previous_seconds = glfwGetTime();
		double current_seconds = glfwGetTime();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;

		_update_fps_counter( g_window );
		// wipe the drawing surface clear
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glViewport( 0, 0, g_gl_width, g_gl_height );

		glUseProgram( shader_programme );
		glBindVertexArray( vao );
		// draw points 0-3 from the currently bound VAO with current in-use shader
		//glDrawArrays( GL_TRIANGLES, 0, 3 );
		glDrawElements(GL_TRIANGLES, ntri*3, GL_UNSIGNED_INT, 0);
		// update other events like input handling
		glfwPollEvents();

		/*-----------------------------move camera
		 * here-------------------------------*/
		// control keys
		bool cam_moved = false;
		if ( glfwGetKey( g_window, GLFW_KEY_A ) ) {
			cam_pos[0] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_D ) ) {
			cam_pos[0] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_PAGE_UP ) ) {
			cam_pos[1] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_PAGE_DOWN ) ) {
			cam_pos[1] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_W ) ) {
			cam_pos[2] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_S ) ) {
			cam_pos[2] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
			cam_yaw += cam_yaw_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
			cam_yaw -= cam_yaw_speed * elapsed_seconds;
			cam_moved = true;
		}
		/* update view matrix */
		if ( cam_moved ) {
			mat4 T = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1],
																								 -cam_pos[2] ) ); // cam translation
			mat4 R = rotate_y_deg( identity_mat4(), -cam_yaw );					//
			mat4 view_mat = R * T;
			glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
		}

		if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( g_window, 1 );
		}
		// put the stuff we've been drawing onto the display
		
		glfwSwapBuffers( g_window );
		cont_fps++;
	}
	double t_final = glfwGetTime();
	t_final= t_final - t_inicial;
	printf("tiempo final %f\n", t_final);
	fps_final = cont_fps/t_final;
	printf("fps :%f\n", fps_final);
	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;


}
