GFXTest by learn_more and evolution536.

What is it?
-----------------------------------------

GFXTest is a lightweight graphics testing application that allows multiple
rendering methods to be tested in one simple application. It can be used
to test anything graphics related like hooks, custom drawing or menus.
Newer versions of GFXTest also handle mouse and keyboard input. This is
very useful when it comes to testing custom menus or textures that obtain
input and utilize it.

How to use?
-----------------------------------------

For basic usage, keep all files in the same directory and run the GFXtest
executable. The first renderer that loads succesfully will utilize the main
window. To switch renderer, click one in the list on the top right pane of
the window. If there are no other renderers visible it means that all other
renderers failed to load. To launch from the command line, use one of the 
provided parameters. To get help about the parameters view the manual in 
the forum thread on forum.unknowncheats.me or type: "gfxtest ?" in the 
command prompt.

How to write plugins?
----------------------------------------

Since the second release of GFXTest a plugin SDK is packaged. The plugin SDK
includes a header file and a library file to link against. To write a plugin,
create a new solution in Visual Studio that outputs a Dynamic Link Library (DLL).
Include the header file and the library file to link against and implement the
documented functionality from the SDK header. The documentation is located inside
the SDK header as well.