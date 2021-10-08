# GraphicsEngineVulkan
Getting started with Vulkan

<h1 align="center">
  <br>
  <a href="https://jotrocken.blog/"><img src="images/logo.png" alt="OpenGLEngine" width="200"></a>
  <br>
  Cataglyphis_Engine
  <br>
</h1>

<h4 align="center">A graphics engine built on top of Vulkan <a href="https://jotrocken.blog/" target="_blank"></a>.</h4>

<!-- <p align="center">
  <a href="https://paypal.me/JonasHeinle?locale.x=de_DE">
    <img src="https://img.shields.io/badge/$-donate-ff69b4.svg?maxAge=2592000&amp;style=flat">
  </a>
</p> -->

<p align="center">
  <a href="#key-features">Key Features</a> •
  <a href="#how-to-use">How To Use</a> •
  <a href="#download">Download</a> •
  <a href="#credits">Credits</a> •
  <a href="#related">Related</a> •
  <a href="#license">License</a>
</p>

<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
      <ul>
        <li><a href="#key-features">Key Features</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
        <li><a href="#shaders">Shaders</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgements">Acknowledgements</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project

[![Kataglyphis Engine][product-screenshot]](https://jotrocken.blog/)

The thought behind this project is to implement modern algortihms and 
techniques that modern graphic engines rely on. 
Furthermore it should serve as a framework to enable further investigations
in own research topics.
Feel free to contribute and adding stuff :)
Reminder: This project is based on my interest in implementing algortihms by
my own and make own research. Hence you might encounter some bugs. Feel free 
to report.

### Key Features


|          Feature                |   Implement Status | ◾ Other Configs |
| --------------------------------| :----------------: | :-------------: |
| Directional Lights              |         ✔️         |        ❌      |
| Point Lights                    |         ✔️         |        ❌      |
| Spot Lights                     |         ✔️         |        ❌      |
| Directional Shadow Mapping      |         ✔️         |        ❌      |
| Omni-Directional Shadow Mapping |         ✔️         |        ❌      |

### Built With

* [Vulkan 1.2](https://www.vulkan.org/)
* [GLM](https://github.com/g-truc/glm)
* [GLFW](https://www.glfw.org/)
* [ASSIMP](https://github.com/assimp/assimp)
* [STB](https://github.com/nothings/stb)
* [DOXYGEN](https://www.doxygen.nl/index.html)
* [GTEST](https://github.com/google/googletest)

<!-- GETTING STARTED -->
## Getting Started

You might only clone the repo and get to go immediately :)

### Prerequisites

You will need OpenGL. If you want to build documantaries you will need [DOXYGEN] (https://www.doxygen.nl/index.html).

### Shaders

You will have to compile the shaders manually. A proper script to automate it is 
provided.

For Linux user: Open Terminal and navigate to "Resources/Shader" directory. Then: 
                ```sh
                chmod +x compile.sh
                ./compile.sh
                ```

For Windows user: run the batch file (double click on the 'compile.bat' in the Resources/Shader folder)

### Installation

1. Clone the repo
   ```sh
   git clone --recurse-submodules git@github.com:Kataglyphis/GraphicsEngineVulkan.git
   ```
   Important for init the submodules


<!-- USAGE EXAMPLES -->
## Usage

_For more examples, please refer to the [Documentation](https://jotrocken.blog/)_


<!-- ROADMAP -->
## Roadmap
Upcoming :)
<!-- See the [open issues](https://github.com/othneildrew/Best-README-Template/issues) for a list of proposed features (and known issues). -->



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request



<!-- LICENSE -->
## License

Distributed under the GPL-3.0 License. See `LICENSE` for more information.


<!-- CONTACT -->
## Contact

Jonas Heinle - [@your_twitter](https://twitter.com/Cataglyphis_) - jonasheinle@googlemail.com

Project Link: [https://github.com/Kataglyphis/GraphicsEngineVulkan](https://github.com/Kataglyphis/OpenGLEngine)



<!-- ACKNOWLEDGEMENTS -->
## Acknowledgements


## Literature 

Some very helpful literature, tutorials, etc. 

Vulkan
* [Udemy course by Ben Cook](https://www.udemy.com/share/102M903@JMHgpMsdMW336k2s5Ftz9FMx769wYAEQ7p6GMAPBsFuVUbWRgq7k2uY6qBCG6UWNPQ==/)

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/othneildrew/Best-README-Template.svg?style=for-the-badge
[contributors-url]: https://github.com/othneildrew/Best-README-Template/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/othneildrew/Best-README-Template.svg?style=for-the-badge
[forks-url]: https://github.com/othneildrew/Best-README-Template/network/members
[stars-shield]: https://img.shields.io/github/stars/othneildrew/Best-README-Template.svg?style=for-the-badge
[stars-url]: https://github.com/othneildrew/Best-README-Template/stargazers
[issues-shield]: https://img.shields.io/github/issues/othneildrew/Best-README-Template.svg?style=for-the-badge
[issues-url]: https://github.com/othneildrew/Best-README-Template/issues
[license-shield]: https://img.shields.io/github/license/othneildrew/Best-README-Template.svg?style=for-the-badge
[license-url]: https://github.com/othneildrew/Best-README-Template/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://www.linkedin.com/in/jonas-heinle-0b2a301a0/
[product-screenshot]: images/Screenshot.png



