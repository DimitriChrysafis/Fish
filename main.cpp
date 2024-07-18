#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;
const float fishScale = 0.3;
const sf::Color BLACK = sf::Color(0, 0, 0);
const sf::Color WHITE = sf::Color(255, 255, 255);
const sf::Color BODY_COLOR = sf::Color(58, 124, 165);
const sf::Color FIN_COLOR = sf::Color(129, 195, 215);

class Chain {
public:
    std::vector<sf::Vector2f> joints;
    std::vector<float> angles;
    float segmentLength;

    Chain(sf::Vector2f origin, int numSegments, float segmentLength)
            : segmentLength(segmentLength) {
        for (int i = 0; i < numSegments; ++i) {
            joints.push_back(origin);
            angles.push_back(0.0f);
        }
    }

    void resolve(sf::Vector2f targetPos) {
        joints[0] = targetPos;
        for (int i = 1; i < joints.size(); ++i) {
            sf::Vector2f direction = joints[i] - joints[i - 1];
            if (std::sqrt(direction.x * direction.x + direction.y * direction.y) > 0) {
                float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                direction /= len; // Normalize
                direction *= segmentLength; // Scale to segment length
            }
            joints[i] = joints[i - 1] + direction;
            angles[i] = std::atan2(direction.y, direction.x);
        }
    }
};

class Fish {
public:
    Chain spine;
    std::vector<float> bodyWidth;

    Fish(sf::Vector2f origin)
            : spine(origin, 12, 64 * fishScale) {
        float bodyWidths[] = {34, 41, 42, 41, 38, 32, 26, 19, 16, 9};
        for (int i = 0; i < 10; ++i) {
            bodyWidth.push_back(bodyWidths[i] * fishScale);
        }
    }

    void resolve(sf::Vector2f mousePos) {
        sf::Vector2f headPos = spine.joints[0];
        sf::Vector2f direction = mousePos - headPos;
        float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (len > 0) {
            direction /= len; // Normalize
        }
        sf::Vector2f targetPos = headPos + direction * (16 * fishScale);
        spine.resolve(targetPos);
    }

    void display(sf::RenderWindow &window) {
        window.clear(BLACK);

        sf::Vector2f* j = &spine.joints[0];
        float* a = &spine.angles[0];

        float head_to_mid1 = relative_angle_diff(a[0], a[6]);
        float head_to_mid2 = relative_angle_diff(a[0], a[7]);
        float head_to_tail = head_to_mid1 + relative_angle_diff(a[6], a[11]);

        draw_fin(window, 3, M_PI / 3, 160 * fishScale, 64 * fishScale, a[2] - M_PI / 4);
        draw_fin(window, 3, -M_PI / 3, 160 * fishScale, 64 * fishScale, a[2] + M_PI / 4);
        draw_fin(window, 7, M_PI / 2, 96 * fishScale, 32 * fishScale, a[6] - M_PI / 4);
        draw_fin(window, 7, -M_PI / 2, 96 * fishScale, 32 * fishScale, a[6] + M_PI / 4);

        draw_caudal_fin(window, j, a, head_to_tail);
        draw_body(window, j, a);
        draw_dorsal_fin(window, j, a, head_to_mid1, head_to_mid2);

        draw_eye(window, j[0], a[0], M_PI / 2, -18 * fishScale);
        draw_eye(window, j[0], a[0], -M_PI / 2, -18 * fishScale);

        window.display();
    }

private:
    void draw_fin(sf::RenderWindow &window, int index, float angleOffset, float width, float height, float rotation) {
        sf::Vector2f pos = get_pos(index, angleOffset, 0);
        sf::RectangleShape fin(sf::Vector2f(width, height));
        fin.setFillColor(FIN_COLOR);
        fin.setOrigin(width / 2, height / 2);
        fin.setPosition(pos);
        fin.setRotation(rotation * 180 / M_PI);
        window.draw(fin);
    }

    void draw_caudal_fin(sf::RenderWindow &window, sf::Vector2f* j, float* a, float head_to_tail) {
        std::vector<sf::Vector2f> points;
        for (int i = 8; i < 12; ++i) {
            float tail_width = 1.5f * head_to_tail * (i - 8) * (i - 8) * fishScale;
            points.push_back(get_vertex(j[i], a[i], tail_width, -M_PI / 2));
        }
        for (int i = 11; i >= 8; --i) {
            float tail_width = std::max(-13.0f, std::min(13.0f, head_to_tail * 6)) * fishScale;
            points.push_back(get_vertex(j[i], a[i], tail_width, M_PI / 2));
        }
        draw_polygon(window, points, BODY_COLOR, 2);
    }

    void draw_body(sf::RenderWindow &window, sf::Vector2f* j, float* a) {
        std::vector<sf::Vector2f> points;
        for (int i = 0; i < 10; ++i) {
            points.push_back(get_vertex(j[i], a[i], bodyWidth[i], M_PI / 2));
        }
        points.push_back(get_vertex(j[9], a[9], bodyWidth[9], M_PI));
        for (int i = 9; i >= 0; --i) {
            points.push_back(get_vertex(j[i], a[i], bodyWidth[i], -M_PI / 2));
        }
        points.push_back(get_vertex(j[0], a[0], bodyWidth[0], -M_PI / 6));
        points.push_back(get_vertex(j[0], a[0], bodyWidth[0], 0));
        points.push_back(get_vertex(j[0], a[0], bodyWidth[0], M_PI / 6));
        draw_polygon(window, points, BODY_COLOR, 2);
    }

    void draw_dorsal_fin(sf::RenderWindow &window, sf::Vector2f* j, float* a, float head_to_mid1, float head_to_mid2) {
        std::vector<sf::Vector2f> points = {
                j[4],
                j[7],
                sf::Vector2f(j[6].x + cos(a[6] + M_PI / 2) * head_to_mid2 * 16 * fishScale,
                             j[6].y + sin(a[6] + M_PI / 2) * head_to_mid2 * 16 * fishScale),
                sf::Vector2f(j[5].x + cos(a[5] + M_PI / 2) * head_to_mid1 * 16 * fishScale,
                             j[5].y + sin(a[5] + M_PI / 2) * head_to_mid1 * 16 * fishScale),
                j[4]
        };
        draw_polygon(window, points, FIN_COLOR, 2);
    }

    void draw_eye(sf::RenderWindow &window, sf::Vector2f pos, float angle, float angleOffset, float lengthOffset) {
        sf::Vector2f eye_pos = get_vertex(pos, angle, 24 * fishScale, angleOffset, lengthOffset);
        sf::CircleShape eye(12 * fishScale);
        eye.setFillColor(WHITE);
        eye.setPosition(eye_pos);
        window.draw(eye);
    }

    sf::Vector2f get_vertex(sf::Vector2f pos, float angle, float length, float angleOffset = 0, float lengthOffset = 0) {
        float offsetAngle = angle + angleOffset;
        float offsetLength = length + lengthOffset;
        return sf::Vector2f(pos.x + cos(offsetAngle) * offsetLength,
                            pos.y + sin(offsetAngle) * offsetLength);
    }

    sf::Vector2f get_pos(int i, float angleOffset, float lengthOffset) {
        return get_vertex(spine.joints[i], spine.angles[i], bodyWidth[i], angleOffset, lengthOffset);
    }

    float relative_angle_diff(float angle1, float angle2) {
        return atan2(sin(angle2 - angle1), cos(angle2 - angle1));
    }

    void draw_polygon(sf::RenderWindow &window, const std::vector<sf::Vector2f>& points, const sf::Color& color, float outlineThickness) {
        sf::ConvexShape shape;
        shape.setPointCount(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            shape.setPoint(i, points[i]);
        }
        shape.setFillColor(color);
        shape.setOutlineColor(WHITE);
        shape.setOutlineThickness(outlineThickness);
        window.draw(shape);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Procedural Animation: Fish Following Mouse");
    Fish fish(sf::Vector2f(WIDTH / 2, HEIGHT / 2));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        fish.resolve(sf::Vector2f(sf::Mouse::getPosition(window)));
        fish.display(window);
        sf::sleep(sf::milliseconds(16)); // ~60 FPS
    }

    return 0;
}
