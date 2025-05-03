#include <raylib.h>

const int screen_width = 1280;
const int screen_height = 720;
const int max_score = 5;

int player_score = 0;
int cpu_score = 0;
bool hard_mode = false;

Sound player_scored_sound;
Sound cpu_scored_sound;

class Ball {
public:
    float x, y;
    int radius;
    float speed_x, speed_y;

    void Draw() {
        DrawCircle((int)x, (int)y, radius, YELLOW);
    }

    void Update() {
        x += speed_x;
        y += speed_y;

        float max_speed = 12.0f;
        if (speed_x > max_speed) speed_x = max_speed;
        if (speed_y > max_speed) speed_y = max_speed;
        if (speed_x < -max_speed) speed_x = -max_speed;
        if (speed_y < -max_speed) speed_y = -max_speed;

        if (y - radius <= 0 || y + radius >= screen_height) {
            speed_y *= -1;
        }

        if (x - radius <= 0) {
            cpu_score++;
            PlaySound(cpu_scored_sound);
            Reset();
        }

        if (x + radius >= screen_width) {
            player_score++;
            PlaySound(player_scored_sound);
            Reset();
        }
    }

    void Reset() {
        x = screen_width / 2;
        y = screen_height / 2;
        int dir_x = GetRandomValue(0, 1) == 0 ? -1 : 1;
        int dir_y = GetRandomValue(0, 1) == 0 ? -1 : 1;
        speed_x = 8 * dir_x;
        speed_y = 8 * dir_y;
    }
};

class Paddle {
protected:
    void LimitMovement() {
        if (y < 0) y = 0;
        if (y + height > screen_height) y = screen_height - height;
    }

public:
    float x, y;
    float width, height;
    float speed;

    void Draw() {
        DrawRectangleRounded({x, y, width, height}, 0.8f, 0, RED);
    }

    void Move() {
        if (IsKeyDown(KEY_UP)) y -= speed;
        if (IsKeyDown(KEY_DOWN)) y += speed;
        LimitMovement();
    }
};

class CPU : public Paddle {
public:
    void AutoMove(float ball_y) {
        float tracking_speed = hard_mode ? 1.0f : 0.4f;
        float randomness = hard_mode ? 0 : GetRandomValue(-8, 8);

        float move_amount = (ball_y - (y + height / 2)) * tracking_speed + randomness;

        float max_speed = hard_mode ? 8.0f : 5.0f;
        if (move_amount > max_speed) move_amount = max_speed;
        if (move_amount < -max_speed) move_amount = -max_speed;

        y += move_amount;
        LimitMovement();
    }
};

Ball ball;
Paddle player;
CPU cpu;

void InitGame() {
    ball.radius = 15;
    ball.Reset();

    player.width = 20;
    player.height = 120;
    player.x = 10;
    player.y = screen_height / 2 - player.height / 2;
    player.speed = 10;

    cpu.width = 20;
    cpu.height = 120;
    cpu.x = screen_width - cpu.width - 10;
    cpu.y = screen_height / 2 - cpu.height / 2;
    cpu.speed = 10;

    player_scored_sound = LoadSound("scored.mp3");
    cpu_scored_sound = LoadSound("scored.mp3");
}

void HandleCollision() {
    if (CheckCollisionCircleRec({ball.x, ball.y}, ball.radius, {player.x, player.y, player.width, player.height})) {
        ball.speed_x *= -1.1f;
        ball.speed_y += (ball.y - (player.y + player.height / 2)) / 10;
    }

    if (CheckCollisionCircleRec({ball.x, ball.y}, ball.radius, {cpu.x, cpu.y, cpu.width, cpu.height})) {
        ball.speed_x *= -1.1f;
        ball.speed_y += (ball.y - (cpu.y + cpu.height / 2)) / 10;
    }
}

void UpdateGame() {
    ball.Update();
    player.Move();
    cpu.AutoMove(ball.y);
    HandleCollision();
}

void DrawGame() {
    BeginDrawing();
    ClearBackground(DARKBLUE);

    DrawCircle(screen_width / 2, screen_height / 2, 150, BLUE);
    DrawLine(screen_width / 2, 0, screen_width / 2, screen_height, WHITE);

    ball.Draw();
    player.Draw();
    cpu.Draw();

    int font_size = 80;
    DrawText(TextFormat("%i", player_score), screen_width / 4 - MeasureText(TextFormat("%i", player_score), font_size) / 2, 20, font_size, WHITE);
    DrawText(TextFormat("%i", cpu_score), 3 * screen_width / 4 - MeasureText(TextFormat("%i", cpu_score), font_size) / 2, 20, font_size, WHITE);

    EndDrawing();
}

void ShowStartScreen() {
    InitAudioDevice();
    Music bgm = LoadMusicStream("nhaccho.mp3");
    PlayMusicStream(bgm);

    while (!IsKeyPressed(KEY_ENTER) && !WindowShouldClose()) {
        UpdateMusicStream(bgm);
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("PICKLEBALL", screen_width / 2 - MeasureText("PICKLEBALL", 50) / 2, screen_height / 2 - 60, 50, YELLOW);
        DrawText("Press ENTER to Start", screen_width / 2 - MeasureText("Press ENTER to Start", 20) / 2, screen_height / 2 + 20, 20, WHITE);
        EndDrawing();
    }

    StopMusicStream(bgm);
    UnloadMusicStream(bgm);
}

void ChooseMode() {
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("CHOOSE MODE", screen_width / 2 - MeasureText("CHOOSE MODE", 40) / 2, screen_height / 2 - 100, 40, YELLOW);
        DrawText("Press E for EASY", screen_width / 2 - MeasureText("Press E for EASY", 20) / 2, screen_height / 2, 20, WHITE);
        DrawText("Press H for HARD", screen_width / 2 - MeasureText("Press H for HARD", 20) / 2, screen_height / 2 + 40, 20, WHITE);
        EndDrawing();

        if (IsKeyPressed(KEY_E)) {
            hard_mode = false;
            break;
        }
        if (IsKeyPressed(KEY_H)) {
            hard_mode = true;
            break;
        }
    }
}

void ShowEndScreen(bool player_won) {
    const char* result = player_won ? "YOU WIN!" : "CPU WINS!";

    while (!IsKeyPressed(KEY_ENTER) && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(result, screen_width / 2 - MeasureText(result, 50) / 2, screen_height / 2 - 60, 50, YELLOW);
        DrawText("Press ENTER to Exit", screen_width / 2 - MeasureText("Press ENTER to Exit", 20) / 2, screen_height / 2 + 20, 20, WHITE);
        EndDrawing();
    }
}

void UnloadResources() {
    UnloadSound(player_scored_sound);
    UnloadSound(cpu_scored_sound);
    CloseAudioDevice();
}

int main() {
    InitWindow(screen_width, screen_height, "Pickleball");
    SetTargetFPS(60);

    ShowStartScreen();
    ChooseMode();
    InitGame();

    bool game_over = false;
    bool player_won = false;

    while (!WindowShouldClose() && !game_over) {
        UpdateGame();
        DrawGame();

        if (player_score == max_score) {
            game_over = true;
            player_won = true;
        } else if (cpu_score == max_score) {
            game_over = true;
            player_won = false;
        }
    }

    ShowEndScreen(player_won);
    UnloadResources();
    CloseWindow();
    return 0;
}
