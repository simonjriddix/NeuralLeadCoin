#!/bin/bash

# Specify number of cores in compilation process
CORES="`nproc`"

# Verificare se è stato passato un parametro
if [[ -z "$1" ]]; then
  echo "Error: No OS specified."
  echo "Usage: $0 <OS>"
  echo "Example: $0 Linux"
  exit 1
fi

# Impostare il sistema operativo dal parametro passato
OS="$1"
ARCH="x86_64"

# Controllare se la variabile OS corrisponde a uno degli OS identificabili
VALID_OS=("mingw32" "WSL2" "Linux" "MacOSX" "BSD" "Solaris" "Android" "iOS")

if [[ ! " ${VALID_OS[@]} " =~ " ${OS} " ]]; then
  echo "Errore: operating system not recognized: $OS"
  echo "Valid OS are: ${VALID_OS[*]}"
  exit 1
fi

# Controllare se la variabile OS corrisponde a uno degli OS identificabili
VALID_ARCHs=("x86_64" "arm64" "aarch64" "MIPS64" "powerpc64" "RISCV")

if [[ ! " ${VALID_ARCHs[@]} " =~ " ${ARCH} " ]]; then
  echo "Errore: architecture not recognized: $ARCH"
  echo "Valid OS are: ${VALID_ARCHs[*]}"
  exit 1
fi

# Definire la directory base
BASEDIR="$PWD/depends/NeuralLeadQHash"
#CUDA="-L/usr/local/cuda/lib64 -lcudart -lcublas"

# Impostare la variabile LIBRARY_IN_COMPILATION in base al sistema operativo
if [[ "$OS" == "mingw32" ]]; then
  LIBRARY_IN_COMPILATION=" -L$BASEDIR/bin/$OS/$ARCH/ -lNeuralLeadQHash -lgomp "
elif [[ "$OS" == "WSL2" ]]; then
  LIBRARY_IN_COMPILATION=" -L$BASEDIR/bin/$OS/$ARCH/ -lNeuralLeadQHash -lgomp "
elif [[ "$OS" == "Linux" ]]; then
  LIBRARY_IN_COMPILATION=" -L$BASEDIR/bin/$OS/$ARCH/ -lNeuralLeadQHash -lcoscienza.core -lgomp -lm -lpthread "
elif [[ "$OS" == "MacOSX" ]]; then
  LIBRARY_IN_COMPILATION=" -L$BASEDIR/bin/$OS/$ARCH/ -lNeuralLeadQHash "
elif [[ "$OS" == "BSD" ]]; then
  LIBRARY_IN_COMPILATION=" -L$BASEDIR/bin/$OS/$ARCH/ -lNeuralLeadQHash "
elif [[ "$OS" == "Solaris" ]]; then
  LIBRARY_IN_COMPILATION=" -L$BASEDIR/bin/$OS/$ARCH/ -lNeuralLeadQHash "
elif [[ "$OS" == "Android" ]]; then
  LIBRARY_IN_COMPILATION=" -L$BASEDIR/bin/$OS/$ARCH/ -lNeuralLeadQHash "
elif [[ "$OS" == "iOS" ]]; then
  LIBRARY_IN_COMPILATION=" -L$BASEDIR/bin/$OS/$ARCH/ -lNeuralLeadQHash "
fi

# Prepare output directory

mkdir -p $PWD/bin
mkdir -p $PWD/bin/$ARCH
mkdir -p $PWD/bin/$ARCH/$OS

if [[ "$OS" == "mingw32" ]]; then
  # Install dependencies target mingw32/Windows
  sudo apt install -y build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git libeigen3-dev g++-mingw-w64-x86-64
  echo ""
  echo ""
  echo "SELECT POSIX"
  echo ""
  sudo update-alternatives --config x86_64-w64-mingw32-g++
  cd depends
  make HOST=x86_64-w64-mingw32 -j$CORES
  cd ..
elif [[ "$OS" == "Linux" ]]; then
  # Install dependecies Ubuntu
  sudo apt-get install libeigen3-dev make automake cmake curl g++-multilib libtool binutils-gold bsdmainutils pkg-config python3 patch git
  cd depends
  make HOST=x86_64-pc-linux-gnu -j$CORES
  cd ..
fi

# Generate config and compilation files

./autogen.sh

# Disable some usefull flags in WSL2

if [[ "$OS" == "WSL2" ]]; then
  PATH=$(echo "$PATH" | sed -e 's/:\/mnt.*//g')
  sudo bash -c "echo 0 > /proc/sys/fs/binfmt_misc/status"
fi

cd depends && git clone --recursive https://github.com/simonjriddix/NeuralLeadQHash
cd ..

#SUPER_FLAGS="-fno-rounding-math -mno-fma -mfpmath=sse -msse2 -ffloat-store -fno-fast-math -fsignaling-nans -ffp-contract=off"

if [[ "$OS" == "mingw32" ]]; then
  # Build source
  CFLAGS="-DQPP_FP_DOUBLE -DQPP_IDX_DEFAULT -DQPP_BIGINT_INT -DUSE_DLL_API_NLHASH" CXXFLAGS="-DQPP_FP_DOUBLE -DQPP_IDX_DEFAULT -DQPP_BIGINT_INT -DUSE_DLL_API_NLHASH" CPPFLAGS="-DQPP_FP_DOUBLE -DQPP_IDX_DEFAULT -DQPP_BIGINT_INT -DUSE_DLL_API_NLHASH" CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site ./configure --prefix=/
  make clean && make -j$CORES EXTRA_LDFLAGS="-lbcrypt $LIBRARY_IN_COMPILATION"
  mv ./src/*.exe ./bin/$ARCH/$OS/
  mv ./src/qt/*.exe ./bin/$ARCH/$OS/
  cp ./depends/NeuralLeadQHash/bin/$OS/$ARCH/.dll ./bin/$ARCH/$OS/
elif [[ "$OS" == "Linux" ]]; then
  CFLAGS="-DQPP_FP_DOUBLE -DQPP_IDX_DEFAULT -DQPP_BIGINT_INT -DEIGEN_NO_DEBUG -I/usr/include/eigen3" CPPFLAGS="-DQPP_FP_DOUBLE -DQPP_IDX_DEFAULT -DQPP_BIGINT_INT -DEIGEN_NO_DEBUG -I/usr/include/eigen3" CXXFLAGS="-DQPP_FP_DOUBLE -DQPP_IDX_DEFAULT -DQPP_BIGINT_INT -DEIGEN_NO_DEBUG -I/usr/include/eigen3" CONFIG_SITE=$PWD/depends/x86_64-pc-linux-gnu/share/config.site ./configure --prefix=/ --enable-c++17 --enable-shared=no --enable-static=yes
  make clean && make -j$CORES EXTRA_LDFLAGS="$LIBRARY_IN_COMPILATION"
  mv ./src/neuralleadcoind ./bin/$ARCH/$OS/neuralleadcoind
  mv ./src/neuralleadcoin-cli ./bin/$ARCH/$OS/neuralleadcoin-cli
  mv ./src/neuralleadcoin-tx ./bin/$ARCH/$OS/neuralleadcoin-tx
  mv ./src/neuralleadcoin-wallet ./bin/$ARCH/$OS/neuralleadcoin-wallet
  mv ./src/bench/bench_bitcoin ./bin/$ARCH/$OS/bench_bitcoin
  mv ./src/qt/neuralleadcoin-qt ./bin/$ARCH/$OS/neuralleadcoin-qt
elif [[ "$OS" == "MacOSX" ]]; then
  echo "need implementation"
elif [[ "$OS" == "BSD" ]]; then
  echo "need implementation"
elif [[ "$OS" == "Solaris" ]]; then
  echo "need implementation"
elif [[ "$OS" == "Android" ]]; then
  echo "need implementation"
elif [[ "$OS" == "iOS" ]]; then
  echo "need implementation"
fi

if [[ "$OS" == "WSL2" ]]; then
  sudo bash -c "echo 1 > /proc/sys/fs/binfmt_misc/status"
fi

# Copy brain files
cp -R ./neuralleadqhash ./bin/$ARCH/$OS/

echo "Built done. All files are in $PWD/bin"
