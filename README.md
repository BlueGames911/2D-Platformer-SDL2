# 2D Platformer made in SDL2

#### YouTube Link: https://youtu.be/A092AKttPbY

## Table of Contents
1. [Description](#description)
2. [Features](#features)
3. [Technologies Used](#technologies-used)
4. [Project Structure](#-project-structure)
5. [How to Run](#how-to-run)
6. [Controls](#controls)
7. [Design Decisions](#design-decisions)
8. [Development Journey](#development-log)


## Description

This project began as my CS50 Final Project, but it quickly became much more than that:

1. A way to hone my C programming skills.
2. My first attempt at building something close to a game engine.
3. A chance to gain real experience in game development, my dream field.
4. A prototype for a game a friend and I want to create someday (likely in MonoGame).

I chose to write this in C with SDL2 because:

1. David from CS50 made C my favourite language.
2. I always admired people who made games without full engines.
3. I used to watch SDL tutorials long before I understood programming.
4. SDL is cool—maybe not as magical as I imagined, but still great.

I learned a lot from this project. I love game programming, but wow, it can drive you crazy. This project was fun, incredibly challenging, and extremely informative. I now have skills I can apply to future projects and a much deeper appreciation for how games work under the hood (haha David reference).

## Features

### Core Gameplay
- **Fluid Combat System** - Multi-stage combo attacks with frame-perfect (ish) hit detection
- **Dynamic Movement** - Running, jumping, crouching, air attacks and acceleration based movement
- **Enemy AI** - Patrol-based enemies with hit detection and knockback
- **Multiple Levels** - Seamless level transitions with trigger zones
- **Health System** - Dynamic health bars with smooth animations
- **Dynamic loading of almost all elements in the game** - Levels, the player config, settings, etc are all loaded at runtime from JSON data files, an idea I am proud of indeed
- **Data is easily editable without having to change code or recompile** - As stated above, this is possible with the use of JSON files.

## Technologies Used

- **Language:** C (C11 standard)
- **Graphics:** SDL2, SDL2_image
- **Text Rendering:** SDL2_ttf
- **JSON Parsing:** cJSON library
- **Build System:** Visual Studio 2022 / GCC (CMake planned in the future)
- **Level Editor:** Tiled Map Editor 

## How to Run

### Prerequisites
- SDL2 development libraries (GCC or VS is useable)
- SDL2_image
- SDL2_mixer
- SDL2_ttf
- cJSON library
- C compiler (GCC/Clang/MSVC) (Not needed if Visual Studio is installed)

### Windows (Visual Studio Community)
1. Clone the repository
2. Open `project.sln` in Visual Studio 2022
3. Build solution (F7)
4. Run (F5)

### Controls

1. **moveLeft**: Left
2. **moveRight**: Right
3. **jump**: Space
4. **crouch**: Left Shift
5. **attack**: Z
6. **dash**: X 

#### **Controls are fully customizable in `settings.json`**

## Design Decisions
#### Why C instead of a Game Engine?
I chose to build this game in pure C with SDL2 rather than using Unity or Godot for several reasons:

1. **Deep Learning** - Building from scratch forced me to understand how game engines work internally or **"under the hood"** as David often says.
2. **CS50 Challenge** - I wanted to push beyond my capabilities to produce something I really struggled with, that would make the success all the more worth it
3. **Performance Understanding** - Managing memory, pointers, and low-level systems taught me optimization, and pain true pain.
4. **Portfolio Piece** - Demonstrates strong fundamentals in systems programming

## Development Log

### Week 1 ~ 2 (The beginnings)
- Needless to say, I have never built a game engine before, or a functional game even using game engines themselves. Before I started out, I watched a bunch of tutorials on this (And I didn't find much, turns out C and SDL2 aren't covered much). I watched a beginner tutorial by Programming Rainbow. This taught me the basics on how SDL2 works, and what I should do when starting a project, I got my SDL code style from him, well some of it ([Link to playlist](https://youtube.com/playlist?list=PLO02jwa2ZaiCgilk8EEVnfnGWA0LNu4As&si=f1DQgF0KWd_anIRw)). Since this tutorial was very bare bones, I supplemented my knowledge gap with other sources, being discord (SDL Enthusiasts Group), google, and chatGPT. I would use these sources to find out, what to do next, and if Im doing what Im currently busy with well at all. As for chatGPT, I used it as a semi project planner. 
- Starting out was fine, I installed my development libraries, decided on my project architecture (How my files are seperated) and made my core systems, that being my window and renderer. At this point in time, I had not thought of a "Game manager" yet, so all I had was init.c/h, and main.c. I made a way to exit the game, and from then on, I started with my player.
- I focused on first displaying a simple cube to the screen, with the ability to move and jump and unable to go past the boundaries of the screen. All this wasn't too hard, especially thanks to [this tutorial](https://youtu.be/XfZ6WrV5Z7Y?si=fe5Yu186F5rmZvY2). After that I focused on displaying the player's sprite to the screen, before that I started with a monolithic Player struct containing all movement and rendering logic. This worked fine for a single entity. Starting out I had a very manual approach, I would load the player's spritesheet in `init.c` and manually assign spritesheet coordinates in code for **all the animations**. I did not have a dynamic collision box either yet (Or one at all). 
- I had a simple movement system that was velocity based, with simple gravity as well. I also had a file called `constants.c` which I would use to store constant variables like the resolution, gravity force, run speed, tile size and scale, it is of no use to me now after some changes I made much later down the line. By the end of the 2 weeks I had a player struct, a few working animations and attack animations, and an adequate movement system. 

### Week 2 ~ 4 (A working level and Camera)
- The time came for my game to have a level with actual visible and collideable tiles, instead of the dark void my player was standing on. This was so difficult for me to figure out. I researched about this quite a bit, and landed on a free program on [itch.io](https://itch.io/) called Tiled. This was perfect for me, as it meant I didn't have to manually write out levels in code (Something I genuinely thought I had to do) and it exported to CSV, which I could use in my engine. At first I had a file called `tilemap.c` which was absolutely atrocious, I rendered each layer in its own function same with loading as well, and simply adding a new layer or tilemap required a lot of work, even worse, this system could only handle one level. This file, and the previous `player.c` will be included in my project directory, for reference. Spoiler, Im not very creative when it somes to level design :).
- When it came down to collision physics, I actually struggled a bit. Not to contextualize it, but to get it into code format, and when I eventually did, there were a few bugs I never got around to fixing. One being collision with a tile when moving left on the X axis, would make me teleport on top of the block, when I rewrote my physics system (spoiler) this just didn't happen anymore. 
- After implementing a level, I focused on making a functionable camera that would follow the player, but exceeding level bounds. It worked fine. 

### Week 5 ~ 6 **(The Great Refactor)**  
- At this point in time, I stepped back and took a look at my code, how I was doing things, and if I was happy with the direction I was moving in, needless to say, no I wasn't. I looked at my code, I saw hardcoded values, repetitions, and systems that I thought didn't make sense or simply did not like at all. To put it simply, I was disappointed with my code. I had so many resources to my disposal and I was disappointed that this was all I did. I decided to take a break from syntax in general, and actually look at my code, what it does, and what needs to go. First on the list was my `tilemap.c/h`, it was incredibly inefficient and just lack luster. Code was repeated, values hardcoded in, and a terrible issue with layers, adding new ones to be exact, not to mension my code only supported one level, if I were to implement more, it would have been filled with thousands of lines of repitions.

#### How to store levels
- I began asking myself, I'm not really sure what a level actually is. How does Unity do it? Etc. Even more so, I asked myself how do most people store levels. I looked at a "scene" from Unity and thought, "Isn't this file just describing the level?" (Yes those were my exact thoughts) and decided on something, I want a file I can load, and my engine will know how to load the level based on what that file tells it. I didn't know what type of file system I was going to use, I thought about txt files, files with no extension, or something I make myself but in the end, I landed on JSON files because of 3 reasons:
    1. I used JavaScript on a few problem sets, so I thought JSON files might be easy to get into.
    2. I like how organized they look.
    3. I found out c has support for JSON files in the form of the cJSON library.
#### How to describe levels in Code
- When I chose JSON as my chosen file type, I went back to answer, how would I describe a level. 
I decided it must have :
    - Tilesets, being the tilemap image that I load which contains all tiles for a given tileset. This will have a width and height, a scale so I can make them bigger and smaller on the go, a texture to actually display, and a tilersPerRow variable, this would make it easy for me when rendering the level, meaning no hardcoded values too
    - Layers, which is kind of an idea I stole from Tiled, which was to have different "layers" that would render at different Z coordinates, or in my case, in order. Each layer would have a string of the name of the tileset it's using (Which now looking back, could've easily been a pointer instead), a path to the CSV file exported from Tiled that puts into integers how the level is structured, whether the layers itself is collideable, meaning can the player and other entities make collisions with it, if so, it will have a list of collideable tiles (I used Tiled to see each individual tile number on each tilemap), and finally, an array of integers, which would be used to store the contents of the CSV file. Thinking of this simple structure actually took me an hour or 2, I vividly remember jotting things down and just struggling to think of a way to translate this into a struct.
    - Most importantly (to me atleast), backgrounds, with parallax, which is so cool.   
- From there I had the core of what I wanted my levels to be. I then had the idea of a simple JSON file that would have the paths to other JSON files that described individual levels, this way I could keep track of how many levels there are too, and so I began the creation of the new `level.c/h`, which was actually very fun. And I structured my first `level.json` file too. After a grueling task of learning cJSON syntax, which I must say is quite verbose, I had a simple `level.c` that would load a level based on the JSON file, load CSVs and textures provided by the paths in the JSON file, and render each layer based on the order they are in the JSON file. This took me a **long**, long time, and I'm very proud of it to this day. 

#### Refactoring my entire player code
- Since I was refactoring my level code entirely, I decided I should do the same for the player, because it too suffered from most of the problems `tilemap.c/h` had. 
I started from scratch again, at the player struct itself. Something I really spent a lot of time with was the animation system, since I wanted animation data to be pulled from a JSON file. It was fun recreating the player struct, so many things I could now change and optimize, what **wasn't** fun though, was coding the player from scratch, again. It dawned on me that I had to code a parser again. I tried to get Copilot to write it for me, since I knew it would be a lot of code, and in turn, time wasted, but, let's just say Copilot had issues. It generated the code, but it barely worked, which is weird because I already had the format of my JSON file figured out which I gave it as a reference but no, I had to go in and fix a lot of things on my own, sad. 
- While coding the player again, I decided I wanted a new movement system. This came as a result of me playing Hollow Knight and Cave Story recently. During playtime I questioned why their movement felt so good, compared to my previous one at least. I asked ChatGPT and there I learned about acceleration based movement, where acceleration and deceleration are used alongside velocity, which I thought was cool. I watched a couple of tutorials on it, and tried my best to translate it to code. The final result, I'd say I'm fine with it.
- I then overhauled my state system, so I could properly handle attacks (Before attacks were just animations, nothing else). I created an attacks struct and an attack stage struct, this came from me finding out my attack animations link into each other, like a combo attack, which was so cool, but hard to do with a single attack struct. Getting attack stages to work properly took a while, a long while but I came to a point where it worked, and after all that, I was happy with just that. As a little note, functions like `void Entity_StartAttackStage(Entity* e, int attackIndex, int stageIdx)` did not exist yet, the entity struct as a whole didn't exist yet, this was all player specific code at the time (spoiler, the entity code was just remodeled code that I wrote in `player.c`).
- As I was working on the player again, I remembered an idea I had a while ago to change the player's collision box based on his current animation, this was brought forth by some of his animations leaning to far to the right/left, and obviously the crouch animation's hitbox had to be smaller, so to each animation I added this:
```json
"collisionBox": {
  "w": 10,
  "h": 29,
  "offsetX": 0,
  "offsetY": 0
} 
```   
This allowed me to dynamically control each animation's collision box, though I had a few bugs at the start, which was my fault of course, but I got it working in the end. More parsing code for me, yay.

#### Finally adding a GameManager
- I soon ran into a major problem. Now that I supported multiple levels, how will I switch between those levels, who will keep track of how many levels there are, which one is the previous, next and current. I decided on some type scene manager, trying to imitate Unity in some way. I initially just wanted it to know information about the number of levels, when to load and unload each one, and when to load a new level. It was about to be a levelManager instead, but I began thinking of something else, settings, yes, thinking of settings changed the whole trajectory. I decided I also want a system to keep track of settings, then I thought of a pause menu, and I decided I want something to keep track of the game state. With every new edition, the new answer became clear, I needed a central game manager. Something that houses **ALL** the data my game will need, somewhere I could keep my pointers too, instead of just passing them around everywhere. 
- Thus I started with the creation of my GameManager struct. I made sure it had everything my code would need: a pointer to the player, a pointer to the level, the camera struct, the main systems (window and renderer), the settings values, etc. I cannot tell you how proud I am of this. Now I had a main entry point and exit point, which I was missing in the beginning as well. Not to mention my `main.c` looks so clean now, just look at it:
```c
int main(int argc, char* argv[])
{
    GameManager* gm = GameManager_Create("settings.json", "Game v0.4");
    if (!gm) {
        fprintf(stderr, "Failed to initialize GameManager\n");
        return EXIT_FAILURE;
    }

    float deltaTime;
    Uint32 last_ticks = SDL_GetTicks();
    Uint32 current_ticks;

    while (gm->running) {
        current_ticks = SDL_GetTicks();
        deltaTime = (current_ticks - last_ticks) / 1000.0f;
        last_ticks = current_ticks;

        GameManager_HandleInput(gm);
        GameManager_Update(gm, deltaTime);
        GameManager_Render(gm);

        SDL_Delay(15);
    }

    GameManager_Destroy(gm);
    printf("Successfully cleaned up and exiting.\n");
    return EXIT_SUCCESS;
}
```
Basic I know, but clean nontheless.
#### Implementing Settings
- I got started with `settings.c/h` around the same time I half finished the GameManager struct. I decided I at least need a better way to keep track of my resolution, because keeping it in `constants.h` just wasn't doing it for me at all. I got started creating the settings struct, and thought of other things I might need, just to populate it and make it look more professional. I decided to add booleans for full screen and V-Sync for additional functionality, just because I can. I asked Copilot what I could add next, and it recommended audio settings, and controls. I had already thought of implementing customizable controls, but decided to push the thought aside because I had no idea how I would do that. I decided why not, and asked Copilot for help. I finished the settings struct by adding audio settings (Though left useless for now), controls, and a debug button to display collision boxes, and more information in the future. As I got started with `settings.c` I ran into a problem, I had no idea how to parse strings to SDL_ScanCodes, so I created the JSON parser, left out the controls, implemented a default controls feature, and asked Copilot to generate `static bool json_get_bool(cJSON *obj, const char *key, bool def)`. 

#### Enemies
- I was finally done with my player, again, well mostly, and now he needed something to hit. This was something I was the most afraid of doing, because never have I coded enemy AI, not even in Unity, though I never used it that much. I got started with an enemy struct. As I was writing the struct, I realized it shares a lot of data with the player, therefore plenty of code was repeated, so I decided it's time to create an entity struct and that's what I did, basically copy/pasting from the enemy struct, and putting animation specific structs into `animation.h`. The enemy struct itself did not take me long. I got started with `enemy.c` and again, I had more parsing code, but this one was fun, especially `enemy** LoadEnemiesFromFile(const char* path, mainSystems systems, Level *lvl)`. So I coded the two enemy loading functions, with a little help by Copilot by having it create `char* GetConfigPathForType(const char* type)`. Since I had a newly created `entity.c`, I didn't have to create more rendering, physics and other code so it went by fine, until I realized something.
- I already had enemy sprites ready that I got from [itch.io](https://itch.io/), the main thing was that I had different enemy types, meaning I couldn't write the same logic for all of them, some would throw projectiles, some would fly, some would block, and I didn't just want to throw a bunch of if statements in my update function. I had no idea what to do about this, so I asked ChatGPT, and it recommended I use something called V-Tables. From what I understand, V-Tables try to imitate polymorphism to some extent. With this, each enemy would share the same struct, but if it has unique behavior, I can code its own functions for it and link them to the V-Table, it seemed complicated, cause I saw pointers in function definitions, but I don't think it's so bad now. I created logic for my first enemy ever, the Goblin, though I haven't added any other enemy due to lack of time, but I just might. Coding the Goblin's behavior took **SO** long, it took me about 2 days just to get him to walk around (Patrol), and that's not counting the full day I took to get him to spawn. If there's any code I had the most problems and bugs with, more than anything else I've worked with, yes even the player and level code.
- I coded a few states for the Goblin, and got him to stop next to the player when in attack range. Now came the time for him to attack. I coded his attack state logic, which was mostly copy/paste from the player. The attacks or should I say his singular attack was not very hard to code so I went to the next step, a way to get the Goblin to get hurt or die. Hurt logic was a bit complicated. I knew I had to play the hurt animation, but I also wanted the Goblin to be invulnerable during the hurt animation, so I made a timer, a hurtTimer to track how long until the enemy can be hurt again (Yes I know I have an invulnerable field in my JSON files under animations, I completely forgot about it, it would've been much simpler to use that.) I played around with hitstop and knockBack a bit, they don't feel particularly good, but hey, they work. 
- Now came the time for death code. At first, I wanted to free an enemy each time it was killed by the player, and I did, not realizing what a stupid idea that was. It worked, sometimes, the other times I would get exception errors. After all that was done I made hurt and death logic for the player, very similar to the enemy code as well. I wanted the player to fade out when he died, so that's what I did, I was now approaching the final stretch.

### Week 7 ~ 8 (Finishing up)

#### UI
- For the last few touches I wanted to add UI to my game, even if very minimal. The feature I wanted most, was a health bar, for both the enemies and the player. I first created a UI struct, to store all the fonts and UI elements I'd need, then got started with my health bar struct. I initially thought I should make the Player's health bar be tied to the world, like I do with the enemies, but my Discord friends said I shouldn't, so I made the Player's health bar be tied to the screen. In my render functions, I actually didn't cache the textures before, I built them there and destroyed them there every frame, but of course when I showed some friends on Discord, they weren't happy, so I cached the textures instead, only updating when the percentage changed and only building the HP texture once.
- I felt I was ready to submit, after one more thing though. I wanted a death screen that allowed me to respawn and restart the level. This was my first time making something like this, but I quickly found out it wasnt very different from what I did with the healthbars. 
- Since I was in the UI mood, I added what I call a "level trigger". This was used to transition between levels. Even right now, just a day after implementing it, I can tell there's a lot I can do to make it better, but I was in a rush o.o . I slapped together a makeshift second level (Yes, even now months later, I'm not much of a designer).

## Conclusion

This project pushed me harder than anything I’ve built so far.
It taught me about:

- Low-level systems programming
- Engine architecture
- Data driven design
- Debugging complex interactions
- Patience, a lot of it infact  
And most importantly…
It further strengthened my love for games and programming.




