
#include <vector>
#include "glm/glm.hpp"


enum scene {
    SCENE_BEACH,
    SCENE_SURFACE,
    SCENE_WATER,
    SCENE_OBJ,
    SCENE_PORTAL,
    SCENE_SOFT
};

void loadScene(
    scene s,
    std::vector<glm::vec4>  &vertecies,
    std::vector<int>        &triangles,
    std::vector<glm::vec4>  &ball_pos,
    std::vector<float>      &ball_radius
);
