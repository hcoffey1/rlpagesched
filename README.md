# RL Page Scheduler

## Intro
Inspired by the <a href="https://www.blagodurov.net/files/hpdc002-doudaliA.pdf">Kleio paper</a>, which explores using machine intelligence
to organize pages in a heterogeneous memory system, this project is a crude attempt at using temporal difference learning in a simulated
environment to control page placement.

Note: I've realized that this project should be written in python for the course work,
and have added a python version that accepts the same command line arguments.
The python version currently cannot save or load models,
but is still able to execute the RL based scheduler.
## Dependencies
CMake is used to build the project, but there should not be any extra non-standard C libraries used.

Build steps:
<pre>
<code>
mkdir build
cd build
cmake ..
make
</code>
</pre>

## Usage

**Command line arguments**
|Argument|Description|
|---        |---|
|-c X  |Config file path|
|-t X       |Memory trace file path|
|-s X       |Scheduler type|
|-e X       |Epochs to run if using RL scheduler|
|-LM X    |File path to load model from (optional)|
|-SM X |File path to save model to (optional)|

Schedulers available and their argument encodings are:
<code>[history : 0, oracle : 1, rl : 2]</code>

## References
Kleio Paper:
https://www.blagodurov.net/files/hpdc002-doudaliA.pdf

## Examples

Run the executable using config file <code>config.txt</code>, trace file <code>medium.log</code>, with the RL scheduler, saving and loading the model
from the same file, and running for 10000 epochs.
<pre>
<code>
./pagesim -c ../config.txt -t ../medium.log -s 2 -SM test_4.model -LM test_4.model -e 10000 
</code>
</pre>

Running the Python3 version:
<pre>
<code>
python pageSim.py -c config.txt -s 2 -t medium.log  
</code>
</pre>

A sample trace file <code>small.log</code> is included in the repo.