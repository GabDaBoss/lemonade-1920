#include <stdbool.h>
void Scene_GameLoop();

typedef void (*UpdateFunc)(void);

void Scene_SetUpdateTo(UpdateFunc);
void Scene_Quit();
