#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

#pragma region Vars

Color lightGreen = { 173, 204, 96, 255 };
Color darkGreen = { 43, 51, 24, 255 };

int offset = 70;
int cellSize = 25;
int cellCount = 25;
int screenSize = cellSize * cellCount;

double lastUpdateTime = 0;

#pragma endregion

// Function to check if a certain time interval has passed
bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Function to check if an element is present in a deque
bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }

    return false;
}

class Snake
{
public:
    deque<Vector2> body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
    Vector2 direction = { 1, 0 };
    bool addSegment = false;

    // Function to draw the snake on the screen
    void Draw()
    {
        for (int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{ offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize };
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    // Function to update the snake's position
    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));

        if (addSegment == true)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    // Function to reset the snake's position and direction
    void Reset()
    {
        body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
        direction = { 1, 0 };
    }
};

class Food {
public:
    Vector2 position;
    Texture2D texture;

    // Constructor to initialize the food's position and texture
    Food(deque<Vector2> snakeBody)
    {
        Image food = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(food);
        UnloadImage(food);
        position = GenerateRandomPosition(snakeBody);
    }

    // Destructor to unload the food's texture
    ~Food()
    {
        UnloadTexture(texture);
    }

    // Function to draw the food on the screen
    void Draw()
    {
        int positionX = position.x * cellSize;
        int positionY = position.y * cellSize;
        DrawTexture(texture, offset + positionX, offset + positionY, WHITE);
    }

    // Function to generate a random cell position
    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);

        return Vector2{ x, y };
    }

    // Function to generate a random position for the food that is not occupied by the snake's body
    Vector2 GenerateRandomPosition(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }

        return position;
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound collidingSound;

    // Constructor to initialize the game and load sounds
    Game()
    {
        InitAudioDevice();
        collidingSound = LoadSound("Sounds/collidingSound.mp3");
        eatSound = LoadSound("Sounds/pointSound.mp3");
    }

    // Destructor to unload sounds
    ~Game()
    {
        UnloadSound(collidingSound);
        UnloadSound(eatSound);
        CloseAudioDevice();
    }

    // Function to update the game state
    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckFoodCollision();
            CheckEdgeCollision();
            CheckTailCollision();
        }
    }

    // Function to draw the game on the screen
    void Draw()
    {
        snake.Draw();
        food.Draw();
    }

    // Function to check if the snake has collided with the food
    void CheckFoodCollision()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            cout << "EATING FOOD" << endl;
            food.position = food.GenerateRandomPosition(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    // Function to check if the snake has collided with the edges of the screen
    void CheckEdgeCollision()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1 || snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    // Function to check if the snake has collided with its own tail
    void CheckTailCollision()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();

        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    // Function to handle game over state
    void GameOver()
    {
        cout << "COLIDED" << endl;

        snake.Reset();
        food.position = food.GenerateRandomPosition(snake.body);
        running = false;
        score = 0;
        PlaySound(collidingSound);
    }
};

int main()
{
    cout << "Starting Game" << endl;
    InitWindow(2 * offset + screenSize, 2 * offset + screenSize, "Snake Game");
    SetTargetFPS(60);

    Game game = Game();

    while (!WindowShouldClose())
    {
        BeginDrawing();

        if (EventTriggered(0.2))
        {
            game.Update();
        }

        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
        {
            cout << "UP" << endl;
            game.snake.direction = { 0, -1 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
        {
            cout << "DOWN" << endl;
            game.snake.direction = { 0, 1 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
        {
            cout << "LEFT" << endl;
            game.snake.direction = { -1, 0 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
        {
            cout << "RIGHT" << endl;
            game.snake.direction = { 1, 0 };
            game.running = true;
        }

        //Drawing Background
        ClearBackground(lightGreen);
        DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset - 5, (float)screenSize + 10, (float)screenSize + 10 }, 5, darkGreen);
        DrawText("Retro Snake Game", offset, 20, 40, darkGreen);
        DrawText(TextFormat("%i", game.score), offset, offset + screenSize + 10, 40, darkGreen);
        game.Draw();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}