#include "scene.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "glm/glm.hpp"




void loadScene(
    scene s,
    std::vector<glm::vec4>  &vertecies,
    std::vector<int>        &triangles,
    std::vector<glm::vec4>  &ball_pos,
    std::vector<float>      &ball_radius
) {

    vertecies.clear();
    triangles.clear();
    ball_pos.clear();
    ball_radius.clear();

    const char* filepath;
    switch (s) {
        case SCENE_BEACH:   filepath = "resources/beach.scene";     break;
        case SCENE_SURFACE: filepath = "resources/surface.scene";   break;
        case SCENE_WATER:   filepath = "resources/water.scene";     break;
        case SCENE_OBJ:     filepath = "resources/obj.scene";       break;
        case SCENE_PORTAL:  filepath = "resources/portal.scene";    break;
        case SCENE_SOFT:    filepath = "resources/soft.scene";      break;
    }
    // FILE *fp = fopen(filename, "r");
    // if (fp == NULL)
    //     printf("Error reading %s file\n", filename);

    std::ifstream f(filepath);
    std::string token;

    while(f >> token) {
        glm::vec4 v(0.0, 0.0, 0.0, 1.0);
        switch (token[0]) {
            case 'v':
                f >> v.x >> v.y >> v.z;
                vertecies.push_back(v);
                break;
            case 'f':
                int f0, f1, f2;
                f >> f0 >> f1 >> f2;
                triangles.push_back(f0);
                triangles.push_back(f1);
                triangles.push_back(f2);
                break;
            case 'b':
                float size;
                f >> v.x >> v.y >> v.z >> size;
                ball_pos.push_back(v);
                ball_radius.push_back(size);
            default:
                std::getline(f, token);
                break;
        }
    }
}
