//   >>>>>  T-I-N-Y  L-A-N-D-E-R v1.0 for ATTINY85  GPLv3 <<<<
//              Programmer: (c) Roger Buehler 2020
//              Adapted for FastTinyDriver by SlickAlex
//              Official: https://github.com/tscha70/ | https://www.tinyjoypad.com
//              This version: uses FastTinyDriver for the SSD1306 OLED; 

#include "FastTinyDriver.h"
#include "spritebank.h"
#include "gameinterface.h"

void setup() {
  TinyOLED_init();
  // Fill screen black (formerly SSD1306.ssd1306_fillscreen(0x00))
  for (uint8_t y = 0; y < 8; y++) {
    ssd1306_selectPage(y);
    for (uint8_t x = 0; x < 128; x++) i2c_write(0x00);
    i2c_stop();
  }
  TINYJOYPAD_INIT();
}

void loop() {
  DIGITAL score;
  DIGITAL velX;
  DIGITAL velY;
  GAME game;
BEGIN:
  game.Level = 1;
  game.Score = 0;
  game.Lives = 4;
  while (1) {
    Tiny_Flip(1, &game, &score, &velX, &velY);
    if (digitalRead(1) == 0) {
      if (JOYPAD_UP){ 
        game.Level = 10;
        ALERTSOUND();
      }
      else if (JOYPAD_DOWN) {
        game.Lives = 255;
        ALERTSOUND();
      }
      else {
        SOUND(100, 125);
        SOUND(50, 125);
      }
      goto START;
    }
  }
START:
  initGame(&game);
  INTROSOUND();
  while (1) {
    fillData(game.Score, &score);
    fillData(game.velocityX, &velX);
    fillData(game.velocityY, &velY);
    moveShip(&game);
    changeSpeed(&game);

    Tiny_Flip(0, &game, &score, &velX, &velY);
    if (game.EndCounter > 8) {
      if (game.HasLanded)
      {
        showAllScoresAndBonuses(&game, &score, &velX, &velY);
        delay(500);
        goto START;
      }
      else
      {
        delay (2000);
        if (game.Lives > 0)
          goto START;
        goto BEGIN;
      }
    }
    if (game.ShipExplode > 0 || game.Collision)
      game.EndCounter++;
    if (game.HasLanded)
      game.EndCounter = 10;
  }
}

void initGame (GAME * game)
{
  SETNEXTLEVEL(game->Level, game);

  game->velocityY = 0;
  game->velocityX = 0;
  game->velXCounter = 0;
  game->velYCounter = 0;
  game->ShipExplode = 0;
  game->Toggle = true;
  game->Collision = false;
  game->HasLanded = false;
  game->EndCounter = 0;
  game->Stars = 0;
}

void showAllScoresAndBonuses(GAME *game, DIGITAL *score, DIGITAL *velX, DIGITAL *velY)
{
  VICTORYSOUND();
  game->Level++;
  delay (1000);
  uint8_t bonusPoints = 0;

  // add bonus points
  if ((abs(game->velocityY)) <= BONUSSPEED2)
    bonusPoints++;
  if ((abs(game->velocityY)) <= BONUSSPEED1)
    bonusPoints++;
  if (game->Fuel >= game->FuelBonus)
    bonusPoints++;

  for (game->Stars = 1; game->Stars <= bonusPoints; game->Stars++)
  {
    Tiny_Flip(2, game, score, velX, velY);
    HAPPYSOUND();
    delay(500);
  }
  game->Stars--;

  uint16_t newScore = game->Score + game->LevelScore  + (game->LevelScore * bonusPoints );
  while (game->Score < newScore)
  {
    game->Score++;
    fillData(game->Score, score);
    Tiny_Flip(2, game, score, velX, velY);
    SOUND(129, 2);
  }
}

void changeSpeed(GAME * game)
{
  game->ThrustLEFT = JOYPAD_LEFT;
  game->ThrustRIGHT = JOYPAD_RIGHT;
  game->ThrustUP = JOYPAD_FIRE;
  game->Toggle = !game->Toggle;

  if (game->ThrustLEFT && game->Fuel > 0)
  {
    game->Fuel -= (FULLTHRUST / 2);
    game->velocityX += TrustX;
    if ((game->velocityX) > VLimit)
      game->velocityX  = VLimit;
  }
  else if (game->ThrustRIGHT && game->Fuel > 0)
  {
    game->Fuel -= (FULLTHRUST / 2);
    game->velocityX -= TrustX;
    if ((game->velocityX) < -VLimit)
      game->velocityX  = -VLimit;
  }

  if (game->ThrustUP && game->Fuel > 0)
  {
    game->Fuel -= (FULLTHRUST * 2);
    game->velocityY += TrustY;
    if ((game->velocityY) > VLimit)
      game->velocityY  = VLimit;
  }
  else
  {
    game->velocityY -= (GRAVITYDECY);
    if ((game->velocityY) < -VLimit)
      game->velocityY  = -VLimit;
  }

  if ((game->Fuel) <= 0)
    game->Fuel = 0;
}

void moveShip(GAME * game)
{
  if (game->ShipExplode > 0 || game->Collision || game->HasLanded) return;

  game->velXCounter += abs(game->velocityX);
  game->velYCounter += abs(game->velocityY);

  if ((game->velXCounter) >= MoveX) {
    game->velXCounter = 0;
    if ((game->velocityX) > 0)
      game->ShipPosX += 1;
    if ((game->velocityX) < 0)
      game->ShipPosX -= 1;
  }

  if (game->velYCounter >= MoveY) {
    uint8_t inc = (abs(game->velocityY) / ACCELERATOR) + 1;
    (game->velYCounter) = 0;
    if ((game->velocityY) > 0)
      game->ShipPosY -= inc;
    if (game->velocityY < 0)
      game->ShipPosY += inc;
  }

  // boundaries....
  if (game->ShipPosX > 121)
  {
    game->ShipPosX = 121;
  }
  else if (game->ShipPosX < 23)
  {
    game->ShipPosX = 23;
  }
  if (game->ShipPosY > 55)
  {
    game->ShipPosY = 55;
  }
}

void fillData(long myValue, DIGITAL * data)
{
  SPLITDIGITS(abs(myValue), data->D);
  data->IsNegative = (myValue < 0);
}

uint8_t ScoreDisplay(uint8_t x, uint8_t y, DIGITAL * score) {
  if  ((y != 1) || (x < SCOREOFFSET) || (x > (SCOREOFFSET + (SCOREDIGITS * DIGITSIZE) - 1))) {
    return 0;
  }
  uint8_t part =  (x - SCOREOFFSET) / (DIGITSIZE);
  return pgm_read_byte(&DIGITS[x - SCOREOFFSET - (DIGITSIZE * part) + (score->D[(SCOREDIGITS - 1) - part] * DIGITSIZE)]);
}

uint8_t VelocityDisplay(uint8_t x, uint8_t y, DIGITAL * velocity, uint8_t horizontal)
{
  if ((horizontal == 1 && y != 4) || (horizontal == 0 && y != 5)) {
    return 0;
  }
  if ((x < VELOOFFSET) || (x > (VELOOFFSET + (VELODIGITS * DIGITSIZE)) - 1)) {
    return 0;
  }
  if ((x >= VELOOFFSET) && (x < (VELOOFFSET + DIGITSIZE))) {
    return pgm_read_byte(&DIGITS[x - VELOOFFSET + ((10 + (velocity->IsNegative)) * DIGITSIZE)]);
  }
  uint8_t part =  ((x - VELOOFFSET) / (DIGITSIZE));
  return pgm_read_byte(&DIGITS[x - VELOOFFSET - (DIGITSIZE * part) + (velocity->D[(VELODIGITS - 1) - part] * DIGITSIZE)]);
}
uint8_t DashboardDisplay(uint8_t x, uint8_t y, GAME * game)
{
  if (x >= 0 && x <= 22) {
    return pgm_read_byte(&DASHBOARD[x + y * 23]);
  }
  return 0x00;
}

uint8_t LanderDisplay(uint8_t x, uint8_t y, GAME * game) {
  uint8_t line = game->ShipPosY / 8;
  uint8_t offset = game->ShipPosY % 8;
  if (y == line || ((y == line + 1) && offset > 0))
  {
    if (((x - game->ShipPosX) >= 0) && ((x - game->ShipPosX) < 7)) {
      uint8_t sprite = getLanderSprite (x, y, game);
      if (offset == 0 && y == line)
        return sprite;
      if (offset > 0 && y == line)
        return sprite << offset;
      if (offset > 0 && y == (line + 1))
        return sprite >> (8 - offset);
    }
  }
  return 0x00;
}

uint8_t getLanderSprite(uint8_t x, uint8_t y, GAME * game)
{
  uint8_t sprite = 0x00;
  if (game->ShipExplode > 0)
  {
    sprite = pgm_read_byte(&LANDER[(x - game->ShipPosX) + ((8 - (game->ShipExplode)) * 7) ]);
    SOUND(20 * game->ShipExplode, 10);
    (game->ShipExplode)--;
    if (game->ShipExplode < 1)
      game->ShipExplode = 3;
    return sprite;
  }
  if (game->ThrustLEFT)
    sprite = pgm_read_byte(&LANDER[(x - game->ShipPosX) + 21]);
  else if (game->ThrustRIGHT)
    sprite = pgm_read_byte(&LANDER[(x - game->ShipPosX) + 28]);
  else
    sprite = pgm_read_byte(&LANDER[(x - game->ShipPosX) ]);
  if (game->ThrustUP && game->Toggle && game->Fuel > 0)
    return (sprite |= pgm_read_byte(&LANDER[(x - game->ShipPosX) + 14]));
  else
    return (sprite |= pgm_read_byte(&LANDER[(x - game->ShipPosX) + 7]));
}

uint8_t FuelDisplay(uint8_t x, uint8_t y, GAME * game)
{
  if (y != 6) return 0x00;
  if (x > 4 && x <= 19)
  {
    if ((game->Fuel / 1000) + 1 > x - 4 || ((x - 4 == 1) && game->Fuel > 0))
      return 0xF8;
    else
      return 0x00;
  }
  return 0x00;
}

uint8_t GameDisplay(uint8_t x, uint8_t y, GAME * game)
{
  const uint8_t offset = 23;
  if (x >= offset)
  {
    uint8_t frame;
    if (x == offset || x == 127)
      frame = 0xFF;
    else
      frame = GETLANDSCAPE(x - offset, y, ((game->Level - 1) * 2), game);
    uint8_t ship = LanderDisplay(x, y, game);
    if (y == 7 && x >= (game->LandingPadLEFT + offset) && x <= (game->LandingPadRIGHT + offset))
    {
      if (ship != 0 && (0xFC | ship) != (0xFC + ship))
      {
        if (abs(game->velocityY) <= LANDINGSPEED && (game->ShipPosX >= game->LandingPadLEFT + offset) && (game->ShipPosX + 7 <= game->LandingPadRIGHT + offset) )
        {
          game->HasLanded = true;
          return frame | ship;
        }
        else
        {
          if (!game->Collision)
            game->Lives--;
          game->ShipExplode = 3;
          game->Collision = true;

          return frame | LanderDisplay(x, y, game);
        }
      }
    }
    else if  ((frame != 0 && ship != 0) && (frame | ship) != (frame + ship))
    {
      if (!game->Collision)
        game->Lives--;
      game->ShipExplode = 3;
      game->Collision = true;
      return frame | LanderDisplay(x, y, game);
    }
    return frame | ship;
  }
  return 0x00;
}

uint8_t StarsDisplay(uint8_t x, uint8_t y, GAME * game)
{
  const uint8_t o1 = 23;
  uint8_t bg = 0x00;
  if (y == 0 && x > o1)
    bg |= 0x01;
  if (x == o1)
    bg |= 0xFF;
  if (x == 127)
    bg |= 0xFF;
  if (y == 7 && x > o1)
    bg |= 0x80;
  const uint8_t offset = 40;
  if (y > 1 && y < 5)
  {
    if (x > offset &&  x < (offset + 72))
    {
      if (game->Stars > (x - offset) / 24)
        return pgm_read_byte(&STARFULL[((x - offset) % 24) + ((y - 2) * 24)] );
      else
        return pgm_read_byte(&STAROUTLINE[((x - offset) % 24) + ((y - 2) * 24)] );
    }
  }
  return bg;
}

uint8_t LivesDisplay(uint8_t x, uint8_t y, GAME * game)
{
  const uint8_t offset = 1;
  if (y == 7 && x >= offset && x < (4 * 5) + offset)
    if (game->Lives > (x - offset) / 5)
      return pgm_read_byte(&LIVE[(x - offset) % 5]);
  return 0x00;
}

void Tiny_Flip(uint8_t mode, GAME * game, DIGITAL * score, DIGITAL * velX, DIGITAL * velY) {
  for (uint8_t y = 0; y < 8; y++) {
    ssd1306_selectPage(y);
    for (uint8_t x = 0; x < 128; x++) {
      uint8_t px;
      if (mode == 0) {
        px = GameDisplay(x, y, game) | LivesDisplay(x, y, game) | DashboardDisplay(x, y, game)
           | ScoreDisplay(x, y, score) | VelocityDisplay(x, y, velX, 1) | VelocityDisplay(x, y, velY, 0)
           | FuelDisplay(x, y, game);
      } else if (mode == 1) {
        px = pgm_read_byte(&INTRO[x + (y * 128)]);
      } else if (mode == 2) {
        px = StarsDisplay(x, y, game) | LivesDisplay(x, y, game) | DashboardDisplay(x, y, game)
           | ScoreDisplay(x, y, score) | VelocityDisplay(x, y, velX, 1) | VelocityDisplay(x, y, velY, 0)
           | FuelDisplay(x, y, game);
      }
      i2c_write(px);
    }
    i2c_stop();
  }
}