Kitty Viewer
============
Kitty Viewer is a fork of Linden Lab's [viewer development][] repo with a small number of
interesting features added.

Binaries
--------
Binaries will be posted at the [Downloads][] page on GitHub, for both Windows and OS X.
There are currently no plans to provide Linux binaries unless there is actual demand.
Given there is currently no demand for *any* platform, this seems unlikely to be a problem
in the near future.

Nightly builds may be provided if I ever manage to assemble the necessary infrastructure.

Features
--------
Kitty Viewer is, ultimately, Viewer 2 based, with the following extra features:

- Direct Flickr uploads when taking snapshots
- WindLight in inventory, via specially named notecards (\*.wl, \*.ww)
- [Growl][] notifications (OS X only, for now)
- "Now playing" notifications for the current stream (inspired by, but not taken from, [Imprudence][])
- "Viewer profiles", which allow you to quickly swap between different sets of settings.
- (more is coming)

Additionally, there are some minor usability tweaks to Viewer 2:

- The Ctrl-T shortcut for the IM window from 1.23 has been restored
- Right clicking the info button in the address bar brings up the About Land floater
- The "Nearby" tab in the sidebar gives distance to everyone
- Agent UUIDs show in profiles, and can be copied from the mouse hover floater

And, of course, we pick up LL's new work pretty fast. For instance, we have detachable sidebar tabs
already.

Transparency
------------
All work on this viewer will be made available here, including the website. Source will be
added continuously, rather than waiting for a release.

However, I will not, necessarily, immediately push the branches I am currently working in. If
you wish to see what I'm currently working on, you can ask me (on [Twitter][] or similar),
and I will push the (likely broken) branch up for all to see.

On Version Numbers
------------
It is worth noting that this git repo currently has a rather bizarre structure, which the build
script depends on for now; notably, tags mark the *start* of work on a release. If you want a
specific release, you should take the tip of the appropriate branch - e.g. `maint-0.1.0` for
the 0.1.0 build. The actual build number (e.g. the `6` in `0.1.0.6`) is the number of commits
since I tagged `v0.1.0`. I realise this makes the tags on the [Downloads][] page somewhat useless,
and will look into changing the system at some point.

[Imprudence]: http://imprudenceviewer.org/
[Growl]: http://growl.info/
[Viewer Development]: http://hg.secondlife.com/viewer-development
[Twitter]: http://twitter.com/KatharineBerry
[Downloads]: http://github.com/Katharine/kittyviewer/downloads
