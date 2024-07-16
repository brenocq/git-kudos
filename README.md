<h3 align="center">
    <img width="100" src="https://github.com/user-attachments/assets/2a3bf635-c5e2-47a6-8478-82b4b9cf732c"/><br/>
    <code>git-kudos</code>
</h3>

<h6 align="center">
    <a href="https://github.com/brenocq/git-kudos#usage">Usage</a>
    ·
    <a href="https://github.com/brenocq/git-kudos#installation">Installation</a>
    ·
    <a href="https://github.com/brenocq/git-kudos#contributing">Contributing</a>
</h6>

<div align="center">
  <a href="https://github.com/brenocq/git-kudos/actions/workflows/linux.yml">
    <img src="https://github.com/brenocq/git-kudos/actions/workflows/linux.yml/badge.svg" alt="🐧 Linux">
  </a>
  <a href="https://github.com/brenocq/git-kudos/actions/workflows/macos.yml">
    <img src="https://github.com/brenocq/git-kudos/actions/workflows/macos.yml/badge.svg" alt="🍎 MacOS">
  </a>
  <a href="https://github.com/brenocq/git-kudos/actions/workflows/windows.yml">
    <img src="https://github.com/brenocq/git-kudos/actions/workflows/windows.yml/badge.svg" alt="🪟 Windows">
  </a>
</div>

`git-kudos` is an open source project that lists the contributions of each author in a Git repository. It shows who has written the most lines of code, making it easy to recognize and celebrate everyone's hard work. Give them their well-deserved kudos!🎉

## Usage

<p align="center">
    <img src="https://github.com/user-attachments/assets/c569a1e2-15ac-4026-93a2-39e33402da67">
</p>

```
~ ❯ git-kudos --help
Usage: git-kudos [-d | --detailed] [<paths>] [-x | --exclude <paths-to-exclude>]

Options:
  -h, --help                        Print this help message
  -v, --version                     Print version
  -d, --detailed                    Output detailed list of files
  -x, --exclude <paths-to-exclude>  Exclude specified paths

Examples:
  git-kudos                                   Kudos for current path
  git-kudos include/menu/ file.txt data/*.js  Kudos for specified files and folders
  git-kudos out[A-C].csv                      Kudos for outA.csv outB.csv outC.csv
  git-kudos alg[15].rs                        Kudos for alg1.rs alg5.rs
  git-kudos **.{h,c,hpp,cpp}                  Kudos for by C/C++ files
  git-kudos src/**/test.js                    Kudos for text.js files inside src/
  git-kudos src/**/test/*.cpp                 Kudos for .cpp files inside test folders
  git-kudos src/**Renderer*.*                 Kudos for files that contain "Renderer"
  git-kudos src/**.{h,cpp} -x src/*/test/     Kudos for C++ files and exclude test folders
  git-kudos **.py -d                          Detailed kudos for .py files
```

## Installation

<h4><img width="20" src="https://github.com/user-attachments/assets/24e01504-b9f9-47d2-b566-c0f74427768f">&nbsp; Source</h4>

```bash
cd git-kudos
cmake -S . -B build
cmake --build build
sudo cmake --install build
```

&nbsp;
<h4><img width="20" src="https://github.com/user-attachments/assets/9a87f037-99dc-482e-ad61-e0e88a3c5231">&nbsp; Linux</h4>

```bash
yay -S git-kudos
```

&nbsp;
<h4><img width="20" src="https://github.com/user-attachments/assets/f33cd73a-52e5-498f-869d-4812dde30f9b">&nbsp; MacOS</h4>

```bash
TODO
```

&nbsp;
<h4><img width="20" src="https://github.com/user-attachments/assets/1dc3e2ed-91f3-4db8-9c1d-906f81561ce5">&nbsp; Windows</h4>

```bash
TODO
```

## Contributing
All contributions are welcome! Whether you're reporting [issues](https://github.com/brenocq/git-kudos/issues/new), suggesting new features, or submitting [pull requests](https://github.com/brenocq/git-kudos/compare), your help is greatly appreciated. 🫶

## License
[MIT](LICENSE) © [brenocq](brenocq.com)
