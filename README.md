(We are working on publications on different ridesharing algorithms using this
system, but we could use some help and expertise! If you would like to find out
more or possibly collaborate, please reach out to me at jamesjpan@outlook.com)

# libcargo (under development)

- *This is the code for the Cargo benchmarking platform and web interface. For
benchmark instances and road networks, visit
[Cargo_benchmark](https://github.com/jamjpan/Cargo_benchmark)*

- *Check out [Getting Started](docs/getting-started.tex) to get up and running
quickly.*

Cargo is a platform for implementing, evaluating, and visualizing dynamic
ridesharing algorithms. The core platform is distributed as a C++11 static
library. The web interface for visualization (`www`) uses
[nodejs](https://nodejs.org).

Ridesharing is a type of vehicle routing problem (VRP). The objective in
dynamic ridesharing is usually to route vehicles to service customers as they
continually appear on a road network while minimizing travel distance. Examples
of ridesharing companies include Uber, Lyft, and DiDi. Cargo aims to simplify
implementing ridesharing algorithms by providing a rich data model of common
ridesharing concepts (`Vehicle`, `Customer`, `Route`, etc.) and functions. It
also allows these algorithms to be evaluated through dynamic real-time
simulation.

## Screenshot
![Grabby](grabby.svg)

Here is an example of an algorithm running in real time against the simulator
inside `tmux`.  (ignore the jumbled text, that is an artifact of terminal
capture).  The top pane shows algorithm output and the bottom shows simulator
output (standard out). Under the hood, special `print` statements within an
algorithm are redirected to a [named pipe](https://linux.die.net/man/3/mkfifo)
on the file system that can be consumed using e.g. `cat`.

The algorithm (`example/grabby`) demonstrates a hybrid top-k greedy algorithm
and is implemented in less than 50 lines (not including the copyright at the
top; blank lines; include statements).

## Web Interface

The web interface is separate from the core simulator. Cargo can be thought of
as two parts: the C++ static library that includes the simulator and generic
algorithm classes, and the web interface that provides an intuitive way for
users to interact with the library. The web interface also comprises two parts,
the frontend and backend, both written in Javascript. The frontend depends on
[THREE.js](https://threejs.org) for graphics rendering, and the backend uses
[node.js](https://nodejs.org) for the web server and
[socket.io](https://socket.io) for real-time communication with the frontend.

The interface visualizes simulations by reading specially-formatted `*.dat` and
`*.feed` (named pipe) files, generated by the Cargo simulator, and drawing the
corresponding visuals on the HTML canvas. Thanks to THREE.js support for GPU
shaders, the interface can visualize 50,000 moving vehicles at 120 frames per
second on my 2016 Lenovo X1 Carbon and similar hardware.

(insert svg here)

The interface also allows for some interactivity. The `gui` namespace provides
several functions that users can use to pass drawing commands to the frontend.
Technically, these commands simply print specially formatted text to standard
out. Then when the web backend parses these texts, it instructs the frontend
to perform the drawing command. A `pause` function can be used to wait on
the user before continuing to the next step of the simulation. These two
features are illustrated below. In the user code, `gui::newroute` is used
to draw the yellow route, and `pause` is used to freeze the real-time
running of the simulation and visualization until after the user presses
`Enter`. The top pane is the browser window and the bottom pane is a separate
terminal window.

![Web Interaction](web.png)

## Installation and Usage

See the documentation (`doc`) for installation and usage instructions.

## Roadmap

Ultimately Cargo aims to become a benchmarking tool for ridesharing algorithms.
An application aiming at such a lofty goal as benchmarking should
ensure (1) no algorithm gets any unfair advantage by exploiting
some quirk of the tool, and (2) the benchmark reflects the conditions that an
algorithm would encounter in the real world. The former is accomplished by
documenting and verifying exactly the expected behavior of the library
API. The latter can only ever been an approximation, hence the limitations must
also be well documented.

Thus on the road map are (1) testing of the API and (2) documentation. Farther
down the line, perhaps (3) new features, such as variable speeds, traffic,
traffic control lights and signs, more accurate streets, ...

## Bugs and Contributing

If you discover a bug, or have a suggestion, please
[Submit an Issue](https://github.com/jamjpan/Cargo/issues/new)!

You can also submit a pull request, against your own branch or against the dev
branch.

## License

Cargo is distributed under the MIT License.

