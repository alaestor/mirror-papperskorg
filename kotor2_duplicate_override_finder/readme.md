**A quick and dirty script written to find and list duplicate KOTOR2 overrides.**

It will search the current working directory tree for any filenames that occur in more than one place. It excludes some extentions, and treats `.tpc` as an alias for `.tga`

**How To**

Just place the python script inside of KOTOR2's override folder and [run it in a terminal](https://duckduckgo.com/?q=How+to+Run+Python+File+in+windows)

**Example Output**

```
Finding duplicated filenames.
  Excluding: ['.txt', '.rtf']
  Aliasing: '.tga' <- ['.tpc']
  Path: Z:\SteamLibrary\steamapps\common\Knights of the Old Republic II\override

0. n_tsfoff01.tga has 2 duplicates:
  .\Better TSF Uniforms - HD Upscaled Textures (Enhanced Vanilla)\N_TSFOff01.tga
  .\Textures Improvement Project HD Reskin\N_TSFOff01.tga


1. n_tsfoffh.tga has 2 duplicates:
  .\Better TSF Uniforms - HD Upscaled Textures (Enhanced Vanilla)\N_TSFOffH.tga
  .\Textures Improvement Project HD Reskin\N_TSFOffH.tga


2. n_tsfoff_f01.tga has 2 duplicates:
  .\Better TSF Uniforms - HD Upscaled Textures (Enhanced Vanilla)\N_TSFOff_F01.tga
  .\Textures Improvement Project HD Reskin\N_TSFOff_F01.tga


3. n_tsfoff_fh.tga has 2 duplicates:
  .\Better TSF Uniforms - HD Upscaled Textures (Enhanced Vanilla)\N_TSFOff_FH.tga
  .\Textures Improvement Project HD Reskin\N_TSFOff_FH.tga
```

In the future, I may add a GUI or at least a file dialog, but I'm lazy so don't count on it.