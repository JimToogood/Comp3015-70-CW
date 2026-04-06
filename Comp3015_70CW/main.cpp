#include "scenebasic_uniform.h"
#include "helper/scene.h"
#include "helper/scenerunner.h"

using namespace std;
using namespace glm;


int main(int argc, char* argv[]) {
	SceneRunner runner("70CW_Jim_Toogood");

	unique_ptr<Scene> scene;
	scene = unique_ptr<Scene>(new SceneBasic_Uniform());

	return runner.run(*scene);
}
