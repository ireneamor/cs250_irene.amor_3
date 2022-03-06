/****************************************************************************************/
/*!
\file   TankFunctions.h
\author Irene Amor Mendez
\par    email: irene.amor@digipen.edu
\par    DigiPen login: irene.amor
\par    Course: CS250
\par    Assignment #2
\date   11/02/2022
\brief

This file contains the implementation of the following class functions for the
Tank assignment.
Functions include:	Tank_Initialize, Viewport_Transformation, Perspective_Transform,
					ModelToWorld, Tank_Update, GetInput

Hours spent on this assignment: ~20

*/
/****************************************************************************************/

#include <SFML/Graphics.hpp>

#include "FrameBuffer.h"		//Frame buffer class
#include "Rasterizer.h"			//Rasterizer class
#include "CS250Parser.h"		//Parser class
#include "Math/Matrix4.h"		//Matrix 4*4 class
#include "Math/Point4.h"		//Point of size 4 class


class Tank
{
public:

	//------------
	//Functions
	//------------

	void Tank_Initialize();							//Initialize tank object
	void Tank_Update();								//Renders the current state of the tank

	void Viewport_Transformation();					//Calculate the viewport transformation matrix
	void Perspective_Transform();					//Calculate the perspective projection matrix

	Matrix4 ModelToWorld(CS250Parser::Transform obj, bool scale = true);
	Matrix4 WorldToCamera_GRM();

	bool GetInput();

	CS250Parser::Transform* FindObject(std::string obj);

	void FirstPersonCamera();
	void RootedCamera();
	void ThirdPersonCamera();


	//------------
	//Variables
	//------------

	const int WIDTH = 1280;			//Window size
	const int HEIGHT = 960;

private:
	float view_width;				//Viewport size
	float view_height;

	size_t max_faces;					//Number of faces per shape

	CS250Parser* parser;			//Parser with input data

	Matrix4 viewport;				//Matrices that only need to be computed once
	Matrix4 persp_transf;
	
	Matrix4 m2w_body;				//Model to world transformation of the body

	Point4 color[12];				//Color of each triangle

	bool draw_mode_solid = true;	//Drawing mode

	//enum obj { body, turret, joint, gun, wheel1, wheel2, wheel3, wheel4, TOTAL };
	size_t TOTAL_obj = 0;

	Point4  camera_position;
	Vector4 camera_view;
	Vector4 camera_up;

	int camera_persp = 0;
	enum camera{first, rooted, third};
};