#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIDs;   // New bone ID attribute
layout (location = 4) in vec4 aBoneWeights; // New bone weight attribute

const int MAX_BONES = 100;
uniform mat4 bone_transforms[MAX_BONES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 bone_transform = mat4(1.0);
    bone_transform += bone_transforms[aBoneIDs[0]] * aBoneWeights[0];
    bone_transform += bone_transforms[aBoneIDs[1]] * aBoneWeights[1];
    bone_transform += bone_transforms[aBoneIDs[2]] * aBoneWeights[2];
    bone_transform += bone_transforms[aBoneIDs[3]] * aBoneWeights[3];

    gl_Position = projection * view * model * bone_transform * vec4(aPos, 1.0);
}