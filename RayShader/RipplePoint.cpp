#include "RipplePoint.h"

//CONSTRUCTOR
RipplePoint::RipplePoint(const glm::vec2& position, float amplitude,
	float decay, float frequency, float speed) :
	m_Position(position),
	m_Amplitude(amplitude),
	m_Decay(decay),
	m_Frequency(frequency),
	m_Speed(speed),
	m_Time(-0.3f) {
}

//DESTRUCTOR
RipplePoint::~RipplePoint() {
}

//PUBLIC MEMBER FUNCTIONS
float RipplePoint::computeHeight(const glm::vec2& point) const {

	float dis = glm::distance(m_Position, point) -
		(m_Time * m_Speed);

	float ampDecay = m_Amplitude * ((5.5 - m_Time) / 5.5f);

	return ampDecay * exp(m_Decay * (dis * dis)) *
		cos(m_Frequency * dis);
}

void RipplePoint::update(float deltaTime) {

	m_Time += deltaTime;
}

bool RipplePoint::shouldRemove() const {

	return m_Time > 8.0f;
}
