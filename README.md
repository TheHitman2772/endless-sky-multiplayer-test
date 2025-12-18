# **This is not an official extension or creation of the official Endless Sky game!**
### This is simply a *work in progress* multiplayer test, partially because I have always wanted multiplayer in Endless Sky and partially because I got the idea from using Claude Code to make myself all sorts of tools recently. Claude Sonnet and Opus have generated correct code with no errors in about 98% of all the coding projects I've thrown at it so far, compared to around 85% on Google gemini 3 pro and around 45% on ChatGPT GPT5. After doing a handful of complex coding tests with it, I decided to run this gauntlet of painful and complex refactoring plus generation of a whole new backend system, and we're gonna see what it does. I'm aiming for 'completion' at a version of endless sky where two people can simply be in the same system together and have a synced up world state. Even if it breaks from landing or jumping to another system, I'll consider that a success and then anything extra after that is basically happy extras.
### So yeah, AI disclosure, this whole thing is going to be like 98% AI generated expansion cause that's what we're testing here. I highly encourage you to go play the base game if you don't like that. Actually I highly encourage you to play the base game *regardless*.

# Endless Sky

Explore other star systems. Earn money by trading, carrying passengers, or completing missions. Use your earnings to buy a better ship or to upgrade the weapons and engines on your current one. Blow up pirates. Take sides in a civil war. Or leave human space behind and hope to find some friendly aliens whose culture is more civilized than your own...

------

Endless Sky is a sandbox-style space exploration game similar to Elite, Escape Velocity, or Star Control. You start out as the captain of a tiny spaceship and can choose what to do from there. The game includes a major plot line and many minor missions, but you can choose whether you want to play through the plot or strike out on your own as a merchant or bounty hunter or explorer.

See the [player's manual](https://github.com/endless-sky/endless-sky/wiki/PlayersManual) for more information, or the [home page](https://endless-sky.github.io/) for screenshots and the occasional blog post.

## Installing the game

Official releases of Endless Sky are available as direct downloads from [GitHub](https://github.com/endless-sky/endless-sky/releases/latest), on [Steam](https://store.steampowered.com/app/404410/Endless_Sky/), on [GOG](https://gog.com/game/endless_sky), and on [Flathub](https://flathub.org/apps/details/io.github.endless_sky.endless_sky). Other package managers may also include the game, though the specific version provided may not be up-to-date.

## System Requirements

Endless Sky has very minimal system requirements, meaning most systems should be able to run the game. The most restrictive requirement is likely that your device must support at least OpenGL 3.

|Hardware | Minimum | Recommended |
|---|----:|----:|
|RAM | 750 MB | 2 GB |
|Graphics | OpenGL 2.0* | OpenGL 3.0 |
|Screen Resolution | 1024x768 | 1920x1080 |
|Storage Free | 400 MB | 1.5 GB |

\* For OpenGL 2 devices, [custom shaders](https://github.com/endless-sky/endless-sky/wiki/CreatingPlugins#shaders) are needed.

|Operating System | Minimum Version |
|---|---|
|Linux | Any modern distribution (equivalent of Ubuntu 20.04) |
|MacOS | 10.7 |
|Windows | XP (5.1) |

It may be possible to run Endless Sky on other operating systems, though it is not officially supported.

## Building from source

Development is done using [CMake](https://cmake.org) to compile the project. Most popular IDEs are supported through their respective CMake integration.

For full installation instructions, consult the [Build Instructions](docs/readme-developer.md) readme.

## Contributing

As a free and open source game, Endless Sky is the product of many people's work. Contributions of artwork, storylines, and other writing are most in-demand, though there is a loosely defined [roadmap](https://github.com/endless-sky/endless-sky/wiki/DevelopmentRoadmap). Those who wish to [contribute](docs/CONTRIBUTING.md) are encouraged to review the [wiki](https://github.com/endless-sky/endless-sky/wiki), and to post in the [community-run Discord](https://discord.gg/ZeuASSx) beforehand. Those who prefer to use Steam can use its [discussion rooms](https://steamcommunity.com/app/404410/discussions/) as well, or GitHub's [discussion zone](https://github.com/endless-sky/endless-sky/discussions).

Endless Sky's main discussion and development area was once [Google Groups](https://groups.google.com/g/endless-sky), but due to factors outside our control, it is now inaccessible to new users, and should not be used anymore.

## Licensing

Endless Sky is a free, open source game. The [source code](https://github.com/endless-sky/endless-sky/) is available under the GPL v3 license, and all the artwork is either public domain or released under a variety of Creative Commons (and similarly permissive) licenses. (To determine the copyright status of any of the artwork, consult the [copyright file](https://github.com/endless-sky/endless-sky/blob/master/copyright).)
