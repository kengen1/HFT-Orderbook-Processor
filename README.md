# Order Book Depth Snapshot Generator

This application processes a stream of order book messages to produce snapshots of the top N levels of price depth, reflecting the most recent changes in the order book for specific symbols. Each snapshot is produced only when there is a visible change in the depth for a symbol, as defined by the input stream.

## Getting Started

To run the application, you will need an input file (`input.stream`) containing the binary encoded order book messages. The application will output an `output.log` file containing the snapshots and a `debug.log` file with detailed actions related to the input processing.

### Prerequisites

- C++ Compiler (GCC for Linux/macOS, MSVC for Windows)
- Command line terminal or equivalent

### Running the Application

#### On Windows

Use the following command in the Command Prompt (cmd) or PowerShell, assuming `main.exe` is in the current directory:

```cmd
type input2.stream | main.exe 5
```

#### On macOS and Linux

The equivalent command in terminal command for macOS and Linux distributions is:

```bash
cat input2.stream | ./main 5
```
Ensure `main` is executable by running chmod +x main if necessary.

### Output Files

- **output.log** : Contains the price depth snapshots, one per line, in a human-readable format. Each line includes a sequence number, symbol, and the top N levels of bids and asks.
- **debug.log**: Provides detailed logging of actions related to the input stream, useful for debugging purposes.

### Building the Application
To compile the application, navigate to the directory containing the source files and run the appropriate compilation command for your C++ compiler. For example, on Linux or macOS:

```bash
g++ -o main main.cpp
```

And on Windows, using Visual Studio's command line tools:

```cmd
cl main.cpp /Fe:main.exe
```

**Note**
This application was developed on a Windows machine using MinGW-w64 and the GNU Compiler Collection (g++). It was compiled and tested with g++ to ensure functionality. While developed on Windows, the application is designed to be cross-platform and should compile and run on macOS and Linux environments with the appropriate adjustments.
