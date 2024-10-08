#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/", "src/map/"])
sources = Glob("src/*.cpp")
sources += Glob("src/builders/*.cpp")
sources += Glob("src/map/*.cpp")

try:
	doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
	sources.append(doc_data)
except AttributeError:
	print("Not including class reference as we're targeting a pre-4.3 baseline.")

if env["platform"] == "windows" and env["target"] == "template_debug":
	env.Append(LINKFLAGS=["/DEBUG"])

if env["platform"] == "osx":
	library = env.SharedLibrary(
		"addons/tbloader/bin/libtbloader.{}.framework/libtbloader.{}".format(
			env["platform"], env["platform"]
		),
		source=sources,
	)
else:
	library = env.SharedLibrary(
		"addons/tbloader/bin/tbloader.{}.{}{}".format(
			env["platform"], env["arch"], env["SHLIBSUFFIX"]
		),
		source=sources,
	)

Default(library)
