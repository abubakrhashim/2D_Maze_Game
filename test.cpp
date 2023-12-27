#include "raylib.h"
#include <vector>

class Cell {
public:
    int i, j;
    bool walls[4] = { true, true, true, true };
    bool visited = false;

    Cell(int i, int j) : i(i), j(j) {}

    void Show(int w, int targetX, int targetY) const {
        int x = i * w;
        int y = j * w;

        // Check if the cell is the destination cell, and change its color
        if (i == targetX && j == targetY) {
            DrawRectangle(x, y, w, w, GOLD);  // Change GOLD to the desired color
        }
        else {
            if (walls[0]) DrawLine(x, y, x + w, y, BLACK);
            if (walls[1]) DrawLine(x + w, y, x + w, y + w, BLACK);
            if (walls[2]) DrawLine(x + w, y + w, x, y + w, BLACK);
            if (walls[3]) DrawLine(x, y + w, x, y, BLACK);

            if (visited) {
                DrawRectangle(x, y, w, w, VIOLET);
            }
        }
    }

    void CheckNeighbors(const std::vector<Cell>& grid, int cols, int rows, std::vector<Cell*>& neighbors) const {
        if (i > 0 && !grid[index(i - 1, j, cols)].visited) {
            neighbors.push_back((Cell*)&grid[index(i - 1, j, cols)]);
        }
        if (i < cols - 1 && !grid[index(i + 1, j, cols)].visited) {
            neighbors.push_back((Cell*)&grid[index(i + 1, j, cols)]);
        }
        if (j > 0 && !grid[index(i, j - 1, cols)].visited) {
            neighbors.push_back((Cell*)&grid[index(i, j - 1, cols)]);
        }
        if (j < rows - 1 && !grid[index(i, j + 1, cols)].visited) {
            neighbors.push_back((Cell*)&grid[index(i, j + 1, cols)]);
        }
    }

    static int index(int i, int j, int cols) {
        return i + j * cols;
    }
};

void RemoveWalls(Cell& a, Cell& b) {
    int x = a.i - b.i;
    if (x == 1) {
        a.walls[3] = false;
        b.walls[1] = false;
    }
    else if (x == -1) {
        a.walls[1] = false;
        b.walls[3] = false;
    }

    int y = a.j - b.j;
    if (y == 1) {
        a.walls[0] = false;
        b.walls[2] = false;
    }
    else if (y == -1) {
        a.walls[2] = false;
        b.walls[0] = false;
    }
}

class Player {
public:
    int x, y, speed;
    bool keyProcessed;
    bool hasWon;
    double winTime;

    Player() : x(0), y(0), speed(1), keyProcessed(false), hasWon(false), winTime(0.0) {}

    void move(int dx, int dy, const std::vector<Cell>& grid, int cols, int rows) {
        if (hasWon) return;

        int newX = x + dx;
        int newY = y + dy;

        if (!keyProcessed && newX >= 0 && newX < cols && newY >= 0 && newY < rows) {
            int currentCellIndex = Cell::index(x, y, cols);
            int newCellIndex = Cell::index(newX, newY, cols);

            if (dx > 0 && !grid[currentCellIndex].walls[1] && !grid[newCellIndex].walls[3]) {
                x = newX;
            }
            else if (dx < 0 && !grid[currentCellIndex].walls[3] && !grid[newCellIndex].walls[1]) {
                x = newX;
            }

            if (dy > 0 && !grid[currentCellIndex].walls[2] && !grid[newCellIndex].walls[0]) {
                y = newY;
            }
            else if (dy < 0 && !grid[currentCellIndex].walls[0] && !grid[newCellIndex].walls[2]) {
                y = newY;
            }

            keyProcessed = true;
        }
    }

    void resetKeyProcessed() {
        keyProcessed = false;
    }

    void Draw(int w) const {
        DrawRectangle(x * w, y * w, w, w, RED);
    }
};

typedef struct Button {
    Rectangle rect;
    Color color;
    const char* text;
} Button;

void init_button(Button* button, Rectangle rect, Color color, const char* text) {
    button->rect = rect;
    button->color = color;
    button->text = text;
}

bool is_mouse_over_button(Button button) {
    return CheckCollisionPointRec(GetMousePosition(), button.rect);
}

int main() {
    Player player;
    const int screenWidth = 400;
    const int screenHeight = 400;
    const int w = 40;
    const int cols = screenWidth / w;
    const int rows = screenHeight / w;
    std::vector<Cell> grid;
    std::vector<Cell*> stack;

    for (int j = 0; j < rows; ++j) {
        for (int i = 0; i < cols; ++i) {
            grid.emplace_back(i, j);
        }
    }

    Cell* current = &grid[0];

    InitWindow(screenWidth, screenHeight, "Maze Game");
    Button startButton = { 0 };
    init_button(&startButton, Rectangle{ 100, 150, 200, 50 }, GREEN, "Start");

    Button restartButton = { 0 };
    init_button(&restartButton, Rectangle{ 100, 150, 200, 50 }, GREEN, "Restart");

    SetTargetFPS(60);
    bool runGame = false;
    bool showEndScreen = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (!showEndScreen) {
            DrawRectangleRec(startButton.rect, startButton.color);

            if (is_mouse_over_button(startButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                runGame = true;
            }

            DrawText(startButton.text, startButton.rect.x + 25, startButton.rect.y + 15, 20, BLACK);
        }
        else {
            DrawText("Congratulations, you won!!!", screenWidth / 2 - MeasureText("Congratulations, you won!!!", 20) / 2, screenHeight / 2, 20, DARKGRAY);
            DrawRectangleRec(restartButton.rect, restartButton.color);
            DrawText(restartButton.text, restartButton.rect.x + 25, restartButton.rect.y + 15, 20, BLACK);

            if (is_mouse_over_button(restartButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                showEndScreen = false;
                player = Player();
                grid.clear();
                stack.clear();

                for (int j = 0; j < rows; ++j) {
                    for (int i = 0; i < cols; ++i) {
                        grid.emplace_back(i, j);
                    }
                }

                current = &grid[0];
            }
        }

        EndDrawing();

        if (runGame) {
            while (!WindowShouldClose() && runGame) {
                BeginDrawing();
                ClearBackground(GetColor(0x333333FF));

                for (Cell& cell : grid) {
                    cell.Show(w, cols - 1, rows - 1);  // Pass destination cell coordinates
                }

                std::vector<Cell*> neighbors;
                current->CheckNeighbors(grid, cols, rows, neighbors);

                if (!neighbors.empty()) {
                    Cell* next = neighbors[GetRandomValue(0, neighbors.size() - 1)];

                    next->visited = true;
                    stack.push_back(current);

                    RemoveWalls(*current, *next);
                    current = next;
                }
                else if (!stack.empty()) {
                    current = stack.back();
                    stack.pop_back();
                }


                int dy = 0, dx = 0;

                if (IsKeyDown(KEY_RIGHT)) dx = 1;
                if (IsKeyDown(KEY_LEFT)) dx = -1;
                if (IsKeyDown(KEY_UP)) dy = -1;
                if (IsKeyDown(KEY_DOWN)) dy = 1;

                player.move(dx, dy, grid, cols, rows);
                player.Draw(w);

                int targetX = cols - 1;
                int targetY = rows - 1;

                if (!player.hasWon && player.x == targetX && player.y == targetY) {
                    runGame = false;
                    showEndScreen = true;
                    player.hasWon = true;
                    player.winTime = GetTime();
                }

                if (!IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_UP) && !IsKeyDown(KEY_DOWN)) {
                    player.resetKeyProcessed();
                }

                EndDrawing();
            }
        }
    }

    CloseWindow();

    return 0;
}
