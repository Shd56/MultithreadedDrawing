#include <raylib.h>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

// Size of the grid
const int GRID_COLS = 80;
const int GRID_ROWS = 45;

// Shared grid: each cell stores a "color id"
int grid[GRID_ROWS][GRID_COLS];

std::mutex gridMutex;   // protects access to grid
bool running = true;    // threads keep running while this is true

// Thread function: each thread paints cells with its own color
void painterThread(int threadId, int colorId) {
    while (running) {
        // choose a random cell
        int col = GetRandomValue(0, GRID_COLS - 1);
        int row = GetRandomValue(0, GRID_ROWS - 1);

        {
            // lock while writing to the grid
            std::lock_guard<std::mutex> lock(gridMutex);
            grid[row][col] = colorId;
        }

        // small sleep so CPU is not 100%
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

int main() {
    // Create a window
    InitWindow(800, 450, "Multithreaded Drawing - Part 1");
    SetTargetFPS(60);

    // Initialize grid to 0 (background)
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            grid[r][c] = 0;
        }
    }

    // Start multiple painter threads with different color IDs
    std::vector<std::thread> threads;
    threads.emplace_back(painterThread, 0, 1); // Thread 0 - color 1 (RED)
    threads.emplace_back(painterThread, 1, 2); // Thread 1 - color 2 (BLUE)
    threads.emplace_back(painterThread, 2, 3); // Thread 2 - color 3 (GREEN)
    threads.emplace_back(painterThread, 3, 4); // Thread 3 - color 4 (YELLOW)

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        float cellW = (float)GetScreenWidth() / GRID_COLS;
        float cellH = (float)GetScreenHeight() / GRID_ROWS;

        // Draw each cell of the grid
        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                int cellValue;
                {
                    // read grid under lock to be safe
                    std::lock_guard<std::mutex> lock(gridMutex);
                    cellValue = grid[r][c];
                }

                Color col = DARKGRAY;
                if (cellValue == 1)      col = RED;
                else if (cellValue == 2) col = BLUE;
                else if (cellValue == 3) col = GREEN;
                else if (cellValue == 4) col = YELLOW;

                DrawRectangle(
                    (int)(c * cellW),
                    (int)(r * cellH),
                    (int)(cellW + 1),
                    (int)(cellH + 1),
                    col
                );
            }
        }

        DrawText("Part 1: threads painting random cells", 10, 10, 20, RAYWHITE);

        EndDrawing();
    }

    // Tell threads to stop
    running = false;

    // Wait for all threads to finish
    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }

    CloseWindow();
    return 0;
}
