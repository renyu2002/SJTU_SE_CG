#ifndef RAYSHADER_RIPPLEPOINT_H_
#	define RAYSHADER_RIPPLEPOINT_H_

#include "glm/glm.hpp"
#include <iostream>
#include <math.h>

#include "util.h"

class RipplePoint {
public:

	//CONSTRUCTOR
	/*!Creates a new ripple point
	@position the position of the centre of the ripple
	@amplitude the height of the ripples
	@decay the decay rate of the ripples
	@frequency the frequency of the ripples
	@speed the speed of the ripple*/
	RipplePoint(const glm::vec2& position, float amplitude,
		float decay, float frequency, float speed);

	//DESTRUCTOR
	/*!Destroys the ripple point*/
	~RipplePoint();

	//PUBLIC MEMBER FUNCTIONS
	/*!Computes the height for the given point*/
	float computeHeight(const glm::vec2& point) const;

	/*!Updates with the ripple with delta time*/
	void update(float deltaTime);

	/*!@return if the ripple should be removed*/
	bool shouldRemove() const;

private:

	//VARIABLES
	//the position of the ripple point
	glm::vec2 m_Position;

	//the amplitude of the ripples
	float m_Amplitude;
	//the decay rate of the ripples
	float m_Decay;
	//the frequency of the ripples
	float m_Frequency;
	//the speed of the ripple
	float m_Speed;
	//the time passed since the ripple began
	float m_Time;

    //MACROS
    DISALLOW_COPY_AND_ASSIGN(RipplePoint);
};

#endif