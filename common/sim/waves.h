

extern bool g_started;
extern long g_startTick;
extern long g_lastWave;

#define SPAWN_DELAY	(60*1000)

void NextWave();
void Start();
void UpdateTimes();