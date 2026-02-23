#include "raylib.h"
#include "raymath.h"
#include "stdio.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define GRAVITY 1000.0f
#define JUMP_SPEED 500.0f
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

void updatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float deltaTime, Sound* sounds, int ground_y);
void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);

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
        {{0, ground_y, screenWidth, 10}, 0, GREEN}
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
		updatePlayer(&player, envItems, envItemsLength, deltaTime, sounds, ground_y);

		camera.zoom += ((float)GetMouseWheelMove()*0.05f);

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

		if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            player.position = (Vector2){ 400, 280 };
            player.velocity.y = 0.0f;
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

void updatePlayer(Player* player, EnvItem* envItems, int envItemsLength, float deltaTime, Sound* sounds, int ground_y)
{
	int width = GetScreenWidth();
	int height = GetScreenHeight();

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

    // player->position.y += player->velocity.y * deltaTime;
    // player->velocity.y += GRAVITY * deltaTime;
    
    // // prevent out of bound
    // if (player->position.x <= 0 || player->position.x >= width)
    // {
    // 	player->canJump = false;
    // }
    // else if (player->position.y >= ground_y - player->size.y)
    // {
    // 	player->position.y = ground_y - player->size.y;
    // 	player->velocity.y = 0;
    // 	player->canJump = true;
    // }

    printf("%f\n", player->velocity.y);
		
    bool hitObstacle = false;
    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        Vector2 *p = &(player->position);
        if (ei->blocking &&
            ei->rect.x <= p->x &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y &&
            ei->rect.y <= p->y + player->velocity.y*deltaTime)
        {
            hitObstacle = true;
            player->velocity.y = 0.0f;  
            p->y = ei->rect.y - player->size.y;
            break;
        }
    }

    if (!hitObstacle)
    {
        player->position.y += player->velocity.y*deltaTime;
        player->velocity.y += GRAVITY*deltaTime;
        player->canJump = false;
    }
    else player->canJump = true;
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
