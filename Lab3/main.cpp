#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>

using namespace glm;

// --- 基础结构定义 ---
struct Ray {
    vec3 origin, direction;
};

struct Material {
    vec3 color;
    float specular;
    float reflect;
};

struct HitRecord {
    float t;
    vec3 p, normal;
    Material mat;
};

// --- 场景物体定义 ---
struct Sphere {
    vec3 center;
    float radius;
    Material mat;

    bool intersect(const Ray& r, float& t_out) const {
        vec3 oc = r.origin - center;
        float a = dot(r.direction, r.direction);
        float b = 2.0f * dot(oc, r.direction);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return false;
        float t = (-b - sqrt(discriminant)) / (2.0f * a);
        if (t < 0.001f) t = (-b + sqrt(discriminant)) / (2.0f * a);
        if (t < 0.001f) return false;
        t_out = t;
        return true;
    }
};

struct Plane {
    vec3 point, normal;
    Material mat;

    bool intersect(const Ray& r, float& t_out) const {
        float denom = dot(normal, r.direction);
        if (abs(denom) > 1e-6) {
            float t = dot(point - r.origin, normal) / denom;
            if (t >= 0.001f) {
                t_out = t;
                return true;
            }
        }
        return false;
    }
};

// --- 全局变量 ---
const int W = 600, H = 600;
float pixelBuffer[W * H * 3];
vec3 camPos = vec3(0, 1, 5);
vec3 lightPos = vec3(2, 4, 0);
bool needsUpdate = true; // 效率优化标记

std::vector<Sphere> spheres;
std::vector<Plane> planes;

// --- 核心光线追踪逻辑 ---
vec3 trace(const Ray& r, int depth) {
    if (depth > 3) return vec3(0, 0, 0); // 递归深度限制

    float closest_t = 1e20f;
    HitRecord rec;
    bool hitAnything = false;

    // 1. 与所有物体求交
    for (auto& s : spheres) {
        float t;
        if (s.intersect(r, t) && t < closest_t) {
            closest_t = t;
            rec.t = t; rec.p = r.origin + t * r.direction;
            rec.normal = normalize(rec.p - s.center);
            rec.mat = s.mat; hitAnything = true;
        }
    }
    for (auto& p : planes) {
        float t;
        if (p.intersect(r, t) && t < closest_t) {
            closest_t = t;
            rec.t = t; rec.p = r.origin + t * r.direction;
            rec.normal = p.normal;
            rec.mat = p.mat; hitAnything = true;
        }
    }

    if (!hitAnything) return vec3(0.05, 0.05, 0.1); // 背景色

    // 2. 光照计算 (Blinn-Phong)
    vec3 lightDir = normalize(lightPos - rec.p);
    vec3 viewDir = normalize(camPos - rec.p);
    
    // 阴影射线 (Shadow Ray)
    Ray shadowRay = { rec.p + rec.normal * 0.001f, lightDir };
    bool inShadow = false;
    float shadow_t;
    for (auto& s : spheres) if (s.intersect(shadowRay, shadow_t)) inShadow = true;
    for (auto& p : planes) if (p.intersect(shadowRay, shadow_t) && shadow_t < length(lightPos - rec.p)) inShadow = true;

    vec3 ambient = 0.2f * rec.mat.color;
    if (inShadow) return ambient;

    float diff = max(dot(rec.normal, lightDir), 0.0f);
    vec3 diffuse = diff * rec.mat.color;

    vec3 reflectDir = reflect(-lightDir, rec.normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
    vec3 specular = spec * vec3(1.0f) * rec.mat.specular;

    // 递归反射
    if (rec.mat.reflect > 0) {
        Ray nextRay = { rec.p + rec.normal * 0.001f, reflect(-viewDir, rec.normal) };
        return mix(ambient + diffuse + specular, trace(nextRay, depth + 1), rec.mat.reflect);
    }

    return ambient + diffuse + specular;
}

// --- 渲染渲染循环 ---
void render() {
    if (!needsUpdate) return;
    
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            // 生成从摄像机出发的射线
            float u = (x - W / 2.0f) / H;
            float v = (y - H / 2.0f) / H;
            Ray r = { camPos, normalize(vec3(u, v, -1)) };

            vec3 color = trace(r, 0);
            int idx = (y * W + x) * 3;
            pixelBuffer[idx] = color.r;
            pixelBuffer[idx + 1] = color.g;
            pixelBuffer[idx + 2] = color.b;
        }
    }
    needsUpdate = false;
    std::cout << "Scene Refreshed (Ray Tracing Done)." << std::endl;
}

// --- GLUT 回调 ---
void display() {
    render();
    glClear(GL_COLOR_BUFFER_BIT);
    // 核心：直接绘制像素阵列，绕过 OpenGL 3D 渲染管线
    glDrawPixels(W, H, GL_RGB, GL_FLOAT, pixelBuffer);
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    float step = 0.5f;
    if (key == 'w') camPos.z -= step;
    if (key == 's') camPos.z += step;
    if (key == 'a') camPos.x -= step;
    if (key == 'd') camPos.x += step;
    if (key == 'i') lightPos.y += step; // 调节光源
    if (key == 'k') lightPos.y -= step;
    if (key == 'j') lightPos.x -= step;
    if (key == 'l') lightPos.x += step;
    needsUpdate = true;
    glutPostRedisplay();
}

void initScene() {
    // 材质定义
    Material whiteWall = { vec3(0.9, 0.9, 0.9), 0.0, 0.0 };
    Material brownFloor = { vec3(0.4, 0.2, 0.1), 0.0, 0.0 };
    Material redSphere = { vec3(1.0, 0.2, 0.2), 0.8, 0.3 };
    Material blueSphere = { vec3(0.2, 0.2, 1.0), 0.8, 0.3 };

    // 场景布置
    spheres.push_back({ vec3(-0.8, -0.5, -3.5), 0.5f, redSphere });
    spheres.push_back({ vec3(0.7, -0.4, -4.5), 0.6f, blueSphere });
    
    planes.push_back({ vec3(0, -1, 0), vec3(0, 1, 0), brownFloor }); // 地面
    planes.push_back({ vec3(0, 0, -6), vec3(0, 0, 1), whiteWall });  // 后墙
    planes.push_back({ vec3(-2.5, 0, 0), vec3(1, 0, 0), whiteWall }); // 左墙
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(W, H);
    glutCreateWindow("Lab 3: Manual Ray Tracing Scene");

    initScene();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    std::cout << "Ray Tracing Controls:" << std::endl;
    std::cout << "[WASD] Move Camera (Observe)" << std::endl;
    std::cout << "[IJKL] Move Light Source" << std::endl;

    glutMainLoop();
    return 0;
}