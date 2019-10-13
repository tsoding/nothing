#ifndef CONFIG_H_
#define CONFIG_H_

// TODO(#1085): PLAYER_DEATH_LEVEL is hardcoded
//   Should be customizable in the Level Editor
#define PLAYER_DEATH_LEVEL 1000.0f

#define LEVEL_EDITOR_DETH_LEVEL_COLOR hsla(0.0f, 0.8f, 0.6f, 1.0f)

#define BACKGROUND_LAYERS_COUNT 3
#define BACKGROUND_LAYERS_STEP 0.2f
#define BACKGROUND_TURDS_PER_CHUNK 5
#define BACKGROUND_CHUNK_WIDTH 500.0f
#define BACKGROUND_CHUNK_HEIGHT 500.0f

#define ENTITY_MAX_ID_SIZE 36

#endif  // CONFIG_H_
