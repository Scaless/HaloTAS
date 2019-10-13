---
layout: post
title:  "Fast Isn't Fast Enough"
date:   2019-10-13 12:18:04 -0400
categories: halo
---

[github.com/Scaless/HaloTAS](https://github.com/Scaless/HaloTAS)

If you would like to support my projects, check out my [Patreon](https://www.patreon.com/scalesllc).

---

Recently a new geo-bump (teleporting your character with level geometry) was found on the level Keyes. If you are not entirely familiar with Halo 1 speedrunning, Keyes is by far the most optimized speedrun level in the game. The current IL record for Keyes on Easy difficulty is 2 minutes and 18 seconds. With current strats we think it is theoretically possible to get a 2:17 with perfect RNG and movement. 

The new geo-bump strategy is very difficult to pull off consistently. In 200 attempts I was only able to do it twice. It requires incredibly precise movement and look direction while being inside of a wall with no currently consistent human-possible setup.

I used this as an opportunity to test the viability of this strat with HaloTAS. Given how optimized the level is, shaving off even a second is quite considerable. 

This TAS recording represents an 'average' flood bump and a frame perfect geo-bump:

<iframe width="560" height="315" src="https://www.youtube.com/embed/w64Q50cA-8M" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

Unfortuntely, the strat wastes too much time setting up the 2nd bump and is just too inconsistent to be viable to use in ILs. Even in a TAS run with perfectly manipulated RNG it may still be slower than the normal out of bounds platforming. 

For me however, this was an amazing test of HaloTAS. The recording utilizes all of the core aspects of the game:

* Player movement and view angles
* Enemy AI
* Physics engine
* RNG is manipulated correctly
	* AR spread is based on RNG, causing each bullet to be randomly distributed in a cone. The recording kills the grunt in the same way every time, even causing the same death animation. The amount of time that the flood takes to revive is also based on RNG.

I was pretty happy with this recording, but I wanted to go *faster*.

One of the main pain points of the current TAS tools is iteration time. Starting input in a new TAS is simple enough, but once you are dozens of seconds into a level it is tedious to constantly restart the level in real-time. Most levels are several minutes long, so this had to be addressed eventually. 

My first attempt to improve iteration speed was to use the internal Halo engine functions `core_save` and `core_load`. `core_save` takes a snapshot of game state and saves it to a file, while `core_load` restores state from that file. Immediately I ran into problems doing this:

* The snapshot of game state does not include RNG, so the sequence will immediately be out of sync after loading a `core_save`. I resolved this by saving the RNG state in the application and restoring it after the game is reverted.
* The engine only maintains one `core.bin` file that state is saved to. When you `core_save` it overwrites this file with the new state.
* If you `core_load` on a different map than the one you saved on, the game crashes.
* Your last input state is preserved after a `core_load`. For example: if you are holding W when you issue the `core_load`, you will be holding W when the revert occurs. If you were not holding W on the tick that you loaded into, you've now desynced. I wasted a ton of time working on desynced files before figuring this out.

In the long run this system wasn't going to work. I started messing around in [Ghidra](https://www.nsa.gov/resources/everyone/ghidra/) looking for alternatives. 

If we want to simulate 10000 ticks quickly to get to the end of a recording, you might wonder why we can't just call the tick function directly. 

Normally Halo 1 is limited to 30 ticks/second. In each tick, engine components are updated: physics, AI, movement, scripts, etc. Mouse/keyboard/gamepad input, however, is tracked on a per-frame basis. Since the game is capable of running at an unlocked framerate, your view needs to update at a faster rate so it is interpolated from your inputs. If we called the tick function directly, we would not get updated input information and end up with 10000 ticks of the same input which is quite useless.

One of the features of Halo PC is the launch flag `-timedemo`. When you start the game with this flag, it will immediately play through the opening cutscene of a few levels then close the game and save a file with some FPS benchmark information (you can see my results [here!]({{ site.baseurl }}/assets/timedemo.txt)).

I noticed that using `-timedemo` you are not restricted to 30 ticks/second! This makes sense as it wants to run as fast as possible for a good benchmark result. 

By twiddling a few bits in the tick handling system we can trick the game into thinking we are in benchmark mode, allowing us to run the game at *incredible hihg speed* while maintaining input integrity. This is the same input sequence as the previous video, but with the new benchmark system:

<iframe width="560" height="315" src="https://www.youtube.com/embed/xSPpZSJTSkI" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

With this new system we can quickly advance to a specific tick anywhere in the playback within seconds, and I can finally strip out the flawed `core_x` utilities. 

This is my first 'blog' post on this project, let me know if I should keep making these in the future.

~Scales
