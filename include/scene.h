#include <stdbool.h>
void Scene_GameLoop();

typedef bool (*UpdateFunc)(void);

void Scene_SetUpdateTo(UpdateFunc);
