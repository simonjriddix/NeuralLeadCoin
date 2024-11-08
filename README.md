# NeuralLead Coin

[NeuralLead Coin](https://www.neurallead.com)

## What is NeuralLead Coin?

[NeuralLead Coin](https://www.neurallead.com) is a digital currency that enables instant payments to anyone, anywhere in the world. NeuralLead Coin uses peer-to-peer technology to operate without any central authority: transactions and currency issuance are managed collectively by the network. NeuralLead Coin is the name of the open-source software that enables the use of this currency.

For more information, as well as an immediately usable binary version of the NeuralLead Coin software, visit [https://www.neurallead.com](https://www.neurallead.com) or read the [modern whitepaper](https://www.neurallead.com/NeuralLeadCoin-NLEAD-Modern-White-Paper.pdf).

## License

NeuralLead Coin is released under the terms of the MIT license. See [COPYING](COPYING) for more information or visit [https://opensource.org/licenses/MIT](https://opensource.org/licenses/MIT).

## Development Process

The `master` branch is regularly built (see `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be completely stable. [Tags](https://github.com/simonjriddix/neuralleadcoin/tags) are created regularly from release branches to indicate new official, stable releases of NeuralLead Coin.

The contribution workflow is detailed in [CONTRIBUTING.md](CONTRIBUTING.md), and helpful tips for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).

## Algorithm - NeuralLeadQHash

NeuralLead Coin uses a new open-source hashing algorithm based on Quantum and Artificial Intelligence algorithms. The code is available on GitHub, along with precompiled and optimized libraries, in the [repository link](https://github.com/simonjriddix/neuralleadqhash).

For more technical information, visit the [tech information page](https://...).

## Automated Build

> An automated build script is provided to streamline the compilation process. It is currently tested on Linux (Ubuntu versions 20/22/24) and WSL2 Ubuntu 22/24, with target builds for Windows and Linux on x86_64 architecture.

To install `git`:

	bash
	sudo apt update && sudo apt upgrade -y
	sudo apt install git

Clone the repository:

	git clone --recursive https://github.com/simonjriddix/neuralleadcoin

To compile for Linux (x86_64 architecture):
	
	./init_os Linux

To compile for Windows (x86_64 architecture):
	
	./init_os mingw32

Wait for the operation to complete. The process may take several minutes depending on your hardware’s performance. When the compilation finishes without errors, the executables will be available in the bin/ directory.

## Manual Build

*NeuralLeadQHash*:
To compile the libraries for the algorithm, follow the instructions in the repository mentioned in the 'Algorithm' section. Alternatively, you can use the precompiled libraries available in the repository. Place the precompiled library files in the depends/arch/os directory, replacing 'arch' and 'os' with the appropriate architecture and operating system.

*NeuralLead Coin*:
See the files in the `doc/` folder, under `build-*` for detailed instructions.