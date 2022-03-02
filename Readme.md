# TrenchBroom Loader for Godot
Made as an alternative to [Qodot](https://github.com/QodotPlugin/qodot-plugin), using much of the
same map parsing code using the original [libmap](https://github.com/QodotPlugin/libmap) and a
modified [C++ port](https://github.com/EIRTeam/qodot/tree/4.0) of it.

# Why not Qodot?
Qodot is great! It works really well. I initially made TBLoader because I wanted to try several
different approaches to creating meshes, including creating a bunch of `CSGMesh3D` inside of
`CSGCombiner3D`, but that ended up being [problematic](https://github.com/godotengine/godot/issues/58637).
This feature still exists in TBLoader if you want to try it, but it doesn't support texturing.

Since Qodot currently doesn't officially support Godot 4, I decided to take on the challenge and
rewrite most of it in godot-cpp mostly for fun, but also because the only other existing Godot 4
fork is made as an Engine Module, which requires recompiling the entire engine.

If you're already using Qodot on Godot 3, there's probably not much reason for you to use this. It
is also not backwards compatible with Qodot, and probably does a few things differently.

# TrenchBroom game config
The `tb-gameconfig` folder contains a game configuration for this addon. This includes a simple FGD
which will have some common entities that create Godot nodes. Simply place the files in a folder
called `Godot` inside the `games` folder of your TrenchBroom installation. So you would have
`games/Godot/<files>`.

# Credits
* [Qodot](https://github.com/QodotPlugin/qodot-plugin)
* [Original libmap](https://github.com/QodotPlugin/libmap)
* [EIRTeam libmap-cpp](https://github.com/EIRTeam/qodot/tree/4.0) (modified)
