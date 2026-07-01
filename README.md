# Custom Two-Pass Assembler

This project is a fully functional Two-Pass Assembler developed in C. It is designed to read, parse, and translate custom assembly language source code into machine code for a simulated 10-bit machine architecture.

## 🚀 Project Overview

The assembler processes assembly files through a robust pipeline, handling macro expansions, resolving symbolic labels, and encoding instructions and data into a specific Base-4 formatting. 

### Core Capabilities
* **Pre-Assembler (Macro Processor):** Scans the source code to identify and seamlessly expand macros (`mcro` / `mcroend`) before the main assembly process begins.
* **Two-Pass Translation:**
  * **First Pass:** Analyzes the code, counts memory allocation (IC & DC), identifies directives and instructions, and builds a comprehensive Symbol Table.
  * **Second Pass:** Resolves forward references, finalizes the binary encoding of missing operands, and generates the required output files.
* **10-Bit Architecture:** Operates on a simulated memory model where each word and register is exactly 10 bits long.
* **Multiple Addressing Modes:** Supports 4 distinct addressing modes: Immediate, Direct, Matrix Access (2D arrays), and Direct Register.

## 📁 Output Files

For every valid input file (e.g., `program.as`), the assembler generates:
* `program.am`: The source code after macro expansion.
* `program.ob`: The object file containing the translated machine code (represented in a unique Base-4 format).
* `program.ent`: A list of all entry point symbols and their assigned addresses.
* `program.ext`: A list of all external symbol references and the addresses where they are used.

## 🛠️ Build and Run

### Compilation
The project includes a `makefile` for easy compilation. Open your terminal and run:
```bash
make
