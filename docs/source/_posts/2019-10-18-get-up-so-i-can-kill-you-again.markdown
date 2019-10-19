---
layout: post
title:  "Get Up, So I Can Kill You Again!"
date:   2019-10-18 -0400
categories: halo
---

Today we're going to go back in time! (Warning: contains eurobeat):

<iframe width="640" height="360" src="https://www.youtube.com/embed/iyX9cpZZ-AI" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

As you can hopefully see, progress is going well on the rewind feature in HaloTAS. We can now easily go forward and backward in time which greatly helps with iteration time on TAS files.

You may notice the screen is shaking a bit in the video, which is not an effect I added. Any screen effect that you incur over the course of re-playing a demo will persist until it is rendered to completion, including: AR shots, grenade blasts (flashes screen yellow), taking damage (flashes screen red), and some scripted events. 

During a fast-forward, we disable the renderer to greatly speed up the time it takes to simulate ticks by not issuing any draw calls for each frame. A side effect of this is that the screen effect timers never advance through their animations and will end up stacking on top of each other. Effects can range from annoying to unplayable, such as grenade explosions:

![screen_effect_explosion.jpg]({{site.baseurl}}/assets/screen_effect_explosion.jpg)

Until I find a true way to disable screen effects without drastically editing game files, I've implemented a workaround that works fairly well. The process to fast-forward to tick 1000 would be:

1. Reset the map
2. Disable the renderer
3. Simulate ticks 0 - 940
4. Enable the renderer, but disable framebuffer updates
5. Simulate ticks 941 - 1000
6. Enable screen updates

We spend the last 60 ticks rendering to nowhere<sup>1</sup>, but doing it this way lets screen effects run to completion and provides us with a smooth transition when traversing through time:

<iframe width="640" height="360" src="https://www.youtube.com/embed/LUNra1tLzT4" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

---

### TimeSplitters

While working on these time-altering tools, we made an interesting discovery<sup>2</sup>: disabling or enabling sound actually changes how the game runs. To understand why this is, we first need to discuss how the flow of time works in-game. From the start of a level, there are two paths we can take:

1. Skip the opening cutscene with Enter or Esc
2. Let the opening cutscene play to completion

![tick_flowchart.png]({{site.baseurl}}/assets/tick_flowchart.png)

In speedrun/TAS gameplay, (1) is always preferred when possible as skipping the cutscene is faster. Around half of levels do not have skippable opening cutscenes so the (2) path is taken in those cases. By skipping the cutscene we are re-setting the timeline back to Tick 2, but not *completely*. 

There exists a single global RNG value that is referenced by every consumer of RNG in the game. This RNG value is only ever directly set when the map is restarted. Every time an RNG roll is consumed, the RNG counter advances forward. In heavy firefights and intense scenes, this value may be advanced hundreds to thousands of times per second.

To loop back around to how sound affects the game, here are the tick sequences for two scenarios. Both scenarios restart the map and press enter on Tick 2, the first possible tick to skip the opening cutscene. Blue ticks represent time spent in cutscene, while the red tick is the first tick of player control.

Sound Enabled : 0 - 1 - <span style='color:blue'>2</span> - <span style='color:red'>2</span> - 3 - 4 ... <br>
Sound Disabled: 0 - 1 - <span style='color:blue'>2 - 3 - 4 - 5 - 6 - 7 - 8</span> - <span style='color:red'>2</span> - 3 - 4 ...

With sound disabled, we spend 6 extra ticks in the cutscene. During those 6 ticks, events are occuring that advance the RNG counter. When we resume player control, the differing RNG sequence will cause the simulation to diverge and result in a desync. We are still investigating WHY this is the case, but for the time being we need leave the sound system enabled for our TAS adventures. This is especially unfortunate for the fast-forward utilities of the TAS as disabling the sound system speeds up fast-forward by around 50%. 

### Moving Forward

The focus for the next few updates is going to be on tackling the remaining sources of desync and general tool usability. I would like to make a public release by the end of the year should the project be in a state that I deem appropriate. I do not want to prematurely release tools that become incompatible or create replays that are not truly repeatable.

~Scales

<sup>1</sup> Using Game Capture in OBS you can actually see the last 60 frames rendering in the preview, but the Halo window itself does not update. Window Capture works appropriately.

<sup>2</sup> When I brought this up in the community, some people already knew that disabling sound could have adverse effects like more crashes, but not the specifics of what was affected.

---

[github.com/Scaless/HaloTAS](https://github.com/Scaless/HaloTAS)

If you would like to support my projects, check out my [Patreon](https://www.patreon.com/scalesllc).