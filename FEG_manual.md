
<h1>: Forward in Time Spatial Simulator on Evolutionary Game Theory </h1>


Authors: Aggelos Koropoulis (koropoulis.a@gmail.com), Pavlos Pavlidis (pavlidisp@gmail.com)

Version:1.0

<h2>Summary</h2>

FEG is a forward spatial simulating software for Genetic Data. It sets a continues, map and places i) a user-defined number of preys and ii) a user-defined number of predators on the map. After the creation of the map, forward in time, these predators interact with each other to try and capture prey following either of three strategies. Either they try to Synergize with the other, either they try to Compete with them or they simply Ignore them and pursue prey by themselves. The amount of prey secured (payoff of the game) determines a predator's fitness. Predators are sampled during this forward in time step, ending up creating a table of polymorphisms. A goal of WTF is to showcase the impact of the spatial parameter and the interaction between predators into the evolutionary trajectory of populations.
The tool can be used to simulate scenarios where the area where the population lives is of importance and a game theory-like model of evolution is followed.

<h2>Download and Compile</h2>

The following commands can be used to download the necessary prerequisite libraries before running WTF. The latest version of the gcc compiler is highly recommended, for sections of the program to run parallel:

$sudo apt-get install -y gsl-bin libgsl-dbg libgsl-dev

The above command is used to install the gsl library containing mathematical functions like binomial distribution.

To install the source code of the project run the following commands

```shell
$ mkdir <directoryname>
$ cd <directoryname>
$ wget https://github.com/aggelosk/Sps
$ unzip master.zip
$ make all
```
A test run by simply using the
```shell
$ ./feg -rnds 1 -pred 1 -prey 1 -smpl 1 1
```
command is recommended to verify the tool was downloaded correctly and every required library is installed. Output provided should not be of any significance.

<h2>In-tool Help</h2>

To get help with understanding the parameters of WTF, the following command may be typed into the terminal, showing all command line parameter options.

```shell
$ ./feg –help
```
Analytically, those command line parameters are:

```shell
Command Line Parameters setting the simulation:


$ -seed: user-defined seed for the random number generator function.

$ -rnds: determines the number of generations aka rounds of game the program will simulate. Parameter value must be a positive integer.

$ -burn: determines the number of generations for the burn-in phase, a preliminary phase where predators only reproduce based on random genetic drift. Parameter value must be a positive integer.

$ -pred: determines the number of predators each generation might have. Parameter value must be a positive integer.

$ -prey: determines the number of preys in the simulation. Parameter value must be a positive integer.

$ -ppos: sets the initial position for a prey. Can be repeated for multiple preys. Parameter value must be a positive integer.

$ -size: determines the size of the genotype as an array of unsigned integers. Parameter value must be a positive integer.

$ -pinf: determines the positions of the genotype affecting the phenotype and how many these areas will be. The first positive integer given sets the number of positions, and then each position is determined by the next few integers.

$ -neutral_model: instead of using EGT to determine the fitness of each predator, a neutral model only influenced by the distance between predators is used. Parameter value must be a positive integer.


$ -nsyn: number of initial predators using the Synergy strategy. Parameter value must be a positive integer.

$ -nign: number of initial predators using the Ignore strategy. Parameter value must be a positive integer.

$ -ncom: number of initial predators using the Competition strategy. Parameter value must be a positive integer.

$ -neut: number of initial predators with an all-zero (no mutations) genotype. Parameter value must be a positive integer.

$ -synr: below this percentage of aggression a predator selects the Synergy strategy. Parameter value must be a positive float.

$ -comr: above this percentage of aggression a predator selects the Competition strategy. Parameter value must be a positive float.

$ -mutr: determines the chance that a mutation occurs on a single individual. Parameter value must be a positive float.

$ -recr: determines the chance that a recombination occurs on a single individual. Parameter value must be a positive float.

$ -bttl: determines a change in the population size of a predator. Takes two parameters. The first signals the generation of the change and the second the new population size. Can be repeated for multiple changes.

$ -evnt: determines the addition or removal of a prey. Takes two or four parameters. The first determines the type of the event (0 removal 1 addition). The second the generation of the change and if defined, the other two the latitude and the longtitude of the prey at hand. Can be repeated for multiple events.

$ -smpl: Sampling event. Takes two parameters, one signaling the generation of the event and the second one the number of samples. Parameter value must be a positive integer.

$ -sbrn: Sampling event during the burn-in process. Takes two parameters, one signaling the generation of the event and the second one the number of samples. Parameter value must be a positive integer.

```

<h2>Output Files</h2>
SpS creates a number of files as output. File type was set as .txt to work in every known system. Can be changed within the code.

* <b> ms_mutation_table.txt</b>: contains the table of polymorphisms generated by the process. The table is formatted similar to <i>ms</i>.

* <b>strategy_percent.txt</b>: for each sampling instance, print the percentage of predators selecting each strategy in the population. Also for the initial state and after the burn-in phase is complete.

* <b>seed.txt</b>: log file which stores both the seed used as input to the random number generator and the run time of the process.

<h2> Examples </h2>

```shell
./feg -rnds 3000 -prey 1 -pred 5 -synr 0.2 -comr 0.6 -smpl 1000 10
```

The above command will simulate a scenario with 5 predators and 1 prey for 3000 generations. Below 20% aggression, a predator will select the Synergy strategy and above 60% a predator will be competitive. After simulating 1000 generations 10 predators will be sampled.

```shell
./feg -burn 1000 -rnds 3000 -prey 1 -pred 5 -pinf 2 0 2 -mutr 0.03 -synr 0.2 -comr 0.6 -smpl 1000 10 -seed 123
```
Variation of the scenario descibed above. A burn-in phase will precede the simulation and will run for 1000 generation. The mutation rate here is set at 3% and the areas of the genotype affecting the phenotype are two. The first(0) unsigned integer describing bases 0 - 63, and the one describing bases 128 - 195.

```shell
./feg -rnds 3000 -prey 1 -pred 5 -synr 0.2 -comr 0.6 -smpl 1000 10 -neutral_model
```

Another variation of the first scenario. In this case, the predators will reproduce solely based on genetic drift and the distance between each other. No game will be played.
