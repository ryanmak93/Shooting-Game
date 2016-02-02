#include <LedControl.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12,11,5,4,3,2);

int dataIn = 10;
int clk = 9;
int load = 8;
LedControl lc=LedControl(dataIn,clk,load,1);

int pot1 = 0;
int pot2 = 1;
int button1 = 6;
int button2 = 7;

int shotCollision = 0;
int gameSpeed = 1;
int lifeCount = 3;
int gameTime = 60;
int currentTime;

int count1_1;
int count2_1;
int count1_2;
int count2_2;

int player1[3] = {0,1,2};
int player2[3] = {5,6,7};
int p1read1;
int p1read2;
int p2read1;
int p2read2;
int p1_life = 3;
int p2_life = 3;
int p1_shots[9][2];
int p2_shots[9][2];
int p1t1 = 0;
int p2t1 = 0;
int p1t2;
int p2t2;
String endString[4] = {"","Player1 wins","Player2 wins","DRAW"};
int game_set = 0;
int game_end = 0;
int count;


void setup()
{
  pinMode(button1,INPUT);
  pinMode(button2,INPUT);
  pinMode(pot1,INPUT);
  pinMode(pot2,INPUT);
  lcd.begin(16,2);
  Serial.begin(9600);
  lc.shutdown(0,false);//wake up max7219
  lc.setIntensity(0,8);
  analogWrite(3,35);//write contrast to lcd
  count = 0;
}

void gameSettings()
{
  while(!game_set)
  {   
    Serial.println("Game Settings:");
    Serial.println("1-Game Speed(default:1)");
    Serial.println("2-Life Count(default:3)");
    Serial.println("3-Time Limit (negative for no time limit,default:60)");
    Serial.println("4-Shot collision (0  or 1, default 0)");
    Serial.println("5-Start Game");
    while(!Serial.available()){}
    int input = Serial.parseInt();     
    if(input == 1)
    {
       Serial.println("Enter number between 1 and 3:");
       while(!Serial.available()){}
       int input = Serial.parseInt(); 
       if(input >= 0 && input <=3)
       {
         gameSpeed = input;
       }
       else
         Serial.println("Invalid input:");
    }
    if(input == 2)
    {
      Serial.println("Enter number, negative for no life limit:");
       while(!Serial.available()){}
       int input = Serial.parseInt(); 
       lifeCount = input;   
    }
    if(input == 3)
    {
       Serial.println("Enter number, negative for no time limit,:");;
       while(!Serial.available()){}
       int input = Serial.parseInt(); 
       gameTime = input;   
    }
    if(input == 4)
    {
       Serial.println("Enter 0 or 1 to toggle shot collision:");;
       while(!Serial.available()){}
       int input = Serial.parseInt(); 
       if(input == 0 || input == 1)
       {
         shotCollision = input;
       }
    }
    if(input == 5)
    {
      game_set = 1;
    }
  }
}

void loop()
{
  if(game_set == 0)
  {
    gameSettings();
    p1_life = lifeCount;
    p2_life = lifeCount;
    p1t1 = 0;
    p2t2 = 0;
    currentTime = gameTime;
    game_end = 0;
    reset_shots();
    count1_1 = 0;
    count2_1 = 0;
    count1_2 = 4;
    count2_2 = 4;
  }
  if(game_end > 0)
  {
    lcd.clear();
    lcd.print(endString[game_end]);
    game_set = 0;
  }
  else
  {
    
   lc.clearDisplay(0);
   
   //movement
   p1read1 = analogRead(pot1);
   p2read1 = analogRead(pot2);
   if(p1read1-p1read2 > 20 || p1read1-p1read2 < -20) //use value of 20 so the small changes in analog aren't registered, smooths out the movement
   {
     movePlayer(player1,p1read1-p1read2);
   }
   if(p2read1-p2read2 > 20 || p2read1-p2read2 < -20)
   {
     movePlayer(player2,p2read1-p2read2);
   }
   
   //shooting
   //use count to put a cooldown on the shots
   count1_1 = count;
   count2_1 = count;
   int p1shot = count1_1 - count1_2;
   int p2shot = count2_1 - count2_2;
   if(digitalRead(button1) == HIGH && p1shot > 3)
   {
     p1shoot();
     count1_2 = count;     
   }
   if(digitalRead(button2) == HIGH && p2shot > 3)
   {
     p2shoot();
     count2_2 = count;
   }
   advanceShots();
   if(shotCollision)
   {
     collision();
   }
   //write all the appropriate LEDS to high
   ledPlayers();
   ledShots();
   
   p1read2 = analogRead(pot1);
   p2read2 = analogRead(pot2);
   count++;
   if(gameTime > 0)
   {
     if(count % 3 == 0)
       currentTime--;
     if(currentTime == 0)
     {
       if(p1_life < p2_life)
         game_end = 1;
       else if(p1_life > p2_life)
         game_end = 2;
       else
         game_end = 3;
     }
     
   }
   updateLCD();
   delay(500 - gameSpeed*100);//delay set by game speed
  }

}



//to make it the same for both players, movement will be up and down instead of left and right
void movePlayer(int player[],int movement)
{
  if(movement > 0)//move up
  {
    if(player[2] != 7)
    {
      player[0]++;
      player[1]++;
      player[2]++;
    }
  }
  else
    if(player[0] != 0)//move down
    {
      player[0]--;
      player[1]--;
      player[2]--;
    }
}

void p1shoot()
{
  int i;
  for(i=0;i<9;i++)
  {
    if(p1_shots[i][0] == -1)
    {
      p1_shots[i][0] = 0;
      p1_shots[i][1] = player1[1];
      //Serial.println("shot");
      return;
    }
  }
}

void p2shoot()
{
  int i;
  for(i=0;i<9;i++)
  {
    if(p2_shots[i][0] == -1)
    {
      p2_shots[i][0] = 7;
      p2_shots[i][1] = player2[1];
      return;
    }
  }
}

void advanceShots()
{
   //advance each existing shot
   int i;
   for(i=0;i<9;i++)
   {
     if(p1_shots[i][0] != -1)
     {
       p1_shots[i][0]++;
       
       if(p1_shots[i][0] == 7)
       {         
         if(p1_shots[i][1] == player2[0] || p1_shots[i][1] == player2[1] || p1_shots[i][1] == player2[2])
         {           
           p2_life--;
           if(p2_life == 0)
           {
             game_end = 1;
           }
         }
         p1_shots[i][0] = -1;
       }
     }    
   
     if(p2_shots[i][0] != -1)
     {
       p2_shots[i][0]--;
       
       if(p2_shots[i][0] == 0)
       {
         if(p2_shots[i][1] == player1[0] || p2_shots[i][1] == player1[1] || p2_shots[i][1] == player1[2])
         {
           p1_life--;
           if(p1_life == 0)
           {
            game_end = 2;
           }
         }
         p2_shots[i][0] = -1;
       }
     }       
   }
}

void ledPlayers()
{
  int i;
  for(i=0;i<3;i++)
  {
    lc.setLed(0,0,player1[i],true);
    lc.setLed(0,7,player2[i],true);
  }
}

void ledShots()
{
  int i;
  for(i=0;i<9;i++)
  {
    if(p1_shots[i][0] != -1)
    {
      lc.setLed(0, p1_shots[i][0], p1_shots[i][1],true);
    }
    if(p2_shots[i][0] != -1)
    {
      lc.setLed(0,p2_shots[i][0],p2_shots[i][1],true);
    }    
  }
}

void updateLCD()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Time:");
  if(gameTime > 0)
  {
    lcd.print(currentTime);
  }
  else
  {
    lcd.print("--");
  }
  lcd.setCursor(0,1);
  lcd.print("1:");
  lcd.print(p1_life);
  lcd.print("  |  ");
  lcd.print("2:");
  lcd.print(p2_life);
}

void collision()
{
  int i;
  int j;
  for(i=0;i<9;i++)
  {
    for(j=0;j<9;j++)
    {
      if((p1_shots[i][0] == p2_shots[j][0] || p1_shots[i][0] + 1 == p2_shots[j][0] )&& p1_shots[i][1] == p2_shots[j][1])
      {//if both shots are in same position, they cancel out
        p1_shots[i][0] = -1;
        p2_shots[j][0] = -1;
        
      }
    }
  }
}

void reset_shots()//set the arrays to -1
{
  int i;
  for(i=0;i<9;i++)
  {
    p1_shots[i][0] = -1;
    p2_shots[i][0] = -1; 
  }
}
