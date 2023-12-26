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
#define CLIP_DEPTH 0.0001

static const Color BACKGROUND_COLOR = {50, 50, 50};

struct vec2 {
    int x, y;
};

struct Wall {
    struct vec2 p0, p1;
    float height;
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

struct vec2 transform(struct vec2 point, struct player *player) {
    struct vec2 transformed = point;
    transformed.x -= player->pos.x;
    transformed.y -= player->pos.y;

    transformed = rotatePoint(transformed, -player->rotation - PI / 2);  // +PI/2 to turn the normalized angle to by 90°; then it is upwards
    return transformed;
}


struct vec2 screenCoor(struct vec2 point, struct player *player, float wallHeight) {
    float depth = point.y;
    float depthScale = 1 / depth * GetScreenHeight();

    struct vec2 transformed = {.x=point.x, .y=wallHeight / 2};
    transformed.x *= depthScale;
    transformed.y *= depthScale;

    transformed.x += GetScreenWidth() / 2;
    transformed.y += 0;

    return transformed;
}

struct vec2 getPlayerDirection(struct player *player) {
    struct vec2 heading = {
        .x = player->pos.x + (HEADING_LENGTH + PLAYER_RADIUS) * cos(player->rotation),
        .y = player->pos.y + (HEADING_LENGTH + PLAYER_RADIUS) * sin(player->rotation)
    };
    return heading;
}

struct Wall clipWall(struct vec2 p0, struct vec2 p1, Color color) {
    struct vec2 front, back; 
    if (p1.y > CLIP_DEPTH) {
        front = p0;
        back = p1;
    }
    else {
        front = p1;
        back = p0;
    }

    float size = front.y - back.y;
    float percentage = front.y / size;
    float clipX = front.x + (back.x - front.x) * percentage;

    struct Wall clippedWall = {.p0=front, .p1={.x=clipX, .y=CLIP_DEPTH}, .color=color};    
    return clippedWall;
}

void updateScreen(struct WallList *wallList, struct player *player) {
    ClearBackground(BACKGROUND_COLOR);
    BeginDrawing();

    int vcenter = GetScreenHeight() / 2;

    struct Color vcenterColor = {100, 100, 100, 255};
    DrawLine(0, vcenter, GetScreenWidth(), vcenter, vcenterColor);

    for (int i = 0; i < wallList->wallCount; i++) {
        struct Wall *wall = &wallList->walls[i];
        struct vec2 p0 = transform(wall->p0, player);
        struct vec2 p1 = transform(wall->p1, player);

        if (p0.y <= CLIP_DEPTH && p1.y <= CLIP_DEPTH) {
            p0 = screenCoor(p0, player, wall->height);
            p1 = screenCoor(p1, player, wall->height);
        }
        else if (p0.y > CLIP_DEPTH && p1.y > CLIP_DEPTH) {
            continue;
        }
        else {
            struct Wall clippedWall = clipWall(p0, p1, wall->color);
            p0 = screenCoor(clippedWall.p0, player, wall->height);
            p1 = screenCoor(clippedWall.p1, player, wall->height);
        }
        // bottom
        DrawLine(p0.x, vcenter + p0.y, p1.x, vcenter + p1.y, wall->color);

        // top
        DrawLine(p0.x, vcenter - p0.y, p1.x, vcenter - p1.y, wall->color);

        // vertical
        if (wall->p0.y > CLIP_DEPTH) {
            DrawLine(p0.x, vcenter - p0.y, p0.x, vcenter + p0.y, wall->color);
        }
        if (wall->p1.y > CLIP_DEPTH) {
            DrawLine(p1.x, vcenter - p1.y, p1.x, vcenter + p1.y, wall->color);
        }
    }

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

    wallList.walls = malloc(3 * sizeof(struct Wall));
    if (wallList.walls == NULL) {
        printf("Failure creating wall list\n");
        return EXIT_FAILURE;
    }

    // triangle:
    struct Wall w1 = {.p0={.x = 30, .y=50}, .p1={.x = 50, .y=60}, .color={0, 0, 200, 255}, .height=5};
    wallList.walls[0] = w1;
    wallList.wallCount++;

    struct Wall w2 = {.p0={.x = 30, .y=50}, .p1={.x = 40, .y=80}, .color={0, 0, 200, 255}, .height=5};
    wallList.walls[1] = w2;
    wallList.wallCount++;

    struct Wall w3 = {.p0={.x = 40, .y=80}, .p1={.x = 50, .y=60}, .color={0, 0, 200, 255}, .height=5};
    wallList.walls[2] = w3;
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
            p.rotation += PI / 30;
        }
        if (IsKeyDown(KEY_D)) {
            p.rotation -= (PI / 30);
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
