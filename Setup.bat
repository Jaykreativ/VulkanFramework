mkdir Dependencies
cd Dependencies
git clone https://github.com/glfw/glfw --recursive
git clone https://github.com/g-truc/glm --recursive
cd..
cmake .
cmake --build .