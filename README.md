# RL Page Scheduler

## Intro
Inspired by the <a href="https://www.blagodurov.net/files/hpdc002-doudaliA.pdf">Kleio paper</a>, which explores using machine intelligence
to organize pages in a heterogeneous memory system, this project is a crude attempt at using temporal difference learning in a simulated
environment to control page placement.


## Dependencies
CMake is used to build the project, but there should not be any extra non-standard C libraries used.
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

## References
Kleio Paper:
https://www.blagodurov.net/files/hpdc002-doudaliA.pdf

## Examples