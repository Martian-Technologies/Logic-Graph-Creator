#ifndef evalConfig_h
#define evalConfig_h

struct EvalConfig {
	long long int targetTickrate = 0; // in ticks per minute
	bool tickrateLimiter = true;
	bool running = false;
	bool realistic = true;
};

#endif // evalConfig_h