<h1 align="center">DataStorm</h1>

## Description

DataStorm is a software that insert into a database the data of a CSV or Excel file. The software is able to insert the
data into a new table or into an existing table. Using C++ and Qt, the software can insert the data very efficiently and
into different database management systems like MySQL, PostgreSQL or SQLite.

## Features

<label>
<input type="checkbox" style="margin-right: 10px" checked>
</label> Insert data into a new table <br>
<label>
<input type="checkbox" style="margin-right: 10px" checked>
</label> Insert data into an existing table <br>
<label>
<input type="checkbox" style="margin-right: 10px" checked>
</label> From CSV file <br>
<label>
<input type="checkbox" style="margin-right: 10px">
</label> From Excel file <br>

## Platforms

<div align="center">
<img src="https://img.shields.io/badge/OS-MacOS-informational?style=flat&logo=apple&logoColor=white&color=2bbc8a" alt="MacOS" />
<img src="https://img.shields.io/badge/OS-Linux-informational?style=flat&logo=linux&logoColor=white&color=2bbc8a" alt="Linux" />
<img src="https://img.shields.io/badge/OS-Windows-informational?style=flat&logo=windows&logoColor=white&color=2bbc8a" alt="Windows" />
</div>

## Requirements

<div align="center">

<a href="https://isocpp.org/">
<img src="https://img.shields.io/badge/C++-17-informational?style=flat&logo=c%2B%2B&logoColor=white&color=2bbc8a" alt="C++" />
</a>

<a href="https://cmake.org/">
<img src="https://img.shields.io/badge/CMake-3.10-informational?style=flat&logo=cmake&logoColor=white&color=2bbc8a" alt="CMake" />
</a>

<a href="https://www.qt.io/">
<img src="https://img.shields.io/badge/Qt-6-informational?style=flat&logo=qt&logoColor=white&color=2bbc8a" alt="Qt 6" />
</a>

<a href="https://www.gnu.org/software/make/">
<img src="https://img.shields.io/badge/Make-4.1-informational?style=flat&logo=gnu-make&logoColor=white&color=2bbc8a" alt="Make" />
</a>

<a href="https://gcc.gnu.org/">
<img src="https://img.shields.io/badge/GCC-17-informational?style=flat&logo=gcc&logoColor=white&color=2bbc8a" alt="GCC" />
</a>

</div>

## Installation

To install the software, you need to follow the steps below:

1. Clone the repository:

```bash
git clone https://github.com/Maxime-Cllt/DataStorm.git
```

2. Create a build directory:

```bash
mkdir build
cd build
```

3. Generate the build files:

```bash
cmake ..
```

4. Compile the software:

```bash
make
```

5. Run the software:

```bash
./DataStorm
```

## Benchmark

<table style="width:100%; text-align: center; justify-content: center; display: flex; border-collapse: collapse;">
    <tr>
        <th style="font-weight: bolder; border: 1px solid white;">File</th>
        <th style="font-weight: bolder; border: 1px solid white;">Size (Ko)</th>
        <th style="font-weight: bolder; border: 1px solid white;">Time to import (ms)</th>
    </tr>
    <tr>
        <td style="border: 1px solid white;">csv</td>
        <td style="border: 1px solid white;">2348</td>
        <td style="border: 1px solid white;">200</td>
    </tr>
    <tr>
        <td style="border: 1px solid white;">csv</td>
        <td style="border: 1px solid white;">10 000</td>
        <td style="border: 1px solid white;">1900</td>
    </tr>
</table>
