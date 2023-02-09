# TrenchBroom Loader for Godot
Made as an alternative to [Qodot](https://github.com/QodotPlugin/qodot-plugin), using much of the
same map parsing code using the original [libmap](https://github.com/QodotPlugin/libmap) and a
modified [C++ port](https://github.com/EIRTeam/qodot/tree/4.0) of it.

# Why not Qodot?
Qodot is great! It works really well. I initially made TBLoader because I wanted to try several
different approaches to creating meshes, including creating a bunch of `CSGMesh3D` inside of
`CSGCombiner3D`, but that ended up being [problematic](https://github.com/godotengine/godot/issues/58637).

When I originally made this, Qodot didn't officially support Godot 4. So I decided to take on the
challenge and rewrite most of it in godot-cpp mostly for fun. Nowadays, Qodot has a .Net version
available which does support Godot 4: https://github.com/QodotPlugin/Qodot

If you're already using Qodot, there might not be a lot of reasons for you to use this. It is not
backwards compatible with Qodot, and does a few things differently.

# Usage
To install TBLoader, you can either install it through [AssetLib](https://godotengine.org/asset-library/asset/1265)
(search for "TrenchBroom Loader"), or by downloading a [release from Github](https://github.com/codecat/godot-tbloader/releases)
and extracting it to your project's `addons` folder, so that you have a structure like this:
```
project/addons/tbloader/plugin.cfg
```

You might have to manually enable the plugin from your project settings. In the Godot editor, click
on Project -> Project Settings, and go to the Addons tab. Check the "Enable" box next to TBLoader.

To build a level's geometry, create a `TBLoader` node in your scene hierarchy. In the properties of
the node you can select where your `.map` file is located, plus some more useful settings. With the
node still selected and the 3D view open, you will see a button `Build Meshes` in the toolbar the 3D
view. Click that button to build the geometry.

# TrenchBroom game config
The `tb-gameconfig` folder contains a game configuration for this addon. This includes a simple FGD
which will have some common entities that create Godot nodes. Simply place the files in a folder
called `Godot` inside the `games` folder of your TrenchBroom installation, so you would have
`games/Godot/<files>`.

# Entities
The following brush entities are supported by default:

* `worldspawn` and `func_group`: Mesh instances and collision shapes
* `area`: [`Area3D`](https://docs.godotengine.org/en/latest/classes/class_area3d.html)

The following point entities are supported by default:
* `light`: [`OmniLight3D`](https://docs.godotengine.org/en/latest/classes/class_omnilight3d.html)

## Custom entities
You can make custom entities as well. This works by loading and instantiating a `PackedScene` object
based on the class name. For example, the class name `foo_bar_foobar` will try to load one of the
following in this order:

* `res://entities/foo_bar_foobar.tscn`
* `res://entities/foo/bar_foobar.tscn`
* `res://entities/foo/bar/foobar.tscn`

The first resource it finds will be loaded and instantiated. The root for this (`res://entities` by
default) can be changed in the `TBLoader` node properties.

Any properties set on the entity will be set directly on the instantiated node using
[`_set`](https://docs.godotengine.org/en/latest/classes/class_object.html#class-object-method-set)
and [`_get`](https://docs.godotengine.org/en/latest/classes/class_object.html#class-object-method-get).
The getter is used first to determine the type of the property.

# Textures
To see your textures in TrenchBroom, navigate to `Preferences` -> `Godot` -> `Game Path` and set it to your project's root directory. TrenchBroom will be able to see textures in your project. TBLoader will look for your textures in the `res://textures` directory. If you have a material (`rust.material`) in the same folder and with the same name as your texture (`rust.png`), TBLoader will load your material instead.

> Note: This currently only works with textures in `.png` format, and materials in `.material` or `.tres`.

# Building
On all platforms, the build process is the same. Make sure scons is installed, and then just run
`python -m SCons target=template_release` to build.

On Mac, the process is the same, but you will have to codesign and notarize your resulting binary as
well if you want it to run on consumer hardware. To do this, you need to already have the notary
tool configured on your machine (you need a keychain profile), and then run:

```
$ codesign -s "Developer ID Application: Your Name (1234567890)" libtbloader.macos.universal.dylib --timestamp
$ zip archive.zip libtbloader.macos.universal.dylib
$ xcrun notarytool submit --keychain-profile "Profile Name" --wait archive.zip
```

# Credits
* [Qodot](https://github.com/QodotPlugin/qodot-plugin)
* [Original libmap](https://github.com/QodotPlugin/libmap)
* [EIRTeam libmap-cpp](https://github.com/EIRTeam/qodot/tree/4.0) (modified)
