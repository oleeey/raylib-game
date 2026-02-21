#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(800, 800, "My Game");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	int height = GetScreenHeight();
	int width = GetScreenWidth();
	
	Vector2 pos = {(float) width / 2, (float) height / 2};
	Vector2 size = {50, 50};
	
	SetTargetFPS(60);

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		if (IsKeyDown(KEY_LEFT)) pos.x -= 5.0f;
		if (IsKeyDown(KEY_RIGHT)) pos.x += 5.0f;
		if (IsKeyDown(KEY_UP)) pos.y -= 5.0f;
		if (IsKeyDown(KEY_DOWN)) pos.y += 5.0f;
	
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(WHITE);
		DrawRectangleV(pos, size, BLACK);


		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
