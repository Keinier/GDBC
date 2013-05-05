// GDBC.cpp: define las funciones exportadas de la aplicación DLL.
//

#include "stdafx.h"
#include "GDBC.h"


// Ejemplo de variable exportada
GDBC_API int nGDBC=0;

// Ejemplo de función exportada.
GDBC_API int fnGDBC(void)
{
	return 42;
}

// Constructor de clase exportada.
// Consultar GDBC.h para definir la clase
CGDBC::CGDBC()
{
	return;
}
