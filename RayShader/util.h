#ifndef RAYSHADER_VALUES_H_
#   define RAYSHADER_VALUES_H_

//is true to debug height mapping
const bool DEBUG_HEIGHT_MAP = false;

//for converting radians and degrees
const float DEGREES_TO_RADIANS = 0.0174532925;
const float RADIANS_TO_DEGREES = 57.2957795;

//pi
const float PI = acos(0.0) * 2.0;

//put in private section of class to disallow construction
#define DISALLOW_CONSTRUCTION(TypeName) \
TypeName();                            \
TypeName(const TypeName&);             \
void operator=(const TypeName&)

//put in private section of class to disallow copy and assignment
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&);             \
void operator=(const TypeName&)

inline bool utilFloatEquals(float a, float b) {

	return (a > b - 0.00001f && a < b + 0.00001f);
}

#endif
