#pragma once

// Std. Includes
#include <vector>
#include <iostream>

// GL Includes
// #include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef float GLfloat;
typedef unsigned char GLboolean;

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const GLfloat YAW = 45.0f;//从正X转向正Y
const GLfloat PITCH = 45.0f;//从xoy平面转向正Z
const GLfloat SPEED = 2.0f;
const GLfloat ZOOM = 45.0f;
const GLfloat DISTANCE = 4.0f;
const GLfloat SENSITIVTY = 0.4f;
const GLfloat DRAGSSTV = 0.005f;
const GLfloat VIEWPORTHEIGHT = 600;


// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Center;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Eular Angles
	GLfloat Yaw;
	GLfloat Pitch;
	// Camera options
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;
	GLfloat DragSensitivity;
	GLfloat Zoom;
	GLfloat Distance;
	GLfloat ViewportHeight;
	
	// Factors
	GLfloat Factor(){ return (Zoom / ZOOM) * (Distance / DISTANCE) * (VIEWPORTHEIGHT / ViewportHeight); }

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) 
		: Center(glm::vec3(0.0f, 0.0f, 0.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), DragSensitivity(DRAGSSTV), Zoom(ZOOM), Distance(DISTANCE), ViewportHeight(VIEWPORTHEIGHT)
	{
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) 
		: Center(glm::vec3(0.0f, 0.0f, 0.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), DragSensitivity(DRAGSSTV), Zoom(ZOOM), Distance(DISTANCE), ViewportHeight(VIEWPORTHEIGHT)
	{
		this->Position = glm::vec3(posX, posY, posZ);
		this->WorldUp = glm::vec3(upX, upY, upZ);
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->Center + this->Position, this->Center, this->Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->MovementSpeed * deltaTime;
		if (direction == FORWARD)
			this->Distance -= velocity;
		if (direction == BACKWARD)
			this->Distance += velocity;
		if (direction == LEFT)
			this->Center += this->Right * velocity;
		if (direction == RIGHT)
			this->Center -= this->Right * velocity;
		updateCameraVectors();
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
	{
		GLfloat sstv = Factor() * this->MouseSensitivity;
		xoffset *= sstv;
		yoffset *= sstv;

		this->Yaw += xoffset;
		this->Pitch -= yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->Pitch > 89.0f)
				this->Pitch = 89.0f;
			if (this->Pitch < -89.0f)
				this->Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
	}

	void ProcessMouseDragging(GLfloat xoffset, GLfloat yoffset)
	{
		GLfloat sstv = Factor() * this->DragSensitivity;

		xoffset *= sstv;
		yoffset *= sstv;

		glm::vec3 Back(Position.x, Position.y, 0.0f);
		Back = glm::normalize(Back);
		glm::vec3 Left(-Right);

		Center += xoffset*Left + yoffset*Back;

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(GLfloat yoffset)
	{
		GLfloat min = 1.0f, max = 90.0f, sstv = 0.03f;
		if (this->Zoom >= min && this->Zoom <= max)
			this->Zoom -= yoffset * sstv;
		if (this->Zoom <= min)
			this->Zoom = min;
		if (this->Zoom >= max)
			this->Zoom = max;
	}

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 direct = glm::normalize(glm::vec3(
			sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch)),
			cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch)),
			sin(glm::radians(this->Pitch))
			));
		this->Position = Distance * direct;
		// Also re-calculate the Right and Up vector
		this->Right = glm::normalize(glm::cross(-direct, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->Up = glm::normalize(glm::cross(this->Right, -direct));
	}
};
