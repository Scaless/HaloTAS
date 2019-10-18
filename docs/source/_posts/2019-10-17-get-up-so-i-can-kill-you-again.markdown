---
layout: post
title:  "Get Up, So I Can Kill You Again!"
date:   2019-10-17 -0400
categories: halo
---

#### Today, we're going back in time!
(Warning: contains eurobeat):

<iframe width="560" height="315" src="https://www.youtube.com/embed/iyX9cpZZ-AI" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

As you can hopefully see, progress is going well on the rewind feature in HaloTAS. We can now easily go forward and backward in time which greatly helps with iteration time on TAS files.

You may notice the screen is shaking a bit in the video, which is not an effect I added. Any screen effect that you incur over the course of re-playing a demo will persist until it is rendered to completion, including: AR shots, grenade blasts (flashes screen yellow), taking damage (flashes screen red), and some scripted events. 

During a fast-forward, we disable the renderer to greatly speed up the time it takes to simulate ticks by not issuing any draw calls for each frame. A side effect of this is that the screen effect timers never advance through their animations and will end up stacking on top of each other. Effects can range from annoying to unplayable, such as grenade explosions:

![screen_effect_explosion.jpg]({{site.baseurl}}/assets/screen_effect_explosion.jpg)

Until I find a true way to disable screen effects without drastically editing game files, for the time being I've implement a workaround that works fairly well. The process to fast-forward to tick 1000 would be:

1. Reset the map
2. Disable the renderer
3. Simulate ticks 0 - 940
4. Enable the renderer, but disable framebuffer updates
5. Simulate ticks 941 - 1000
6. Enable screen updates

We spend the last 60 ticks rendering to nowhere*, but doing it this way lets screen effects run to completion and provides us with a smooth transition when traversing through time:

<iframe width="560" height="315" src="https://www.youtube.com/embed/LUNra1tLzT4" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

~Scales

\* Using Game Capture in OBS you can actually see the last 60 frames rendering in the preview, but the Halo window itself does not update. Window Capture works appropriately.

---

[github.com/Scaless/HaloTAS](https://github.com/Scaless/HaloTAS)

If you would like to support my projects, check out my [Patreon](https://www.patreon.com/scalesllc).