#include "FlappyBird.h"

// ---------------
// game loop
// ---------------
void FlappyBirdClass::game_loop()
{
    // ===============
    // prepare game variables
    // draw floor
    // ===============
    // instead of calculating the distance of the floor from the screen height each time store it in a variable
    unsigned char GAMEH = TFTH - FLOORH;
    // draw the floor once, we will not overwrite on this area in-game
    // black line
    M5m.Lcd.drawFastHLine(0, GAMEH, TFTW, TFT_BLACK);
    // grass and stripe
    M5m.Lcd.fillRect(0, GAMEH + 1, TFTW2, GRASSH, GRASSCOL);
    M5m.Lcd.fillRect(TFTW2, GAMEH + 1, TFTW2, GRASSH, GRASSCOL2);
    // black line
    M5m.Lcd.drawFastHLine(0, GAMEH + GRASSH, TFTW, TFT_BLACK);
    // mud
    M5m.Lcd.fillRect(0, GAMEH + GRASSH + 1, TFTW, FLOORH - GRASSH, FLOORCOL);
    // grass x position (for stripe animation)
    long grassx = TFTW;
    // game loop time variables
    double delta, old_time, next_game_tick, current_time;
    next_game_tick = current_time = millis();
    int loops;
    // passed pipe flag to count flappy_bird_score
    bool passed_pipe = false;
    // temp var for setAddrWindow
    unsigned char px;

    while (1)
    {
        loops = 0;
        while (millis() > next_game_tick && loops < MAX_FRAMESKIP)
        {
            // ===============
            // input
            // ===============
            if (M5m.BtnB.wasPressed())
            {
                // if the bird is not too close to the top of the screen apply jump force
                if (bird.y > BIRDH2 * 0.5)
                {
                    bird.vel_y = -JUMP_FORCE;
                }
                // else zero velocity
                else
                {
                    bird.vel_y = 0;
                }
            }
            M5m.update();

            // ===============
            // update
            // ===============
            // calculate delta time
            // ---------------
            old_time = current_time;
            current_time = millis();
            delta = (current_time - old_time) / 1000;

            // bird
            // ---------------
            bird.vel_y += GRAVITY * delta;
            bird.y += bird.vel_y;

            // pipe
            // ---------------
            pipes.x -= SPEED;
            // if pipe reached edge of the screen reset its position and gap
            if (pipes.x < -PIPEW)
            {
                pipes.x = TFTW;
                pipes.gap_y = random(10, GAMEH - (10 + GAPHEIGHT));
            }

            // ---------------
            next_game_tick += SKIP_TICKS;
            loops++;
        }

        // ===============
        // draw
        // ===============
        // pipe
        // ---------------
        // we save cycles if we avoid drawing the pipe when outside the screen
        if (pipes.x >= 0 && pipes.x < TFTW)
        {
            // pipe color
            M5m.Lcd.drawFastVLine(pipes.x + 3, 0, pipes.gap_y, PIPECOL);
            M5m.Lcd.drawFastVLine(pipes.x + 3, pipes.gap_y + GAPHEIGHT + 1, GAMEH - (pipes.gap_y + GAPHEIGHT + 1), PIPECOL);
            // highlight
            M5m.Lcd.drawFastVLine(pipes.x, 0, pipes.gap_y, PIPEHIGHCOL);
            M5m.Lcd.drawFastVLine(pipes.x, pipes.gap_y + GAPHEIGHT + 1, GAMEH - (pipes.gap_y + GAPHEIGHT + 1), PIPEHIGHCOL);
            // bottom and top border of pipe
            M5m.Lcd.drawPixel(pipes.x, pipes.gap_y, PIPESEAMCOL);
            M5m.Lcd.drawPixel(pipes.x, pipes.gap_y + GAPHEIGHT, PIPESEAMCOL);
            // pipe seam
            M5m.Lcd.drawPixel(pipes.x, pipes.gap_y - 6, PIPESEAMCOL);
            M5m.Lcd.drawPixel(pipes.x, pipes.gap_y + GAPHEIGHT + 6, PIPESEAMCOL);
            M5m.Lcd.drawPixel(pipes.x + 3, pipes.gap_y - 6, PIPESEAMCOL);
            M5m.Lcd.drawPixel(pipes.x + 3, pipes.gap_y + GAPHEIGHT + 6, PIPESEAMCOL);
        }
        // erase behind pipe
        if (pipes.x <= TFTW)
        {
            M5m.Lcd.drawFastVLine(pipes.x + PIPEW, 0, GAMEH, BCKGRDCOL);
        }
        // bird
        // ---------------
        tmpx = BIRDW - 1;
        do
        {
            px = bird.x + tmpx + BIRDW;
            // clear bird at previous position stored in old_y
            // we can't just erase the pixels before and after current position
            // because of the non-linear bird movement (it would leave 'dirty' pixels)
            tmpy = BIRDH - 1;
            do
            {
                M5m.Lcd.drawPixel(px, bird.old_y + tmpy, BCKGRDCOL);
            } while (tmpy--);
            // draw bird sprite at new position
            tmpy = BIRDH - 1;
            do
            {
                M5m.Lcd.drawPixel(px, bird.y + tmpy, birdcol[tmpx + (tmpy * BIRDW)]);
            } while (tmpy--);
        } while (tmpx--);
        // save position to erase bird on next draw
        bird.old_y = bird.y;
        // grass stripes
        // ---------------
        grassx -= SPEED;
        if (grassx < 0)
        {
            grassx = TFTW;
        }
        M5m.Lcd.drawFastVLine(grassx % TFTW, GAMEH + 1, GRASSH - 1, GRASSCOL);
        M5m.Lcd.drawFastVLine((grassx + 64) % TFTW, GAMEH + 1, GRASSH - 1, GRASSCOL2);

        // ===============
        // collision
        // ===============
        // if the bird hit the ground game over
        if (bird.y > GAMEH - BIRDH)
        {
            break;
        }
        // checking for bird collision with pipe
        if (bird.x + BIRDW >= pipes.x - BIRDW2 && bird.x <= pipes.x + PIPEW - BIRDW)
        {
            // bird entered a pipe, check for collision
            if (bird.y < pipes.gap_y || bird.y + BIRDH > pipes.gap_y + GAPHEIGHT)
            {
                break;
            }
            else
            {
                passed_pipe = true;
            }
        }
        // if bird has passed the pipe increase flappy_bird_score
        else if (bird.x > pipes.x + PIPEW - BIRDW && passed_pipe)
        {
            passed_pipe = false;
            // erase flappy_bird_score with background color
            M5m.Lcd.setTextColor(BCKGRDCOL);
            M5m.Lcd.drawNumber(flappy_bird_score, TFTW2, 4);
            // set text color back to white for new flappy_bird_score
            M5m.Lcd.setTextColor(TFT_WHITE);
            // increase flappy_bird_score since we successfully passed a pipe
            flappy_bird_score++;
        }
        // update flappy_bird_score
        // ---------------
        M5m.Lcd.drawNumber(flappy_bird_score, TFTW2, 4);
    }
    // add a small delay to show how the player lost
    vTaskDelay(1200 / portTICK_PERIOD_MS);
}

void FlappyBirdClass::game_init()
{
    // clear screen
    M5m.Lcd.fillScreen(BCKGRDCOL);
    // reset flappy_bird_score
    flappy_bird_score = 0;
    // init bird
    bird.x = 144;
    bird.y = bird.old_y = TFTH2 - BIRDH;
    bird.vel_y = -JUMP_FORCE;
    tmpx = tmpy = 0;
    // generate new random seed for the pipe gape
    // randomSeed(analogRead(0)); // pin 0 is ADC2 pin die niet gebruikt kan worden met Wifi 
    // init pipe
    pipes.x = 0;
    pipes.gap_y = random(20, TFTH - 60);
}

// ---------------
// game start
// ---------------
void FlappyBirdClass::game_start()
{
    M5m.Lcd.fillScreen(TFT_BLACK);
    M5m.Lcd.fillRect(10, TFTH2 - 20, TFTW - 20, 1, TFT_WHITE);
    M5m.Lcd.fillRect(10, TFTH2 + 32, TFTW - 20, 1, TFT_WHITE);
    M5m.Lcd.setTextColor(TFT_WHITE);
    M5m.Lcd.setTextSize(3);
    // half width - num char * char width in pixels
    M5m.Lcd.drawString("FLAPPY", TFTW2 - (6 * 9), TFTH2 - 16);
    M5m.Lcd.setTextSize(3);
    M5m.Lcd.drawString("-BIRD-", TFTW2 - (6 * 9), TFTH2 + 8);
    M5m.Lcd.setTextSize(2);
    M5m.Lcd.drawString("M5Stack", 10, TFTH2 - 36);
    M5m.Lcd.drawString("Mod by Calin", TFTW2 - (17 * 9), TFTH2 + 36);
    while (1)
    {
        // wait for push button
        if (M5m.BtnB.wasPressed())
        {
            break;
        }
        M5m.update();
    }
    // init game settings
    game_init();
}

void FlappyBirdClass::resetMaxflappy_bird_score()
{
    preferences.begin("Fpsc", false);
    preferences.putInt("sc", flappy_bird_score);
    preferences.end();
}

// ---------------
// game over
// ---------------
void FlappyBirdClass::game_over()
{
    M5m.Lcd.fillScreen(TFT_BLACK);
    preferences.begin("Fpsc", false);
    maxflappy_bird_score = preferences.getInt("sc", 0);

    if (flappy_bird_score > maxflappy_bird_score)
    {
        preferences.putInt("sc", flappy_bird_score);
        maxflappy_bird_score = flappy_bird_score;
        M5m.Lcd.setTextColor(TFT_RED);
        M5m.Lcd.setTextSize(2);
        //M5m.Lcd.setCursor(TFTW2 - (13 * 6), TFTH2 - 26);
        M5m.Lcd.drawString("NEW HIGHSCORE", TFTW2 - (13 * 6), TFTH2 - 26);
    }
    preferences.end();

    M5m.Lcd.setTextColor(TFT_WHITE);
    M5m.Lcd.setTextSize(3);
    // half width - num char * char width in pixels
    M5m.Lcd.drawString("GAME OVER", TFTW2 - (9 * 9), TFTH2 - 6);
    M5m.Lcd.setTextSize(2);
    M5m.Lcd.drawString("Score: " + String(flappy_bird_score), 10, 10);
    M5m.Lcd.drawString("press button", TFTW2 - (12 * 6), TFTH2 + 18);
    M5m.Lcd.drawString("Max Score:" + String(maxflappy_bird_score), 10, 28);
    M5m.Lcd.drawString("ScoreR", M5m.Lcd.width()*5/80, 221); // 30
    M5m.Lcd.drawString("Start", M5m.Lcd.width()*33/80, 221); // 132
    M5m.Lcd.drawString("Exit", M5m.Lcd.width()*58/80, 221); // 232
    while (1)
    {
        if (M5m.BtnA.wasPressed())
        {
            resetMaxflappy_bird_score();
        }
        if (M5m.BtnB.wasPressed())
        {
            Run();
        }
        if (M5m.BtnC.wasPressed())
        {
            return;
        }
        M5m.update();
    }
}

void FlappyBirdClass::Run()
{
    // put your main code here
    game_start();
    game_loop();
    game_over();
}

FlappyBirdClass::FlappyBirdClass()
{
}

FlappyBirdClass::~FlappyBirdClass()
{
    M5m.Lcd.fillScreen(0);
    M5m.Lcd.setTextSize(1);
	M5m.Lcd.setTextFont(1);
    M5m.drawAppMenu(F("GAMES"), F("ESC"), F("SELECT"), F("LIST"));
    M5m.showList();
}