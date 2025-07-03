Of course, here is a professionally formatted README.md file based on the instructions you provided.

-----

# ImageStreamerApp

This repository contains the source code for the ImageStreamerApp, a Qt5-based application. This guide provides instructions on how to build and run the application on a local Linux system and how to cross-compile it for an embedded system.

-----

## 🚀 Getting Started on a Local Linux System

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

Before you begin, ensure you have **Qt5** installed on your Linux system.

### Building and Running the Application

1.  **Clone the repository:**
    Open your terminal and clone this repository to your local machine.

    ```sh
    git clone <my-repository>
    cd my-repository
    ```

2.  **Generate the Makefile:**
    Use `qmake` to generate the Makefile for the project.

    ```sh
    qmake ImageStreamerApp.pro
    ```

3.  **Compile the source code:**
    Run the `make` command to build the application.

    ```sh
    make
    ```

4.  **Execute the application:**
    Once the compilation is complete, you can run the application.

    ```sh
    ./ZenQtApp
    ```

-----

## Cross-Compilation for Embedded Systems

This section will guide you through the process of cross-compiling the application to generate a binary file that can be run on a target embedded system.

### Prerequisites

Ensure you have the necessary cross-compilation toolchain and environment set up. For this project, you'll need to source the Poky environment script.

### Building the Binary

1.  **Source the environment:**
    In your terminal, source the environment setup script for your cross-compilation toolchain. The exact path may vary depending on your setup.

    ```sh
     source /opt/poky-atmel/5.0.3/environment-setup-armv5e-poky-linux-gnueabi 
    ```

2.  **Clone the repository:**
    If you haven't already, clone the project repository.

    ```sh
    git clone <my-repository>
    cd my-repository
    ```

3.  **Generate the Makefile:**
    Use `qmake` to create the Makefile, which will now be configured for cross-compilation.

    ```sh
    qmake ImageStreamerApp.pro
    ```

4.  **Compile the project:**
    Build the application using the `make` command.

    ```sh
    make
    ```

### Deploying to the Embedded System

1.  **Transfer the binary:**
    After a successful build, copy the generated `ZenQtApp` executable to your target embedded system. You can use tools like `scp` for this.

    ```sh
    scp ZenQtApp user@<embedded-system-ip>:/path/to/destination/
    ```

2.  **Run on the target:**
    Connect to your embedded system's shell (e.g., via SSH) and execute the application.

    ```sh
    ./ZenQtApp
    ```