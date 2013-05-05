#ifdef GDBC_EXPORTS
#define GDBC_API __declspec(dllexport)
#else
#define GDBC_API __declspec(dllimport)
#endif

// Clase exportada de GDBC.dll
class GDBC_API CGDBC {
public:
	CGDBC(void);
	// TODO: agregar métodos aquí.
};

extern GDBC_API int nGDBC;

GDBC_API int fnGDBC(void);
