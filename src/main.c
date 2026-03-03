#include "raylib.h"
#include "raymath.h"
#include "stdio.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define GRAVITY 1000.0f
#define JUMP_SPEED 600.0f
#define MOVE_SPEED 500.0f

typedef struct Player {
	Vector2 size, position, velocity;
	bool canJump;
} Player;

typedef struct EnvItem {
	Rectangle rect;
	int blocking;
	Color color;
} EnvItem;

void updatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float deltaTime, Sound* sounds, Camera2D* camera);
void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void resetPlayer(Player* player, Camera2D* camera);

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	const int screenWidth = 800;
	const int screenHeight = 800;

	// Create the window and OpenGL context
	InitWindow(screenWidth, screenHeight, "My Game");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	InitAudioDevice();
	Sound sound_jump = LoadSound("jump.mp3");
	Sound sounds[] = { sound_jump };

	int ground_y = screenHeight - 100;

	Player player = { 0 };
	player.size = (Vector2) {50, 50};
	player.position = (Vector2) {(float) screenWidth / 2, (float) ground_y - player.size.y};
	player.velocity = (Vector2) {(float) 0.0f, (float) 0.0f};
	player.canJump = false;

	EnvItem envItems[] = {
        {{0, ground_y, screenWidth, 10}, 1, GREEN},
        {{0, ground_y - 100, screenWidth, 10}, 1, GREEN}
    };

    int envItemsLength = sizeof(envItems)/sizeof(envItems[0]);

	Camera2D camera = { 0 };
	camera.target = player.position;
	camera.offset = (Vector2) {screenWidth / 2.0f, screenHeight / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	// Store pointers to the multiple update camera functions
    void (*cameraUpdaters[])(Camera2D*, Player*, EnvItem*, int, float, int, int) = {
        UpdateCameraCenter,
        UpdateCameraCenterSmoothFollow,
    };

	int cameraOption = 0;
    int cameraUpdatersLength = sizeof(cameraUpdaters)/sizeof(cameraUpdaters[0]);

	SetTargetFPS(60);

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		float deltaTime = GetFrameTime();
		updatePlayer(&player, envItems, envItemsLength, deltaTime, sounds, &camera);

		camera.zoom += ((float)GetMouseWheelMove()*0.05f);

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

		if (IsKeyPressed(KEY_R))
        {
            resetPlayer(&player, &camera);
        }

        if (IsKeyPressed(KEY_C)) cameraOption = (cameraOption + 1)%cameraUpdatersLength;

		// Call update camera function by its pointer
        cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);

		// drawing
		BeginDrawing();

			// Setup the back buffer for drawing (clear color and depth buffers)
			ClearBackground(WHITE);

			BeginMode2D(camera);

                for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

				//Rectangle groundRect = {0, ground_y, screenWidth, 10};
				//DrawRectangleRec(groundRect, GREEN);
				Rectangle playerRect = {player.position.x, player.position.y, player.size.x, player.size.y};
				DrawRectangleRec(playerRect, BLACK);

			EndMode2D();

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up

	CloseAudioDevice();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}

void updatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float deltaTime, Sound* sounds, Camera2D* camera)
{
	int width = GetScreenWidth();
	int height = GetScreenHeight();
    bool down = false;

	if (IsKeyDown(KEY_LEFT)) 
    {
        player->position.x -= deltaTime * MOVE_SPEED;
    }
    if (IsKeyDown(KEY_RIGHT)) 
    {
        player->position.x += deltaTime * MOVE_SPEED;
    }
    if (IsKeyDown(KEY_UP) && player->canJump) 
    {
        player->velocity.y = -JUMP_SPEED;
        player->canJump = false;
        PlaySound(sounds[0]);
    }
    if (IsKeyDown(KEY_DOWN) && player->canJump)
    {
        down = true;
    }

    /*
    1) Apply acceleration -> update velocity
    2) Predict next position
    3) Detect collision using old & new position
    4) Resolve collision
    5) Commit final position
    */

    player->velocity.y += GRAVITY * deltaTime;
    float newY = player->position.y + player->velocity.y * deltaTime;
    bool hitObstacle = false;

    if (newY > 2000)
    {
        resetPlayer(player, camera);
        return;
    }

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem* ei = envItems + i;

        if (!ei->blocking || down)
        {
            continue;
        }

        bool horizontalOverlap = 
            player->position.x + player->size.x > ei->rect.x &&
            player->position.x < ei->rect.x + ei->rect.width;

        bool falling = player->velocity.y > 0;

        if (horizontalOverlap && falling)
        {
            float playerBottom = newY + player->size.y;

            if (playerBottom >= ei->rect.y &&
                player->position.y + player->size.y <= ei->rect.y)
            {
                hitObstacle = true;
                player->velocity.y = 0.0f;
                newY = ei->rect.y - player->size.y;
                break;
            }
        }

    }

    player->position.y = newY;
    player->canJump = hitObstacle;
}

void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
	camera->offset = (Vector2) { width / 2.0f, height / 2.0f };
    camera->target = player->position;
}

void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    static float minSpeed = 30;
    static float minEffectLength = 10;
    static float fractionSpeed = 0.8f;

    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    Vector2 diff = Vector2Subtract(player->position, camera->target);
    float length = Vector2Length(diff);

    if (length > minEffectLength)
    {
        float speed = fmaxf(fractionSpeed*length, minSpeed);
        camera->target = Vector2Add(camera->target, Vector2Scale(diff, speed*delta/length));
    }
}

void resetPlayer(Player* player, Camera2D* camera)
{
    camera->zoom = 1.0f;
    player->position = (Vector2){ 400, 280 };
    player->velocity.y = 0.0f;
}
