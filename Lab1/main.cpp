#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

// --- 数据结构 ---
struct Vertex {
    float x, y, z;
};

struct Face {
    std::vector<int> vIndices;
};

// --- 全局变量 ---
std::vector<Vertex> g_vertices;
std::vector<Face> g_faces;

// 交互控制
float g_rotX = 20.0f, g_rotY = 45.0f; 
float g_transX = 0.0f, g_transY = 0.0f;
float g_zoom = 1.2f;
int g_lastMouseX, g_lastMouseY;
int g_activeButton = -1;

// --- 1. 鲁棒的 OBJ 模型加载器 ---
bool loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    g_vertices.clear();
    g_faces.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            Vertex v;
            ss >> v.x >> v.y >> v.z;
            g_vertices.push_back(v);
        } 
        else if (prefix == "f") {
            Face f;
            std::string segment;
            while (ss >> segment) {
                // 处理 "v/vt/vn" 格式，只取第一个 '/' 之前的索引
                size_t firstSlash = segment.find('/');
                int vIdx = std::stoi(segment.substr(0, firstSlash));
                f.vIndices.push_back(vIdx - 1);
            }
            g_faces.push_back(f);
        }
    }
    std::cout << "Successfully loaded: " << g_vertices.size() << " vertices." << std::endl;
    return true;
}

// --- 2. 法向量计算 (实现真实感光照的关键) ---
void calculateAndSetNormal(const Face& f) {
    if (f.vIndices.size() < 3) return;
    Vertex v1 = g_vertices[f.vIndices[0]];
    Vertex v2 = g_vertices[f.vIndices[1]];
    Vertex v3 = g_vertices[f.vIndices[2]];

    // 向量叉乘
    float ax = v2.x - v1.x, ay = v2.y - v1.y, az = v2.z - v1.z;
    float bx = v3.x - v1.x, by = v3.y - v1.y, bz = v3.z - v1.z;
    float nx = ay * bz - az * by;
    float ny = az * bx - ax * bz;
    float nz = ax * by - ay * bx;

    float len = sqrt(nx * nx + ny * ny + nz * nz);
    if (len > 0) glNormal3f(nx / len, ny / len, nz / len);
}

// --- 3. 环境初始化 ---
void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // 重要：开启法线自动归一化，防止 glScalef 导致模型变白
    glEnable(GL_NORMALIZE); 

    // 材质设置：青铜质感
    GLfloat mat_ambient[]   = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat mat_diffuse[]   = { 0.8f, 0.4f, 0.1f, 1.0f };
    GLfloat mat_specular[]  = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat mat_shininess[] = { 60.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // 光源位置
    GLfloat light_pos[] = { 5.0f, 5.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f); // 深蓝色背景
}

// --- 4. 主渲染回调 ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 摄像机位置
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);

    // --- 交互变换 (顺序：平移 -> 旋转 -> 缩放) ---
    glTranslatef(g_transX, g_transY, 0.0f);
    glRotatef(g_rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(g_rotY, 0.0f, 1.0f, 0.0f);
    glScalef(g_zoom, g_zoom, g_zoom);

    // 仓库未附带 OBJ 模型时，使用 GLUT 内置茶壶。
    if (g_faces.empty()) {
        glutSolidTeapot(1.0);
    } else {
        for (const auto& f : g_faces) {
            calculateAndSetNormal(f);
            glBegin(GL_POLYGON);
            for (int idx : f.vIndices) {
                glVertex3f(g_vertices[idx].x, g_vertices[idx].y, g_vertices[idx].z);
            }
            glEnd();
        }
    }

    glutSwapBuffers();
}

// --- 5. 鼠标与交互逻辑 ---
void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        g_activeButton = button;
        g_lastMouseX = x;
        g_lastMouseY = y;
    } else {
        g_activeButton = -1;
    }

    // 滚轮缩放支持
    if (button == 3) g_zoom *= 1.1f;
    if (button == 4) g_zoom *= 0.9f;

    glutPostRedisplay();
}

void motion(int x, int y) {
    int dx = x - g_lastMouseX;
    int dy = y - g_lastMouseY;

    if (g_activeButton == GLUT_LEFT_BUTTON) {
        // 左键控制旋转
        g_rotY += dx * 0.4f;
        g_rotX += dy * 0.4f;
    } 
    else if (g_activeButton == GLUT_RIGHT_BUTTON) {
        // 右键控制平移
        g_transX += dx * 0.008f;
        g_transY -= dy * 0.008f;
    }

    g_lastMouseX = x;
    g_lastMouseY = y;
    glutPostRedisplay();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    const std::string modelPath = argc > 1 ? argv[1] : "";
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Computer Graphics Lab 1 - Perfect Teapot");

    // 可以传入 OBJ 文件路径；默认使用 GLUT 内置茶壶。
    if (!modelPath.empty() && !loadOBJ(modelPath)) {
        std::cerr << "Cannot load OBJ model: " << modelPath << std::endl;
        return 1;
    }

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    std::cout << "--- Interaction Controls ---" << std::endl;
    std::cout << "Left Mouse:  Rotate" << std::endl;
    std::cout << "Right Mouse: Translate (Pan)" << std::endl;
    std::cout << "Scroll/Mid:  Zoom" << std::endl;

    glutMainLoop();
    return 0;
}
