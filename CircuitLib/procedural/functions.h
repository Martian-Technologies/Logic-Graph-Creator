#ifndef functions_h
#define functions_h

#define externalFunction(functionName) __attribute__((import_module("env"))) __attribute__((import_name(#functionName)))
// #define exportedVar(type, name) type name; type getname
#define exportedVar(type, name, Name) extern const type name; const type get##Name() { return name; }

#ifdef __cplusplus
extern "C" {
#endif

	typedef unsigned int connection_end_id_t;
	typedef unsigned char block_size_t;
	typedef unsigned int BlockType;
	typedef char Rotation;
	typedef int cord_t;

	exportedVar(char*, UUID, UUID)
	exportedVar(char*, name, Name)

	externalFunction(tryInsertBlock)
	bool tryInsertBlock(cord_t x, cord_t y, Rotation rotation, BlockType blockType);

	externalFunction(tryCreateConnection)
	bool tryCreateConnection(cord_t outputX, cord_t outputY, cord_t inputX, cord_t inputY);
	
	externalFunction(addConnectionInput)
	void addConnectionInput(cord_t blockX, cord_t blockY, cord_t portX, cord_t portY);

	externalFunction(addConnectionOutput)
	void addConnectionOutput(cord_t blockX, cord_t blockY, cord_t portX, cord_t portY);
	
	externalFunction(setSize)
	void setSize(cord_t width, cord_t height);

	externalFunction(logInfo)
	void logInfo(const char* msg);

	externalFunction(logError)
	void logError(const char* msg);

#ifdef __cplusplus
}
#endif

#endif /* functions_h */
