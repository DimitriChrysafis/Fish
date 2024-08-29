#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

using namespace std;
using namespace sf;

const int width = 800;
const int height = 600;
const float fishScale = 0.3;
const Color black = Color(0, 0, 0);
const Color white = Color(255, 255, 255);
const Color bodyColor = Color(58, 124, 165);
const Color finColor = Color(129, 195, 215);

class Chain {
public:
    vector<Vector2f> joints;
    vector<float> angles;
    float segmentLength;

    Chain(Vector2f origin, int numSegments, float segmentLength)
            : segmentLength(segmentLength) {
        for (int i = 0; i < numSegments; ++i) {
            joints.push_back(origin);
            angles.push_back(0.0f);
        }
    }

    void resolve(Vector2f targetPos) {
        joints[0] = targetPos;
        for (int i = 1; i < joints.size(); ++i) {
            Vector2f direction = joints[i] - joints[i - 1];
            if (sqrt(direction.x * direction.x + direction.y * direction.y) > 0) {
                float len = sqrt(direction.x * direction.x + direction.y * direction.y);
                direction /= len;
                direction *= segmentLength;
            }
            joints[i] = joints[i - 1] + direction;
            angles[i] = atan2(direction.y, direction.x);
        }
    }
};

class Fish {
public:
    Chain spine;
    vector<float> bodyWidth;

    Fish(Vector2f origin)
            : spine(origin, 12, 64 * fishScale) {
        float bodyWidths[] = {34, 41, 42, 41, 38, 32, 26, 19, 16, 9};
        for (int i = 0; i < 10; ++i) {
            bodyWidth.push_back(bodyWidths[i] * fishScale);
        }
    }

    void resolve(Vector2f mousePos) {
        Vector2f headPos = spine.joints[0];
        Vector2f direction = mousePos - headPos;
        float len = sqrt(direction.x * direction.x + direction.y * direction.y);
        if (len > 0) {
            direction /= len;
        }
        Vector2f targetPos = headPos + direction * (16 * fishScale);
        spine.resolve(targetPos);
    }

    void display(RenderWindow& window) {
        window.clear(black);

        Vector2f* j = &spine.joints[0];
        float* a = &spine.angles[0];

        float headToMid1 = relativeAngleDiff(a[0], a[6]);
        float headToMid2 = relativeAngleDiff(a[0], a[7]);
        float headToTail = headToMid1 + relativeAngleDiff(a[6], a[11]);

        drawFin(window, 3, M_PI / 3, 160 * fishScale, 64 * fishScale, a[2] - M_PI / 4);
        drawFin(window, 3, -M_PI / 3, 160 * fishScale, 64 * fishScale, a[2] + M_PI / 4);
        drawFin(window, 7, M_PI / 2, 96 * fishScale, 32 * fishScale, a[6] - M_PI / 4);
        drawFin(window, 7, -M_PI / 2, 96 * fishScale, 32 * fishScale, a[6] + M_PI / 4);

        drawCaudalFin(window, j, a, headToTail);
        drawBody(window, j, a);
        drawDorsalFin(window, j, a, headToMid1, headToMid2);

        drawEye(window, j[0], a[0], M_PI / 2, -18 * fishScale);
        drawEye(window, j[0], a[0], -M_PI / 2, -18 * fishScale);

        window.display();
    }

private:
    void drawFin(RenderWindow& window, int index, float angleOffset, float width, float height, float rotation) {
        Vector2f pos = getPos(index, angleOffset, 0);
        RectangleShape fin(Vector2f(width, height));
        fin.setFillColor(finColor);
        fin.setOrigin(width / 2, height / 2);
        fin.setPosition(pos);
        fin.setRotation(rotation * 180 / M_PI);
        window.draw(fin);
    }

    void drawCaudalFin(RenderWindow& window, Vector2f* j, float* a, float headToTail) {
        vector<Vector2f> points;
        for (int i = 8; i < 12; ++i) {
            float tailWidth = 1.5f * headToTail * (i - 8) * (i - 8) * fishScale;
            points.push_back(getVertex(j[i], a[i], tailWidth, -M_PI / 2));
        }
        for (int i = 11; i >= 8; --i) {
            float tailWidth = max(-13.0f, min(13.0f, headToTail * 6)) * fishScale;
            points.push_back(getVertex(j[i], a[i], tailWidth, M_PI / 2));
        }
        drawPolygon(window, points, bodyColor, 2);
    }

    void drawBody(RenderWindow& window, Vector2f* j, float* a) {
        vector<Vector2f> points;
        for (int i = 0; i < 10; ++i) {
            points.push_back(getVertex(j[i], a[i], bodyWidth[i], M_PI / 2));
        }
        points.push_back(getVertex(j[9], a[9], bodyWidth[9], M_PI));
        for (int i = 9; i >= 0; --i) {
            points.push_back(getVertex(j[i], a[i], bodyWidth[i], -M_PI / 2));
        }
        points.push_back(getVertex(j[0], a[0], bodyWidth[0], -M_PI / 6));
        points.push_back(getVertex(j[0], a[0], bodyWidth[0], 0));
        points.push_back(getVertex(j[0], a[0], bodyWidth[0], M_PI / 6));
        drawPolygon(window, points, bodyColor, 2);
    }

    void drawDorsalFin(RenderWindow& window, Vector2f* j, float* a, float headToMid1, float headToMid2) {
        vector<Vector2f> points = {
                j[4],
                j[7],
                Vector2f(j[6].x + cos(a[6] + M_PI / 2) * headToMid2 * 16 * fishScale,
                         j[6].y + sin(a[6] + M_PI / 2) * headToMid2 * 16 * fishScale),
                Vector2f(j[5].x + cos(a[5] + M_PI / 2) * headToMid1 * 16 * fishScale,
                         j[5].y + sin(a[5] + M_PI / 2) * headToMid1 * 16 * fishScale),
                j[4]
        };
        drawPolygon(window, points, finColor, 2);
    }

    void drawEye(RenderWindow& window, Vector2f pos, float angle, float angleOffset, float lengthOffset) {
        Vector2f eyePos = getVertex(pos, angle, 24 * fishScale, angleOffset, lengthOffset);
        CircleShape eye(12 * fishScale);
        eye.setFillColor(white);
        eye.setPosition(eyePos);
        window.draw(eye);
    }

    Vector2f getVertex(Vector2f pos, float angle, float length, float angleOffset = 0, float lengthOffset = 0) {
        float offsetAngle = angle + angleOffset;
        float offsetLength = length + lengthOffset;
        return Vector2f(pos.x + cos(offsetAngle) * offsetLength,
                        pos.y + sin(offsetAngle) * offsetLength);
    }

    Vector2f getPos(int i, float angleOffset, float lengthOffset) {
        return getVertex(spine.joints[i], spine.angles[i], bodyWidth[i], angleOffset, lengthOffset);
    }

    float relativeAngleDiff(float angle1, float angle2) {
        return atan2(sin(angle2 - angle1), cos(angle2 - angle1));
    }

    void drawPolygon(RenderWindow& window, const vector<Vector2f>& points, const Color& color, float outlineThickness) {
        ConvexShape shape;
        shape.setPointCount(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            shape.setPoint(i, points[i]);
        }
        shape.setFillColor(color);
        shape.setOutlineColor(white);
        shape.setOutlineThickness(outlineThickness);
        window.draw(shape);
    }
};

int main() {
    RenderWindow window(VideoMode(width, height), "A Fish");
    Fish fish(Vector2f(width / 2, height / 2));

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        fish.resolve(Vector2f(Mouse::getPosition(window)));
        fish.display(window);
        sleep(milliseconds(16));
    }

    return 0;
}
