#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <raylib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_SPEED 10
#define SCALE 5
#define HEADING_LENGTH 30 / SCALE
#define PLAYER_RADIUS SCALE / 2

static const Color BACKGROUND_COLOR = {50, 50, 50};

struct vec2 {
    int x, y;
};

struct Wall {
    struct vec2 p0, p1;
    struct Color color;
};

struct WallList {
    struct Wall *walls;
    int wallCount;
};

struct player {
    struct vec2 pos;
    double rotation;
};

struct vec2 rotatePoint(struct vec2 point, float angle) {
    struct vec2 rotatedPoint = {
        .x = point.x * cos(angle) - point.y * sin(angle),
        .y = point.x * sin(angle) + point.y * cos(angle)
    };
    return rotatedPoint;
}

struct vec2 screenCoor(struct vec2 point, struct player *player) {
    struct vec2 transformed = point;
    transformed.x -= player->pos.x;
    transformed.y -= player->pos.y;

    transformed = rotatePoint(transformed, -player->rotation - PI / 2);  // +PI/2 to turn the normalized angle to by 90°; then it is upwards

    transformed.x *= SCALE;
    transformed.y *= SCALE;

    transformed.x += GetScreenWidth() / 2;
    transformed.y += GetScreenHeight() / 2;

    return transformed;
}

struct vec2 getPlayerDirection(struct player *player) {
    struct vec2 heading = {
        .x = player->pos.x + (HEADING_LENGTH + PLAYER_RADIUS) * cos(player->rotation),
        .y = player->pos.y + (HEADING_LENGTH + PLAYER_RADIUS) * sin(player->rotation)
    };
    return heading;
}

void updateScreen(struct WallList *wallList, struct player *player) {
    ClearBackground(BACKGROUND_COLOR);
    BeginDrawing();

    for (int i = 0; i < wallList->wallCount; i++) {
        struct Wall *wall = &wallList->walls[i];
        struct vec2 p0 = screenCoor(wall->p0, player);
        struct vec2 p1 = screenCoor(wall->p1, player);
        DrawLine(p0.x, p0.y, p1.x, p1.y, wall->color);
    }

    struct vec2 playerPos = screenCoor(player->pos, player); 
    Color playerColor = {255, 0, 0, 255};
    DrawCircle(playerPos.x, playerPos.y, PLAYER_RADIUS, playerColor);

    struct vec2 direction = screenCoor(getPlayerDirection(player), player);
    DrawLine(playerPos.x, playerPos.y, direction.x, direction.y, playerColor);

    EndDrawing();
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong");

    SetTargetFPS(30);

    struct WallList wallList = {
        .wallCount = 0,
        .walls = NULL
    };

    wallList.walls = malloc(sizeof(struct Wall));
    if (wallList.walls == NULL) {
        printf("Failure creating wall list\n");
        return EXIT_FAILURE;
    }
    struct Wall w = {.p0={.x = 30, .y=50}, .p1={.x = 50, .y=60}, .color={0, 0, 200, 255}};
    wallList.walls[0] = w;
    wallList.wallCount++;

    struct player p = {.pos = {.x = 0, .y = 0}, .rotation=0};

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_W)) {
            p.pos.x += PLAYER_SPEED * cos(p.rotation);
            p.pos.y += PLAYER_SPEED * sin(p.rotation);
        }
        if (IsKeyDown(KEY_S)) {
            p.pos.x -= PLAYER_SPEED * cos(p.rotation);
            p.pos.y -= PLAYER_SPEED * sin(p.rotation);
        }
        if (IsKeyDown(KEY_A)) {
            p.rotation -= PI / 15;
        }
        if (IsKeyDown(KEY_D)) {
            p.rotation += (PI / 15);
        }

        if (IsKeyDown(KEY_E)) {
            printf("Player rot: %f\n", p.rotation);
            printf("Player pos: (%i, %i)\n", p.pos.x, p.pos.y);
            struct vec2 h = getPlayerDirection(&p);
            printf("Player dir: (%i, %i)\n", h.x, h.y);
            printf("Diff: %lf\n", sqrt(pow(h.x - p.pos.x, 2) + pow(h.y - p.pos.y, 2)));
        }

        while (p.rotation < 0) {
            p.rotation += 2 * PI;
        }
        while (p.rotation >= 2 * PI) {
            p.rotation -= 2 * PI;
        }

        updateScreen(&wallList, &p);
    }
    return 0;
}