# Installation {#installation}

This project utilizes the [conan package manager](https://conan.io/) to manage third-party libraries as dependencies as well as provide the install targets of this project as a packaged library.

## Hahn-Schickard Conan repository

The precompiled package and it's recipe can be accessed from Hahn-Schickard conan repository. If you do not have it in your conan remotes list, you can add it with the following command:

```bash
conan remote add hahn-schickard https://conan-repo.hahn-schickard.de/artifactory/api/conan/hs-artifactory
```

In case you are trying to download a non public version, you will need to authenticate with Hahn-Schickard conan credentials. 

<div class="tabbed">
- <b class="tab-title">conan v1.x</b>
    ```bash
    conan user -p <PASSWORD or API_KEY> -r hahn-schickard <USERNAME>
    ```

- <b class="tab-title">conan v2.x</b>
    ```bash
    conan remote auth hahn-schickard
    ```

</div>

If you do not possess Hahn-Schickard conan user account, please contact Hahn-Schickard to discuss the possibility of getting one.

## Creating the conan package locally
In case you do not want to use the Hahn-Schickard conan repository and have access to the source files, you can create the package locally. 
To do so, run the following command in project root directory

<div class="tabbed">
- <b class="tab-title">conan v1.x</b>
    ```bash
    conan create . ${VERSION}@${USER}/${CHANEL} --build=missing
    ```

- <b class="tab-title">conan v2.x</b>
    ```bash
    conan create . --version=${VERSION} --user=${USER} --channel=${CHANEL} --build=missing
    ```

</div>

## Package options

Since this project is only meant for testing purposes, we recommend that the consuming project mark this dependency as a **test_requires** (preferred) or **build_requires** requirement instead of the typical **requires** one.

When using this library as a conan package, users can configure the [`BUILD_SHARED_LIBS`](https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html) and [`POSITION_INDEPENDENT_CODE`](https://cmake.org/cmake/help/latest/prop_tgt/POSITION_INDEPENDENT_CODE.html#prop_tgt:POSITION_INDEPENDENT_CODE) cmake options via conan:

<div class="tabbed">
- <b class="tab-title">conanfile.txt</b>
    ```
    [test_requires]
    information_model_mocks/[~1.0]

    [options]
    information_model_mocks/*:shared = True
    information_model_mocks/*:fPIC = True
    ```

    For more information see `conanfile.txt` [options](https://docs.conan.io/2/reference/conanfile_txt.html#options) documentation

- <b class="tab-title">conanfile.py</b>
    ```python
    from conan import ConanFile
    from conan.tools.cmake import cmake_layout

    class PackageConan(ConanFile):
        settings = "os", "compiler", "build_type", "arch"
        generators = "CMakeDeps", "CMakeToolchain"

        def requirements(self):
            self.test_requires("information_model_mocks/[~1.0]")

        def configure(self):    
            self.options["information_model_mocks"].shared = True
            self.options["information_model_mocks"].fPIC = True


        def layout(self):
            cmake_layout(self)
    ```

    For more information see `conanfile.py` [configure()](https://docs.conan.io/2/reference/conanfile/methods/configure.html) documentation
    
</div>
