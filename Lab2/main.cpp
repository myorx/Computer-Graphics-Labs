#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// --- 实验数据结构 ---
struct Pose {
    glm::vec3 position;
    glm::quat orientation;
};

enum AnimationMode { MODE_TRANSLATION_ONLY, MODE_COMBINED };

// --- 全局变量 ---
Pose g_startPose, g_endPose, g_currentPose;
AnimationMode g_mode = MODE_COMBINED; // 默认演示平移+旋转
float g_animTime = 0.0f;
bool g_isAnimating = false;
float g_animSpeed = 0.006f;

// --- 1. UI 渲染辅助 ---
void renderText(float x, float y, std::string text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
    glEnable(GL_LIGHTING);
}

// --- 2. 初始化姿态 ---
void initPoses() {
    // 起点：左下角，初始姿态（壶嘴向左）
    g_startPose.position = glm::vec3(-2.8f, -1.0f, 0.0f);
    g_startPose.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // 终点：右上角，旋转后的姿态（绕Y轴转180度，壶嘴向右）
    g_endPose.position = glm::vec3(2.8f, 1.5f, 0.0f);
    // 设置一个明显的旋转：180度旋转
    g_endPose.orientation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    g_currentPose = g_startPose;
    g_animTime = 0.0f;
}

// --- 3. 核心插值逻辑 ---
void update(int value) {
    if (g_isAnimating) {
        g_animTime += g_animSpeed;
        if (g_animTime > 1.0f) {
            g_animTime = 1.0f;
            g_isAnimating = false;
        }

        // 所有模式共有的线性平移 (Lerp)
        g_currentPose.position = glm::mix(g_startPose.position, g_endPose.position, g_animTime);

        // 旋转处理
        if (g_mode == MODE_COMBINED) {
            // 模式 2：使用四元数球面插值 (Slerp) 达成最终旋转状态
            g_currentPose.orientation = glm::slerp(g_startPose.orientation, g_endPose.orientation, g_animTime);
        } else {
            // 模式 1：保持起始旋转不变，无法达成最终旋转状态
            g_currentPose.orientation = g_startPose.orientation;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// --- 4. 绘图逻辑 ---
void drawTeapot(Pose p, bool isGhost, glm::vec3 color) {
    glPushMatrix();
    glTranslatef(p.position.x, p.position.y, p.position.z);
    
    // 四元数转矩阵
    glm::mat4 rotMat = glm::toMat4(p.orientation);
    glMultMatrixf(glm::value_ptr(rotMat));

    if (isGhost) {
        glDisable(GL_LIGHTING);
        glLineWidth(1.0f);
        glColor3f(color.r, color.g, color.b);
        glutWireTeapot(0.7); // 虚影
        glEnable(GL_LIGHTING);
    } else {
        GLfloat mat_diffuse[] = { 0.1f, 0.6f, 0.9f, 1.0f }; // 实体茶壶为蓝色
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glutSolidTeapot(0.7);
    }
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 0, 10, 0, 0, 0, 0, 1, 0);

    // 光源设置
    GLfloat lightPos[] = { 5.0f, 5.0f, 10.0f, 1.0f };
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // --- 绘制起始虚影 (红色) ---
    drawTeapot(g_startPose, true, glm::vec3(1.0, 0.3, 0.3));

    // --- 绘制目标终点虚影 (绿色) ---
    // 关键：模式 1 终点不旋转，模式 2 终点显示旋转后的样子
    Pose targetGhost = g_endPose;
    if (g_mode == MODE_TRANSLATION_ONLY) {
        targetGhost.orientation = g_startPose.orientation; // 模式1终点姿态=起点姿态
    }
    drawTeapot(targetGhost, true, glm::vec3(0.3, 1.0, 0.3));

    // --- 绘制运动主体 ---
    drawTeapot(g_currentPose, false, glm::vec3(1, 1, 1));

    // --- 2D UI 界面 ---
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    
    std::string modeTxt = (g_mode == MODE_TRANSLATION_ONLY) ? "MODE 1: Translation ONLY" : "MODE 2: Combined (Translation + SLERP Rotation)";
    renderText(20, 560, modeTxt);
    renderText(20, 530, "--------------------------------------------------");
    renderText(20, 500, "Press [1]: Switch to Translation Only");
    renderText(20, 470, "Press [2]: Switch to Translation + Rotation");
    renderText(20, 440, "Press [Space]: START Animation");
    renderText(20, 410, "Press [R]: Reset to Start");
    
    if(g_mode == MODE_TRANSLATION_ONLY) 
        renderText(20, 50, "Note: Final orientation stays the SAME as start.", GLUT_BITMAP_HELVETICA_12);
    else 
        renderText(20, 50, "Note: Final orientation is REACHED via Quaternion SLERP.", GLUT_BITMAP_HELVETICA_12);

    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

// --- 5. 交互处理 ---
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '1':
            g_mode = MODE_TRANSLATION_ONLY;
            g_animTime = 0.0f; g_isAnimating = false;
            g_currentPose = g_startPose;
            std::cout << "Switched to Mode 1: Translation Only" << std::endl;
            break;
        case '2':
            g_mode = MODE_COMBINED;
            g_animTime = 0.0f; g_isAnimating = false;
            g_currentPose = g_startPose;
            std::cout << "Switched to Mode 2: Translation + Rotation" << std::endl;
            break;
        case ' ':
            g_isAnimating = true;
            if (g_animTime >= 1.0f) g_animTime = 0.0f;
            break;
        case 'r': case 'R':
            g_animTime = 0.0f; g_isAnimating = false;
            g_currentPose = g_startPose;
            break;
    }
    glutPostRedisplay();
}

void initGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0/600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Lab 2: Translation vs. Quaternion Rotation");

    initPoses();
    initGL();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}