//<codepage = utf8
// файлы проекта
#include "cservice.h"

int main(int argc, char *argv[])
{
	// создание сервиса
	CService service( argc, argv );

	// запуск сервиса
	return service.exec();
}
