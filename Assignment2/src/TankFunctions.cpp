
#include "TankFunctions.h"  //Header file


/**
* @brief Tank_Initialize: initialize tank object
*
* @param (void)
*/
void Tank::Tank_Initialize()
{
    //Read input file
    parser = new CS250Parser;
    parser->LoadDataFromFile("input.txt");

    //Set viewport size
    view_width = parser->right - parser->left;
    view_height = parser->top - parser->bottom;

    camera_position = parser->position;
    camera_view = parser->view;
    camera_up = parser->up;

    //Number of faces per cube and number of vertices per face
    max_faces = parser->faces.size();
    TOTAL_obj = parser->objects.size();

    //Get view matrix
    Viewport_Transformation();
    RootedCamera();

    //Get the color of each face
    //They are the same for all the cubes
    for (int j = 0; j < max_faces; j++)
    {
        //Normalize color
        color[j] = parser->colors[j];
        color[j].r = color[j].r / 255;
        color[j].g = color[j].g / 255;
        color[j].b = color[j].b / 255;
    }
}


/**
* @brief Tank_Update: renders the current state of the tank
*
* @param (void)
*/
void Tank::Tank_Update()
{
    //Get inputs from the user
    draw_mode_solid = GetInput();
    
    if (camera_persp == first)
        FirstPersonCamera();
    else if (camera_persp == third)
        ThirdPersonCamera();

    //Calculate the new state of each object
    for (int obj = 0; obj < TOTAL_obj; obj++)
    {     
        //Need to calculate the model to world matrix first
        //Because it is the same for the whole object
        Matrix4 m2w = ModelToWorld(parser->objects[obj], true);
        Matrix4 w2c = WorldToCamera_GRM();

        //Vertices of the cube
        for (int i = 0; i < max_faces; i++)
        {
            auto face = parser->faces[i];
            Rasterizer::Vertex vtx[3];      //Each vertex of the triangle

            for (int j = 0; j < 3; j++)
            {
                //Get vertices: color
                vtx[j].color = color[i];

                //Get vertices: position
                vtx[j].position = parser->vertices[face.indices[j]];

                //Transform vertices: perspective division and model to world
                vtx[j].position = persp_transf * w2c * m2w * vtx[j].position;

                //Transform vertices:: perspective division
                vtx[j].position.x = vtx[j].position.x / vtx[j].position.w;
                vtx[j].position.y = vtx[j].position.y / vtx[j].position.w;
                vtx[j].position.z = vtx[j].position.z / vtx[j].position.w;
                vtx[j].position.w = vtx[j].position.w / vtx[j].position.w;

                //Transform vertices:: view transformation
                vtx[j].position = viewport * vtx[j].position;
            }

            //Draw the object
            if (draw_mode_solid)
                Rasterizer::DrawTriangleSolid(vtx[0], vtx[1], vtx[2]);
            else
            {
                //Every line composing the triangle
                Rasterizer::DrawMidpointLine(vtx[0], vtx[1]);
                Rasterizer::DrawMidpointLine(vtx[1], vtx[2]);
                Rasterizer::DrawMidpointLine(vtx[2], vtx[0]);
            }
        }

    }

}


/**
* @brief Viewport_Transformation: calculate the viewport transformation matrix
*
* @param (void)
*/
void Tank::Viewport_Transformation()
{
    //Viewport transformation
    viewport.Identity();
    viewport.m[0][0] =  WIDTH / view_width;
    viewport.m[0][3] =  WIDTH / 2.f;
    viewport.m[1][1] = -HEIGHT / view_height;
    viewport.m[1][3] =  HEIGHT / 2.f;

}


/**
* @brief Perspective_Projection: calculate the perspective projection matrix
*
* @param (void)
*/
void Tank::Perspective_Transform()
{
    float a = static_cast<float>(WIDTH) / HEIGHT;

    //Perspective projectio//n
    persp_transf.m[0][0] = parser->distance / view_width;       //parser->focal;//
    persp_transf.m[1][1] = a * parser->distance / view_width;   //parser->focal;//
    persp_transf.m[2][2] = (-parser->nearPlane - parser->farPlane) / (parser->farPlane - parser->nearPlane);
    persp_transf.m[2][2] = (-2 * parser->nearPlane * parser->farPlane) / (parser->farPlane - parser->nearPlane);
    persp_transf.m[3][2] = -1;

    if (camera_persp == rooted)
    {
        Matrix4 persp_transf2;
        persp_transf2.Identity();
        persp_transf2.m[3][2] = -1 / parser->focal;
        persp_transf2.m[3][3] = 0;
    }
}



void Tank::FirstPersonCamera()
{
    CS250Parser::Transform* turret = FindObject("turret");
    CS250Parser::Transform* joint = FindObject("joint");
    CS250Parser::Transform* body = FindObject("body");

    float angle = turret->rot.y + body->rot.y;

    camera_position = turret->pos;

    Vector4 turret_Z;
    turret_Z.z = sin(angle);
    turret_Z.y = sin(turret->rot.y);
    turret_Z.x = cos(angle);
    
    camera_view = turret_Z;
    camera_up = Vector4(0,1,0);

    //camera_right = camera_view.Cross(camera_up);

    Perspective_Transform();
}

void Tank::RootedCamera()
{
    camera_position = parser->position;
    camera_view = parser->view;
    camera_up = parser->up;

    Perspective_Transform();
}

void Tank::ThirdPersonCamera()
{
    CS250Parser::Transform *body_ = FindObject("body");
    CS250Parser::Transform *joint_ = FindObject("joint");

    Vector4 body_Z, body_Y;

    float angle_bodyY = body_->rot.y;
    body_Z.x = cos(angle_bodyY);
    body_Z.y = 0;                 //??
    body_Z.z = sin(angle_bodyY);

    float angle_jointX = joint_->rot.x;
    body_Y.x = cos(angle_jointX);
    body_Y.y = sin(angle_jointX); 
    body_Y.z = 0;                    //??

    Vector4 tank_fwd = body_Z;
    Vector4 tank_up = body_Y;


    camera_position = body_->pos - tank_fwd*parser->distance + tank_up*parser->height;
    camera_view = (body_->pos - camera_position) / (body_->pos - camera_position).Length();
    
    Vector4 camera_right = camera_view.Cross(camera_up);
    camera_up = camera_right.Cross(camera_view);

    Perspective_Transform();
}


Matrix4 Tank::WorldToCamera_GRM()
{
    //Translation
    Matrix4 Transl;
    {
        Transl.Identity();
        Transl.m[0][3] = -camera_position.x;
        Transl.m[1][3] = -camera_position.y;
        Transl.m[2][3] = -camera_position.z;
    }

    //Rotation
    Matrix4 Rot, RotX, RotY, RotZ;
    {
        //Rotation y-axis
        float length_Y = sqrt((camera_view.x * camera_view.x) + (camera_view.z * camera_view.z));
        float cos_Y =  camera_view.z / length_Y;
        float sin_Y = -camera_view.x / length_Y;

        RotY.Identity();
        RotY.m[0][0] = cos_Y;
        RotY.m[0][2] = sin_Y;
        RotY.m[2][0] = -sin_Y;
        RotY.m[2][2] = cos_Y;


        //Rotation x-axis
        float length_X = sqrt((camera_view.x * camera_view.x) + (camera_view.y * camera_view.y) + (camera_view.z * camera_view.z));
        float cos_X = -length_Y / length_X;
        float sin_X = -camera_view.y / length_X;

        RotX.Identity();
        RotX.m[1][1] = cos_X;
        RotX.m[1][2] = -sin_X;
        RotX.m[2][1] = sin_X;
        RotX.m[2][2] = cos_X;


        Vector4 up_prime = RotX * RotY * Transl * camera_up;

        //Rotation z-axis
        float length_Z = sqrt((up_prime.x * up_prime.x) + (up_prime.y * up_prime.y));
        float l = up_prime.Length();
        float cos_Z = up_prime.y / length_Z;
        float sin_Z = up_prime.x / length_Z;

        RotZ.Identity();
        RotZ.m[0][0] = cos_Z;
        RotZ.m[0][1] = -sin_Z;
        RotZ.m[1][0] = sin_Z;
        RotZ.m[1][1] = cos_Z;

        //Concatenate rotations
        Rot = RotZ * RotX * RotY;
    }

    Matrix4 w2c = RotZ * RotX * RotY * Transl;
    return w2c;
}


/**
* @brief ModelToWorld:  calculate the model to world matrix of the object
*
* @param obj:           object to find
* @return               the transform of the found object
*/
CS250Parser::Transform* Tank::FindObject(std::string obj)
{
    for (int i = 0; i < TOTAL_obj; i++)
    {
        //Find the object
        if (!strcmp(obj.c_str(), parser->objects[i].name.c_str()))
            return &parser->objects[i];
    }

    //If it is never found
    return nullptr;
}


/**
* @brief ModelToWorld:  calculate the model to world matrix of the object
*
* @param obj:           object to calculate the matrix for
* @param scale:         whether to calculate the scale
* @return               model to world matrix
*/
Matrix4 Tank::ModelToWorld(CS250Parser::Transform obj, bool scale)
{
    //Translation
    Matrix4 Transl;
    {
        Transl.Identity();
        Transl.m[0][3] = obj.pos.x;
        Transl.m[1][3] = obj.pos.y;
        Transl.m[2][3] = obj.pos.z;
    }

    //Rotation
    Matrix4 Rot, RotX, RotY, RotZ;
    Vector4 angle = obj.rot;
    {
        //Rotation x-axis
        RotX.Identity();
        RotX.m[1][1] = cos(angle.x);
        RotX.m[1][2] = -sin(angle.x);
        RotX.m[2][1] = sin(angle.x);
        RotX.m[2][2] = cos(angle.x);

        //Rotation y-axis
        RotY.Identity();
        RotY.m[0][0] = cos(angle.y);
        RotY.m[0][2] = sin(angle.y);
        RotY.m[2][0] = -sin(angle.y);
        RotY.m[2][2] = cos(angle.y);

        //Rotation z-axis
        RotZ.Identity();
        RotZ.m[0][0] = cos(angle.z);
        RotZ.m[0][1] = -sin(angle.z);
        RotZ.m[1][0] = sin(angle.z);
        RotZ.m[1][1] = cos(angle.z);

        //Concatenate rotations
        Rot = RotZ * RotY * RotX;
    }

    //Scale
    Matrix4 Scale;
    Scale.Identity();

    if (scale)
    {
        Scale.m[0][0] = obj.sca.x;
        Scale.m[1][1] = obj.sca.y;
        Scale.m[2][2] = obj.sca.z;
    }

    //Complete concatenation for the m2w matrix
    Matrix4 m2w = Transl * Rot * Scale;

    //If there is a parent, multiply its the M2W matrix
    if (strcmp(obj.parent.c_str(), "None"))
    {
        CS250Parser::Transform* parent = FindObject(obj.parent);

        if(parent)
            m2w = ModelToWorld(*parent, false) * m2w;
    }

    return m2w;
}


/**
* @brief GetInput:  change tank with input from user
*
* @return           whether to draw the tank as solid
*/
bool Tank::GetInput()
{
    //Tank body rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        FindObject("body")->rot.y += 0.05f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        FindObject("body")->rot.y -= 0.05f;


    //Turret rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        FindObject("turret")->rot.y += 0.05f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        FindObject("turret")->rot.y -= 0.05f;


    //Gun rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
        FindObject("joint")->rot.x += 0.05f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        FindObject("joint")->rot.x -= 0.05f;


    //Move tank forward
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        //Move body
        CS250Parser::Transform* body = FindObject("body");
        body->pos.z += 1.f * cos(body->rot.y);
        body->pos.x += 1.f * sin(body->rot.y);

        //Turn wheels
        FindObject("wheel1")->rot.x += 0.1f;
        FindObject("wheel2")->rot.x += 0.1f;
        FindObject("wheel3")->rot.x += 0.1f;
        FindObject("wheel4")->rot.x += 0.1f;

    }

    //Check solid/wireframe mode
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
        return false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
        return true;

    //Switch camera mode
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
        camera_persp = first;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4))
    {
        camera_persp = rooted;
        RootedCamera();         //Don't need to call it every time
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5))
        camera_persp = third;


    //Camera distance
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
        parser->distance--;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
        parser->distance++;


    //Camera height
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::H))
        parser->height--;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
        parser->height++;



    return draw_mode_solid;
}
