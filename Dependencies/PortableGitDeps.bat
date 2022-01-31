:: This file will fetch the most upto date dependencies


rd AutoRepos /s /q
::                                     This is the url for the "64-bit Git for Windows Portable" download option from https://git-scm.com/download/win
::										If the link stops working you will just need to go and copy the most current one
powershell -command "Invoke-WebRequest https://github.com/git-for-windows/git/releases/download/v2.34.1.windows.1/PortableGit-2.34.1-64-bit.7z.exe -Outfile gitinstall.exe"
gitinstall.exe -y
PortableGit\bin\git.exe clone https://github.com/charles-lunarg/vk-bootstrap.git AutoRepos\vk-bootstrap
PortableGit\bin\git.exe clone https://github.com/gabime/spdlog.git AutoRepos\spdlog
PortableGit\bin\git.exe clone https://github.com/g-truc/glm.git AutoRepos\glm
PortableGit\bin\git.exe clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git AutoRepos\VulkanMemoryAllocator
PortableGit\bin\git.exe clone https://github.com/taskflow/taskflow.git AutoRepos\taskflow


del glfw.zip /q
rd glfw /s /q
::										 This is the link for the "64-bit Windows binaries" download option from https://www.glfw.org/download.html
::										If the link stops working you will just need to go and copy the most current one
powershell -command "Invoke-WebRequest https://github.com/glfw/glfw/releases/download/3.3.6/glfw-3.3.6.bin.WIN64.zip -Outfile glfw.zip"
powershell -command "Expand-Archive glfw.zip"

del glfw.zip /q
rd PortableGit /s /q
del gitinstall.exe