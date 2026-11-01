# CHANGEME_PROJECT_NAME

CHANGEME_PROJECT_DESC

---

This is a template for my C-like projects.

The `main.cpp` Hello World serves as a quick refresher for doxygen.

The project layout is informed by [Pitchfork](https://github.com/vector-of-bool/pitchfork) layout conventions.

I strongly recommend using [SemVer](https://semver.org/) and [Conventional Commits](https://www.conventionalcommits.org).

---

## Getting Started

### Using This Template

1. Create a repo on GitHub using this template, or make a fresh repo and copy the files from this one.

2. Change or delete the `LICENSE` file.

3. Change occurances of `CHANGEME_PROJECT_NAME` and `CHANGEME_PROJECT_DESC` in the following files:
   
   - `CMakeLists.txt`
   
   - `Doxyfile`
   
   - `README.md`

4. Configure git submodules, `external/` dependencies, and `CMakeLists.txt`.

### Generating Documentation

This template uses [Doxygen](https://www.doxygen.nl/) to generate example docs.

Documentation is generated from `src` files and `docs/*.`

1. (optional) Run `git submodules init` to pull the latest [Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css) for a more modern doxygen aesthetic. This configuration was tested using Doxygen Awesome v2.2.1 with Doxygen 1.9.7, so you may wish to run `git -C .\external\doxygen-awesome-css checkout v2.2.1` to use a known-good version.

2. Run `doxygen` where the `Doxyfile` is located (in the project's root directory) to generate documentation from the source files.

3. Open `./docs/index.html` in a browser.

### Building

For windows, I've provided scripts you can [run](https://stackoverflow.com/questions/58524451/how-to-run-powershell-script-from-ps1-file) to compile, provided you have appropriate toolchains in your environment's `PATH`.

- `compile-mingw.ps1` to use [MinGW's](https://www.winlibs.com/) `g++` 

- `compile-msvc.ps1` to use Microsoft's `cl`

If you're using another compiler or if you're on Linux, then you'll have to figure it out yourself. If you're lost, you can use the compile script as an example but you'll need change the `-G` argument to something more appropriate for your environment.
